// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/EnemyGameplayAbility_MovingRectangleDamage.h"

#include "WarriorGameplayTags.h"

UEnemyGameplayAbility_MovingRectangleDamage::UEnemyGameplayAbility_MovingRectangleDamage(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	bCancelAbilityOnInputRelease = false;

	FGameplayTagContainer MovingRectangleDamageTags;
	MovingRectangleDamageTags.AddTag(WarriorGameplayTags::Enemy_Ability_MovingRectangleDamage);
	SetAssetTags(MovingRectangleDamageTags);
}

bool UEnemyGameplayAbility_MovingRectangleDamage::CanActivateAbility(
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

	return DamageAreaClass && DamageEffectClass;
}

void UEnemyGameplayAbility_MovingRectangleDamage::ActivateAbility(
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

	if (!DamageAreaClass || !DamageEffectClass)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (bSpawnDamageAreaOnActivate)
	{
		AWarriorMovingRectangleDamageArea* DamageArea = SpawnConfiguredMovingRectangleDamageArea();
		BP_OnDamageAreaSpawned(DamageArea);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, DamageArea == nullptr);
		return;
	}

	BP_OnMovingRectangleDamageAbilityActivated();
}

AWarriorMovingRectangleDamageArea*
UEnemyGameplayAbility_MovingRectangleDamage::SpawnConfiguredMovingRectangleDamageArea()
{
	return SpawnMovingRectangleDamageArea(
		DamageAreaClass,
		DamageEffectClass,
		DamageScalableFloat,
		AttackImpactData,
		DamageAreaConfig,
		SpawnForwardOffset,
		SpawnHeightOffset);
}
