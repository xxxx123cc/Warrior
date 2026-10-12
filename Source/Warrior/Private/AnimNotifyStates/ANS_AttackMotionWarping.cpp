// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyStates/ANS_AttackMotionWarping.h"

#include "Characters/WarriorHeroCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MotionWarpingComponent.h"
#include "WarriorFunctionLibrary.h"
#include "WarriorGameplayTags.h"

UANS_AttackMotionWarping::UANS_AttackMotionWarping()
{
	AttackMagnetObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

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

	FVector MagnetLocation = FVector::ZeroVector;
	FRotator MagnetRotation = FRotator::ZeroRotator;
	if (TryResolveAttackMagnetWarpTarget(*OwnerActor, MagnetLocation, MagnetRotation))
	{
		MotionWarpingComponent->AddOrUpdateWarpTargetFromLocationAndRotation(
			WarpTargetName,
			MagnetLocation,
			MagnetRotation);
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

bool UANS_AttackMotionWarping::TryResolveAttackMagnetWarpTarget(
	const AActor& OwnerActor,
	FVector& OutLocation,
	FRotator& OutRotation) const
{
	if (!bEnableAttackMagnet)
	{
		return false;
	}

	const APawn* OwnerPawn = Cast<APawn>(&OwnerActor);
	if (!OwnerPawn)
	{
		return false;
	}

	AActor* TargetActor = FindAttackMagnetTarget(*OwnerPawn);
	if (!TargetActor)
	{
		return false;
	}

	FVector AttackDirection = FVector::ZeroVector;
	if (!ResolveAttackMagnetTraceDirection(*OwnerPawn, AttackDirection))
	{
		return false;
	}

	FVector TargetLocation = TargetActor->GetActorLocation();
	TargetLocation.Z = OwnerActor.GetActorLocation().Z;

	FVector TargetSideDirection = OwnerActor.GetActorLocation() - TargetLocation;
	TargetSideDirection.Z = 0.f;
	if (!TargetSideDirection.Normalize())
	{
		TargetSideDirection = -AttackDirection;
	}

	const float StopDistance = FMath::Max(0.f, AttackMagnetStopDistance);
	const float MinMoveDistance = FMath::Max(0.f, AttackMagnetMinMoveDistance);
	FVector MagnetLocation = TargetLocation + TargetSideDirection * StopDistance;
	MagnetLocation.Z = OwnerActor.GetActorLocation().Z;

	const bool bShouldMoveToTarget =
		FVector::DistSquared2D(OwnerActor.GetActorLocation(), MagnetLocation) > FMath::Square(MinMoveDistance);

	OutLocation = bShouldMoveToTarget ? MagnetLocation : OwnerActor.GetActorLocation();
	OutLocation.Z = OwnerActor.GetActorLocation().Z;

	FVector FacingDirection = TargetLocation - OutLocation;
	FacingDirection.Z = 0.f;
	if (!FacingDirection.Normalize())
	{
		FacingDirection = TargetLocation - OwnerActor.GetActorLocation();
		FacingDirection.Z = 0.f;
	}

	if (!FacingDirection.Normalize())
	{
		FacingDirection = AttackDirection;
	}

	OutRotation = FacingDirection.ToOrientationRotator();
	OutRotation.Pitch = 0.f;
	OutRotation.Roll = 0.f;
	const_cast<AActor&>(OwnerActor).SetActorRotation(OutRotation);

	return true;
}

AActor* UANS_AttackMotionWarping::FindAttackMagnetTarget(const APawn& OwnerPawn) const
{
	if (AttackMagnetTraceDistance <= 0.f)
	{
		return nullptr;
	}

	FVector TraceDirection = FVector::ZeroVector;
	ResolveAttackMagnetTraceDirection(OwnerPawn, TraceDirection);
	if (TraceDirection.IsNearlyZero())
	{
		return nullptr;
	}

	const FVector BoxHalfSize(
		FMath::Abs(AttackMagnetTraceBoxHalfSize.X),
		FMath::Abs(AttackMagnetTraceBoxHalfSize.Y),
		FMath::Abs(AttackMagnetTraceBoxHalfSize.Z));
	if (BoxHalfSize.X <= 0.f || BoxHalfSize.Y <= 0.f || BoxHalfSize.Z <= 0.f)
	{
		return nullptr;
	}

	TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes = AttackMagnetObjectTypes;
	if (TraceObjectTypes.IsEmpty())
	{
		TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	}

	const FVector TraceStart = OwnerPawn.GetActorLocation();
	const FVector TraceEnd = TraceStart + TraceDirection * AttackMagnetTraceDistance;
	const FRotator TraceRotation = TraceDirection.ToOrientationRotator();

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(const_cast<APawn*>(&OwnerPawn));

	TArray<FHitResult> HitResults;
	UKismetSystemLibrary::BoxTraceMultiForObjects(
		&OwnerPawn,
		TraceStart,
		TraceEnd,
		BoxHalfSize,
		TraceRotation,
		TraceObjectTypes,
		false,
		ActorsToIgnore,
		bDrawAttackMagnetDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
		HitResults,
		true);

	AActor* BestTargetActor = nullptr;
	float BestYawDelta = TNumericLimits<float>::Max();
	float BestDistanceSquared = TNumericLimits<float>::Max();
	const float TraceYaw = TraceDirection.ToOrientationRotator().Yaw;

	for (const FHitResult& HitResult : HitResults)
	{
		APawn* TargetPawn = Cast<APawn>(HitResult.GetActor());
		if (!TargetPawn ||
			TargetPawn == &OwnerPawn ||
			!UWarriorFunctionLibrary::IsTargetPawnHostile(const_cast<APawn*>(&OwnerPawn), TargetPawn) ||
			UWarriorFunctionLibrary::NativeDoesActorHaveTag(TargetPawn, WarriorGameplayTags::Shared_Status_Death))
		{
			continue;
		}

		FVector OwnerToTarget = TargetPawn->GetActorLocation() - OwnerPawn.GetActorLocation();
		OwnerToTarget.Z = 0.f;
		if (!OwnerToTarget.Normalize())
		{
			continue;
		}

		const float TargetYaw = OwnerToTarget.ToOrientationRotator().Yaw;
		const float YawDelta = FMath::Abs(FMath::FindDeltaAngleDegrees(TraceYaw, TargetYaw));

		if (MaxYawDelta > 0.f && MaxYawDelta < 180.f)
		{
			if (YawDelta > MaxYawDelta)
			{
				continue;
			}
		}

		const float DistanceSquared = FVector::DistSquared2D(OwnerPawn.GetActorLocation(), TargetPawn->GetActorLocation());
		const bool bBetterFacingMatch = YawDelta < BestYawDelta - 1.f;
		const bool bSameFacingMatch = FMath::IsNearlyEqual(YawDelta, BestYawDelta, 1.f);
		if (bBetterFacingMatch || (bSameFacingMatch && DistanceSquared < BestDistanceSquared))
		{
			BestYawDelta = YawDelta;
			BestDistanceSquared = DistanceSquared;
			BestTargetActor = TargetPawn;
		}
	}

	return BestTargetActor;
}

bool UANS_AttackMotionWarping::ResolveAttackMagnetTraceDirection(
	const APawn& OwnerPawn,
	FVector& OutDirection) const
{
	if (bPreferControllerYawForAttackMagnet && GetControllerYawDirection(OwnerPawn, OutDirection))
	{
		return true;
	}

	if (ResolveDesiredDirection(OwnerPawn, OutDirection))
	{
		return true;
	}

	FVector ForwardDirection = OwnerPawn.GetActorForwardVector();
	ForwardDirection.Z = 0.f;
	if (ForwardDirection.IsNearlyZero())
	{
		return false;
	}

	OutDirection = ForwardDirection.GetSafeNormal();
	return true;
}

bool UANS_AttackMotionWarping::ResolveAttackMagnetSideDirection(
	const AActor& OwnerActor,
	const AActor& TargetActor,
	FVector& OutSideDirection) const
{
	FVector TargetToOwner = OwnerActor.GetActorLocation() - TargetActor.GetActorLocation();
	TargetToOwner.Z = 0.f;
	if (!TargetToOwner.Normalize())
	{
		return false;
	}

	if (!bSnapAttackMagnetToTargetSide)
	{
		OutSideDirection = TargetToOwner;
		return true;
	}

	FVector TargetForward = TargetActor.GetActorForwardVector();
	TargetForward.Z = 0.f;
	if (!TargetForward.Normalize())
	{
		return false;
	}

	FVector TargetRight = TargetActor.GetActorRightVector();
	TargetRight.Z = 0.f;
	if (!TargetRight.Normalize())
	{
		return false;
	}

	const float ForwardDot = FVector::DotProduct(TargetForward, TargetToOwner);
	const float RightDot = FVector::DotProduct(TargetRight, TargetToOwner);

	OutSideDirection = FMath::Abs(ForwardDot) >= FMath::Abs(RightDot)
		? TargetForward * FMath::Sign(ForwardDot)
		: TargetRight * FMath::Sign(RightDot);

	return !OutSideDirection.IsNearlyZero();
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

	if (bUseActorForward && GetActorForwardDirection(OwnerActor, OutDirection))
	{
		return true;
	}

	return bUseControllerYawFallback && GetControllerYawDirection(OwnerActor, OutDirection);
}

bool UANS_AttackMotionWarping::GetActorForwardDirection(const AActor& OwnerActor, FVector& OutDirection) const
{
	FVector ForwardDirection = OwnerActor.GetActorForwardVector();
	ForwardDirection.Z = 0.f;
	if (ForwardDirection.SizeSquared() < KINDA_SMALL_NUMBER)
	{
		return false;
	}

	OutDirection = ForwardDirection.GetSafeNormal();
	return true;
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
