// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/PickUps/WarriorPickUpBase.h"
#include "WarriorInventoryPickUp.generated.h"

class UDataAsset_InventoryItemDefinition;
class UStaticMeshComponent;

UCLASS()
class WARRIOR_API AWarriorInventoryPickUp : public AWarriorPickUpBase
{
	GENERATED_BODY()

public:
	AWarriorInventoryPickUp();

	UFUNCTION(BlueprintCallable, Category="WarriorPickUp|Inventory")
	void SetQuantity(int32 NewQuantity);

	UFUNCTION(BlueprintPure, Category="WarriorPickUp|Inventory")
	int32 GetQuantity() const;

protected:
	virtual void OnPickUpMeshBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="WarriorPickUp")
	UStaticMeshComponent* PickUpStaticMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="WarriorPickUp|Inventory")
	UDataAsset_InventoryItemDefinition* ItemDefinition = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="WarriorPickUp|Inventory", meta=(ClampMin="1"))
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="WarriorPickUp|Inventory")
	bool bDestroyWhenFullyPickedUp = true;

	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="On Inventory Pick Up Succeeded"))
	void BP_OnInventoryPickUpSucceeded(AActor* Picker, int32 AddedQuantity, int32 RemainingQuantity);

	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="On Inventory Pick Up Failed"))
	void BP_OnInventoryPickUpFailed(AActor* Picker);
};
