// Fill out your copyright notice in the Description page of Project Settings.

#include "Items/Weapons/WarriorWeaponBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"
#include "WarriorFunctionLibrary.h"
#include "WarriorGameplayTags.h"
#include "WarriorFunctionLibrary.h"

namespace
{
	constexpr float WeaponClashStateFallbackDuration = 0.5f;
}

AWarriorWeaponBase::AWarriorWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	WeaponMesh->SetGenerateOverlapEvents(false);
	WeaponMesh->CanCharacterStepUpOn = ECB_No;
	WeaponMesh->SetCanEverAffectNavigation(false);

	WeaponCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("WeaponCollisionBox"));
	WeaponCollisionBox->SetupAttachment(GetRootComponent());
	WeaponCollisionBox->SetBoxExtent(FVector(20.f));
	WeaponCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponCollisionBox->SetCollisionObjectType(ECC_WorldDynamic);
	WeaponCollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	WeaponCollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	WeaponCollisionBox->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	WeaponCollisionBox->SetGenerateOverlapEvents(true);
	WeaponCollisionBox->CanCharacterStepUpOn = ECB_No;
	WeaponCollisionBox->SetCanEverAffectNavigation(false);
	WeaponCollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AWarriorWeaponBase::OnWeaponOverlapBegin);
	WeaponCollisionBox->OnComponentEndOverlap.AddDynamic(this, &AWarriorWeaponBase::OnWeaponOverlapEnd);
}

void AWarriorWeaponBase::OnWeaponOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APawn* WeaponOwningPawn = GetWeaponOwningPawn();

	if (!OtherActor || OtherActor == this || OtherActor == WeaponOwningPawn)
	{
		return;
	}

	if (AWarriorWeaponBase* OtherWeapon = Cast<AWarriorWeaponBase>(OtherActor))
	{
		HandleWeaponClash(OtherWeapon, SweepResult);
		return;
	}

	if (!WeaponOwningPawn)
	{
		return;
	}

	if (APawn* HitPawn = Cast<APawn>(OtherActor))
	{
		if (WeaponOwningPawn->GetController() &&
			HitPawn->GetController() &&
			UWarriorFunctionLibrary::IsTargetPawnHostile(WeaponOwningPawn, HitPawn))
		{
			OnWeaponHitTarget.ExecuteIfBound(OtherActor);
		}
	}
}

void AWarriorWeaponBase::OnWeaponOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APawn* WeaponOwningPawn = GetWeaponOwningPawn();

	if (!OtherActor || OtherActor == WeaponOwningPawn || !WeaponOwningPawn)
	{
		return;
	}

	if (APawn* HitPawn = Cast<APawn>(OtherActor))
	{
		if (WeaponOwningPawn->GetController() &&
			HitPawn->GetController() &&
			UWarriorFunctionLibrary::IsTargetPawnHostile(WeaponOwningPawn, HitPawn))
		{
			OnWeaponEndOverlapTarget.ExecuteIfBound(OtherActor);
		}
	}
}

void AWarriorWeaponBase::HandleWeaponClash(AWarriorWeaponBase* OtherWeapon, const FHitResult& SweepResult)
{
	if (!OtherWeapon || OtherWeapon == this)
	{
		return;
	}

	APawn* WeaponOwningPawn = GetWeaponOwningPawn();
	APawn* OtherWeaponOwningPawn = OtherWeapon->GetWeaponOwningPawn();

	if (!WeaponOwningPawn ||
		!OtherWeaponOwningPawn ||
		WeaponOwningPawn == OtherWeaponOwningPawn ||
		!WeaponOwningPawn->GetController() ||
		!OtherWeaponOwningPawn->GetController())
	{
		return;
	}

	if (!UWarriorFunctionLibrary::IsTargetPawnHostile(WeaponOwningPawn, OtherWeaponOwningPawn))
	{
		return;
	}

	if (GetUniqueID() > OtherWeapon->GetUniqueID())
	{
		return;
	}

	FVector ClashLocation = SweepResult.ImpactPoint;

	if (ClashLocation.IsNearlyZero())
	{
		ClashLocation =
			(WeaponCollisionBox->GetComponentLocation() + OtherWeapon->GetWeaponCollisionMesh()->GetComponentLocation()) * 0.5f;
	}

	NotifyOwnerWeaponClash(OtherWeapon, ClashLocation);
	OtherWeapon->NotifyOwnerWeaponClash(this, ClashLocation);
}

void AWarriorWeaponBase::NotifyOwnerWeaponClash(AWarriorWeaponBase* OtherWeapon, const FVector& ClashLocation) const
{
	APawn* OwnerPawn = GetWeaponOwningPawn();
	APawn* OtherOwnerPawn = OtherWeapon ? OtherWeapon->GetWeaponOwningPawn() : nullptr;

	if (!OwnerPawn)
	{
		return;
	}

	UWarriorAbilitySystemComponent* WarriorASC =
		Cast<UWarriorAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerPawn));

	if (!WarriorASC || WarriorASC->HasMatchingGameplayTag(WarriorGameplayTags::Shared_Status_Clashing))
	{
		return;
	}

	WarriorASC->AddLooseGameplayTag(WarriorGameplayTags::Shared_Status_Clashing);

	if (UWorld* World = GetWorld())
	{
		FTimerHandle ClashStateResetTimer;
		TWeakObjectPtr<UWarriorAbilitySystemComponent> WeakWarriorASC = WarriorASC;

		World->GetTimerManager().SetTimer(ClashStateResetTimer, [WeakWarriorASC]()
		{
			if (WeakWarriorASC.IsValid())
			{
				WeakWarriorASC->RemoveLooseGameplayTag(WarriorGameplayTags::Shared_Status_Clashing);
			}
		}, WeaponClashStateFallbackDuration, false);
	}

	FGameplayEffectContextHandle EffectContextHandle = WarriorASC->MakeEffectContext();
	EffectContextHandle.AddInstigator(OwnerPawn, OwnerPawn);
	EffectContextHandle.AddSourceObject(this);
	EffectContextHandle.AddOrigin(ClashLocation);

	FGameplayEventData EventData;
	EventData.EventTag = WarriorGameplayTags::Shared_Event_WeaponClash;
	EventData.Instigator = OwnerPawn;
	EventData.Target = OtherOwnerPawn;
	EventData.OptionalObject = this;
	EventData.OptionalObject2 = OtherWeapon;
	EventData.ContextHandle = EffectContextHandle;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		OwnerPawn,
		WarriorGameplayTags::Shared_Event_WeaponClash,
		EventData);
}

APawn* AWarriorWeaponBase::GetWeaponOwningPawn() const
{
	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		return OwnerPawn;
	}

	return GetInstigator<APawn>();
}
