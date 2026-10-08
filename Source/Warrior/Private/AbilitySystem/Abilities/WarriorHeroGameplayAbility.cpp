// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/WarriorHeroGameplayAbility.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "Characters/WarriorHeroCharacter.h"
#include "Controllers/WarriorHeroController.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TimerManager.h"
#include "WarriorFunctionLibrary.h"
#include "WarriorGameplayTags.h"

AWarriorHeroCharacter* UWarriorHeroGameplayAbility::GetHeroCharacterFromActorInfo() const
{
	// 懒加载缓存：首次调用时从 ActorInfo 取角色并缓存，后续复用，减少重复 Cast。
	// 使用 WeakPtr 避免延长对象生命周期；对象销毁后 IsValid() 会自然失效。
	if (!CachedHeroCharacter.IsValid() && CurrentActorInfo)
	{
		CachedHeroCharacter = Cast<AWarriorHeroCharacter>(CurrentActorInfo->AvatarActor);
	}

	return CachedHeroCharacter.IsValid() ? CachedHeroCharacter.Get() : nullptr;
}

AWarriorHeroController* UWarriorHeroGameplayAbility::GetHeroControllerFromActorInfo() const
{
	// 与角色缓存逻辑一致：按需缓存 Controller，避免频繁查找。
	if (!CachedHeroController.IsValid() && CurrentActorInfo)
	{
		CachedHeroController = Cast<AWarriorHeroController>(CurrentActorInfo->PlayerController);
	}

	return CachedHeroController.IsValid() ? CachedHeroController.Get() : nullptr;
}

UHeroCombatComponent* UWarriorHeroGameplayAbility::GetHeroCombatComponentFromActorInfo() const
{
	// 依赖角色对象拿战斗组件；调用侧应确保当前能力确实绑定在 Hero 身上。
	AWarriorHeroCharacter* HeroCharacter = GetHeroCharacterFromActorInfo();
	return HeroCharacter ? HeroCharacter->GetHeroCombatComponent() : nullptr;
}

bool UWarriorHeroGameplayAbility::IsHeroAirborne() const
{
	const UWarriorAbilitySystemComponent* WarriorASC = GetWarriorASCFromActorInfo();
	return WarriorASC && WarriorASC->IsAvatarAirborne();
}

bool UWarriorHeroGameplayAbility::IsHeroRageActive() const
{
	const UWarriorAbilitySystemComponent* WarriorASC = GetWarriorASCFromActorInfo();
	return WarriorASC && WarriorASC->IsRageActive();
}

bool UWarriorHeroGameplayAbility::IsHeroAttackInputBlocked(FGameplayTag InputTag) const
{
	const UWarriorAbilitySystemComponent* WarriorASC = GetWarriorASCFromActorInfo();
	return WarriorASC && WarriorASC->IsInputBlockedByAttackState(InputTag);
}

TArray<FGameplayTag> UWarriorHeroGameplayAbility::ResolveHeroAbilityInputTagPriority(FGameplayTag InputTag) const
{
	const UWarriorAbilitySystemComponent* WarriorASC = GetWarriorASCFromActorInfo();
	return WarriorASC ? WarriorASC->ResolveAbilityInputTagPriority(InputTag) : TArray<FGameplayTag>();
}

FGameplayEffectSpecHandle UWarriorHeroGameplayAbility::HeroDamageEffectHandle(TSubclassOf<UGameplayEffect> EffectClass,
	float InWeaponBaseDamage, FGameplayTag InCurrentAttackTypeTag, int32 InUsedComboCount)
{
	check(EffectClass)

	// 1) 构建 EffectContext，标记能力、来源对象和施加者，供后续执行计算读取。
	FGameplayEffectContextHandle EffectContextHandle = GetWarriorASCFromActorInfo()->MakeEffectContext();
	EffectContextHandle.SetAbility(this);
	EffectContextHandle.AddSourceObject(GetAvatarActorFromActorInfo());
	EffectContextHandle.AddInstigator(GetAvatarActorFromActorInfo(), GetAvatarActorFromActorInfo());

	// 2) 生成 Outgoing Spec（可理解为“待应用的伤害数据包”）。
	FGameplayEffectSpecHandle EffectSpecHandle = GetWarriorASCFromActorInfo()->MakeOutgoingSpec(EffectClass, GetAbilityLevel(), EffectContextHandle);

	// 3) 通过 SetByCaller 注入运行时参数：基础伤害始终写入，攻击类型按需写入。
	EffectSpecHandle.Data->SetSetByCallerMagnitude(WarriorGameplayTags::Shared_SetByCaller_BaseDamage, InWeaponBaseDamage);

	if (InCurrentAttackTypeTag.IsValid())
	{
		EffectSpecHandle.Data->SetSetByCallerMagnitude(InCurrentAttackTypeTag, InUsedComboCount);
	}

	// 4) 返回 Spec 供外部应用到目标（ApplyGameplayEffectSpecToTarget/Actor 等）。
	return EffectSpecHandle;
}

