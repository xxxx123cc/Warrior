// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Inventory/WarriorInventoryComponent.h"
#include "DataAssets/Inventory/DataAsset_InventoryItemDefinition.h"

UWarriorInventoryComponent::UWarriorInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UWarriorInventoryComponent::TryAddItem(UDataAsset_InventoryItemDefinition* ItemDefinition, int32 Quantity, int32& OutAddedQuantity, int32& OutRemainingQuantity)
{
	OutAddedQuantity = 0;
	OutRemainingQuantity = FMath::Max(Quantity, 0);

	if (!IsValidItemRequest(ItemDefinition, Quantity))
	{
		return false;
	}

	int32 RemainingQuantity = Quantity;
	const int32 MaxStackSize = GetStackSizeForItem(ItemDefinition);

	if (ItemDefinition->CanStack())
	{
		for (FWarriorInventorySlot& InventorySlot : InventorySlots)
		{
			if (!InventorySlot.CanStackWith(ItemDefinition))
			{
				continue;
			}

			const int32 QuantityToAdd = FMath::Min(InventorySlot.GetAvailableStackSpace(), RemainingQuantity);
			InventorySlot.Quantity += QuantityToAdd;
			RemainingQuantity -= QuantityToAdd;

			if (RemainingQuantity <= 0)
			{
				break;
			}
		}
	}

	while (RemainingQuantity > 0 && InventorySlots.Num() < MaxInventorySlots)
	{
		FWarriorInventorySlot& NewSlot = InventorySlots.AddDefaulted_GetRef();
		NewSlot.ItemDefinition = ItemDefinition;
		NewSlot.Quantity = FMath::Min(MaxStackSize, RemainingQuantity);
		RemainingQuantity -= NewSlot.Quantity;
	}

	OutAddedQuantity = Quantity - RemainingQuantity;
	OutRemainingQuantity = RemainingQuantity;

	if (OutAddedQuantity > 0)
	{
		OnInventoryChanged.Broadcast();
		OnInventoryItemAdded.Broadcast(ItemDefinition, OutAddedQuantity, OutRemainingQuantity);
	}

	return OutAddedQuantity > 0;
}

bool UWarriorInventoryComponent::RemoveItem(UDataAsset_InventoryItemDefinition* ItemDefinition, int32 Quantity, int32& OutRemovedQuantity)
{
	OutRemovedQuantity = 0;

	if (!IsValidItemRequest(ItemDefinition, Quantity))
	{
		return false;
	}

	int32 RemainingQuantity = Quantity;

	for (int32 SlotIndex = InventorySlots.Num() - 1; SlotIndex >= 0 && RemainingQuantity > 0; --SlotIndex)
	{
		FWarriorInventorySlot& InventorySlot = InventorySlots[SlotIndex];
		if (InventorySlot.ItemDefinition != ItemDefinition)
		{
			continue;
		}

		const int32 QuantityToRemove = FMath::Min(InventorySlot.Quantity, RemainingQuantity);
		InventorySlot.Quantity -= QuantityToRemove;
		RemainingQuantity -= QuantityToRemove;
		OutRemovedQuantity += QuantityToRemove;

		if (InventorySlot.Quantity <= 0)
		{
			InventorySlots.RemoveAt(SlotIndex);
		}
	}

	if (OutRemovedQuantity > 0)
	{
		OnInventoryChanged.Broadcast();
		OnInventoryItemRemoved.Broadcast(ItemDefinition, OutRemovedQuantity);
	}

	return OutRemovedQuantity > 0;
}

