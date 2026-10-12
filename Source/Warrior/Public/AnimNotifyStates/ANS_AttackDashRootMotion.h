// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GameFramework/RootMotionSource.h"
#include "ANS_AttackDashRootMotion.generated.h"

class UCharacterMovementComponent;
class UCurveFloat;

/**
 * Adds controllable dash distance over a montage notify window.
 *
 * Use this when the attack animation has the right pose/timing but its authored
 * root motion does not travel far enough.
 */
UCLASS(meta = (DisplayName = "Attack Dash Root Motion"))
class WARRIOR_API UANS_AttackDashRootMotion : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UANS_AttackDashRootMotion();

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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash", meta = (ClampMin = "0.0"))
	float DashDistance = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	bool bAdditive = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	bool bFaceDashDirection = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	bool bStopMovementOnBegin = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	bool bRemoveRootMotionSourceOnEnd = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	FName RootMotionSourceName = TEXT("AttackDashRootMotion");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash", meta = (ClampMin = "0", ClampMax = "65535"))
	int32 RootMotionPriority = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	TObjectPtr<UCurveFloat> StrengthOverTime = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	bool bEnableGravity = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	ERootMotionFinishVelocityMode VelocityOnFinishMode = ERootMotionFinishVelocityMode::SetVelocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	FVector SetVelocityOnFinish = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash", meta = (ClampMin = "0.0"))
	float ClampVelocityOnFinish = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direction")
	bool bUseControllerYawWhenTargetLocked = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direction")
	bool bUseMovementInput = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direction")
	bool bUseActorForward = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direction")
	bool bUseControllerYawFallback = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direction", meta = (ClampMin = "0.0"))
	float MinDirectionalInput = 0.1f;

private:
	struct FActiveDashRootMotion
	{
		TWeakObjectPtr<UCharacterMovementComponent> MovementComponent;
		uint16 RootMotionSourceID = static_cast<uint16>(ERootMotionSourceID::Invalid);
	};

	TMap<TWeakObjectPtr<USkeletalMeshComponent>, FActiveDashRootMotion> ActiveRootMotionSources;

	void RemoveActiveRootMotionSource(USkeletalMeshComponent* MeshComp);
	bool ResolveDashDirection(const AActor& OwnerActor, FVector& OutDirection) const;
	bool GetActorForwardDirection(const AActor& OwnerActor, FVector& OutDirection) const;
	bool GetMovementInputDirection(const AActor& OwnerActor, FVector& OutDirection) const;
	bool GetControllerYawDirection(const AActor& OwnerActor, FVector& OutDirection) const;
};
