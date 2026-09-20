// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/WarriorAbilitySystemComponent.h"

#include "AbilitySystem/Abilities/WarriorGameplayAbility.h"
#include "AbilitySystem/Abilities/WarriorHeroGameplayAbility.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "WarriorGameplayTags.h"

void UWarriorAbilitySystemComponent::OnAbilityInputPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	const TArray<FGameplayTag> InputTagPriority = ResolveAbilityInputTagPriority(InputTag);
	for (const FGameplayTag& ResolvedInputTag : InputTagPriority)
	{
		if (TryHandleAbilityInput(ResolvedInputTag))
		{
			return;
		}
	}
}

TArray<FGameplayTag> UWarriorAbilitySystemComponent::ResolveAbilityInputTagPriority(const FGameplayTag& InputTag) const
{
	TArray<FGameplayTag> InputTagPriority;
	if (!InputTag.IsValid() || IsInputBlockedByAttackState(InputTag))
	{
		return InputTagPriority;
	}

	const bool bIsLightAttack = InputTag.MatchesTagExact(WarriorGameplayTags::InputTag_LightAttack_Axe);
	const bool bIsHeavyAttack = InputTag.MatchesTagExact(WarriorGameplayTags::InputTag_HeavyAttack_Axe);

	if (!bIsLightAttack && !bIsHeavyAttack)
	{
		InputTagPriority.Add(InputTag);
		return InputTagPriority;
	}

	const bool bIsAirborne = IsAvatarAirborne();
	const bool bIsInRage = IsRageActive();

	if (bIsInRage)
	{
		if (bIsAirborne)
		{
			if (bIsLightAttack)
			{
				InputTagPriority.AddUnique(WarriorGameplayTags::InputTag_LightAttack_Axe_Rage_Air);
			}
		}
		else
		{
			InputTagPriority.AddUnique(bIsLightAttack
				? WarriorGameplayTags::InputTag_LightAttack_Axe_Rage_Ground
				: WarriorGameplayTags::InputTag_HeavyAttack_Axe_Rage_Ground);
		}
	}
	else if (bIsAirborne)
	{
		InputTagPriority.AddUnique(WarriorGameplayTags::InputTag_HeavyAttack_Axe_Air);
	}

	InputTagPriority.AddUnique(InputTag);
	return InputTagPriority;
}

bool UWarriorAbilitySystemComponent::IsAvatarAirborne() const
{
	const ACharacter* AvatarCharacter = Cast<ACharacter>(GetAvatarActor());
	if (!AvatarCharacter)
	{
		return false;
	}

	const UCharacterMovementComponent* MovementComponent = AvatarCharacter->GetCharacterMovement();
	return MovementComponent && MovementComponent->IsFalling();
}

bool UWarriorAbilitySystemComponent::IsRageActive() const
{
	return HasMatchingGameplayTag(WarriorGameplayTags::Player_Status_Rage_Active);
}

bool UWarriorAbilitySystemComponent::IsInputBlockedByAttackState(const FGameplayTag& InputTag) const
{
	const bool bIsHeavyAttack = InputTag.MatchesTagExact(WarriorGameplayTags::InputTag_HeavyAttack_Axe);
	return bIsHeavyAttack && IsRageActive() && IsAvatarAirborne();
}

bool UWarriorAbilitySystemComponent::TryHandleAbilityInput(const FGameplayTag& InputTag)
{
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (!AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			continue;
		}

		const bool bIsToggleableInput = InputTag.MatchesTag(WarriorGameplayTags::InputTag_Toggleable);
		const bool bIsRageInput = InputTag.MatchesTagExact(WarriorGameplayTags::InputTag_Toggleable_Rage);
		const bool bIsAbilityActive = AbilitySpec.IsActive();

		if (bIsToggleableInput && bIsAbilityActive)
		{
			if (bIsRageInput && HasMatchingGameplayTag(WarriorGameplayTags::Player_Status_Rage_Activating))
			{
				return true;
			}

			CancelAbilityHandle(AbilitySpec.Handle);
			return true;
		}

		if (bIsAbilityActive)
		{
			AbilitySpecInputPressed(AbilitySpec);

PRAGMA_DISABLE_DEPRECATION_WARNINGS
			TArray<UGameplayAbility*> Instances = AbilitySpec.GetAbilityInstances();
			const FGameplayAbilityActivationInfo& ActivationInfo =
				Instances.IsEmpty() ? AbilitySpec.ActivationInfo : Instances.Last()->GetCurrentActivationInfoRef();
PRAGMA_ENABLE_DEPRECATION_WARNINGS

			InvokeReplicatedEvent(
				EAbilityGenericReplicatedEvent::InputPressed,
				AbilitySpec.Handle,
				ActivationInfo.GetActivationPredictionKey());

			if (UWarriorGameplayAbility* WarriorAbility = Cast<UWarriorGameplayAbility>(AbilitySpec.GetPrimaryInstance()))
			{
				WarriorAbility->OnComboInputPressed();
			}

			return true;
		}

		TryActivateAbility(AbilitySpec.Handle);
		return true;
	}

	return false;
}

