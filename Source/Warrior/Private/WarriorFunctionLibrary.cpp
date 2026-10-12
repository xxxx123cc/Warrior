// Fill out your copyright notice in the Description page of Project Settings.


#include "WarriorFunctionLibrary.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/WarriorAttributeSet.h"
#include "Warrior/Public/AbilitySystem/WarriorAbilitySystemComponent.h"
//#include "BlueprintGameplayTagLibrary.h"
//#include "Warrior/Public/Characters/WarriorBaseCharacter.h"
#include"Warrior/Public/Interfaces/PawnCombatInterface.h"
#include "GenericTeamAgentInterface.h"
#include "MeshPaintVisualize.h"
#include "WarriorDebugHelper.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/Combat/PawnCombatComponent.h"
#include "Items/Weapons/WarriorWeaponBase.h"
#include "WarriorGameplayTags.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "SaveGame/WarriorSaveGame.h"
#include "WarriorTypes/WarriorCountDownAction.h"
#include "Warrior/Public/WarriorGameInstance.h"
#include "SaveGame/WarriorSaveGame.h"
#include "Components/UI/HeroUIComponent.h"
#include "Components/UI/EnemyUIComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Characters/WarriorEnemyCharacter.h"
#include "Interfaces/PawnUIInterface.h"
#include "TimerManager.h"

namespace
{
	TMap<const AActor*, int32> PendingSpecialHitReactSerialByTarget;

	int32 MarkPendingSpecialHitReactEvent(const AActor* TargetActor)
	{
		if (!TargetActor)
		{
			return INDEX_NONE;
		}

		int32& PendingSerial = PendingSpecialHitReactSerialByTarget.FindOrAdd(TargetActor);
		++PendingSerial;
		return PendingSerial;
	}

	bool IsPendingSpecialHitReactEvent(const AActor* TargetActor, int32 ExpectedSerial)
	{
		const int32* PendingSerial = PendingSpecialHitReactSerialByTarget.Find(TargetActor);
		return PendingSerial && *PendingSerial == ExpectedSerial;
	}

	bool HasPendingSpecialHitReactEvent(const AActor* TargetActor)
	{
		return TargetActor && PendingSpecialHitReactSerialByTarget.Contains(TargetActor);
	}

	void ClearPendingSpecialHitReactEvent(const AActor* TargetActor, int32 ExpectedSerial)
	{
		if (IsPendingSpecialHitReactEvent(TargetActor, ExpectedSerial))
		{
			PendingSpecialHitReactSerialByTarget.Remove(TargetActor);
		}
	}

	bool IsHitReactAbilitySpec(const FGameplayAbilitySpec& AbilitySpec)
	{
		if (!AbilitySpec.Ability)
		{
			return false;
		}

		const FGameplayTagContainer& AbilityTags = AbilitySpec.Ability->GetAssetTags();
		return AbilityTags.HasTag(WarriorGameplayTags::Shared_Ability_HitReact) ||
			AbilitySpec.GetDynamicSpecSourceTags().HasTag(WarriorGameplayTags::Shared_Ability_HitReact);
	}

	bool HasActiveHitReactAbility(const UAbilitySystemComponent* TargetASC)
	{
		if (!TargetASC)
		{
			return false;
		}

		for (const FGameplayAbilitySpec& AbilitySpec : TargetASC->GetActivatableAbilities())
		{
			if (AbilitySpec.IsActive() && IsHitReactAbilitySpec(AbilitySpec))
			{
				return true;
			}
		}

		return false;
	}

