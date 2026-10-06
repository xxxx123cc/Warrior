// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DataAsset_InventoryItemDefinition.generated.h"

class UTexture2D;

UCLASS(BlueprintType)
class WARRIOR_API UDataAsset_InventoryItemDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category="Warrior|Inventory")
	FName GetItemID() const;

	UFUNCTION(BlueprintPure, Category="Warrior|Inventory")
	bool IsValidItemDefinition() const;

	UFUNCTION(BlueprintPure, Category="Warrior|Inventory")
	bool CanStack() const;

	UFUNCTION(BlueprintPure, Category="Warrior|Inventory")
	int32 GetMaxStackSize() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item")
	FName ItemID = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item")
	TSoftObjectPtr<UTexture2D> SoftItemIconTexture;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stacking")
	bool bCanStack = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stacking", meta=(EditCondition="bCanStack", ClampMin="1"))
	int32 MaxStackSize = 99;
};
