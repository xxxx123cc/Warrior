// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/UI/PawnUIComponent.h"
#include  "WarriorGameplayTags.h"
#include "WarriorTypes/WarriorTeamTypes.h"
#include "HeroUIComponent.generated.h"

class UTexture2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponTextureChangedDelegate, TSoftObjectPtr<UTexture2D>, NewWeaponTexture);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityIconSlotUpdate, FGameplayTag,AbilityTag, TSoftObjectPtr<UMaterialInterface>, AbilityIconMetarial);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FonAbilityCooldownUpdate, FGameplayTag, AbilityInputTag, float, CooldownDuration, float, CooldownRemaining);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStoneInteractedDelegate, bool,bShouldShowInteraction);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTeamSwitchedDelegate, int32, NewSlotIndex);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBenchHealthChangedDelegate, int32, SlotIndex, float, HealthPercent);

/**

 */
UCLASS()
class WARRIOR_API UHeroUIComponent : public UPawnUIComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category="UI")
	FOnPercentChangedDelegate OnCurrentRageChanged;

	UPROPERTY(BlueprintAssignable, Category="UI")
	FOnPercentChangedDelegate OnCurrentBlockValueChanged;

	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category="UI")
	FOnAbilityIconSlotUpdate OnAbilityIconSlotUpdate;

	UPROPERTY(BlueprintAssignable,BlueprintCallable, Category="UI")
	FOnWeaponTextureChangedDelegate OnWeaponTextureChanged;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="UI")
	FonAbilityCooldownUpdate OnAbilityCooldownUpdate;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="UI")
	FOnStoneInteractedDelegate OnStoneInteracted;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "UI")
	FOnTeamSwitchedDelegate OnTeamSwitched;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "UI")
	FOnBenchHealthChangedDelegate OnBenchHealthChanged;

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	TArray<FWarriorTeamMemberState> CachedBenchStates;
};
