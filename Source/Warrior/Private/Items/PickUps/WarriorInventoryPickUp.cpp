// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/PickUps/WarriorInventoryPickUp.h"
#include "Components/Inventory/WarriorInventoryComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

AWarriorInventoryPickUp::AWarriorInventoryPickUp()
{
	PickUpStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickUpStaticMeshComponent"));
	PickUpStaticMeshComponent->SetupAttachment(GetRootComponent());
	PickUpStaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWarriorInventoryPickUp::SetQuantity(int32 NewQuantity)
{
	Quantity = FMath::Max(NewQuantity, 1);
}

int32 AWarriorInventoryPickUp::GetQuantity() const
{
	return Quantity;
}

void AWarriorInventoryPickUp::OnPickUpMeshBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	UWarriorInventoryComponent* InventoryComponent = OtherActor->FindComponentByClass<UWarriorInventoryComponent>();
	if (!InventoryComponent)
	{
		return;
	}

	int32 AddedQuantity = 0;
	int32 RemainingQuantity = Quantity;
	if (!InventoryComponent->TryAddItem(ItemDefinition, Quantity, AddedQuantity, RemainingQuantity))
	{
		BP_OnInventoryPickUpFailed(OtherActor);
		return;
	}

	Quantity = RemainingQuantity;
	BP_OnInventoryPickUpSucceeded(OtherActor, AddedQuantity, RemainingQuantity);

	if (RemainingQuantity > 0)
	{
		return;
	}

	if (PickUpMeshComponent)
	{
		PickUpMeshComponent->SetGenerateOverlapEvents(false);
	}

	if (bDestroyWhenFullyPickedUp)
	{
		Destroy();
	}
}
