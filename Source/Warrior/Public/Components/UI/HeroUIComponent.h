// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/UI/PawnUIComponent.h"
#include  "WarriorGameplayTags.h"
#include "HeroUIComponent.generated.h"

class UTexture2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponTextureChangedDelegate, TSoftObjectPtr<UTexture2D>, NewWeaponTexture);


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityIconSlotUpdate, FGameplayTag,AbilityTag, TSoftObjectPtr<UMaterialInterface>, AbilityIconMetarial);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FonAbilityCooldownUpdate, FGameplayTag, AbilityInputTag, float, CooldownDuration, float, CooldownRemaining);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStoneInteractedDelegate, bool,bShouldShowInteraction);

/**
 * 
 */
UCLASS()
class WARRIOR_API UHeroUIComponent : public UPawnUIComponent
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable, Category="UI")
	FOnPercentChangedDelegate OnCurrentRageChanged;

	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category="UI")
	FOnAbilityIconSlotUpdate OnAbilityIconSlotUpdate;
	
	UPROPERTY(BlueprintAssignable,BlueprintCallable, Category="UI")
	FOnWeaponTextureChangedDelegate OnWeaponTextureChanged;
	
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="UI")
	FonAbilityCooldownUpdate OnAbilityCooldownUpdate;
	
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="UI")
	FOnStoneInteractedDelegate OnStoneInteracted;
};
