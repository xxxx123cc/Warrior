// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Combat/EnemyCombatComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Characters/WarriorEnemyCharacter.h"
#include "Components/BoxComponent.h"
#include "WarriorFunctionLibrary.h"
#include "WarriorGameplayTags.h"

void UEnemyCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	ResetCurrentAttackImpactData();
}

void UEnemyCombatComponent::OnHitTargetActor(AActor* HitActor)
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

	FWarriorAttackImpactData ActiveAttackImpactData = CurrentAttackImpactData;
	if (UWarriorFunctionLibrary::NativeDoesActorHaveTag(GetOwningPawn(), WarriorGameplayTags::Enemy_Status_UnBlockable))
	{
		ActiveAttackImpactData.bCanBeBlocked = false;
	}

	FGameplayEventData EventData;
	EventData.Instigator = GetOwningPawn();
	EventData.Target = HitActor;
	EventData.EventMagnitude = ActiveAttackImpactData.BlockCost;
	UWarriorFunctionLibrary::AddAttackImpactDataToGameplayEventData(EventData, ActiveAttackImpactData);

	if (ActiveAttackImpactData.bCanBeDodged && UWarriorFunctionLibrary::IsActorInDodgeIFrame(HitActor))
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			HitActor,
			WarriorGameplayTags::Player_Event_SuccessDodge,
			EventData);
		return;
	}

	bool bIsValidBlock = false;
	const bool bIsPlayerBlocking =
		UWarriorFunctionLibrary::NativeDoesActorHaveTag(HitActor, WarriorGameplayTags::Player_Status_Blocking);

	if (bIsPlayerBlocking && ActiveAttackImpactData.bCanBeBlocked)
	{
		bIsValidBlock = UWarriorFunctionLibrary::IsValidBlock(GetOwningPawn(), HitActor);
	}

	if (bIsValidBlock)
	{
		UWarriorFunctionLibrary::HandleSuccessfulBlock(
			HitActor,
			GetOwningPawn(),
			ActiveAttackImpactData.BlockCost,
			EventData);
		return;
	}

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		GetOwningPawn(),
		WarriorGameplayTags::Shared_Event_MeleeHit,
		EventData);
}

void UEnemyCombatComponent::OnWeaponEndOverlapTarget(AActor* EndOverlapActor)
{
	Super::OnWeaponEndOverlapTarget(EndOverlapActor);
}

void UEnemyCombatComponent::ToggleCurrentWeaponCollision(bool bEnableCollision, EToggleDamageType ToggleDamageType)
{
	Super::ToggleCurrentWeaponCollision(bEnableCollision, ToggleDamageType);

	if (!bEnableCollision)
	{
		ResetCurrentAttackImpactData();
	}
}

void UEnemyCombatComponent::ToggleCurrentHandCollision(bool bEnableCollision, EToggleDamageType ToggleDamageType)
{
	AWarriorEnemyCharacter* OwningCharacter = GetOwningPawn<AWarriorEnemyCharacter>();
	check(OwningCharacter);

	switch (ToggleDamageType)
	{
		case EToggleDamageType::LeftHand:
			OwningCharacter->GetLeftBoxComponent()->SetCollisionEnabled(
				bEnableCollision ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
			break;
		case EToggleDamageType::RightHand:
			OwningCharacter->GetRightBoxComponent()->SetCollisionEnabled(
				bEnableCollision ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
			break;
		default:
			break;
	}

	if (!bEnableCollision)
	{
		OverlapActors.Empty();
		ResetCurrentAttackImpactData();
	}
}

void UEnemyCombatComponent::SetCurrentAttackImpactData(const FWarriorAttackImpactData& InAttackImpactData)
{
	CurrentAttackImpactData = InAttackImpactData;
}

void UEnemyCombatComponent::ResetCurrentAttackImpactData()
{
	CurrentAttackImpactData = DefaultAttackImpactData;
	CurrentAttackImpactData.BlockCost = SuccessfulBlockCost;
}