bool UWarriorInventoryComponent::RemoveItemByID(FName ItemID, int32 Quantity, int32& OutRemovedQuantity)
{
	OutRemovedQuantity = 0;

	if (ItemID.IsNone() || Quantity <= 0)
	{
		return false;
	}

	int32 RemainingQuantity = Quantity;
	UDataAsset_InventoryItemDefinition* RemovedItemDefinition = nullptr;

	for (int32 SlotIndex = InventorySlots.Num() - 1; SlotIndex >= 0 && RemainingQuantity > 0; --SlotIndex)
	{
		FWarriorInventorySlot& InventorySlot = InventorySlots[SlotIndex];
		if (!InventorySlot.ItemDefinition || InventorySlot.ItemDefinition->GetItemID() != ItemID)
		{
			continue;
		}

		if (!RemovedItemDefinition)
		{
			RemovedItemDefinition = InventorySlot.ItemDefinition;
		}

		const int32 QuantityToRemove = FMath::Min(InventorySlot.Quantity, RemainingQuantity);
		InventorySlot.Quantity -= QuantityToRemove;
		RemainingQuantity -= QuantityToRemove;
		OutRemovedQuantity += QuantityToRemove;

		if (InventorySlot.Quantity <= 0)
		{
			InventorySlots.RemoveAt(SlotIndex);
		}
	}

	if (OutRemovedQuantity > 0)
	{
		OnInventoryChanged.Broadcast();
		OnInventoryItemRemoved.Broadcast(RemovedItemDefinition, OutRemovedQuantity);
	}

	return OutRemovedQuantity > 0;
}

void UWarriorInventoryComponent::ClearInventory()
{
	if (InventorySlots.IsEmpty())
	{
		return;
	}

	InventorySlots.Empty();
	OnInventoryChanged.Broadcast();
}

bool UWarriorInventoryComponent::HasRoomForItem(UDataAsset_InventoryItemDefinition* ItemDefinition, int32 Quantity) const
{
	if (!IsValidItemRequest(ItemDefinition, Quantity))
	{
		return false;
	}

	int32 RemainingQuantity = Quantity;

	if (ItemDefinition->CanStack())
	{
		for (const FWarriorInventorySlot& InventorySlot : InventorySlots)
		{
			if (InventorySlot.CanStackWith(ItemDefinition))
			{
				RemainingQuantity -= FMath::Min(InventorySlot.GetAvailableStackSpace(), RemainingQuantity);
			}

			if (RemainingQuantity <= 0)
			{
				return true;
			}
		}
	}

	const int32 FreeSlotCount = FMath::Max(MaxInventorySlots - InventorySlots.Num(), 0);
	return RemainingQuantity <= FreeSlotCount * GetStackSizeForItem(ItemDefinition);
}

int32 UWarriorInventoryComponent::GetTotalQuantity(UDataAsset_InventoryItemDefinition* ItemDefinition) const
{
	if (!ItemDefinition)
	{
		return 0;
	}

	int32 TotalQuantity = 0;
	for (const FWarriorInventorySlot& InventorySlot : InventorySlots)
	{
		if (InventorySlot.ItemDefinition == ItemDefinition)
		{
			TotalQuantity += InventorySlot.Quantity;
		}
	}

	return TotalQuantity;
}

int32 UWarriorInventoryComponent::GetTotalQuantityByID(FName ItemID) const
{
	if (ItemID.IsNone())
	{
		return 0;
	}

	int32 TotalQuantity = 0;
	for (const FWarriorInventorySlot& InventorySlot : InventorySlots)
	{
		if (InventorySlot.ItemDefinition && InventorySlot.ItemDefinition->GetItemID() == ItemID)
		{
			TotalQuantity += InventorySlot.Quantity;
		}
	}

	return TotalQuantity;
}

TArray<FWarriorInventorySlot> UWarriorInventoryComponent::GetInventorySlots() const
{
	return InventorySlots;
}

int32 UWarriorInventoryComponent::GetUsedSlotCount() const
{
	return InventorySlots.Num();
}

int32 UWarriorInventoryComponent::GetMaxInventorySlots() const
{
	return MaxInventorySlots;
}

bool UWarriorInventoryComponent::IsValidItemRequest(const UDataAsset_InventoryItemDefinition* ItemDefinition, int32 Quantity) const
{
	return IsValid(ItemDefinition) && ItemDefinition->IsValidItemDefinition() && Quantity > 0;
}

int32 UWarriorInventoryComponent::GetStackSizeForItem(const UDataAsset_InventoryItemDefinition* ItemDefinition) const
{
	return ItemDefinition ? ItemDefinition->GetMaxStackSize() : 1;
}
