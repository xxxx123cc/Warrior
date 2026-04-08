// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAssets/StartUpData/DataAsset_EnemyStartUpData.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/WarriorEnemyGameplayAbility.h"
void UDataAsset_EnemyStartUpData::GivenToAbilitySystemComponent(UWarriorAbilitySystemComponent* InAscToGive,
	int32 ApplyLevel)
{
	Super::GivenToAbilitySystemComponent(InAscToGive, ApplyLevel);
	
	if (EnemyCombatAbilities.Num() == 0)
		return;
	
	for (const TSubclassOf<UWarriorEnemyGameplayAbility>& EnemyAbility : EnemyCombatAbilities)
	{
		if (!EnemyAbility)
			continue;
		FGameplayAbilitySpec EnemyAbilitySpec(EnemyAbility);
		EnemyAbilitySpec.SourceObject = InAscToGive->GetAvatarActor();
		EnemyAbilitySpec.Level = ApplyLevel;
		
		
		InAscToGive->GiveAbility(EnemyAbilitySpec);
	}
	
}
