// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/WarriorGameplayAbility.h"
#include "WarriorTypes/WarriorStructTypes.h"
#include "WarriorEnemyGameplayAbility.generated.h"

class AWarriorEnemyCharacter;
class UEnemyCombatComponent;
/**
 * 
 */
UCLASS()
class WARRIOR_API UWarriorEnemyGameplayAbility : public UWarriorGameplayAbility
{
	GENERATED_BODY()
public:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

	UFUNCTION(BlueprintPure,Category="Warrior|Ability")
	AWarriorEnemyCharacter* GetEnemyCharacterFromActorInfor(); 
	UFUNCTION(BlueprintPure,Category="Warrior|Ability")
	UEnemyCombatComponent* GetEnemyCombatComponent();
	UFUNCTION(BlueprintPure,Category="Warrior|Ability")
	FGameplayEffectSpecHandle EnemyDamageEffectHandle(TSubclassOf<UGameplayEffect>EffectClass,const FScalableFloat& InDamageScalableFloat);

	UFUNCTION(BlueprintPure,Category="Warrior|Ability")
	FGameplayEffectSpecHandle EnemyDamageEffectHandleWithAttackImpactData(
		TSubclassOf<UGameplayEffect> EffectClass,
		const FScalableFloat& InDamageScalableFloat,
		const FWarriorAttackImpactData& AttackImpactData);

	UFUNCTION(BlueprintCallable, Category="Warrior|Ability|Attack")
	void SetCurrentAttackImpactData(const FWarriorAttackImpactData& AttackImpactData);

	UFUNCTION(BlueprintCallable, Category="Warrior|Ability|Attack")
	void ResetCurrentAttackImpactData();

	UFUNCTION(BlueprintCallable, Category="Warrior|Ability|AOE", meta=(ClampMin="0.0", DisplayName="Apply Enemy Damage In Radius"))
	int32 ApplyEnemyDamageEffectSpecHandleToPawnsInRadius(
		const FGameplayEffectSpecHandle& InEffectSpecHandle,
		FVector Origin,
		float Radius,
		bool bCanBeBlocked = true,
		float BlockCost = 1.f,
		bool bDrawDebug = false);

	UFUNCTION(BlueprintCallable, Category="Warrior|Ability|AOE", meta=(ClampMin="0.0", DisplayName="Apply Enemy Damage In Radius With Attack Impact"))
	int32 ApplyEnemyDamageEffectSpecHandleToPawnsInRadiusWithAttackImpactData(
		const FGameplayEffectSpecHandle& InEffectSpecHandle,
		FVector Origin,
		float Radius,
		const FWarriorAttackImpactData& AttackImpactData,
		bool bDrawDebug = false);
	
	
	
	TWeakObjectPtr<AWarriorEnemyCharacter> CachedEnemyCharacter;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Warrior|Attack")
	FWarriorAttackImpactData DefaultAttackImpactData;
	
private:
	int32 ApplyEnemyDamageEffectSpecHandleToPawnsInRadiusInternal(
		const FGameplayEffectSpecHandle& InEffectSpecHandle,
		FVector Origin,
		float Radius,
		const FWarriorAttackImpactData& AttackImpactData,
		bool bDrawDebug);
	
};
