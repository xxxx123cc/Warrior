// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyStates/ANS_AttackMotionWarping.h"

#include "Characters/WarriorHeroCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "MotionWarpingComponent.h"
#include "WarriorFunctionLibrary.h"
#include "WarriorGameplayTags.h"

UANS_AttackMotionWarping::UANS_AttackMotionWarping()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(66, 135, 245);
#endif
}

void UANS_AttackMotionWarping::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	UpdateWarpTarget(MeshComp);
}

void UANS_AttackMotionWarping::NotifyTick(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float FrameDeltaTime,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	UpdateWarpTarget(MeshComp);
}

void UANS_AttackMotionWarping::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!bRemoveWarpTargetOnEnd || !MeshComp)
	{
		return;
	}

	if (AActor* OwnerActor = MeshComp->GetOwner())
	{
		if (UMotionWarpingComponent* MotionWarpingComponent = OwnerActor->FindComponentByClass<UMotionWarpingComponent>())
		{
			MotionWarpingComponent->RemoveWarpTarget(WarpTargetName);
		}
	}
}

FString UANS_AttackMotionWarping::GetNotifyName_Implementation() const
{
	return FString::Printf(TEXT("Attack Warp Target: %s"), *WarpTargetName.ToString());
}

void UANS_AttackMotionWarping::UpdateWarpTarget(USkeletalMeshComponent* MeshComp) const
{
	if (!MeshComp || WarpTargetName.IsNone())
	{
		return;
	}

	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	UMotionWarpingComponent* MotionWarpingComponent = OwnerActor->FindComponentByClass<UMotionWarpingComponent>();
	if (!MotionWarpingComponent)
	{
		return;
	}

	FVector DesiredDirection = FVector::ZeroVector;
	if (!ResolveDesiredDirection(*OwnerActor, DesiredDirection))
	{
		return;
	}

	const FRotator TargetRotation = ClampTargetRotation(*OwnerActor, DesiredDirection);
	const FVector TargetLocation = OwnerActor->GetActorLocation() + TargetRotation.Vector() * WarpTargetDistance;

	MotionWarpingComponent->AddOrUpdateWarpTargetFromLocationAndRotation(
		WarpTargetName,
		TargetLocation,
		TargetRotation);
}

bool UANS_AttackMotionWarping::ResolveDesiredDirection(const AActor& OwnerActor, FVector& OutDirection) const
{
	const bool bIsTargetLocked =
		bPreferControllerYawWhenTargetLocked &&
		UWarriorFunctionLibrary::NativeDoesActorHaveTag(const_cast<AActor*>(&OwnerActor), WarriorGameplayTags::Player_Status_TargetLock);

	if (bIsTargetLocked && GetControllerYawDirection(OwnerActor, OutDirection))
	{
		return true;
	}

	if (bUseMovementInput && GetMovementInputDirection(OwnerActor, OutDirection))
	{
		return true;
	}

	return bUseControllerYawFallback && GetControllerYawDirection(OwnerActor, OutDirection);
}

bool UANS_AttackMotionWarping::GetMovementInputDirection(const AActor& OwnerActor, FVector& OutDirection) const
{
	const APawn* OwnerPawn = Cast<APawn>(&OwnerActor);
	if (!OwnerPawn)
	{
		return false;
	}

	FVector MovementInput = OwnerPawn->GetPendingMovementInputVector();
	if (MovementInput.IsNearlyZero())
	{
		MovementInput = OwnerPawn->GetLastMovementInputVector();
	}

	if (MovementInput.IsNearlyZero())
	{
		if (const UCharacterMovementComponent* MovementComponent =
			OwnerActor.FindComponentByClass<UCharacterMovementComponent>())
		{
			MovementInput = MovementComponent->GetCurrentAcceleration();
		}
	}

	MovementInput.Z = 0.f;
	if (MovementInput.SizeSquared() < FMath::Square(MinDirectionalInput))
	{
		return false;
	}

	OutDirection = MovementInput.GetSafeNormal();
	return true;
}

bool UANS_AttackMotionWarping::GetControllerYawDirection(const AActor& OwnerActor, FVector& OutDirection) const
{
	const APawn* OwnerPawn = Cast<APawn>(&OwnerActor);
	const AController* Controller = OwnerPawn ? OwnerPawn->GetController() : nullptr;
	if (!Controller)
	{
		return false;
	}

	const FRotator ControlYawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
	OutDirection = ControlYawRotation.Vector();
	return !OutDirection.IsNearlyZero();
}

FRotator UANS_AttackMotionWarping::ClampTargetRotation(const AActor& OwnerActor, const FVector& DesiredDirection) const
{
	const FRotator CurrentRotation(0.f, OwnerActor.GetActorRotation().Yaw, 0.f);
	const FRotator DesiredRotation(0.f, DesiredDirection.ToOrientationRotator().Yaw, 0.f);

	if (MaxYawDelta <= 0.f || MaxYawDelta >= 180.f)
	{
		return DesiredRotation;
	}

	const float YawDelta = FMath::FindDeltaAngleDegrees(CurrentRotation.Yaw, DesiredRotation.Yaw);
	const float ClampedYawDelta = FMath::Clamp(YawDelta, -MaxYawDelta, MaxYawDelta);
	return FRotator(0.f, CurrentRotation.Yaw + ClampedYawDelta, 0.f);
}
