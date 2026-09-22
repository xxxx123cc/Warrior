// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Combat/PawnCombatComponent.h"
#include "EnemyCombatComponent.generated.h"

/**
 * 
 */
UCLASS()
class WARRIOR_API UEnemyCombatComponent : public UPawnCombatComponent
{
	GENERATED_BODY()
public:
	virtual void OnHitTargetActor(AActor* HitActor) override;
	
	virtual void OnWeaponEndOverlapTarget (AActor* EndOverlapActor) override;
	
	virtual void ToggleCurrentHandCollision(bool bEnableCollision,EToggleDamageType ToggleDamageType=EToggleDamageType::CurrentEquippedWeapon) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Warrior|Combat|Block", meta=(ClampMin="0.0"))
	float SuccessfulBlockCost = 1.f;
	
};
