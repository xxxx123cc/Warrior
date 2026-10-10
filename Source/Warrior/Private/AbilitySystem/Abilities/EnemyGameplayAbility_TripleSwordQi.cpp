// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/EnemyGameplayAbility_TripleSwordQi.h"

#include "GameplayEffect.h"
#include "WarriorGameplayTags.h"

UEnemyGameplayAbility_TripleSwordQi::UEnemyGameplayAbility_TripleSwordQi(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	bCancelAbilityOnInputRelease = false;

	FGameplayTagContainer TripleSwordQiTags;
	TripleSwordQiTags.AddTag(WarriorGameplayTags::Enemy_Ability_Ranged);
	TripleSwordQiTags.AddTag(WarriorGameplayTags::Enemy_Ability_TripleSwordQi);
	SetAssetTags(TripleSwordQiTags);
}

bool UEnemyGameplayAbility_TripleSwordQi::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	return SwordQiData.DamageEffectClass &&
		FMath::Abs(SwordQiData.CollisionBoxSize.X) > 0.f &&
		FMath::Abs(SwordQiData.CollisionBoxSize.Y) > 0.f &&
		FMath::Abs(SwordQiData.CollisionBoxSize.Z) > 0.f;
}

void UEnemyGameplayAbility_TripleSwordQi::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		return;
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!SwordQiData.DamageEffectClass)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const int32 SpawnedCollisionCount = FireTripleSwordQiDamage(SwordQiData);
	BP_OnTripleSwordQiStarted(SpawnedCollisionCount);

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