	void SendHitReactGameplayEvent(AActor* TargetActor, FGameplayTag EventTag, const FGameplayEventData& EventData)
	{
		if (!TargetActor)
		{
			return;
		}

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			TargetActor,
			EventTag,
			EventData);
	}

	void SendHitReactGameplayEventNextTick(AActor* TargetActor, FGameplayTag EventTag, const FGameplayEventData& EventData)
	{
		if (!TargetActor)
		{
			return;
		}

		UWorld* World = TargetActor->GetWorld();
		if (!World)
		{
			SendHitReactGameplayEvent(TargetActor, EventTag, EventData);
			return;
		}

		const TWeakObjectPtr<AActor> WeakTargetActor = TargetActor;
		const AActor* TargetActorKey = TargetActor;
		const int32 PendingSerial = MarkPendingSpecialHitReactEvent(TargetActorKey);
		World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda(
			[WeakTargetActor, TargetActorKey, PendingSerial, EventTag, EventData]()
			{
				if (!IsPendingSpecialHitReactEvent(TargetActorKey, PendingSerial))
				{
					return;
				}

				ClearPendingSpecialHitReactEvent(TargetActorKey, PendingSerial);
				if (!WeakTargetActor.IsValid())
				{
					return;
				}

				SendHitReactGameplayEvent(WeakTargetActor.Get(), EventTag, EventData);
			}));
	}

	void BroadcastBlockValueChanged(AActor* InActor, float CurrentBlockValue, float MaxBlockValue)
	{
		if (!InActor)
		{
			return;
		}

		IPawnUIInterface* PawnUIInterface = Cast<IPawnUIInterface>(InActor);
		if (!PawnUIInterface)
		{
			return;
		}

		if (UHeroUIComponent* HeroUIComponent = PawnUIInterface->GetHeroUIComponent())
		{
			HeroUIComponent->OnCurrentBlockValueChanged.Broadcast(
				MaxBlockValue > 0.f ? CurrentBlockValue / MaxBlockValue : 0.f);
		}
	}

	void BroadcastBossPoiseChanged(AActor* InActor, float CurrentBossPoise, float MaxBossPoise)
	{
		if (!InActor)
		{
			return;
		}

		IPawnUIInterface* PawnUIInterface = Cast<IPawnUIInterface>(InActor);
		if (!PawnUIInterface)
		{
			return;
		}

		if (UEnemyUIComponent* EnemyUIComponent = PawnUIInterface->GetEnemyUIComponent())
		{
			EnemyUIComponent->OnCurrentBossPoiseChanged.Broadcast(
				MaxBossPoise > 0.f ? CurrentBossPoise / MaxBossPoise : 0.f);
		}
	}

	bool HasAttackImpactSetByCallerMagnitude(const FGameplayEffectSpecHandle& InSpecHandle, FGameplayTag Tag)
	{
		return InSpecHandle.IsValid() &&
			InSpecHandle.Data.IsValid() &&
			InSpecHandle.Data->SetByCallerTagMagnitudes.Contains(Tag);
	}

	float GetAttackImpactSetByCallerMagnitude(
		const FGameplayEffectSpecHandle& InSpecHandle,
		FGameplayTag Tag,
		float DefaultValue)
	{
		if (!HasAttackImpactSetByCallerMagnitude(InSpecHandle, Tag))
		{
			return DefaultValue;
		}

		return InSpecHandle.Data->GetSetByCallerMagnitude(Tag, false, DefaultValue);
	}

	bool IsValidUnderAttackDashTarget(AActor* DashActor, AActor* CandidateActor)
	{
		if (!DashActor || !CandidateActor || DashActor == CandidateActor)
		{
			return false;
		}

		APawn* DashPawn = Cast<APawn>(DashActor);
		APawn* CandidatePawn = Cast<APawn>(CandidateActor);
		if (!DashPawn || !CandidatePawn)
		{
			return false;
		}

		return UWarriorFunctionLibrary::IsTargetPawnHostile(DashPawn, CandidatePawn) &&
			!UWarriorFunctionLibrary::NativeDoesActorHaveTag(CandidateActor, WarriorGameplayTags::Shared_Status_Death) &&
			UWarriorFunctionLibrary::NativeDoesActorHaveTag(CandidateActor, WarriorGameplayTags::Enemy_Status_UnderAttack);
	}

	AActor* FindNearestUnderAttackDashTarget(AActor* DashActor)
	{
		if (!DashActor || !DashActor->GetWorld())
		{
			return nullptr;
		}

		const FVector DashActorLocation = DashActor->GetActorLocation();
		AActor* BestTargetActor = nullptr;
		float BestDistanceSquared = TNumericLimits<float>::Max();

		for (TActorIterator<AWarriorEnemyCharacter> It(DashActor->GetWorld()); It; ++It)
		{
			AWarriorEnemyCharacter* EnemyCharacter = *It;
			if (!IsValid(EnemyCharacter) || !IsValidUnderAttackDashTarget(DashActor, EnemyCharacter))
			{
				continue;
			}

			const float DistanceSquared = FVector::DistSquared2D(
				DashActorLocation,
				EnemyCharacter->GetActorLocation());
			if (DistanceSquared < BestDistanceSquared)
			{
				BestTargetActor = EnemyCharacter;
				BestDistanceSquared = DistanceSquared;
			}
		}

		return BestTargetActor;
	}
}
/**
 * @brief 从Actor获取WarriorAbilitySystemComponent
 * @param InActor 目标Actor
 * @return UWarriorAbilitySystemComponent* Warrior自定义的能力系统组件
 * @note 使用CastChecked确保类型安全，如果获取失败会触发断言
 */
UWarriorAbilitySystemComponent* UWarriorFunctionLibrary::NativeGetWarriorAscFromActor(AActor* InActor)
{
	// 确保传入的Actor指针有效
	check(InActor);
	if (!IsValid(InActor))
	{
		return nullptr;
	}
	UAbilitySystemComponent* ASC =
	   UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InActor);
	if (!IsValid(ASC))
	{
		return nullptr;
	}
	// 通过AbilitySystemBlueprintLibrary获取ASC，并强制转换为Warrior自定义类型
	return CastChecked<UWarriorAbilitySystemComponent> (UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InActor));
}

/**
 * @brief 如果Actor没有指定Tag，则添加该Tag
 * @param InActor 目标Actor
 * @param TagToAdd 要添加的GameplayTag
 */
void UWarriorFunctionLibrary::AddGamePlayTagToActorIfNone(AActor* InActor, FGameplayTag TagToAdd)
{
	// 获取Actor的ASC
	UWarriorAbilitySystemComponent* ASC = NativeGetWarriorAscFromActor(InActor);
	
	// 检查ASC有效性，并确保Actor当前不拥有该Tag
	if (ASC && !ASC->HasMatchingGameplayTag(TagToAdd))
	{
		// 添加为LooseGameplayTag（非GE授予的自由标签）
		ASC->AddLooseGameplayTag(TagToAdd);
	}
}

/**
 * @brief 如果Actor拥有指定Tag，则移除该Tag
 * @param InActor 目标Actor
 * @param TagToRemove 要移除的GameplayTag
 */
void UWarriorFunctionLibrary::RemoveGamePlayTagFromActorIfAny(AActor* InActor, FGameplayTag TagToRemove)
{
	// 获取Actor的ASC
	UWarriorAbilitySystemComponent* ASC = NativeGetWarriorAscFromActor(InActor);
	
	// 检查ASC有效性，并确保Actor当前拥有该Tag
	if (ASC && ASC->HasMatchingGameplayTag(TagToRemove))
	{
		// 移除LooseGameplayTag
		ASC->RemoveLooseGameplayTag(TagToRemove);
	}
}

/**
 * @brief 检查Actor是否拥有指定的GameplayTag（C++版本）
 * @param InActor 目标Actor
 * @param TagToCheck 要检查的GameplayTag
 * @return bool 是否拥有该Tag
 */
bool UWarriorFunctionLibrary::NativeDoesActorHaveTag(AActor* InActor, FGameplayTag TagToCheck)
{
	// 获取Actor的ASC
	UWarriorAbilitySystemComponent* ASC = NativeGetWarriorAscFromActor(InActor);
	
	// 如果ASC有效，检查是否拥有指定Tag
	if (ASC)
	{
		return ASC->HasMatchingGameplayTag(TagToCheck);
	}
	
	// ASC无效时返回false（注意：原代码缺少返回值）
	return false;
}

/**
 * @brief 检查Actor是否拥有指定的GameplayTag（Blueprint版本）
 * @param InActor 目标Actor
 * @param TagToCheck 要检查的GameplayTag
 * @param OutConfirm 输出确认结果，用于Blueprint的Exec引脚分支
 */
