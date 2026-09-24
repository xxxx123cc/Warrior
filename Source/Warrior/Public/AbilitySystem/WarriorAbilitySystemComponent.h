// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "WarriorTypes/WarriorStructTypes.h"
#include "WarriorAbilitySystemComponent.generated.h"

/**
 * 
 */
UCLASS()
class WARRIOR_API UWarriorAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
public:
	UWarriorAbilitySystemComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void OnAbilityInputPressed(const FGameplayTag& InputTag);
	void OnAbilityInputReleased(const FGameplayTag& InputTag);

	UFUNCTION(BlueprintPure, Category="Warroir|Ability|Input")
	TArray<FGameplayTag> ResolveAbilityInputTagPriority(const FGameplayTag& InputTag) const;

	UFUNCTION(BlueprintPure, Category="Warroir|Ability|Input")
	bool IsAvatarAirborne() const;

	UFUNCTION(BlueprintPure, Category="Warroir|Ability|Input")
	bool IsRageActive() const;

	UFUNCTION(BlueprintPure, Category="Warroir|Ability|Input")
	bool IsInputBlockedByAttackState(const FGameplayTag& InputTag) const;
	
	UFUNCTION(BlueprintCallable,Category="Warroir|Ability",meta = (ApplyLevel="1"))
	void GrantHeroWeaponAbilities(const TArray<FWarriorHeroAbilitySets>& InDefaultWeaponAbilities,TArray<FWarriorHeroSpecialAbilitySets>InSpecialAbilities,int32 ApplyLevel,TArray<FGameplayAbilitySpecHandle>& OutGrantedAbilitySpecHandles);

	UFUNCTION(BlueprintCallable,Category="Warroir|Ability")
	void RemoveGrantedHeroAbilities(UPARAM(ref) TArray<FGameplayAbilitySpecHandle>& InSpecHandlesToRemove);

	UFUNCTION(BlueprintCallable,Category="Warroir|Ability")
	bool TryActivateAbilityByTag(FGameplayTag AbilityTagToActivate);

	void NotifyBlockValueConsumed(bool bWasGuardBroken);

	float GetBossPoiseDamageOnHit() const { return BossPoiseDamageOnHit; }
	float GetBossPoiseDamageOnWeaponClash() const { return BossPoiseDamageOnWeaponClash; }
	float GetBossPoiseBreakStunDuration() const { return BossPoiseBreakStunDuration; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Warroir|Block", meta=(ClampMin="0.0"))
	float BlockValueRegenRate = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Warroir|Block", meta=(ClampMin="0.0"))
	float BlockValueRegenDelay = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Warroir|Block", meta=(ClampMin="0.0"))
	float GuardBreakBlockValueRegenDelay = 2.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Warroir|Block")
	bool bRegenerateBlockValueWhileBlocking = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Warroir|BossPoise", meta=(ClampMin="0.0"))
	float BossPoiseDamageOnHit = 5.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Warroir|BossPoise", meta=(ClampMin="0.0"))
	float BossPoiseDamageOnWeaponClash = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Warroir|BossPoise", meta=(ClampMin="0.0"))
	float BossPoiseBreakStunDuration = 3.f;

private:
	bool TryHandleAbilityInput(const FGameplayTag& InputTag);
	void RegenerateBlockValue(float DeltaTime);
	void BroadcastCurrentBlockValue(float CurrentBlockValue, float MaxBlockValue) const;

	float BlockValueRegenBlockedUntilTime = 0.f;
};
