// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include"WarriorDebugHelper.h"
void UWarriorAbilitySystemComponent::OnAbilityInputPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
		return;
	for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		TryActivateAbility(AbilitySpec.Handle);
	}

}

void UWarriorAbilitySystemComponent::OnAbilityInputReleased(const FGameplayTag& InputTag)
{
}