void UWarriorFunctionLibrary::BP_DoesActorHaveTag(AActor* InActor, FGameplayTag TagToCheck,
	EWarriorConfirmType& OutConfirm)
{
	// 根据Tag检查结果设置输出枚举值
	// 使用三元运算符：如果拥有Tag则输出Yes，否则输出No
	NativeDoesActorHaveTag(InActor, TagToCheck) ? OutConfirm = EWarriorConfirmType::Yes : OutConfirm = EWarriorConfirmType::No;
}

UPawnCombatComponent* UWarriorFunctionLibrary::NativeGetPawnCombatComponentFromActor(AActor* InActor)
{
	check(InActor)
	if (IPawnCombatInterface*PawnCombatInterface= Cast<IPawnCombatInterface>(InActor))
	{
	return PawnCombatInterface->GetPawnCombatComponent();	
	}
	return nullptr;
}

UPawnCombatComponent* UWarriorFunctionLibrary::BP_GetPawnCombatComponentFromActor(AActor* InActor,
	EWarriorValidType& OutValid)
{
	UPawnCombatComponent*CombatComponent= NativeGetPawnCombatComponentFromActor(InActor) ;
     OutValid= CombatComponent?EWarriorValidType::Valid:EWarriorValidType::InValid;
	 return CombatComponent;
}

bool UWarriorFunctionLibrary::IsTargetPawnHostile(APawn* QueryPawn, APawn* TargetPawn)
{
	// 获取两个Pawn的TeamID
	if (!QueryPawn || !TargetPawn || QueryPawn == TargetPawn)
	{
		return false;
	}

	const IGenericTeamAgentInterface* QueryTeamAgent = Cast<IGenericTeamAgentInterface>(QueryPawn->GetController());
	const IGenericTeamAgentInterface* TargetTeamAgent = Cast<IGenericTeamAgentInterface>(TargetPawn->GetController());
	
	if (!QueryTeamAgent || !TargetTeamAgent)
		return false;

	return QueryTeamAgent->GetGenericTeamId() != TargetTeamAgent->GetGenericTeamId();
}

AActor* UWarriorFunctionLibrary::FindNearestHostileActorInFront(
	APawn* QueryPawn,
	float TraceDistance,
	FVector TraceBoxHalfSize,
	const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes,
	bool bDrawDebugShape)
{
	if (!QueryPawn || ObjectTypes.IsEmpty() || TraceDistance <= 0.f)
	{
		return nullptr;
	}

	const FVector TraceStart = QueryPawn->GetActorLocation();
	const FVector TraceEnd = TraceStart + QueryPawn->GetActorForwardVector() * TraceDistance;
	const FRotator TraceRotation = QueryPawn->GetActorForwardVector().ToOrientationRotator();

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(QueryPawn);

	TArray<FHitResult> HitResults;
	UKismetSystemLibrary::BoxTraceMultiForObjects(
		QueryPawn,
		TraceStart,
		TraceEnd,
		TraceBoxHalfSize,
		TraceRotation,
		ObjectTypes,
		false,
		ActorsToIgnore,
		bDrawDebugShape ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
		HitResults,
		true);

	TArray<AActor*> HostileActors;
	for (const FHitResult& HitResult : HitResults)
	{
		APawn* HitPawn = Cast<APawn>(HitResult.GetActor());
		if (HitPawn && IsTargetPawnHostile(QueryPawn, HitPawn))
		{
			HostileActors.AddUnique(HitPawn);
		}
	}

	if (HostileActors.IsEmpty())
	{
		return nullptr;
	}

	float ClosestDistance = 0.f;
	return UGameplayStatics::FindNearestActor(QueryPawn->GetActorLocation(), HostileActors, ClosestDistance);
}

bool UWarriorFunctionLibrary::DashThroughTarget(
	AActor* DashActor,
	AActor* TargetActor,
	float PassThroughDistance,
	bool bSweep,
	bool bFaceDashDirection,
	FVector& OutDestination,
	FHitResult& OutSweepHit)
{
	OutDestination = FVector::ZeroVector;
	OutSweepHit = FHitResult();

	if (!DashActor)
	{
		return false;
	}

	AActor* ResolvedTargetActor = IsValidUnderAttackDashTarget(DashActor, TargetActor)
		? TargetActor
		: FindNearestUnderAttackDashTarget(DashActor);

	const FVector StartLocation = DashActor->GetActorLocation();
	FVector DashDirection = FVector::ZeroVector;

	if (ResolvedTargetActor)
	{
		DashDirection = ResolvedTargetActor->GetActorLocation() - StartLocation;
		DashDirection.Z = 0.f;
	}
	else
	{
		DashDirection = DashActor->GetActorForwardVector();
		DashDirection.Z = 0.f;
	}

	if (!DashDirection.Normalize())
	{
		return false;
	}

	OutDestination = ResolvedTargetActor
		? ResolvedTargetActor->GetActorLocation() + DashDirection * FMath::Max(0.f, PassThroughDistance)
		: StartLocation + DashDirection * FMath::Max(0.f, PassThroughDistance);
	OutDestination.Z = StartLocation.Z;

	if (bFaceDashDirection)
	{
		FRotator DashRotation = DashDirection.ToOrientationRotator();
		DashRotation.Pitch = 0.f;
		DashRotation.Roll = 0.f;
		DashActor->SetActorRotation(DashRotation);
	}

	if (ACharacter* DashCharacter = Cast<ACharacter>(DashActor))
	{
		if (UCharacterMovementComponent* MovementComponent = DashCharacter->GetCharacterMovement())
		{
			MovementComponent->StopMovementImmediately();
		}
	}

	UPrimitiveComponent* DashRootPrimitive = Cast<UPrimitiveComponent>(DashActor->GetRootComponent());
	UPrimitiveComponent* TargetRootPrimitive = ResolvedTargetActor
		? Cast<UPrimitiveComponent>(ResolvedTargetActor->GetRootComponent())
		: nullptr;
	if (bSweep && DashRootPrimitive && ResolvedTargetActor)
	{
		DashRootPrimitive->IgnoreActorWhenMoving(ResolvedTargetActor, true);
	}
	if (bSweep && TargetRootPrimitive)
	{
		TargetRootPrimitive->IgnoreActorWhenMoving(DashActor, true);
	}

	const bool bMoved = DashActor->SetActorLocation(
		OutDestination,
		bSweep,
		&OutSweepHit,
		ETeleportType::TeleportPhysics);

	if (bSweep && DashRootPrimitive && ResolvedTargetActor)
	{
		DashRootPrimitive->IgnoreActorWhenMoving(ResolvedTargetActor, false);
	}
	if (bSweep && TargetRootPrimitive)
	{
		TargetRootPrimitive->IgnoreActorWhenMoving(DashActor, false);
	}

	if (bSweep && OutSweepHit.bBlockingHit)
	{
		OutDestination = DashActor->GetActorLocation();
	}

	return bMoved;
}

