// 在项目设置的 Description 页面填写版权声明。

#include "AbilitySystem/Abilities/HeroGameplayAbility_TargetLock.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Characters/WarriorHeroCharacter.h"
#include "Components/Combat/HeroCombatComponent.h"
#include "Components/SizeBox.h"
#include "Controllers/WarriorHeroController.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "WarriorFunctionLibrary.h"
#include "WarriorGameplayTags.h"
#include "Widgets/WarriorWidgetBase.h"

void UHeroGameplayAbility_TargetLock::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 技能启动时立即尝试锁定一个目标，并切换到锁定模式的移动与输入配置。
	if (!TryLockOnTarget())
	{
		return;
	}

	InitTargetLockMovement();
	InitTargetLockMappingContext();
}

void UHeroGameplayAbility_TargetLock::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	// 技能结束时必须按启动顺序反向清理，避免 UI、移动状态或输入映射残留。
	ClearLockOnTarget();
	ResetTargetLockMovement();
	ResetTargetLockMappingContext();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UHeroGameplayAbility_TargetLock::OnTargetLockTick(float DeltaTime)
{
	// 每帧先校验锁定状态。目标死亡、玩家死亡或目标失效时都应结束锁定。
	const bool bShouldCancelTargetLock =
		!IsValid(CurrentLockedOnTarget) ||
		UWarriorFunctionLibrary::NativeDoesActorHaveTag(CurrentLockedOnTarget, WarriorGameplayTags::Shared_Status_Death) ||
		UWarriorFunctionLibrary::NativeDoesActorHaveTag(GetHeroCharacterFromActorInfo(), WarriorGameplayTags::Shared_Status_Death);

	if (bShouldCancelTargetLock)
	{
		CancelLockOnTarget();
		return;
	}

	SetTargetLockWidgetPosition();
	RefreshTargetLockMovementState();

	// 翻滚期间不强行覆盖镜头旋转，避免和翻滚输入/动画表现互相抢控制权。
	const bool bShouldOverrideRotation =
		!UWarriorFunctionLibrary::NativeDoesActorHaveTag(GetHeroCharacterFromActorInfo(), WarriorGameplayTags::Player_Status_Rolling);

	if (!bShouldOverrideRotation)
	{
		return;
	}

	// 让镜头持续朝向锁定目标，同时保留一个俯仰偏移，避免视角压得太低。
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(
		GetHeroCharacterFromActorInfo()->GetActorLocation(),
		CurrentLockedOnTarget->GetActorLocation());
	LookAtRotation -= FRotator(TargetLockCameraOffsetDistance, 0.f, 0.f);

	const FRotator CurrentControlRot = GetHeroControllerFromActorInfo()->GetControlRotation();
	const FRotator TargetRot = FMath::RInterpTo(CurrentControlRot, LookAtRotation, DeltaTime, TargetLockRotationInterpSpeed);

	GetHeroControllerFromActorInfo()->SetControlRotation(FRotator(TargetRot.Pitch, TargetRot.Yaw, 0.f));

	if (ShouldUseCombatTargetLockRotation())
	{
		GetHeroCharacterFromActorInfo()->SetActorRotation(FRotator(0.f, TargetRot.Yaw, 0.f));
	}
}

void UHeroGameplayAbility_TargetLock::SwitchTarget(const FGameplayTag& InSwitchDirectionTag)
{
	// 切换目标前重新扫描候选目标，保证敌人移动或死亡后列表仍然有效。
	GetAvailableActorsToLock();

	TArray<AActor*> ActorsOnLeft;
	TArray<AActor*> ActorsOnRight;
	// 根据当前锁定目标把候选目标分成左右两组，再从指定方向里选最近的目标。
	GetAvailableActorsAroundTarget(ActorsOnLeft, ActorsOnRight);

	AActor* NewLockActor = nullptr;
	if (InSwitchDirectionTag == WarriorGameplayTags::Player_Event_SwitchTarget_Left)
	{
		NewLockActor = GetNearestTargetFromAvailableActors(ActorsOnLeft);
	}
	else if (InSwitchDirectionTag == WarriorGameplayTags::Player_Event_SwitchTarget_Right)
	{
		NewLockActor = GetNearestTargetFromAvailableActors(ActorsOnRight);
	}

	if (NewLockActor)
	{
		CurrentLockedOnTarget = NewLockActor;
	}
}

bool UHeroGameplayAbility_TargetLock::TryLockOnTarget()
{
	// 初次锁定时扫描前方盒体范围内的可锁定 Actor。
	GetAvailableActorsToLock();

	if (AvailableActorsToLock.IsEmpty())
	{
		CancelLockOnTarget();
		return false;
	}

	// 默认锁定距离玩家最近的候选目标。
	CurrentLockedOnTarget = GetNearestTargetFromAvailableActors(AvailableActorsToLock);
	if (!CurrentLockedOnTarget)
	{
		CancelLockOnTarget();
		return false;
	}

	DrawTargetLockWidget();
	SetTargetLockWidgetPosition();
	return true;
}

