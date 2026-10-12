// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Components/Combat/PawnCombatComponent.h"
#include "WarriorTypes/WarriorStructTypes.h"
#include "ANS_ToggleDamageCollision.generated.h"

/**
 * Opens a montage damage window for weapon, hand, or foot collision.
 */
UCLASS(meta = (DisplayName = "Toggle Damage Collision"))
class WARRIOR_API UANS_ToggleDamageCollision : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UANS_ToggleDamageCollision();

	virtual void NotifyBegin(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		float TotalDuration,
		const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage")
	EToggleDamageType DamageType = EToggleDamageType::CurrentEquippedWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage")
	bool bOverrideAttackImpactData = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage", meta=(EditCondition="bOverrideAttackImpactData"))
	FWarriorAttackImpactData AttackImpactData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage")
	bool bResetAttackImpactDataOnEnd = true;

private:
	static UPawnCombatComponent* GetCombatComponentFromMeshOwner(const USkeletalMeshComponent* MeshComp);
	static void SetAttackImpactData(UPawnCombatComponent* CombatComponent, const FWarriorAttackImpactData& InAttackImpactData);
	static void ResetAttackImpactData(UPawnCombatComponent* CombatComponent);
};
