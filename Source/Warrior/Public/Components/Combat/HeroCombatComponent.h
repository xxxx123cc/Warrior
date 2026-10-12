// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Combat/PawnCombatComponent.h"
#include "ScalableFloat.h"
#include "WarriorTypes/WarriorStructTypes.h"
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
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable,Category="Warrior|Combat")
	AWarriorHeroWeapon* GetHeroCarriedWeaponByTag(FGameplayTag InWeaponTagToGet) const;
	UFUNCTION(BlueprintCallable,Category="Warrior|Combat")
	AWarriorHeroWeapon* GetHeroCurrentEquippedWeapon() const;
	UFUNCTION(BlueprintCallable,Category="Warrior|Combat")
	float GetHeroCurrentEquipWeaponDamageAtLevel(float InLevel)const ;
	//创建委托
	virtual  void OnHitTargetActor(AActor* HitActor) override;
	virtual void OnWeaponEndOverlapTarget (AActor* EndOverlapActor)override ;

	virtual void ToggleCurrentWeaponCollision(bool bEnableCollision,EToggleDamageType ToggleDamageType=EToggleDamageType::CurrentEquippedWeapon) override;

	virtual void ToggleCurrentHandCollision(bool bEnableCollision,EToggleDamageType ToggleDamageType=EToggleDamageType::CurrentEquippedWeapon) override;

	UFUNCTION(BlueprintCallable, Category="Warrior|Combat|Attack")
	void SetCurrentAttackImpactData(const FWarriorAttackImpactData& InAttackImpactData);

	UFUNCTION(BlueprintCallable, Category="Warrior|Combat|Attack")
	void ResetCurrentAttackImpactData();

	UFUNCTION(BlueprintPure, Category="Warrior|Combat|Attack")
	FWarriorAttackImpactData GetCurrentAttackImpactData() const { return CurrentAttackImpactData; }

	UFUNCTION(BlueprintPure, Category="Warrior|Combat|Attack")
	bool HasCurrentAttackImpactDataOverride() const { return bHasCurrentAttackImpactDataOverride; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Warrior|Combat|Attack")
	FWarriorAttackImpactData DefaultAttackImpactData;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Warrior|Combat|Attack")
	FWarriorAttackImpactData CurrentAttackImpactData;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Warrior|Combat|Attack")
	bool bHasCurrentAttackImpactDataOverride = false;

};
