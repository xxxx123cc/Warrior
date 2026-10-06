// Fill out your copyright notice in the Description page of Project Settings.


#include "WarriorTypes/WarriorInventoryTypes.h"

bool FWarriorInventorySlot::IsEmpty() const
{
	return !ItemDefinition || Quantity <= 0;
}

bool FWarriorInventorySlot::CanStackWith(const UDataAsset_InventoryItemDefinition* InItemDefinition) const
{
	return !IsEmpty() &&
		ItemDefinition == InItemDefinition &&
		ItemDefinition->CanStack() &&
		Quantity < ItemDefinition->GetMaxStackSize();
}

int32 FWarriorInventorySlot::GetAvailableStackSpace() const
{
	if (IsEmpty())
	{
		return 0;
	}

	return FMath::Max(ItemDefinition->GetMaxStackSize() - Quantity, 0);
}