FGameplayTag UWarriorFunctionLibrary::ComputeAttackDirectionTag(AActor* AttackerPawn, AActor* TargetPawn,
	float& OutAngleDifference)
{
	check(AttackerPawn&&TargetPawn)
	
	const FVector TargetForwardVector = TargetPawn->GetActorForwardVector();
	
	const FVector TargetToAttackerPawnNormalized = (AttackerPawn->GetActorLocation() - TargetPawn->GetActorLocation()).GetSafeNormal();
	
	const float DotResult = FVector::DotProduct(TargetForwardVector, TargetToAttackerPawnNormalized);
	
	OutAngleDifference = UKismetMathLibrary::DegAcos(DotResult);
	
	const FVector CrossResult = FVector::CrossProduct(TargetForwardVector, TargetToAttackerPawnNormalized);
	
	if (CrossResult.Z <0)
	{
		OutAngleDifference = -OutAngleDifference;
	}
	if (OutAngleDifference>=-45&&OutAngleDifference<=45)
	{
		return WarriorGameplayTags::Shared_Status_HitReact_Front;
	}
	if (OutAngleDifference>45&&OutAngleDifference<=135)
	{
		return WarriorGameplayTags::Shared_Status_HitReact_Right;
	}
	if (OutAngleDifference<-45&&OutAngleDifference>=-135)
	{
		return WarriorGameplayTags::Shared_Status_HitReact_Left;
	}	
	return WarriorGameplayTags::Shared_Status_HitReact_Back;
	
}


bool UWarriorFunctionLibrary::IsValidBlock(AActor* InAttacker, AActor* InDefender)
{
	check(InAttacker && InDefender);

	constexpr float CloseRangeThreshold = 150.f;
	constexpr float BlockHalfAngleDegrees = 70.f;
	const float BlockDotThreshold = FMath::Cos(FMath::DegreesToRadians(BlockHalfAngleDegrees));

	FVector AttackOrigin = InAttacker->GetActorLocation();

	if (UPawnCombatComponent* AttackerCombatComponent = NativeGetPawnCombatComponentFromActor(InAttacker))
	{
		if (AWarriorWeaponBase* CurrentWeapon = AttackerCombatComponent->GetCurrentEquippedWeapon())
		{
			if (UBoxComponent* WeaponCollisionBox = CurrentWeapon->GetWeaponCollisionMesh())
			{
				AttackOrigin = WeaponCollisionBox->GetComponentLocation();
			}
		}
	}

	FVector DefenderForward = InDefender->GetActorForwardVector();
	DefenderForward.Z = 0.f;
	DefenderForward = DefenderForward.GetSafeNormal();

	if (DefenderForward.IsNearlyZero())
	{
		return false;
	}

	FVector DefenderToAttackOrigin = AttackOrigin - InDefender->GetActorLocation();
	DefenderToAttackOrigin.Z = 0.f;

	float DotResult = -1.f;

	const float PositionDot = FVector::DotProduct(DefenderForward, DefenderToAttackOrigin.GetSafeNormal2D());

	// 近距离时武器盒子和角色根节点会快速穿插，朝向信息比纯位置更稳定
	if (DefenderToAttackOrigin.SizeSquared2D() <= FMath::Square(CloseRangeThreshold))
	{
		FVector AttackerForward = InAttacker->GetActorForwardVector();
		AttackerForward.Z = 0.f;
		AttackerForward = AttackerForward.GetSafeNormal();

		if (AttackerForward.IsNearlyZero())
		{
			return false;
		}

		const float FacingDot = FVector::DotProduct(DefenderForward, -AttackerForward);
		DotResult = FMath::Max(PositionDot, FacingDot);
	}
	else
	{
		DotResult = PositionDot;
	}

	Debug::print(FString::Printf(TEXT("BlockDot: %.2f"), DotResult), FColor::Red);

	return DotResult >= 0.6;
}

bool UWarriorFunctionLibrary::IsActorInDodgeIFrame(AActor* InActor)
{
	return InActor &&
		NativeDoesActorHaveTag(InActor, WarriorGameplayTags::Player_Status_DodgeIFrame);
}

