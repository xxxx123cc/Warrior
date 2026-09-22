// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Combat/HeroCombatComponent.h"
#include "Items/Weapons/WarriorHeroWeapon.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "WarriorDebugHelper.h"
#include "WarriorGameplayTags.h"
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
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetOwningPawn(),WarriorGameplayTags::Shared_Event_MeleeHit,EventData);
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetOwningPawn(),WarriorGameplayTags::Player_Event_HitPause,FGameplayEventData());
}

void UHeroCombatComponent::OnWeaponEndOverlapTarget(AActor* EndOverlapActor)
{
	Super::OnWeaponEndOverlapTarget(EndOverlapActor);
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetOwningPawn(),WarriorGameplayTags::Player_Event_HitPause,FGameplayEventData());
	
}
