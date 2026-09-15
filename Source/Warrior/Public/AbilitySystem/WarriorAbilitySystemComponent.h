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

private:
	bool TryHandleAbilityInput(const FGameplayTag& InputTag);
};
