// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/WarriorEnemyGameplayAbility.h"
#include "WarriorTypes/WarriorStructTypes.h"
#include "EnemyGameplayAbility_TripleSwordQi.generated.h"

/**
 * Enemy ability that applies three sword-qi collision boxes in a small forward spread.
 */
UCLASS()
class WARRIOR_API UEnemyGameplayAbility_TripleSwordQi : public UWarriorEnemyGameplayAbility
{
	GENERATED_BODY()

public:
	UEnemyGameplayAbility_TripleSwordQi(const FObjectInitializer& ObjectInitializer);

	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="TripleSwordQi")
	FWarriorTripleSwordQiData SwordQiData;

	UFUNCTION(BlueprintImplementableEvent, Category="TripleSwordQi")
	void BP_OnTripleSwordQiStarted(int32 SpawnedCollisionCount);
};
