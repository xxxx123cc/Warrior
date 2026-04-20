// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Weapons/WarriorWeaponBase.h"
#include "Components/BoxComponent.h"

AWarriorWeaponBase::AWarriorWeaponBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
//把 PrimaryActorTick.bCanEverTick = false; 的目的简单说就是禁止该 Actor 每帧调用 Tick，从而节省 CPU 开销。
	
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);
	
	WeaponCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("WeaponCollisionBox"));
	WeaponCollisionBox->SetupAttachment(GetRootComponent());
	WeaponCollisionBox->SetBoxExtent(FVector(20.f));
	WeaponCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponCollisionBox->OnComponentBeginOverlap.AddDynamic(this,&AWarriorWeaponBase::OnWeaponOverlapBegin);
	WeaponCollisionBox->OnComponentEndOverlap.AddDynamic(this,&AWarriorWeaponBase::OnWeaponOverlapEnd);
	
}

void AWarriorWeaponBase::OnWeaponOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APawn*WeaponOwningPawn = GetInstigator<APawn>();
	if (APawn* HitAPawn = Cast<APawn>(OtherActor))
	{
		if (WeaponOwningPawn!=HitAPawn)
		{
			OnWeaponHitTarget.ExecuteIfBound(OtherActor);
		}
		//为敌方实现检查
	}
}

void AWarriorWeaponBase::OnWeaponOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APawn*WeaponOwningPawn = GetInstigator<APawn>();
	if (APawn* HitAPawn = Cast<APawn>(OtherActor))
	{
		if (WeaponOwningPawn!=HitAPawn)
		{
			OnWeaponEndOverlapTarget.ExecuteIfBound(OtherActor);
		}
		//为敌方实现检查
	}
}



