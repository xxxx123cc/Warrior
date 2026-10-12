// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimNotifyStates/ANS_ToggleDamageCollision.h"

#include "Components/Combat/EnemyCombatComponent.h"
#include "Components/Combat/HeroCombatComponent.h"
#include "Interfaces/PawnCombatInterface.h"

UANS_ToggleDamageCollision::UANS_ToggleDamageCollision()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(230, 118, 44);
#endif
}

void UANS_ToggleDamageCollision::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	UPawnCombatComponent* CombatComponent = GetCombatComponentFromMeshOwner(MeshComp);
	if (!CombatComponent)
	{
		return;
	}

	if (bOverrideAttackImpactData)
	{
		SetAttackImpactData(CombatComponent, AttackImpactData);
	}

	CombatComponent->ToggleWeaponCollision(true, DamageType);
}

void UANS_ToggleDamageCollision::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	UPawnCombatComponent* CombatComponent = GetCombatComponentFromMeshOwner(MeshComp);
	if (!CombatComponent)
	{
		return;
	}

	CombatComponent->ToggleWeaponCollision(false, DamageType);

	if (bResetAttackImpactDataOnEnd)
	{
		ResetAttackImpactData(CombatComponent);
	}
}

FString UANS_ToggleDamageCollision::GetNotifyName_Implementation() const
{
	switch (DamageType)
	{
		case EToggleDamageType::RightHand:
			return TEXT("Damage Collision: Right Hand");
		case EToggleDamageType::LeftHand:
			return TEXT("Damage Collision: Left Hand");
		case EToggleDamageType::RightFoot:
			return TEXT("Damage Collision: Right Foot");
		case EToggleDamageType::LeftFoot:
			return TEXT("Damage Collision: Left Foot");
		case EToggleDamageType::CurrentEquippedWeapon:
		default:
			return TEXT("Damage Collision: Weapon");
	}
}

UPawnCombatComponent* UANS_ToggleDamageCollision::GetCombatComponentFromMeshOwner(
	const USkeletalMeshComponent* MeshComp)
{
	AActor* OwnerActor = MeshComp ? MeshComp->GetOwner() : nullptr;
	if (!OwnerActor)
	{
		return nullptr;
	}

	IPawnCombatInterface* CombatInterface = Cast<IPawnCombatInterface>(OwnerActor);
	return CombatInterface ? CombatInterface->GetPawnCombatComponent() : nullptr;
}

void UANS_ToggleDamageCollision::SetAttackImpactData(
	UPawnCombatComponent* CombatComponent,
	const FWarriorAttackImpactData& InAttackImpactData)
{
	if (UHeroCombatComponent* HeroCombatComponent = Cast<UHeroCombatComponent>(CombatComponent))
	{
		HeroCombatComponent->SetCurrentAttackImpactData(InAttackImpactData);
		return;
	}

	if (UEnemyCombatComponent* EnemyCombatComponent = Cast<UEnemyCombatComponent>(CombatComponent))
	{
		EnemyCombatComponent->SetCurrentAttackImpactData(InAttackImpactData);
	}
}

void UANS_ToggleDamageCollision::ResetAttackImpactData(UPawnCombatComponent* CombatComponent)
{
	if (UHeroCombatComponent* HeroCombatComponent = Cast<UHeroCombatComponent>(CombatComponent))
	{
		HeroCombatComponent->ResetCurrentAttackImpactData();
		return;
	}

	if (UEnemyCombatComponent* EnemyCombatComponent = Cast<UEnemyCombatComponent>(CombatComponent))
	{
		EnemyCombatComponent->ResetCurrentAttackImpactData();
	}
}
