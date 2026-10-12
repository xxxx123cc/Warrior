// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Combat/HeroCombatComponent.h"
#include "Items/Weapons/WarriorHeroWeapon.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Characters/WarriorHeroCharacter.h"
#include "Components/BoxComponent.h"
#include "WarriorDebugHelper.h"
#include "WarriorFunctionLibrary.h"
#include "WarriorGameplayTags.h"

void UHeroCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	ResetCurrentAttackImpactData();
}

AWarriorHeroWeapon* UHeroCombatComponent::GetHeroCarriedWeaponByTag(FGameplayTag InWeaponTagToGet) const
{
	
		return Cast<AWarriorHeroWeapon>(GetCarriedWeaponByTag(InWeaponTagToGet));
	
	
}

AWarriorHeroWeapon* UHeroCombatComponent::GetHeroCurrentEquippedWeapon() const
{
	return Cast<AWarriorHeroWeapon>(GetCurrentEquippedWeapon());;
}

float UHeroCombatComponent::GetHeroCurrentEquipWeaponDamageAtLevel(float InLevel) const
{
	const FScalableFloat& WeaponBaseDamage = GetHeroCurrentEquippedWeapon()->HeroWeaponData.WeaponBaseDamage;
	 if (!WeaponBaseDamage.IsValid())
	 {
		 return 0.f;
	 }
	return WeaponBaseDamage.GetValueAtLevel(InLevel);
	
}


void UHeroCombatComponent::OnHitTargetActor(AActor* HitActor)
{
	
	if (OverlapActors.Contains(HitActor))
	{
		return;
	}
	OverlapActors.AddUnique(HitActor);

	if (ShouldIgnoreHitDueToWeaponClash(HitActor))
	{
		return;
	}

	FGameplayEventData EventData;
	EventData.Instigator = GetOwningPawn();
	EventData.Target = HitActor;
	EventData.EventMagnitude = CurrentAttackImpactData.BlockCost;
	UWarriorFunctionLibrary::AddAttackImpactDataToGameplayEventData(EventData, CurrentAttackImpactData);
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetOwningPawn(),WarriorGameplayTags::Shared_Event_MeleeHit,EventData);
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetOwningPawn(),WarriorGameplayTags::Player_Event_HitPause,FGameplayEventData());
}

void UHeroCombatComponent::OnWeaponEndOverlapTarget(AActor* EndOverlapActor)
{
	Super::OnWeaponEndOverlapTarget(EndOverlapActor);
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetOwningPawn(),WarriorGameplayTags::Player_Event_HitPause,FGameplayEventData());
	
}

void UHeroCombatComponent::ToggleCurrentWeaponCollision(bool bEnableCollision, EToggleDamageType ToggleDamageType)
{
	Super::ToggleCurrentWeaponCollision(bEnableCollision, ToggleDamageType);

	if (!bEnableCollision)
	{
		ResetCurrentAttackImpactData();
	}
}

void UHeroCombatComponent::ToggleCurrentHandCollision(bool bEnableCollision, EToggleDamageType ToggleDamageType)
{
	AWarriorHeroCharacter* OwningCharacter = Cast<AWarriorHeroCharacter>(GetOwner());
	if (!OwningCharacter)
	{
		return;
	}

	UBoxComponent* CollisionBox = nullptr;
	switch (ToggleDamageType)
	{
		case EToggleDamageType::LeftFoot:
			CollisionBox = OwningCharacter->GetLeftFootBoxComponent();
			break;
		case EToggleDamageType::RightFoot:
			CollisionBox = OwningCharacter->GetRightFootBoxComponent();
			break;
		default:
			break;
	}

	if (!CollisionBox)
	{
		return;
	}

	if (bEnableCollision)
	{
		OverlapActors.Empty();
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		CollisionBox->UpdateOverlaps();
	}
	else
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		OverlapActors.Empty();
		ResetCurrentAttackImpactData();
	}
}

void UHeroCombatComponent::SetCurrentAttackImpactData(const FWarriorAttackImpactData& InAttackImpactData)
{
	CurrentAttackImpactData = InAttackImpactData;
	bHasCurrentAttackImpactDataOverride = true;
}

void UHeroCombatComponent::ResetCurrentAttackImpactData()
{
	CurrentAttackImpactData = DefaultAttackImpactData;
	bHasCurrentAttackImpactDataOverride = false;
}