void UHeroGameplayAbility_TargetLock::GetAvailableActorsToLock()
{
	// 每次扫描前清空缓存，避免使用上一次 Tick 或上一次切换留下的候选目标。
	AvailableActorsToLock.Empty();

	TArray<FHitResult> HitResults;
	// 用角色前方的盒体扫描获取候选目标。具体可锁定类型由 BoxTraceChannel 配置决定。
	UKismetSystemLibrary::BoxTraceMultiForObjects(
		GetHeroCharacterFromActorInfo(),
		GetHeroCharacterFromActorInfo()->GetActorLocation(),
		GetHeroCharacterFromActorInfo()->GetActorLocation() + GetHeroCharacterFromActorInfo()->GetActorForwardVector() * LockOnRange,
		TraceBoxSize / 2.f,
		GetHeroCharacterFromActorInfo()->GetActorForwardVector().ToOrientationRotator(),
		BoxTraceChannel,
		false,
		TArray<AActor*>(),
		bShowPersistentDebugShape ? EDrawDebugTrace::Persistent : EDrawDebugTrace::None,
		HitResults,
		true);

	for (const FHitResult& HitResult : HitResults)
	{
		// 排除自己，并用 AddUnique 避免同一个 Actor 被多个碰撞体重复加入。
		AActor* HitActor = HitResult.GetActor();
		if (HitActor && HitActor != GetHeroCharacterFromActorInfo())
		{
			AvailableActorsToLock.AddUnique(HitActor);
		}
	}
}

AActor* UHeroGameplayAbility_TargetLock::GetNearestTargetFromAvailableActors(const TArray<AActor*>& ActorsToLock)
{
	if (ActorsToLock.IsEmpty())
	{
		return nullptr;
	}

	// UE 内置工具会返回距离给定位置最近的 Actor，这里以玩家位置为基准。
	float ClosestDistance = 0.f;
	return UGameplayStatics::FindNearestActor(GetHeroCharacterFromActorInfo()->GetActorLocation(), ActorsToLock, ClosestDistance);
}

void UHeroGameplayAbility_TargetLock::GetAvailableActorsAroundTarget(
	TArray<AActor*>& OutActorsOnLeft,
	TArray<AActor*>& OutActorsOnRight)
{
	// 当前目标无效时不能继续计算方向，否则会访问空指针。
	if (!IsValid(CurrentLockedOnTarget) || AvailableActorsToLock.IsEmpty())
	{
		CancelLockOnTarget();
		return;
	}

	const FVector PlayerLocation = GetHeroCharacterFromActorInfo()->GetActorLocation();
	// 当前目标方向作为参考方向，其他候选目标会和它做叉乘比较。
	const FVector PlayerToCurrentNormalized = (CurrentLockedOnTarget->GetActorLocation() - PlayerLocation).GetSafeNormal();

	for (AActor* AvailableActor : AvailableActorsToLock)
	{
		if (!IsValid(AvailableActor) || AvailableActor == CurrentLockedOnTarget)
		{
			continue;
		}

		const FVector PlayerToAvailableNormalized = (AvailableActor->GetActorLocation() - PlayerLocation).GetSafeNormal();
		const FVector CrossResult = FVector::CrossProduct(PlayerToCurrentNormalized, PlayerToAvailableNormalized);

		// 叉乘结果的 Z 值用于判断候选目标在当前目标的左侧还是右侧。
		if (CrossResult.Z > 0.f)
		{
			OutActorsOnRight.AddUnique(AvailableActor);
		}
		else
		{
			OutActorsOnLeft.AddUnique(AvailableActor);
		}
	}
}

void UHeroGameplayAbility_TargetLock::DrawTargetLockWidget()
{
	// Widget 已存在时不重复创建，防止界面上叠出多个锁定标记。
	if (CurrentTargetLockWidget)
	{
		return;
	}

	checkf(TargetLockWidgetClass, TEXT("TargetLockWidgetClass is not set in %s"), *GetName());

	APlayerController* OwningPlayer = GetHeroControllerFromActorInfo();
	CurrentTargetLockWidget = CreateWidget<UWarriorWidgetBase>(OwningPlayer, TargetLockWidgetClass);

	check(CurrentTargetLockWidget);

	CurrentTargetLockWidget->AddToViewport();
}

