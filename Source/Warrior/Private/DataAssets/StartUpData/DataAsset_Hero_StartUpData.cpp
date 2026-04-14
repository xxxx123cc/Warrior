// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAssets/StartUpData/DataAsset_Hero_StartUpData.h"
#include "AbilitySystem/Abilities/WarriorHeroGameplayAbility.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "WarriorTypes/WarriorStructTypes.h"

void UDataAsset_Hero_StartUpData::GivenToAbilitySystemComponent(UWarriorAbilitySystemComponent* InAscToGive,
	int32 ApplyLevel)
{
	Super::GivenToAbilitySystemComponent(InAscToGive, ApplyLevel);
	
	for (const FWarriorHeroAbilitySets& AbilitySet : HeroStartUpAbilitySets)
	{
		if (!AbilitySet.IsValid())
			continue;
		FGameplayAbilitySpec AbilitySpec(AbilitySet.AbilityToGrant);
		AbilitySpec.SourceObject = InAscToGive->GetAvatarActor();
		AbilitySpec.Level = ApplyLevel;
		// 将 InputTag 添加到技能的特有源标签中，以便后续通过标签匹配
		AbilitySpec.GetDynamicSpecSourceTags().AddTag(AbilitySet.InputTag);
		
		InAscToGive->GiveAbility(AbilitySpec);
		
	}
}
