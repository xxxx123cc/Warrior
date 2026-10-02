// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_EnableDodgeCancel.generated.h"

/**
 * Opens a montage window where dodge input can cancel the active attack ability.
 */
UCLASS(meta = (DisplayName = "Enable Dodge Cancel"))
class WARRIOR_API UANS_EnableDodgeCancel : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UANS_EnableDodgeCancel();

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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge Cancel")
	bool bRemoveDodgeCancelTagOnEnd = true;
};
