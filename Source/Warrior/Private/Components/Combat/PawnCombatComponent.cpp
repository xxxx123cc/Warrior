// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Combat/PawnCombatComponent.h"
#include "Items/Weapons/WarriorWeaponBase.h"
#include "WarriorDebugHelper.h"
void UPawnCombatComponent::RegisterSpawnedWeapon(FGameplayTag InWeaponTagToRegister,
	AWarriorWeaponBase* InWeaponToRegister, bool bRegisterEquippedWeapon)
{//检查武器标签不存在于Map里，检查要生成的武器是否有效-4.1
	checkf(!CarriedWeaponsMap.Contains((InWeaponTagToRegister)),TEXT("a named %s has already as carried weapon"),*InWeaponTagToRegister.ToString())
	check(InWeaponToRegister);
	
	//将武器标签添加进Map里面
	CarriedWeaponsMap.Emplace(InWeaponTagToRegister,InWeaponToRegister);
	//如果是已装备武器，设置当前武器标签为这个标签-4.1
	if (bRegisterEquippedWeapon)
	{
		CurrentEquippedWeaponTag= InWeaponTagToRegister;
		
	}
	
}

AWarriorWeaponBase* UPawnCombatComponent::GetCarriedWeaponByTag(FGameplayTag InWeaponTagToGet) const
{//检查当前武器是否被携带-4.1
	if (CarriedWeaponsMap.Contains(InWeaponTagToGet))
	{
		return CarriedWeaponsMap[InWeaponTagToGet];
	}
	
		return nullptr;
}

AWarriorWeaponBase* UPawnCombatComponent::GetCurrentEquippedWeaponByTag() const
{//是否有已装备武器-4.1
	if (!CurrentEquippedWeaponTag.IsValid())
	{
		return nullptr;
	}
	return GetCarriedWeaponByTag(CurrentEquippedWeaponTag);
	
}
