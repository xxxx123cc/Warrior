// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include"WarriorDebugHelper.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Abilities/WarriorHeroGameplayAbility.h"


void UWarriorAbilitySystemComponent::OnAbilityInputPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
		return;
    
	for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			// 检查Ability是否正在运行
			if (AbilitySpec.IsActive())
			{
				// 正在运行 → 通知Ability有待处理的连击输入
				if (UWarriorGameplayAbility* WarriorAbility = Cast<UWarriorGameplayAbility>(AbilitySpec.GetPrimaryInstance()))
				{
					WarriorAbility->OnComboInputPressed();
				}
			}
			else
			{
				// 没在运行 → 正常激活
				TryActivateAbility(AbilitySpec.Handle);
			}
		}
	}
}

void UWarriorAbilitySystemComponent::OnAbilityInputReleased(const FGameplayTag& InputTag)
{
}

void UWarriorAbilitySystemComponent::GrantHeroWeaponAbilities(
	const TArray<FWarriorHeroAbilitySets>& InDefaultWeaponAbilities, int32 ApplyLevel,TArray<FGameplayAbilitySpecHandle>& OutGrantedAbilitySpecHandles)
{
	if (InDefaultWeaponAbilities.IsEmpty())
		return;
	for (const FWarriorHeroAbilitySets& AbilitySet : InDefaultWeaponAbilities)
	{
		if (!AbilitySet.IsValid())
			continue;
		FGameplayAbilitySpec AbilitySpec(AbilitySet.AbilityToGrant);
		AbilitySpec.SourceObject = GetAvatarActor();
		AbilitySpec.Level = ApplyLevel;
		// 将 InputTag 添加到技能的特有源标签中，以便后续通过标签匹配
		AbilitySpec.GetDynamicSpecSourceTags().AddTag(AbilitySet.InputTag);
		OutGrantedAbilitySpecHandles.AddUnique(GiveAbility(AbilitySpec));

	}
}

void UWarriorAbilitySystemComponent::RemoveGrantedHeroAbilities(
	TArray<FGameplayAbilitySpecHandle>& InSpecHandlesToRemove)
{if (InSpecHandlesToRemove.IsEmpty())
	return;
	for (const FGameplayAbilitySpecHandle& SpecHandle : InSpecHandlesToRemove)
	{
		ClearAbility(SpecHandle);
	}
	InSpecHandlesToRemove.Empty();
}

bool UWarriorAbilitySystemComponent::TryActivateAbilityByTag(FGameplayTag AbilityTagToActivate)
{
	 check(AbilityTagToActivate.IsValid());
	// for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	// {		if (AbilitySpec.Ability->AbilityTags.HasTagExact(AbilityTagToActivate))
	// 	{			return TryActivateAbility(AbilitySpec.Handle);
	// 	}	}
	// return false;
	TArray<FGameplayAbilitySpec*> FoundAbilitySpecs;
	GetActivatableGameplayAbilitySpecsByAllMatchingTags(AbilityTagToActivate.GetSingleTagContainer(), FoundAbilitySpecs);
	
	if (!FoundAbilitySpecs.IsEmpty())
	{
		const int32 RandomAbilityIndex=FMath::RandRange(0, FoundAbilitySpecs.Num() - 1);
		FGameplayAbilitySpec *SpecToActivate = FoundAbilitySpecs[RandomAbilityIndex];
		
		if (!SpecToActivate->IsActive())
		{
			return TryActivateAbility(SpecToActivate->Handle);
		}
	
	}
		return false;
	
}