void UWarriorFunctionLibrary::SendHitReactEventToActor(
	AActor* TargetActor,
	FGameplayTag EventTag,
	FGameplayEventData EventData)
{
	if (!TargetActor)
	{
		return;
	}

	const FGameplayTag ResolvedEventTag = EventTag.IsValid()
		? EventTag
		: WarriorGameplayTags::Shared_Event_HitReact;

	EventData.EventTag = ResolvedEventTag;
	if (!EventData.Target)
	{
		EventData.Target = TargetActor;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	const bool bIsSpecialHitReact = IsSpecialHitReactEvent(EventData);
	if (bIsSpecialHitReact)
	{
		if (HasPendingSpecialHitReactEvent(TargetActor))
		{
			return;
		}

		const bool bCancelledActiveHitReact = CancelActiveHitReactAbilities(TargetActor);
		if (bCancelledActiveHitReact)
		{
			SendHitReactGameplayEventNextTick(TargetActor, ResolvedEventTag, EventData);
			return;
		}
	}
	else if (HasActiveHitReactAbility(TargetASC))
	{
		return;
	}
	else if (HasPendingSpecialHitReactEvent(TargetActor))
	{
		return;
	}

	SendHitReactGameplayEvent(TargetActor, ResolvedEventTag, EventData);
}

bool UWarriorFunctionLibrary::HandleSuccessfulBlock(
	AActor* InBlocker,
	AActor* InAttacker,
	float BlockCost,
	FGameplayEventData EventData)
{
	if (!InBlocker)
	{
		return false;
	}

	const float AppliedBlockCost = FMath::Max(0.f, BlockCost);

	EventData.Target = InBlocker;
	if (!EventData.Instigator)
	{
		EventData.Instigator = InAttacker;
	}
	EventData.EventMagnitude = AppliedBlockCost;

	bool bGuardBroken = false;

	if (UAbilitySystemComponent* BlockerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InBlocker))
	{
		const float MaxBlockValue =
			BlockerASC->GetNumericAttribute(UWarriorAttributeSet::GetMaxBlockValueAttribute());
		const float CurrentBlockValue =
			BlockerASC->GetNumericAttribute(UWarriorAttributeSet::GetCurrentBlockValueAttribute());

		if (MaxBlockValue > 0.f)
		{
			const float NewCurrentBlockValue = FMath::Clamp(
				CurrentBlockValue - AppliedBlockCost,
				0.f,
				MaxBlockValue);

			BlockerASC->SetNumericAttributeBase(
				UWarriorAttributeSet::GetCurrentBlockValueAttribute(),
				NewCurrentBlockValue);
			BroadcastBlockValueChanged(InBlocker, NewCurrentBlockValue, MaxBlockValue);

			bGuardBroken = NewCurrentBlockValue <= 0.f;
		}
		else
		{
			bGuardBroken = true;
			BroadcastBlockValueChanged(InBlocker, 0.f, 0.f);
		}
	}

	if (UWarriorAbilitySystemComponent* WarriorBlockerASC =
		Cast<UWarriorAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InBlocker)))
	{
		WarriorBlockerASC->NotifyBlockValueConsumed(bGuardBroken);
	}

	if (bGuardBroken)
	{
		if (UAbilitySystemComponent* BlockerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InBlocker))
		{
			BlockerASC->RemoveLooseGameplayTag(WarriorGameplayTags::Player_Status_Blocking);
			BlockerASC->AddLooseGameplayTag(WarriorGameplayTags::Player_Status_GuardBroken);
		}

		FGameplayEventData GuardBreakEventData = EventData;
		GuardBreakEventData.EventTag = WarriorGameplayTags::Player_Event_GuardBreak;

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			InBlocker,
			WarriorGameplayTags::Player_Event_GuardBreak,
			GuardBreakEventData);
	}
	else
	{
		EventData.EventTag = WarriorGameplayTags::Player_Event_SuccessBlock;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			InBlocker,
			WarriorGameplayTags::Player_Event_SuccessBlock,
			EventData);
	}

	return bGuardBroken;
}

void UWarriorFunctionLibrary::ResetBlockValueToMax(AActor* InActor, bool bRemoveGuardBrokenStatus)
{
	if (!InActor)
	{
		return;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InActor);
	if (!ASC)
	{
		return;
	}

	const float MaxBlockValue = ASC->GetNumericAttribute(UWarriorAttributeSet::GetMaxBlockValueAttribute());
	const float NewCurrentBlockValue = FMath::Max(0.f, MaxBlockValue);

	ASC->SetNumericAttributeBase(
		UWarriorAttributeSet::GetCurrentBlockValueAttribute(),
		NewCurrentBlockValue);
	BroadcastBlockValueChanged(InActor, NewCurrentBlockValue, MaxBlockValue);

	if (bRemoveGuardBrokenStatus)
	{
		ASC->RemoveLooseGameplayTag(WarriorGameplayTags::Player_Status_GuardBroken);
	}
}

bool UWarriorFunctionLibrary::ApplyBossPoiseDamage(
	AActor* InTarget,
	AActor* InInstigator,
	float PoiseDamage,
	float StunDuration)
{
	if (!InTarget || PoiseDamage <= 0.f)
	{
		return false;
	}

	if (!Cast<AWarriorEnemyCharacter>(InTarget))
	{
		return false;
	}

	UWarriorAbilitySystemComponent* TargetASC =
		Cast<UWarriorAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InTarget));
	if (!TargetASC ||
		TargetASC->HasMatchingGameplayTag(WarriorGameplayTags::Shared_Status_Death) ||
		TargetASC->HasMatchingGameplayTag(WarriorGameplayTags::Enemy_Status_PoiseBroken))
	{
		return false;
	}

	const float MaxBossPoise = TargetASC->GetNumericAttribute(UWarriorAttributeSet::GetMaxBossPoiseAttribute());
	if (MaxBossPoise <= 0.f)
	{
		return false;
	}

	const float CurrentBossPoise = TargetASC->GetNumericAttribute(UWarriorAttributeSet::GetCurrentBossPoiseAttribute());
	const float NewBossPoise = FMath::Clamp(CurrentBossPoise - PoiseDamage, 0.f, MaxBossPoise);

	TargetASC->SetNumericAttributeBase(UWarriorAttributeSet::GetCurrentBossPoiseAttribute(), NewBossPoise);
	BroadcastBossPoiseChanged(InTarget, NewBossPoise, MaxBossPoise);

	if (NewBossPoise > 0.f)
	{
		return false;
	}

	TargetASC->CancelAbilities();
	TargetASC->AddLooseGameplayTag(WarriorGameplayTags::Enemy_Status_PoiseBroken);

	FGameplayEventData EventData;
	EventData.EventTag = WarriorGameplayTags::Enemy_Event_PoiseBreak;
	EventData.Instigator = InInstigator;
	EventData.Target = InTarget;
	EventData.EventMagnitude = PoiseDamage;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		InTarget,
		WarriorGameplayTags::Enemy_Event_PoiseBreak,
		EventData);

	TargetASC->AddLooseGameplayTag(WarriorGameplayTags::Enemy_Status_Stunned);

	if (UWorld* World = InTarget->GetWorld())
	{
		FTimerHandle PoiseRecoveryTimer;
		TWeakObjectPtr<AActor> WeakTarget = InTarget;
		TWeakObjectPtr<UWarriorAbilitySystemComponent> WeakTargetASC = TargetASC;
		const float RecoveryDelay = FMath::Max(0.f, StunDuration);

		World->GetTimerManager().SetTimer(PoiseRecoveryTimer, [WeakTarget, WeakTargetASC, MaxBossPoise]()
		{
			if (!WeakTarget.IsValid() || !WeakTargetASC.IsValid())
			{
				return;
			}

			if (WeakTargetASC->HasMatchingGameplayTag(WarriorGameplayTags::Shared_Status_Death))
			{
				return;
			}

			WeakTargetASC->RemoveLooseGameplayTag(WarriorGameplayTags::Enemy_Status_PoiseBroken);
			WeakTargetASC->RemoveLooseGameplayTag(WarriorGameplayTags::Enemy_Status_Stunned);
			WeakTargetASC->SetNumericAttributeBase(UWarriorAttributeSet::GetCurrentBossPoiseAttribute(), MaxBossPoise);
			BroadcastBossPoiseChanged(WeakTarget.Get(), MaxBossPoise, MaxBossPoise);
		}, RecoveryDelay, false);
	}

	return true;
}

