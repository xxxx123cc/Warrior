// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAssets/Inventory/DataAsset_InventoryItemDefinition.h"

FName UDataAsset_InventoryItemDefinition::GetItemID() const
{
	return ItemID.IsNone() ? GetFName() : ItemID;
}

bool UDataAsset_InventoryItemDefinition::IsValidItemDefinition() const
{
	return !GetItemID().IsNone();
}

bool UDataAsset_InventoryItemDefinition::CanStack() const
{
	return bCanStack && GetMaxStackSize() > 1;
}

int32 UDataAsset_InventoryItemDefinition::GetMaxStackSize() const
{
	return bCanStack ? FMath::Max(MaxStackSize, 1) : 1;
}
