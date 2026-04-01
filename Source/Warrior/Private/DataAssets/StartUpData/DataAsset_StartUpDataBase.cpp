// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAssets/StartUpData/DataAsset_StartUpDataBase.h"
#include  "Warrior/Public/AbilitySystem/Abilities/WarriorGameplayAbility.h"
#include "Warrior/Public/AbilitySystem/WarriorAbilitySystemComponent.h"
void UDataAsset_StartUpDataBase::GivenToAbilitySystemComponent(UWarriorAbilitySystemComponent* InAscToGive,
	int32 ApplyLevel)
{
	check(InAscToGive);
	GrantAbilities(ActivateOnGivenAbilities, InAscToGive, ApplyLevel);
	GrantAbilities(ReactiveAbilities, InAscToGive, ApplyLevel);
	
}

void  UDataAsset_StartUpDataBase::GrantAbilities(TArray<TSubclassOf<UWarriorGameplayAbility>>& InAbilitiesToGive,
	UWarriorAbilitySystemComponent* InAscToGive, int32 ApplyLevel)
{
	if (InAbilitiesToGive.IsEmpty())
	{
		return;
	}
	for (const TSubclassOf<UWarriorGameplayAbility>& Ability : InAbilitiesToGive)
	{
		if (!Ability)
			continue;
		FGameplayAbilitySpec AbilitySpec(Ability);
		AbilitySpec.SourceObject = InAscToGive->GetAvatarActor();
		AbilitySpec.Level = ApplyLevel;
		
		InAscToGive->GiveAbility(AbilitySpec);
	      
	
	}
}
