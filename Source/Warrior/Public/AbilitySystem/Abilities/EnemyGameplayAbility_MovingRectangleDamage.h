// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/WarriorEnemyGameplayAbility.h"
#include "Items/WarriorMovingRectangleDamageArea.h"
#include "EnemyGameplayAbility_MovingRectangleDamage.generated.h"

class UGameplayEffect;

UCLASS()
class WARRIOR_API UEnemyGameplayAbility_MovingRectangleDamage : public UWarriorEnemyGameplayAbility
{
	GENERATED_BODY()

public:
	UEnemyGameplayAbility_MovingRectangleDamage(const FObjectInitializer& ObjectInitializer);

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

	UFUNCTION(BlueprintCallable, Category="MovingRectangleDamage")
	AWarriorMovingRectangleDamageArea* SpawnConfiguredMovingRectangleDamageArea();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MovingRectangleDamage|Area")
	TSubclassOf<AWarriorMovingRectangleDamageArea> DamageAreaClass = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MovingRectangleDamage|Area")
	FWarriorMovingRectangleDamageAreaConfig DamageAreaConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MovingRectangleDamage|Area")
	bool bSpawnDamageAreaOnActivate = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MovingRectangleDamage|Spawn")
	float SpawnForwardOffset = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MovingRectangleDamage|Spawn")
	float SpawnHeightOffset = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MovingRectangleDamage|Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MovingRectangleDamage|Damage")
	FScalableFloat DamageScalableFloat;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MovingRectangleDamage|Damage")
	FWarriorAttackImpactData AttackImpactData;

	UFUNCTION(BlueprintImplementableEvent, Category="MovingRectangleDamage")
	void BP_OnDamageAreaSpawned(AWarriorMovingRectangleDamageArea* DamageArea);

	UFUNCTION(BlueprintImplementableEvent, Category="MovingRectangleDamage")
	void BP_OnMovingRectangleDamageAbilityActivated();
};
