// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_AttackMotionWarping.generated.h"

class UMotionWarpingComponent;

/**
 * Updates a Motion Warping target during an attack turn window.
 *
 * Add this notify over the same montage range as UE's Motion Warping notify,
 * and make both notify states use the same WarpTargetName.
 */
UCLASS(meta = (DisplayName = "Attack Motion Warp Target"))
class WARRIOR_API UANS_AttackMotionWarping : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UANS_AttackMotionWarping();

	virtual void NotifyBegin(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		float TotalDuration,
		const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyTick(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		float FrameDeltaTime,
		const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Warping")
	FName WarpTargetName = TEXT("AttackTurn");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Warping", meta = (ClampMin = "0.0"))
	float WarpTargetDistance = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Warping", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float MaxYawDelta = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Warping", meta = (ClampMin = "0.0"))
	float MinDirectionalInput = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Warping")
	bool bUseActorForward = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Warping")
	bool bUseMovementInput = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Warping")
	bool bUseControllerYawFallback = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Warping")
	bool bPreferControllerYawWhenTargetLocked = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Warping")
	bool bRemoveWarpTargetOnEnd = true;

private:
	void UpdateWarpTarget(USkeletalMeshComponent* MeshComp) const;
	bool ResolveDesiredDirection(const AActor& OwnerActor, FVector& OutDirection) const;
	bool GetActorForwardDirection(const AActor& OwnerActor, FVector& OutDirection) const;
	bool GetMovementInputDirection(const AActor& OwnerActor, FVector& OutDirection) const;
	bool GetControllerYawDirection(const AActor& OwnerActor, FVector& OutDirection) const;
	FRotator ClampTargetRotation(const AActor& OwnerActor, const FVector& DesiredDirection) const;
};
