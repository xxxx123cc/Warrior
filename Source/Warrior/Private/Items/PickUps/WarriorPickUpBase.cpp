// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/PickUps/WarriorPickUpBase.h"
#include "Components/SphereComponent.h"
// Sets default values
AWarriorPickUpBase::AWarriorPickUpBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	PickUpMeshComponent = CreateDefaultSubobject<USphereComponent>(FName("PickUpMeshComponent"));
	SetRootComponent(PickUpMeshComponent);
	PickUpMeshComponent->InitSphereRadius(50.f);
	PickUpMeshComponent->OnComponentBeginOverlap.AddUniqueDynamic(this,&ThisClass::OnPickUpMeshBeginOverlap);
	
}

void AWarriorPickUpBase::OnPickUpMeshBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
	
}

