// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Combat/PawnCombatComponent.h"
#include "ScalableFloat.h"
#include "HeroCombatComponent.generated.h"

class AWarriorHeroWeapon;
/**
 * 
 */
UCLASS()
class WARRIOR_API UHeroCombatComponent : public UPawnCombatComponent
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable,Category="Warrior|Combat")
	AWarriorHeroWeapon* GetHeroCarriedWeaponByTag(FGameplayTag InWeaponTagToGet) const;
	UFUNCTION(BlueprintCallable,Category="Warrior|Combat")
	AWarriorHeroWeapon* GetHeroCurrentEquippedWeapon() const;
	UFUNCTION(BlueprintCallable,Category="Warrior|Combat")
	float GetHeroCurrentEquipWeaponDamageAtLevel(float InLevel)const ;
	//创建委托
	virtual  void OnHitTargetActor(AActor* HitActor) override;
	virtual void OnWeaponEndOverlapTarget (AActor* EndOverlapActor)override ;
	
	
};
