// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/WarriorEnemyGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "Characters/WarriorEnemyCharacter.h"
#include "Components/Combat/EnemyCombatComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "WarriorFunctionLibrary.h"
#include "WarriorGameplayTags.h"

void UWarriorEnemyGameplayAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	SetCurrentAttackImpactData(DefaultAttackImpactData);

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UWarriorEnemyGameplayAbility::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

AWarriorEnemyCharacter* UWarriorEnemyGameplayAbility::GetEnemyCharacterFromActorInfor()
{
	if (!CachedEnemyCharacter.IsValid())
	{
		CachedEnemyCharacter = Cast<AWarriorEnemyCharacter>(GetAvatarActorFromActorInfo());
	}

	return CachedEnemyCharacter.IsValid() ? CachedEnemyCharacter.Get() : nullptr;
}

UEnemyCombatComponent* UWarriorEnemyGameplayAbility::GetEnemyCombatComponent()
{
	if (AWarriorEnemyCharacter* EnemyCharacter = GetEnemyCharacterFromActorInfor())
	{
		return EnemyCharacter->GetEnemyCombatComponent();
	}

	return nullptr;
}

FGameplayEffectSpecHandle UWarriorEnemyGameplayAbility::EnemyDamageEffectHandle(
	TSubclassOf<UGameplayEffect> EffectClass,
	const FScalableFloat& InDamageScalableFloat)
{
	return EnemyDamageEffectHandleWithAttackImpactData(
		EffectClass,
		InDamageScalableFloat,
		DefaultAttackImpactData);
}

FGameplayEffectSpecHandle UWarriorEnemyGameplayAbility::EnemyDamageEffectHandleWithAttackImpactData(
	TSubclassOf<UGameplayEffect> EffectClass,
	const FScalableFloat& InDamageScalableFloat,
	const FWarriorAttackImpactData& AttackImpactData)
{
	check(EffectClass);

	FGameplayEffectContextHandle EffectContextHandle = GetWarriorASCFromActorInfo()->MakeEffectContext();
	EffectContextHandle.SetAbility(this);
	EffectContextHandle.AddSourceObject(GetAvatarActorFromActorInfo());
	EffectContextHandle.AddInstigator(GetAvatarActorFromActorInfo(), GetAvatarActorFromActorInfo());

	FGameplayEffectSpecHandle EffectSpecHandle =
		GetWarriorASCFromActorInfo()->MakeOutgoingSpec(EffectClass, GetAbilityLevel(), EffectContextHandle);

	EffectSpecHandle.Data->SetSetByCallerMagnitude(
		WarriorGameplayTags::Shared_SetByCaller_BaseDamage,
		InDamageScalableFloat.GetValueAtLevel(GetAbilityLevel()));
	UWarriorFunctionLibrary::SetAttackImpactDataToEffectSpecHandle(EffectSpecHandle, AttackImpactData);

	return EffectSpecHandle;
}

void UWarriorEnemyGameplayAbility::SetCurrentAttackImpactData(const FWarriorAttackImpactData& AttackImpactData)
{
	if (UEnemyCombatComponent* EnemyCombatComponent = GetEnemyCombatComponent())
	{
		EnemyCombatComponent->SetCurrentAttackImpactData(AttackImpactData);
	}
}

void UWarriorEnemyGameplayAbility::ResetCurrentAttackImpactData()
{
	if (UEnemyCombatComponent* EnemyCombatComponent = GetEnemyCombatComponent())
	{
		EnemyCombatComponent->ResetCurrentAttackImpactData();
	}
}

int32 UWarriorEnemyGameplayAbility::ApplyEnemyDamageEffectSpecHandleToPawnsInRadius(
	const FGameplayEffectSpecHandle& InEffectSpecHandle,
	FVector Origin,
	float Radius,
	bool bCanBeBlocked,
	float BlockCost,
	bool bDrawDebug)
{
	FWarriorAttackImpactData AttackImpactData =
		UWarriorFunctionLibrary::GetAttackImpactDataFromEffectSpecHandle(
			InEffectSpecHandle,
			DefaultAttackImpactData);
	AttackImpactData.bCanBeBlocked = bCanBeBlocked;
	AttackImpactData.BlockCost = BlockCost;

	FGameplayEffectSpecHandle EffectSpecHandle = InEffectSpecHandle;
	UWarriorFunctionLibrary::SetAttackImpactDataToEffectSpecHandle(EffectSpecHandle, AttackImpactData);

	return ApplyEnemyDamageEffectSpecHandleToPawnsInRadiusInternal(
		EffectSpecHandle,
		Origin,
		Radius,
		AttackImpactData,
		bDrawDebug);
}