TArray<AActor*> UWarriorHeroGameplayAbility::PullFrontTargetsAndApplyMultiSlashDamage(
	const FGameplayEffectSpecHandle& InEffectSpecHandle,
	const FWarriorHeavyAttackMultiSlashData& SlashData,
	const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes)
{
	TArray<AActor*> TargetActors;
	if (!InEffectSpecHandle.IsValid() || !InEffectSpecHandle.Data.IsValid())
	{
		return TargetActors;
	}

	AWarriorHeroCharacter* HeroCharacter = GetHeroCharacterFromActorInfo();
	UWorld* World = HeroCharacter ? HeroCharacter->GetWorld() : nullptr;
	if (!HeroCharacter || !World)
	{
		return TargetActors;
	}

	TargetActors = FindHostileTargetsInFront(SlashData, ObjectTypes);
	if (TargetActors.IsEmpty())
	{
		return TargetActors;
	}

	const FVector SphereCenter = GetMultiSlashPullSphereCenter(SlashData);
	if (SlashData.bDrawDebug)
	{
		DrawDebugSphere(
			World,
			SphereCenter,
			FMath::Max(0.f, SlashData.PullSphereRadius),
			32,
			FColor::Cyan,
			false,
			2.f);
	}

	TArray<TWeakObjectPtr<AActor>> WeakTargets;
	TArray<FVector> PullLocations;
	WeakTargets.Reserve(TargetActors.Num());
	PullLocations.Reserve(TargetActors.Num());

	for (int32 TargetIndex = 0; TargetIndex < TargetActors.Num(); ++TargetIndex)
	{
		AActor* TargetActor = TargetActors[TargetIndex];
		if (!TargetActor)
		{
			continue;
		}

		WeakTargets.Add(TargetActor);
		PullLocations.Add(GetMultiSlashTargetLocation(SlashData, TargetIndex, TargetActors.Num()));
	}

	if (WeakTargets.IsEmpty())
	{
		TargetActors.Empty();
		return TargetActors;
	}

	const FWarriorAttackImpactData AttackImpactData =
		UWarriorFunctionLibrary::GetAttackImpactDataFromEffectSpecHandle(
			InEffectSpecHandle,
			FWarriorAttackImpactData());
	const int32 SlashCount = FMath::Max(1, SlashData.SlashCount);
	const float SlashInterval = FMath::Max(0.f, SlashData.SlashInterval);

	if (SlashInterval <= KINDA_SMALL_NUMBER)
	{
		for (int32 SlashIndex = 0; SlashIndex < SlashCount; ++SlashIndex)
		{
			ApplyMultiSlashHit(InEffectSpecHandle, AttackImpactData, WeakTargets, PullLocations);
		}

		return TargetActors;
	}

	ApplyMultiSlashHit(InEffectSpecHandle, AttackImpactData, WeakTargets, PullLocations);

	for (int32 SlashIndex = 1; SlashIndex < SlashCount; ++SlashIndex)
	{
		FTimerHandle TimerHandle;
		World->GetTimerManager().SetTimer(
			TimerHandle,
			FTimerDelegate::CreateWeakLambda(
				this,
				[this, InEffectSpecHandle, AttackImpactData, WeakTargets, PullLocations]()
				{
					if (!IsActive())
					{
						return;
					}

					ApplyMultiSlashHit(InEffectSpecHandle, AttackImpactData, WeakTargets, PullLocations);
				}),
			SlashInterval * SlashIndex,
			false);
	}

	return TargetActors;
}

