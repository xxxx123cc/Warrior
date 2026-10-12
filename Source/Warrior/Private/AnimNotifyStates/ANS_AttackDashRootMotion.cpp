// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyStates/ANS_AttackDashRootMotion.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "WarriorFunctionLibrary.h"
#include "WarriorGameplayTags.h"

UANS_AttackDashRootMotion::UANS_AttackDashRootMotion()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(255, 140, 64);
#endif
}

void UANS_AttackDashRootMotion::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp || TotalDuration <= UE_SMALL_NUMBER || DashDistance <= 0.f)
	{
		return;
	}

	ACharacter* OwnerCharacter = Cast<ACharacter>(MeshComp->GetOwner());
	if (!OwnerCharacter)
	{
		return;
	}

	UCharacterMovementComponent* MovementComponent = OwnerCharacter->GetCharacterMovement();
	if (!MovementComponent)
	{
		return;
	}

	FVector DashDirection = FVector::ZeroVector;
	if (!ResolveDashDirection(*OwnerCharacter, DashDirection))
	{
		return;
	}

	RemoveActiveRootMotionSource(MeshComp);

	if (bFaceDashDirection)
	{
		FRotator DashRotation = DashDirection.ToOrientationRotator();
		DashRotation.Pitch = 0.f;
		DashRotation.Roll = 0.f;
		OwnerCharacter->SetActorRotation(DashRotation);
	}

	if (bStopMovementOnBegin)
	{
		MovementComponent->StopMovementImmediately();
	}

	const FName ResolvedSourceName = RootMotionSourceName.IsNone()
		? FName(TEXT("AttackDashRootMotion"))
		: RootMotionSourceName;
	const float DashStrength = DashDistance / TotalDuration;

	TSharedPtr<FRootMotionSource_ConstantForce> ConstantForce = MakeShared<FRootMotionSource_ConstantForce>();
	ConstantForce->InstanceName = ResolvedSourceName;
	ConstantForce->AccumulateMode = bAdditive
		? ERootMotionAccumulateMode::Additive
		: ERootMotionAccumulateMode::Override;
	ConstantForce->Priority = static_cast<uint16>(FMath::Clamp(RootMotionPriority, 0, 65535));
	ConstantForce->Force = DashDirection * DashStrength;
	ConstantForce->Duration = TotalDuration;
	ConstantForce->StrengthOverTime = StrengthOverTime;
	ConstantForce->FinishVelocityParams.Mode = VelocityOnFinishMode;
	ConstantForce->FinishVelocityParams.SetVelocity = SetVelocityOnFinish;
	ConstantForce->FinishVelocityParams.ClampVelocity = ClampVelocityOnFinish;

	if (bEnableGravity)
	{
		ConstantForce->Settings.SetFlag(ERootMotionSourceSettingsFlags::IgnoreZAccumulate);
	}

	FActiveDashRootMotion ActiveRootMotion;
	ActiveRootMotion.MovementComponent = MovementComponent;
	ActiveRootMotion.RootMotionSourceID = MovementComponent->ApplyRootMotionSource(ConstantForce);
	ActiveRootMotionSources.Add(MeshComp, ActiveRootMotion);
}

void UANS_AttackDashRootMotion::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (bRemoveRootMotionSourceOnEnd)
	{
		RemoveActiveRootMotionSource(MeshComp);
		return;
	}

	ActiveRootMotionSources.Remove(MeshComp);
}

FString UANS_AttackDashRootMotion::GetNotifyName_Implementation() const
{
	return TEXT("Attack Dash Root Motion");
}

void UANS_AttackDashRootMotion::RemoveActiveRootMotionSource(USkeletalMeshComponent* MeshComp)
{
	if (!MeshComp)
	{
		return;
	}

	FActiveDashRootMotion ActiveRootMotion;
	if (!ActiveRootMotionSources.RemoveAndCopyValue(MeshComp, ActiveRootMotion))
	{
		return;
	}

	if (UCharacterMovementComponent* MovementComponent = ActiveRootMotion.MovementComponent.Get())
	{
		if (ActiveRootMotion.RootMotionSourceID != static_cast<uint16>(ERootMotionSourceID::Invalid))
		{
			MovementComponent->RemoveRootMotionSourceByID(ActiveRootMotion.RootMotionSourceID);
		}
	}
}

bool UANS_AttackDashRootMotion::ResolveDashDirection(const AActor& OwnerActor, FVector& OutDirection) const
{
	const bool bIsTargetLocked =
		bUseControllerYawWhenTargetLocked &&
		UWarriorFunctionLibrary::NativeDoesActorHaveTag(
			const_cast<AActor*>(&OwnerActor),
			WarriorGameplayTags::Player_Status_TargetLock);

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

bool UANS_AttackDashRootMotion::GetActorForwardDirection(const AActor& OwnerActor, FVector& OutDirection) const
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

bool UANS_AttackDashRootMotion::GetMovementInputDirection(const AActor& OwnerActor, FVector& OutDirection) const
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

bool UANS_AttackDashRootMotion::GetControllerYawDirection(const AActor& OwnerActor, FVector& OutDirection) const
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
