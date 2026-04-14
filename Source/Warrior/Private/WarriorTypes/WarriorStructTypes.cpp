// Fill out your copyright notice in the Description page of Project Settings.


#include "Warrior/Public/WarriorTypes/WarriorStructTypes.h"
#include"Warrior/Public/AbilitySystem/Abilities/WarriorHeroGameplayAbility.h"

bool FWarriorHeroAbilitySets::IsValid() const
{
	return InputTag.IsValid() && AbilityToGrant;
}