void UHeroGameplayAbility_TargetLock::SetTargetLockWidgetPosition()
{
	// 没有 UI 或目标时无需更新屏幕位置。
	if (!CurrentTargetLockWidget || !CurrentLockedOnTarget)
	{
		return;
	}

	FVector2D ScreenPosition;
	// 把目标的世界坐标投影到当前玩家视口坐标。
	UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(
		GetHeroControllerFromActorInfo(),
		CurrentLockedOnTarget->GetActorLocation(),
		ScreenPosition,
		true);

	// Widget 尺寸只缓存一次，后续 Tick 只更新屏幕位置。
	if (TargetLockWidgetSize == FVector2D::ZeroVector)
	{
		CurrentTargetLockWidget->WidgetTree->ForEachWidget([this](UWidget* Widget)
		{
			if (USizeBox* SizeBox = Cast<USizeBox>(Widget))
			{
				TargetLockWidgetSize.X = SizeBox->GetWidthOverride();
				TargetLockWidgetSize.Y = SizeBox->GetHeightOverride();
			}
		});
	}

	ScreenPosition -= TargetLockWidgetSize / 2.f;
	CurrentTargetLockWidget->SetPositionInViewport(ScreenPosition, false);
}

void UHeroGameplayAbility_TargetLock::InitTargetLockMovement()
{
	UCharacterMovementComponent* CharacterMovement = GetHeroCharacterFromActorInfo()->GetCharacterMovement();
	if (!CharacterMovement)
	{
		return;
	}

	// 缓存进入锁定前的朝向参数，结束锁定时恢复。
	bCachedOrientRotationToMovement = CharacterMovement->bOrientRotationToMovement;
	bCachedUseControllerDesiredRotation = CharacterMovement->bUseControllerDesiredRotation;
	bHasCachedTargetLockMovement = true;

	RefreshTargetLockMovementState();
}

void UHeroGameplayAbility_TargetLock::InitTargetLockMappingContext()
{
	// 锁定期间添加专用输入映射，例如切换目标输入。
	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetHeroControllerFromActorInfo()->GetLocalPlayer());

	check(Subsystem);

	Subsystem->AddMappingContext(TargetLockMappingContext, 2);
}

void UHeroGameplayAbility_TargetLock::CancelLockOnTarget()
{
	// 统一走 GAS 的取消流程，让 EndAbility 负责最终清理。
	CancelAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true);
}

void UHeroGameplayAbility_TargetLock::ClearLockOnTarget()
{
	// 清空运行期缓存，避免下一次启动技能时读到旧目标。
	AvailableActorsToLock.Empty();
	CurrentLockedOnTarget = nullptr;

	// 移除锁定 UI，防止技能取消后标记仍留在屏幕上。
	if (CurrentTargetLockWidget)
	{
		CurrentTargetLockWidget->RemoveFromParent();
		CurrentTargetLockWidget = nullptr;
	}

	TargetLockWidgetSize = FVector2D::ZeroVector;
}

void UHeroGameplayAbility_TargetLock::ResetTargetLockMovement()
{
	if (!bHasCachedTargetLockMovement)
	{
		return;
	}

	UCharacterMovementComponent* CharacterMovement = GetHeroCharacterFromActorInfo()->GetCharacterMovement();
	if (!CharacterMovement)
	{
		bHasCachedTargetLockMovement = false;
		return;
	}

	// 恢复进入锁定前缓存的朝向参数。
	CharacterMovement->bOrientRotationToMovement = bCachedOrientRotationToMovement;
	CharacterMovement->bUseControllerDesiredRotation = bCachedUseControllerDesiredRotation;
	bHasCachedTargetLockMovement = false;
}

void UHeroGameplayAbility_TargetLock::ResetTargetLockMappingContext()
{
	// ActorInfo 失效时不要继续访问 LocalPlayer 子系统。
	if (!GetHeroControllerFromActorInfo())
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetHeroControllerFromActorInfo()->GetLocalPlayer());

	check(Subsystem);

	Subsystem->RemoveMappingContext(TargetLockMappingContext);
}

void UHeroGameplayAbility_TargetLock::RefreshTargetLockMovementState()
{
	UCharacterMovementComponent* CharacterMovement = GetHeroCharacterFromActorInfo()->GetCharacterMovement();
	if (!CharacterMovement)
	{
		return;
	}

	// 持有武器时使用战斗锁定朝向：速度保持角色当前走/跑状态。
	if (ShouldUseCombatTargetLockRotation())
	{
		CharacterMovement->bOrientRotationToMovement = false;
		CharacterMovement->bUseControllerDesiredRotation = false;
		return;
	}

	// 未持有武器时恢复普通移动状态，避免空手锁定也强制战斗转向。
	CharacterMovement->bOrientRotationToMovement = bCachedOrientRotationToMovement;
	CharacterMovement->bUseControllerDesiredRotation = bCachedUseControllerDesiredRotation;
}

bool UHeroGameplayAbility_TargetLock::ShouldUseCombatTargetLockRotation()
{
	// 当前是否装备武器决定是否启用战斗锁定转向。
	if (const UHeroCombatComponent* HeroCombatComponent = GetHeroCombatComponentFromActorInfo())
	{
		return HeroCombatComponent->GetHeroCurrentEquippedWeapon() != nullptr;
	}

	return false;
}