bool UWarriorFunctionLibrary::ApplyGameplayEffectHandleToTarget(AActor* Instigator, AActor* TargetActor,
	const FGameplayEffectSpecHandle& InSpecHandle)
{
	UWarriorAbilitySystemComponent* SourceASC= NativeGetWarriorAscFromActor(Instigator);
	UWarriorAbilitySystemComponent* TargetASC = NativeGetWarriorAscFromActor(TargetActor);
	FActiveGameplayEffectHandle ActivateGameplayEffectHandle = SourceASC->ApplyGameplayEffectSpecToTarget(*InSpecHandle.Data,TargetASC);

	const float BaseDamage = InSpecHandle.Data->GetSetByCallerMagnitude(
		WarriorGameplayTags::Shared_SetByCaller_BaseDamage,
		false,
		0.f);

	const bool bWasApplied = ActivateGameplayEffectHandle.WasSuccessfullyApplied();
	if (bWasApplied && BaseDamage > 0.f)
	{
		const FWarriorAttackImpactData AttackImpactData =
			GetAttackImpactDataFromEffectSpecHandle(InSpecHandle, FWarriorAttackImpactData());

		ApplyBossPoiseDamage(
			TargetActor,
			Instigator,
			ResolveBossPoiseDamage(AttackImpactData, TargetASC),
			TargetASC->GetBossPoiseBreakStunDuration());

		ApplyAttackImpactToTarget(
			TargetActor,
			Instigator,
			AttackImpactData);
	}

	return bWasApplied;
	
	
}

void UWarriorFunctionLibrary::SetAttackImpactDataToEffectSpecHandle(
	FGameplayEffectSpecHandle& InOutSpecHandle,
	const FWarriorAttackImpactData& AttackImpactData)
{
	if (!InOutSpecHandle.IsValid() || !InOutSpecHandle.Data.IsValid())
	{
		return;
	}

	InOutSpecHandle.Data->AppendDynamicAssetTags(AttackImpactData.HitReactTags);

	InOutSpecHandle.Data->SetSetByCallerMagnitude(
		WarriorGameplayTags::Shared_SetByCaller_Attack_CanBeBlocked,
		AttackImpactData.bCanBeBlocked ? 1.f : 0.f);
	InOutSpecHandle.Data->SetSetByCallerMagnitude(
		WarriorGameplayTags::Shared_SetByCaller_Attack_CanBeDodged,
		AttackImpactData.bCanBeDodged ? 1.f : 0.f);
	InOutSpecHandle.Data->SetSetByCallerMagnitude(
		WarriorGameplayTags::Shared_SetByCaller_Attack_BlockCost,
		FMath::Max(0.f, AttackImpactData.BlockCost));
	InOutSpecHandle.Data->SetSetByCallerMagnitude(
		WarriorGameplayTags::Shared_SetByCaller_Attack_BossPoiseDamage,
		FMath::Max(-1.f, AttackImpactData.BossPoiseDamage));
	InOutSpecHandle.Data->SetSetByCallerMagnitude(
		WarriorGameplayTags::Shared_SetByCaller_Attack_HorizontalLaunchStrength,
		FMath::Max(0.f, AttackImpactData.HorizontalLaunchStrength));
	InOutSpecHandle.Data->SetSetByCallerMagnitude(
		WarriorGameplayTags::Shared_SetByCaller_Attack_VerticalLaunchStrength,
		FMath::Max(0.f, AttackImpactData.VerticalLaunchStrength));
	InOutSpecHandle.Data->SetSetByCallerMagnitude(
		WarriorGameplayTags::Shared_SetByCaller_Attack_MeshTiltAngle,
		FMath::Clamp(AttackImpactData.MeshTiltAngle, -89.f, 89.f));
	InOutSpecHandle.Data->SetSetByCallerMagnitude(
		WarriorGameplayTags::Shared_SetByCaller_Attack_MeshTiltDuration,
		FMath::Max(0.f, AttackImpactData.MeshTiltDuration));
}

FWarriorAttackImpactData UWarriorFunctionLibrary::GetAttackImpactDataFromEffectSpecHandle(
	const FGameplayEffectSpecHandle& InSpecHandle,
	FWarriorAttackImpactData DefaultAttackImpactData)
{
	FWarriorAttackImpactData AttackImpactData = DefaultAttackImpactData;
	if (InSpecHandle.IsValid() && InSpecHandle.Data.IsValid())
	{
		AttackImpactData.HitReactTags.AppendTags(InSpecHandle.Data->GetDynamicAssetTags());
	}

	AttackImpactData.bCanBeBlocked =
		GetAttackImpactSetByCallerMagnitude(
			InSpecHandle,
			WarriorGameplayTags::Shared_SetByCaller_Attack_CanBeBlocked,
			AttackImpactData.bCanBeBlocked ? 1.f : 0.f) > 0.5f;
	AttackImpactData.bCanBeDodged =
		GetAttackImpactSetByCallerMagnitude(
			InSpecHandle,
			WarriorGameplayTags::Shared_SetByCaller_Attack_CanBeDodged,
			AttackImpactData.bCanBeDodged ? 1.f : 0.f) > 0.5f;
	AttackImpactData.BlockCost =
		FMath::Max(0.f, GetAttackImpactSetByCallerMagnitude(
			InSpecHandle,
			WarriorGameplayTags::Shared_SetByCaller_Attack_BlockCost,
			AttackImpactData.BlockCost));
	AttackImpactData.BossPoiseDamage =
		FMath::Max(-1.f, GetAttackImpactSetByCallerMagnitude(
			InSpecHandle,
			WarriorGameplayTags::Shared_SetByCaller_Attack_BossPoiseDamage,
			AttackImpactData.BossPoiseDamage));
	AttackImpactData.HorizontalLaunchStrength =
		FMath::Max(0.f, GetAttackImpactSetByCallerMagnitude(
			InSpecHandle,
			WarriorGameplayTags::Shared_SetByCaller_Attack_HorizontalLaunchStrength,
			AttackImpactData.HorizontalLaunchStrength));
	AttackImpactData.VerticalLaunchStrength =
		FMath::Max(0.f, GetAttackImpactSetByCallerMagnitude(
			InSpecHandle,
			WarriorGameplayTags::Shared_SetByCaller_Attack_VerticalLaunchStrength,
			AttackImpactData.VerticalLaunchStrength));
	AttackImpactData.MeshTiltAngle =
		FMath::Clamp(GetAttackImpactSetByCallerMagnitude(
			InSpecHandle,
			WarriorGameplayTags::Shared_SetByCaller_Attack_MeshTiltAngle,
			AttackImpactData.MeshTiltAngle), -89.f, 89.f);
	AttackImpactData.MeshTiltDuration =
		FMath::Max(0.f, GetAttackImpactSetByCallerMagnitude(
			InSpecHandle,
			WarriorGameplayTags::Shared_SetByCaller_Attack_MeshTiltDuration,
			AttackImpactData.MeshTiltDuration));

	return AttackImpactData;
}