TArray<AActor*> UWarriorHeroGameplayAbility::FindHostileTargetsInFront(
	const FWarriorHeavyAttackMultiSlashData& SlashData,
	const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes) const
{
	TArray<AActor*> TargetActors;

	APawn* OwningPawn = Cast<APawn>(GetAvatarActorFromActorInfo());
	if (!OwningPawn || SlashData.FrontTraceDistance <= 0.f)
	{
		return TargetActors;
	}

	FVector ForwardVector = OwningPawn->GetActorForwardVector();
	ForwardVector.Z = 0.f;
	ForwardVector = ForwardVector.GetSafeNormal();
	if (ForwardVector.IsNearlyZero())
	{
		return TargetActors;
	}

	FVector BoxHalfSize(
		FMath::Abs(SlashData.FrontTraceBoxHalfSize.X),
		FMath::Abs(SlashData.FrontTraceBoxHalfSize.Y),
		FMath::Abs(SlashData.FrontTraceBoxHalfSize.Z));
	if (BoxHalfSize.X <= 0.f || BoxHalfSize.Y <= 0.f || BoxHalfSize.Z <= 0.f)
	{
		return TargetActors;
	}

	TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes = ObjectTypes;
	if (TraceObjectTypes.IsEmpty())
	{
		TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	}

	const FVector TraceStart = OwningPawn->GetActorLocation();
	const FVector TraceEnd = TraceStart + ForwardVector * SlashData.FrontTraceDistance;
	const FRotator TraceRotation = ForwardVector.ToOrientationRotator();

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(OwningPawn);

	TArray<FHitResult> HitResults;
	UKismetSystemLibrary::BoxTraceMultiForObjects(
		OwningPawn,
		TraceStart,
		TraceEnd,
		BoxHalfSize,
		TraceRotation,
		TraceObjectTypes,
		false,
		ActorsToIgnore,
		SlashData.bDrawDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
		HitResults,
		true);

	for (const FHitResult& HitResult : HitResults)
	{
		APawn* TargetPawn = Cast<APawn>(HitResult.GetActor());
		if (!TargetPawn ||
			TargetActors.Contains(TargetPawn) ||
			!UWarriorFunctionLibrary::IsTargetPawnHostile(OwningPawn, TargetPawn) ||
			UWarriorFunctionLibrary::NativeDoesActorHaveTag(TargetPawn, WarriorGameplayTags::Shared_Status_Death))
		{
			continue;
		}

		TargetActors.Add(TargetPawn);
	}

	const FVector OwnerLocation = OwningPawn->GetActorLocation();
	TargetActors.Sort([OwnerLocation](const AActor& Left, const AActor& Right)
	{
		return FVector::DistSquared(OwnerLocation, Left.GetActorLocation()) <
			FVector::DistSquared(OwnerLocation, Right.GetActorLocation());
	});

	return TargetActors;
}

FVector UWarriorHeroGameplayAbility::GetMultiSlashPullSphereCenter(
	const FWarriorHeavyAttackMultiSlashData& SlashData) const
{
	const AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor)
	{
		return FVector::ZeroVector;
	}

	FVector ForwardVector = AvatarActor->GetActorForwardVector();
	ForwardVector.Z = 0.f;
	ForwardVector = ForwardVector.GetSafeNormal();
	if (ForwardVector.IsNearlyZero())
	{
		ForwardVector = AvatarActor->GetActorForwardVector();
	}

	return AvatarActor->GetActorLocation() +
		ForwardVector * FMath::Max(0.f, SlashData.PullSphereCenterDistance) +
		FVector::UpVector * SlashData.PullSphereHeightOffset;
}

FVector UWarriorHeroGameplayAbility::GetMultiSlashTargetLocation(
	const FWarriorHeavyAttackMultiSlashData& SlashData,
	int32 TargetIndex,
	int32 TargetCount) const
{
	const AActor* AvatarActor = GetAvatarActorFromActorInfo();
	const FVector SphereCenter = GetMultiSlashPullSphereCenter(SlashData);
	if (!AvatarActor || TargetCount <= 1 || SlashData.PullSphereRadius <= 0.f)
	{
		return SphereCenter;
	}

	FVector ForwardVector = AvatarActor->GetActorForwardVector();
	ForwardVector.Z = 0.f;
	ForwardVector = ForwardVector.GetSafeNormal();
	if (ForwardVector.IsNearlyZero())
	{
		ForwardVector = FVector::ForwardVector;
	}

	FVector RightVector = AvatarActor->GetActorRightVector();
	RightVector.Z = 0.f;
	RightVector = RightVector.GetSafeNormal();
	if (RightVector.IsNearlyZero())
	{
		RightVector = FVector::RightVector;
	}

	const float Angle = (2.f * PI * static_cast<float>(TargetIndex)) / static_cast<float>(TargetCount);
	const float RingRadius = SlashData.PullSphereRadius * 0.45f;
	const FVector RingOffset =
		ForwardVector * FMath::Sin(Angle) * RingRadius +
		RightVector * FMath::Cos(Angle) * RingRadius;

	return SphereCenter + RingOffset;
}

