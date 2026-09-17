// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_EnableMoveCancel.generated.h"

/**
 * Opens a montage window where movement input can cancel the active attack ability.
 */
UCLASS(meta = (DisplayName = "Enable Move Cancel"))
class WARRIOR_API UANS_EnableMoveCancel : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UANS_EnableMoveCancel();

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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move Cancel")
	bool bRemoveMoveCancelTagOnEnd = true;
};