float UWarriorFunctionLibrary::ResolveBossPoiseDamage(
	const FWarriorAttackImpactData& AttackImpactData,
	const UWarriorAbilitySystemComponent* TargetASC)
{
	if (AttackImpactData.BossPoiseDamage >= 0.f)
	{
		return AttackImpactData.BossPoiseDamage;
	}

	return TargetASC ? TargetASC->GetBossPoiseDamageOnHit() : 0.f;
}

bool UWarriorFunctionLibrary::ApplyAttackImpactToTarget(
	AActor* InTarget,
	AActor* InInstigator,
	const FWarriorAttackImpactData& AttackImpactData)
{
	ACharacter* TargetCharacter = Cast<ACharacter>(InTarget);
	if (!TargetCharacter || (!AttackImpactData.HasLaunchImpact() && !AttackImpactData.HasMeshTiltImpact()))
	{
		return false;
	}

	FVector ImpactDirection = FVector::ZeroVector;
	if (InInstigator)
	{
		ImpactDirection = InTarget->GetActorLocation() - InInstigator->GetActorLocation();
	}

	ImpactDirection.Z = 0.f;
	ImpactDirection = ImpactDirection.GetSafeNormal();
	if (ImpactDirection.IsNearlyZero())
	{
		ImpactDirection = -TargetCharacter->GetActorForwardVector();
		ImpactDirection.Z = 0.f;
		ImpactDirection = ImpactDirection.GetSafeNormal();
	}

	bool bAppliedImpact = false;

	if (AttackImpactData.HasLaunchImpact() && !ImpactDirection.IsNearlyZero())
	{
		const FVector LaunchVelocity =
			ImpactDirection * FMath::Max(0.f, AttackImpactData.HorizontalLaunchStrength) +
			FVector::UpVector * FMath::Max(0.f, AttackImpactData.VerticalLaunchStrength);

		TargetCharacter->LaunchCharacter(
			LaunchVelocity,
			true,
			AttackImpactData.VerticalLaunchStrength > 0.f);
		bAppliedImpact = true;
	}

	if (AttackImpactData.HasMeshTiltImpact())
	{
		if (USkeletalMeshComponent* MeshComponent = TargetCharacter->GetMesh())
		{
			const FRotator OriginalRelativeRotation = MeshComponent->GetRelativeRotation();
			const FVector LocalImpactDirection =
				TargetCharacter->GetActorTransform().InverseTransformVectorNoScale(ImpactDirection);
			const float TiltAngle = FMath::Clamp(AttackImpactData.MeshTiltAngle, -89.f, 89.f);
			const FRotator TiltOffset(
				-LocalImpactDirection.X * TiltAngle,
				0.f,
				LocalImpactDirection.Y * TiltAngle);

			MeshComponent->SetRelativeRotation(OriginalRelativeRotation + TiltOffset);
			bAppliedImpact = true;

			if (UWorld* World = InTarget->GetWorld())
			{
				FTimerHandle ResetTiltTimerHandle;
				TWeakObjectPtr<USkeletalMeshComponent> WeakMeshComponent = MeshComponent;

				World->GetTimerManager().SetTimer(
					ResetTiltTimerHandle,
					[WeakMeshComponent, OriginalRelativeRotation]()
					{
						if (WeakMeshComponent.IsValid())
						{
							WeakMeshComponent->SetRelativeRotation(OriginalRelativeRotation);
						}
					},
					FMath::Max(0.f, AttackImpactData.MeshTiltDuration),
					false);
			}
		}
	}

	return bAppliedImpact;
}

void UWarriorFunctionLibrary::AddAttackImpactDataToGameplayEventData(
	FGameplayEventData& InOutEventData,
	const FWarriorAttackImpactData& AttackImpactData)
{
	InOutEventData.TargetTags.AppendTags(AttackImpactData.HitReactTags);

	if (!AttackImpactData.bCanBeBlocked)
	{
		InOutEventData.TargetTags.AddTag(WarriorGameplayTags::Enemy_Status_UnBlockable);
	}

	if (AttackImpactData.HasLaunchImpact())
	{
		InOutEventData.TargetTags.AddTag(WarriorGameplayTags::Shared_Status_HitReact_Knockback);
	}

	if (AttackImpactData.VerticalLaunchStrength > 0.f)
	{
		InOutEventData.TargetTags.AddTag(WarriorGameplayTags::Shared_Status_HitReact_Launch);
	}

	if (AttackImpactData.HasMeshTiltImpact())
	{
		InOutEventData.TargetTags.AddTag(WarriorGameplayTags::Shared_Status_HitReact_Tilt);
	}
}