void UWarriorHeroGameplayAbility::PullTargetToMultiSlashSphere(
	AActor* TargetActor,
	const FVector& TargetLocation) const
{
	if (!TargetActor)
	{
		return;
	}

	if (ACharacter* TargetCharacter = Cast<ACharacter>(TargetActor))
	{
		if (UCharacterMovementComponent* MovementComponent = TargetCharacter->GetCharacterMovement())
		{
			MovementComponent->StopMovementImmediately();
		}
	}

	TargetActor->SetActorLocation(TargetLocation, true, nullptr, ETeleportType::TeleportPhysics);
}

void UWarriorHeroGameplayAbility::ApplyMultiSlashHit(
	const FGameplayEffectSpecHandle& InEffectSpecHandle,
	const FWarriorAttackImpactData& AttackImpactData,
	const TArray<TWeakObjectPtr<AActor>>& Targets,
	const TArray<FVector>& PullLocations)
{
	if (!InEffectSpecHandle.IsValid() || !InEffectSpecHandle.Data.IsValid())
	{
		return;
	}

	APawn* OwningPawn = Cast<APawn>(GetAvatarActorFromActorInfo());
	if (!OwningPawn)
	{
		return;
	}

	int32 AppliedCount = 0;
	const int32 TargetCount = FMath::Min(Targets.Num(), PullLocations.Num());
	for (int32 TargetIndex = 0; TargetIndex < TargetCount; ++TargetIndex)
	{
		AActor* TargetActor = Targets[TargetIndex].Get();
		APawn* TargetPawn = Cast<APawn>(TargetActor);
		if (!TargetPawn ||
			!UWarriorFunctionLibrary::IsTargetPawnHostile(OwningPawn, TargetPawn) ||
			UWarriorFunctionLibrary::NativeDoesActorHaveTag(TargetPawn, WarriorGameplayTags::Shared_Status_Death) ||
			!UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetPawn))
		{
			continue;
		}

		PullTargetToMultiSlashSphere(TargetPawn, PullLocations[TargetIndex]);

		const FActiveGameplayEffectHandle EffectHandle =
			NativeApplyEffectSpecHandleToTarget(TargetPawn, InEffectSpecHandle);
		if (!EffectHandle.WasSuccessfullyApplied())
		{
			continue;
		}

		++AppliedCount;

		FGameplayEventData EventData;
		EventData.Instigator = OwningPawn;
		EventData.Target = TargetPawn;
		EventData.EventMagnitude = AttackImpactData.BlockCost;
		UWarriorFunctionLibrary::AddAttackImpactDataToGameplayEventData(EventData, AttackImpactData);

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			TargetPawn,
			WarriorGameplayTags::Shared_Event_HitReact,
			EventData);
	}

	if (AppliedCount > 0)
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			OwningPawn,
			WarriorGameplayTags::Player_Event_HitPause,
			FGameplayEventData());
	}
}

bool UWarriorHeroGameplayAbility::GetAbilityRemainingCooldownByTag(FGameplayTag CooldownTag, float& TimeRemaining,
	float& TotalCooldownTime)
{
	check(CooldownTag.IsValid());
	
	FGameplayEffectQuery EffectQuery = FGameplayEffectQuery::MakeQuery_MatchAllOwningTags(CooldownTag.GetSingleTagContainer());
	TArray<TPair<float,float>>TimeRemainingAndDuration= GetAbilitySystemComponentFromActorInfo()->GetActiveEffectsTimeRemainingAndDuration(EffectQuery);
	
	if (!TimeRemainingAndDuration.IsEmpty())
	{
		TotalCooldownTime = TimeRemainingAndDuration[0].Value;
		TimeRemaining = TimeRemainingAndDuration[0].Key;
		
	}
	
	return TimeRemaining > 0.0f;
	
}
