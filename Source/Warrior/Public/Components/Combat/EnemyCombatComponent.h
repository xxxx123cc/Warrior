// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Combat/PawnCombatComponent.h"
#include "WarriorTypes/WarriorStructTypes.h"
#include "EnemyCombatComponent.generated.h"

/**
 * 
 */
UCLASS()
class WARRIOR_API UEnemyCombatComponent : public UPawnCombatComponent
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;

	virtual void OnHitTargetActor(AActor* HitActor) override;
	
	virtual void OnWeaponEndOverlapTarget (AActor* EndOverlapActor) override;
	
	virtual void ToggleCurrentWeaponCollision(bool bEnableCollision,EToggleDamageType ToggleDamageType=EToggleDamageType::CurrentEquippedWeapon) override;

	virtual void ToggleCurrentHandCollision(bool bEnableCollision,EToggleDamageType ToggleDamageType=EToggleDamageType::CurrentEquippedWeapon) override;

	UFUNCTION(BlueprintCallable, Category="Warrior|Combat|Attack")
	void SetCurrentAttackImpactData(const FWarriorAttackImpactData& InAttackImpactData);

	UFUNCTION(BlueprintCallable, Category="Warrior|Combat|Attack")
	void ResetCurrentAttackImpactData();

	UFUNCTION(BlueprintPure, Category="Warrior|Combat|Attack")
	FWarriorAttackImpactData GetCurrentAttackImpactData() const { return CurrentAttackImpactData; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Warrior|Combat|Block", meta=(ClampMin="0.0"))
	float SuccessfulBlockCost = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Warrior|Combat|Attack")
	FWarriorAttackImpactData DefaultAttackImpactData;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Warrior|Combat|Attack")
	FWarriorAttackImpactData CurrentAttackImpactData;
	
};