int32 UWarriorEnemyGameplayAbility::ApplyEnemyDamageEffectSpecHandleToPawnsInRadiusWithAttackImpactData(
	const FGameplayEffectSpecHandle& InEffectSpecHandle,
	FVector Origin,
	float Radius,
	const FWarriorAttackImpactData& AttackImpactData,
	bool bDrawDebug)
{
	FGameplayEffectSpecHandle EffectSpecHandle = InEffectSpecHandle;
	UWarriorFunctionLibrary::SetAttackImpactDataToEffectSpecHandle(EffectSpecHandle, AttackImpactData);

	return ApplyEnemyDamageEffectSpecHandleToPawnsInRadiusInternal(
		EffectSpecHandle,
		Origin,
		Radius,
		AttackImpactData,
		bDrawDebug);
}

int32 UWarriorEnemyGameplayAbility::ApplyEnemyDamageEffectSpecHandleToPawnsInRadiusInternal(
	const FGameplayEffectSpecHandle& InEffectSpecHandle,
	FVector Origin,
	float Radius,
	const FWarriorAttackImpactData& AttackImpactData,
	bool bDrawDebug)
{
	if (!InEffectSpecHandle.IsValid() || Radius <= 0.f)
	{
		return 0;
	}

	UWorld* World = GetWorld();
	APawn* OwningPawn = Cast<APawn>(GetAvatarActorFromActorInfo());
	if (!World || !OwningPawn)
	{
		return 0;
	}

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(EnemyDamageRadius), false, OwningPawn);
	QueryParams.AddIgnoredActor(OwningPawn);

	TArray<FOverlapResult> OverlapResults;
	World->OverlapMultiByObjectType(
		OverlapResults,
		Origin,
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(Radius),
		QueryParams);

	if (bDrawDebug)
	{
		DrawDebugSphere(World, Origin, Radius, 32, FColor::Red, false, 2.f);
	}

	int32 AppliedCount = 0;
	TSet<AActor*> ProcessedActors;

	for (const FOverlapResult& OverlapResult : OverlapResults)
	{
		APawn* TargetPawn = Cast<APawn>(OverlapResult.GetActor());
		if (!TargetPawn ||
			ProcessedActors.Contains(TargetPawn) ||
			!UWarriorFunctionLibrary::IsTargetPawnHostile(OwningPawn, TargetPawn))
		{
			continue;
		}

		ProcessedActors.Add(TargetPawn);

		FGameplayEventData EventData;
		EventData.Instigator = OwningPawn;
		EventData.Target = TargetPawn;
		EventData.EventMagnitude = AttackImpactData.BlockCost;
		UWarriorFunctionLibrary::AddAttackImpactDataToGameplayEventData(EventData, AttackImpactData);

		if (AttackImpactData.bCanBeDodged && UWarriorFunctionLibrary::IsActorInDodgeIFrame(TargetPawn))
		{
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
				TargetPawn,
				WarriorGameplayTags::Player_Event_SuccessDodge,
				EventData);
			continue;
		}

		const bool bIsTargetBlocking =
			UWarriorFunctionLibrary::NativeDoesActorHaveTag(TargetPawn, WarriorGameplayTags::Player_Status_Blocking);
		const bool bWasBlocked =
			AttackImpactData.bCanBeBlocked &&
			bIsTargetBlocking &&
			UWarriorFunctionLibrary::IsValidBlock(OwningPawn, TargetPawn);

		if (bWasBlocked)
		{
			UWarriorFunctionLibrary::HandleSuccessfulBlock(
				TargetPawn,
				OwningPawn,
				AttackImpactData.BlockCost,
				EventData);
			continue;
		}

		if (!UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetPawn))
		{
			continue;
		}

		const FActiveGameplayEffectHandle EffectHandle =
			NativeApplyEffectSpecHandleToTarget(TargetPawn, InEffectSpecHandle);

		if (EffectHandle.WasSuccessfullyApplied())
		{
			++AppliedCount;

			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
				TargetPawn,
				WarriorGameplayTags::Shared_Event_HitReact,
				EventData);
		}
	}

	return AppliedCount;
}
