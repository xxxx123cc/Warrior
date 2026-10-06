// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PawnExtensionComponentBase.h"
#include "WarriorTypes/WarriorInventoryTypes.h"
#include "WarriorInventoryComponent.generated.h"

class UDataAsset_InventoryItemDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWarriorInventoryChangedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnWarriorInventoryItemAddedDelegate, UDataAsset_InventoryItemDefinition*, ItemDefinition, int32, AddedQuantity, int32, RemainingQuantity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWarriorInventoryItemRemovedDelegate, UDataAsset_InventoryItemDefinition*, ItemDefinition, int32, RemovedQuantity);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WARRIOR_API UWarriorInventoryComponent : public UPawnExtensionComponentBase
{
	GENERATED_BODY()

public:
	UWarriorInventoryComponent();

	UFUNCTION(BlueprintCallable, Category="Warrior|Inventory")
	bool TryAddItem(UDataAsset_InventoryItemDefinition* ItemDefinition, int32 Quantity, int32& OutAddedQuantity, int32& OutRemainingQuantity);

	UFUNCTION(BlueprintCallable, Category="Warrior|Inventory")
	bool RemoveItem(UDataAsset_InventoryItemDefinition* ItemDefinition, int32 Quantity, int32& OutRemovedQuantity);

	UFUNCTION(BlueprintCallable, Category="Warrior|Inventory")
	bool RemoveItemByID(FName ItemID, int32 Quantity, int32& OutRemovedQuantity);

	UFUNCTION(BlueprintCallable, Category="Warrior|Inventory")
	void ClearInventory();

	UFUNCTION(BlueprintPure, Category="Warrior|Inventory")
	bool HasRoomForItem(UDataAsset_InventoryItemDefinition* ItemDefinition, int32 Quantity) const;

	UFUNCTION(BlueprintPure, Category="Warrior|Inventory")
	int32 GetTotalQuantity(UDataAsset_InventoryItemDefinition* ItemDefinition) const;

	UFUNCTION(BlueprintPure, Category="Warrior|Inventory")
	int32 GetTotalQuantityByID(FName ItemID) const;

	UFUNCTION(BlueprintPure, Category="Warrior|Inventory")
	TArray<FWarriorInventorySlot> GetInventorySlots() const;

	UFUNCTION(BlueprintPure, Category="Warrior|Inventory")
	int32 GetUsedSlotCount() const;

	UFUNCTION(BlueprintPure, Category="Warrior|Inventory")
	int32 GetMaxInventorySlots() const;

	const TArray<FWarriorInventorySlot>& GetInventorySlotsRef() const { return InventorySlots; }

	UPROPERTY(BlueprintAssignable, Category="Warrior|Inventory")
	FOnWarriorInventoryChangedDelegate OnInventoryChanged;

	UPROPERTY(BlueprintAssignable, Category="Warrior|Inventory")
	FOnWarriorInventoryItemAddedDelegate OnInventoryItemAdded;

	UPROPERTY(BlueprintAssignable, Category="Warrior|Inventory")
	FOnWarriorInventoryItemRemovedDelegate OnInventoryItemRemoved;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Warrior|Inventory", meta=(ClampMin="1"))
	int32 MaxInventorySlots = 20;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Warrior|Inventory")
	TArray<FWarriorInventorySlot> InventorySlots;

private:
	bool IsValidItemRequest(const UDataAsset_InventoryItemDefinition* ItemDefinition, int32 Quantity) const;
	int32 GetStackSizeForItem(const UDataAsset_InventoryItemDefinition* ItemDefinition) const;
};
