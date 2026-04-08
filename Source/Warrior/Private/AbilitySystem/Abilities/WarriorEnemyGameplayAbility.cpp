// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/WarriorEnemyGameplayAbility.h"
#include "Characters/WarriorEnemyCharacter.h"

AWarriorEnemyCharacter* UWarriorEnemyGameplayAbility::GetEnemyCharacterFromActorInfor()
{
	if (!CachedEnemyCharacter.IsValid())
	CachedEnemyCharacter = Cast<AWarriorEnemyCharacter>(GetAvatarActorFromActorInfo());
	return CachedEnemyCharacter.IsValid() ? CachedEnemyCharacter.Get() : nullptr;
	
}

UEnemyCombatComponent* UWarriorEnemyGameplayAbility::GetEnemyCombatComponent()
{
	return GetEnemyCharacterFromActorInfor()->GetEnemyCombatComponent();
}
