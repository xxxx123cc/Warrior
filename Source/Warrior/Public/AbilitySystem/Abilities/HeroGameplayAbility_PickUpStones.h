// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/WarriorHeroGameplayAbility.h"
#include "HeroGameplayAbility_PickUpStones.generated.h"

class AWarriorStoneBase;
/**
 * 
 */
UCLASS()
class WARRIOR_API UHeroGameplayAbility_PickUpStones : public UWarriorHeroGameplayAbility
{
	GENERATED_BODY()
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	UFUNCTION(BlueprintCallable,Category="Ability|PickUpStones")
	void PickUpStones();
	
	UFUNCTION(BlueprintCallable)
	void ConsumeStones();
	
private:
	UPROPERTY(EditDefaultsOnly,Category="Ability|PickUpStones")
	float BoxTraceDistance;

	UPROPERTY(EditDefaultsOnly,Category="Ability|PickUpStones")
	FVector BoxTraceSize = FVector(100.f,100.f,100.f);
	
	UPROPERTY(EditDefaultsOnly,Category="Ability|PickUpStones")
	TArray<TEnumAsByte<EObjectTypeQuery>> StonesToPickUp;
	
	UPROPERTY(EditDefaultsOnly,Category="Ability|PickUpStones")
	bool bDrawDebugShape=false;
	
	UPROPERTY()
	TArray<AWarriorStoneBase*> PickedUpStones;
	
};
