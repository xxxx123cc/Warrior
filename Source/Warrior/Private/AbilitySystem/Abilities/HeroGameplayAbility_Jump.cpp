// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/HeroGameplayAbility_Jump.h"

#include "Characters/WarriorHeroCharacter.h"
#include "WarriorGameplayTags.h"

UHeroGameplayAbility_Jump::UHeroGameplayAbility_Jump(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	FGameplayTagContainer JumpAbilityTags;
	JumpAbilityTags.AddTag(WarriorGameplayTags::Player_Ability_Jump);
	SetAssetTags(JumpAbilityTags);
}

bool UHeroGameplayAbility_Jump::CanActivateAbility(
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

	const AWarriorHeroCharacter* HeroCharacter =
		ActorInfo ? Cast<AWarriorHeroCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;

	return HeroCharacter && HeroCharacter->CanJump();
}

void UHeroGameplayAbility_Jump::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AWarriorHeroCharacter* HeroCharacter =
		ActorInfo ? Cast<AWarriorHeroCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;

	if (!HeroCharacter)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	HeroCharacter->Jump();
}

void UHeroGameplayAbility_Jump::InputReleased(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	CancelAbility(Handle, ActorInfo, ActivationInfo, true);
}

void UHeroGameplayAbility_Jump::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	StopJumping(ActorInfo);

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UHeroGameplayAbility_Jump::StopJumping(const FGameplayAbilityActorInfo* ActorInfo)
{
	AWarriorHeroCharacter* HeroCharacter =
		ActorInfo ? Cast<AWarriorHeroCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;

	if (HeroCharacter)
	{
		HeroCharacter->StopJumping();
	}
}