bool UWarriorFunctionLibrary::IsSpecialHitReactEvent(const FGameplayEventData& EventData)
{
	const FGameplayTag LargeHitReactTag =
		FGameplayTag::RequestGameplayTag(FName(TEXT("Shared.Ability.HitReact.Large")), false);

	return EventData.TargetTags.HasTagExact(WarriorGameplayTags::Shared_Status_HitReact_Heavy) ||
		EventData.TargetTags.HasTagExact(WarriorGameplayTags::Shared_Status_HitReact_Launch) ||
		EventData.TargetTags.HasTagExact(WarriorGameplayTags::Shared_Status_HitReact_Knockback) ||
		EventData.TargetTags.HasTagExact(WarriorGameplayTags::Shared_Status_HitReact_Tilt) ||
		(LargeHitReactTag.IsValid() && EventData.TargetTags.HasTag(LargeHitReactTag));
}

bool UWarriorFunctionLibrary::CancelActiveHitReactAbilities(AActor* TargetActor)
{
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!TargetASC)
	{
		return false;
	}

	TArray<FGameplayAbilitySpecHandle> HitReactAbilityHandles;
	for (const FGameplayAbilitySpec& AbilitySpec : TargetASC->GetActivatableAbilities())
	{
		if (!AbilitySpec.IsActive() || !IsHitReactAbilitySpec(AbilitySpec))
		{
			continue;
		}

		HitReactAbilityHandles.Add(AbilitySpec.Handle);
	}

	for (const FGameplayAbilitySpecHandle& AbilityHandle : HitReactAbilityHandles)
	{
		TargetASC->CancelAbilityHandle(AbilityHandle);
	}

	return !HitReactAbilityHandles.IsEmpty();
}

float UWarriorFunctionLibrary::GetScalableFloatValueAtLevel(const FScalableFloat& InScalableFloat, float InLevel)
{
	return InScalableFloat.GetValueAtLevel(InLevel);
}

void UWarriorFunctionLibrary::CountDown(const UObject* WorldContextObject, float Duration, float UpdateInterval,
                                        float& OutRemainingTime, EWarriorCountDownInput CountDownInput,UPARAM(DisplayName = "Output") EWarriorCountDownOutput& CountDownOutput,
                                        FLatentActionInfo LatentInfo)
{
	UWorld* World = nullptr;
	
	if (GEngine)
	{
		World =GEngine->GetWorldFromContextObject(WorldContextObject,EGetWorldErrorMode::LogAndReturnNull);
		
	}
	
	if (!World)
	{
	return;
	}
	
	FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
	
	FWarriorCountDownAction* FoundAction = LatentActionManager.FindExistingAction<FWarriorCountDownAction>(LatentInfo.CallbackTarget,LatentInfo.UUID);	
	
	if (CountDownInput == EWarriorCountDownInput::Start)
	{
		if (!FoundAction)
		{
			LatentActionManager.AddNewAction(LatentInfo.CallbackTarget,
				LatentInfo.UUID, 
				new FWarriorCountDownAction(Duration, UpdateInterval, OutRemainingTime, CountDownOutput,LatentInfo));
		}
	}
	else if (CountDownInput == EWarriorCountDownInput::Cancel)
	{
		if (FoundAction)
		{
			FoundAction->CancelAction();
			
		}
		
	}
	
}

UWarriorGameInstance* UWarriorFunctionLibrary::GetWarriorGameInstance(const UObject* WorldContextObject)
{
	if (GEngine)
	{
		if (UWorld*World = GEngine->GetWorldFromContextObject(WorldContextObject,EGetWorldErrorMode::LogAndReturnNull))
		{
			return World->GetGameInstance<UWarriorGameInstance>();
			
		}
		
	}
	return nullptr;
}

void UWarriorFunctionLibrary::ToggleInputMode(const UObject* WorldContextObject, EWarriorInputMode InInputMode)
{
	APlayerController* PlayerController = nullptr;
	
	if (GEngine)
	{
		if (UWorld*World = GEngine->GetWorldFromContextObject(WorldContextObject,EGetWorldErrorMode::LogAndReturnNull))
		{
			PlayerController = World->GetFirstPlayerController();
			
		}
		
	}
	if (!PlayerController)
	{
		return;
	}
	
	FInputModeGameOnly GameOnlyMode;
	FInputModeUIOnly UIOnlyMode;
	FInputModeGameAndUI GameAndUIMode;
	switch (InInputMode)
	{
	case EWarriorInputMode::GameOnly
		:
		PlayerController->SetInputMode(GameOnlyMode);
		PlayerController->SetShowMouseCursor(false);
		break;
		
	case EWarriorInputMode::UIOnly
		:
		PlayerController->SetInputMode(UIOnlyMode);
		PlayerController->SetShowMouseCursor(true);
		break;
		
	case EWarriorInputMode::GameAndUI:
		PlayerController->SetInputMode(GameAndUIMode);
		PlayerController->SetShowMouseCursor(true);
		
		default:
		break;
	}
	
}

void UWarriorFunctionLibrary::SaveCurrentGameDifficulty(WarriorDifficulty InDifficulty)
{
	USaveGame* SaveGame = UGameplayStatics::CreateSaveGameObject(UWarriorSaveGame::StaticClass()); 
    if (UWarriorSaveGame*WarriorSaveGame = Cast<UWarriorSaveGame>(SaveGame))
    {
    	WarriorSaveGame->SavedCurrentDifficult = InDifficulty;
    	
    	const bool bWasSaved = UGameplayStatics::SaveGameToSlot(WarriorSaveGame,WarriorGameplayTags::GameData_SaveGame_Slot_1.GetTag().ToString(),0);
    	
    	
    }
    	
}

bool UWarriorFunctionLibrary::TryLoadSaveGameDifficulty(WarriorDifficulty& OutSavedDifficulty)
{
	if (UGameplayStatics::DoesSaveGameExist(WarriorGameplayTags::GameData_SaveGame_Slot_1.GetTag().ToString(),0))
	{
	 USaveGame* SaveGame =UGameplayStatics::LoadGameFromSlot(WarriorGameplayTags::GameData_SaveGame_Slot_1.GetTag().ToString(),0);
		
		if (UWarriorSaveGame*WarriorSaveGame =Cast<UWarriorSaveGame>( SaveGame))
		{
			OutSavedDifficulty=WarriorSaveGame->SavedCurrentDifficult;
			
			
			return true;
		}
	}
	
	return false;
	
}