void UWarriorAbilitySystemComponent::OnAbilityInputReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid() || !InputTag.MatchesTag(WarriorGameplayTags::InputTag_MustBeHeld))
	{
		return;
	}

	for (FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		if (Spec.GetDynamicSpecSourceTags().HasTagExact(InputTag) && Spec.IsActive())
		{
			AbilitySpecInputReleased(Spec);

PRAGMA_DISABLE_DEPRECATION_WARNINGS
			TArray<UGameplayAbility*> Instances = Spec.GetAbilityInstances();
			const FGameplayAbilityActivationInfo& ActivationInfo =
				Instances.IsEmpty() ? Spec.ActivationInfo : Instances.Last()->GetCurrentActivationInfoRef();
PRAGMA_ENABLE_DEPRECATION_WARNINGS

			InvokeReplicatedEvent(
				EAbilityGenericReplicatedEvent::InputReleased,
				Spec.Handle,
				ActivationInfo.GetActivationPredictionKey());

			UWarriorGameplayAbility* WarriorAbility = Cast<UWarriorGameplayAbility>(Spec.GetPrimaryInstance());
			const bool bShouldCancelOnRelease = !WarriorAbility || WarriorAbility->ShouldCancelAbilityOnInputRelease();
			if (bShouldCancelOnRelease && Spec.IsActive())
			{
				CancelAbilityHandle(Spec.Handle);
			}
		}
	}
}

void UWarriorAbilitySystemComponent::GrantHeroWeaponAbilities(
	const TArray<FWarriorHeroAbilitySets>& InDefaultWeaponAbilities,
	const TArray<FWarriorHeroSpecialAbilitySets> InSpecialAbilities,
	int32 ApplyLevel,
	TArray<FGameplayAbilitySpecHandle>& OutGrantedAbilitySpecHandles)
{
	if (InDefaultWeaponAbilities.IsEmpty())
	{
		return;
	}

	for (const FWarriorHeroAbilitySets& AbilitySet : InDefaultWeaponAbilities)
	{
		if (!AbilitySet.IsValid())
		{
			continue;
		}

		FGameplayAbilitySpec AbilitySpec(AbilitySet.AbilityToGrant);
		AbilitySpec.SourceObject = GetAvatarActor();
		AbilitySpec.Level = ApplyLevel;
		AbilitySpec.GetDynamicSpecSourceTags().AddTag(AbilitySet.InputTag);
		OutGrantedAbilitySpecHandles.AddUnique(GiveAbility(AbilitySpec));
	}

	if (InSpecialAbilities.IsEmpty())
	{
		return;
	}

	for (const FWarriorHeroSpecialAbilitySets& AbilitySet : InSpecialAbilities)
	{
		if (!AbilitySet.IsValid())
		{
			continue;
		}

		FGameplayAbilitySpec AbilitySpec(AbilitySet.AbilityToGrant);
		AbilitySpec.SourceObject = GetAvatarActor();
		AbilitySpec.Level = ApplyLevel;
		AbilitySpec.GetDynamicSpecSourceTags().AddTag(AbilitySet.InputTag);
		OutGrantedAbilitySpecHandles.AddUnique(GiveAbility(AbilitySpec));
	}
}

void UWarriorAbilitySystemComponent::RemoveGrantedHeroAbilities(TArray<FGameplayAbilitySpecHandle>& InSpecHandlesToRemove)
{
	if (InSpecHandlesToRemove.IsEmpty())
	{
		return;
	}

	for (const FGameplayAbilitySpecHandle& SpecHandle : InSpecHandlesToRemove)
	{
		ClearAbility(SpecHandle);
	}

	InSpecHandlesToRemove.Empty();
}

bool UWarriorAbilitySystemComponent::TryActivateAbilityByTag(FGameplayTag AbilityTagToActivate)
{
	check(AbilityTagToActivate.IsValid());

	TArray<FGameplayAbilitySpec*> FoundAbilitySpecs;
	GetActivatableGameplayAbilitySpecsByAllMatchingTags(AbilityTagToActivate.GetSingleTagContainer(), FoundAbilitySpecs);

	if (!FoundAbilitySpecs.IsEmpty())
	{
		const int32 RandomAbilityIndex = FMath::RandRange(0, FoundAbilitySpecs.Num() - 1);
		FGameplayAbilitySpec* SpecToActivate = FoundAbilitySpecs[RandomAbilityIndex];

		if (!SpecToActivate->IsActive())
		{
			return TryActivateAbility(SpecToActivate->Handle);
		}
	}

	return false;
}
