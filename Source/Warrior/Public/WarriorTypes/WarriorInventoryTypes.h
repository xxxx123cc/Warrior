// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataAssets/Inventory/DataAsset_InventoryItemDefinition.h"
#include "WarriorInventoryTypes.generated.h"

USTRUCT(BlueprintType)
struct FWarriorInventorySlot
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	UDataAsset_InventoryItemDefinition* ItemDefinition = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory", meta=(ClampMin="0"))
	int32 Quantity = 0;

	bool IsEmpty() const;
	bool CanStackWith(const UDataAsset_InventoryItemDefinition* InItemDefinition) const;
	int32 GetAvailableStackSpace() const;
};
