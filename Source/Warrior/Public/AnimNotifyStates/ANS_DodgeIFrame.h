// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_DodgeIFrame.generated.h"

/**
 * Opens the valid dodge window for a montage range.
 */
UCLASS(meta = (DisplayName = "Dodge IFrame"))
class WARRIOR_API UANS_DodgeIFrame : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UANS_DodgeIFrame();

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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge")
	bool bGrantInvincibleTag = true;
};
