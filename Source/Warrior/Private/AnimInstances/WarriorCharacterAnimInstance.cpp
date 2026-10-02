// Fill out your copyright notice in the Description page of Project Settings.



#include "AnimInstances/WarriorCharacterAnimInstance.h"
#include "Characters/WarriorBaseCharacter.h"
#include "Characters/WarriorHeroCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"
void UWarriorCharacterAnimInstance::NativeInitializeAnimation()
{
	OwningCharacter = Cast<AWarriorBaseCharacter>(TryGetPawnOwner());
	 
	if (OwningCharacter)
	{
		OwningMovementComponent = OwningCharacter->GetCharacterMovement();
	}
}
void UWarriorCharacterAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
	
	if (!OwningCharacter||!OwningMovementComponent)
	{
		return;
	}
	
	const FVector CurrentVelocity = OwningCharacter->GetVelocity();
	FVector GroundVelocity = CurrentVelocity;
	GroundVelocity.Z = 0.f;

	const FRotator ActorYawRotation(0.f, OwningCharacter->GetActorRotation().Yaw, 0.f);
	const FRotationMatrix ActorYawMatrix(ActorYawRotation);
	const FVector ForwardVector = ActorYawMatrix.GetUnitAxis(EAxis::X);
	const FVector RightVector = ActorYawMatrix.GetUnitAxis(EAxis::Y);

	GroundSpeed = GroundVelocity.Size2D();
	GroundSpeedx = FVector::DotProduct(GroundVelocity, ForwardVector);
	GroundSpeedy = FVector::DotProduct(GroundVelocity, RightVector);

	const AWarriorHeroCharacter* OwningHeroCharacter = Cast<AWarriorHeroCharacter>(OwningCharacter);
	const FVector2D MovementInput = OwningHeroCharacter
		? OwningHeroCharacter->GetCachedMovementInputVector()
		: FVector2D::ZeroVector;
	InputMoveX = MovementInput.X;
	InputMoveY = MovementInput.Y;
	bHasMovementInput = MovementInput.SizeSquared() > KINDA_SMALL_NUMBER;
	MoveStartBlendSpaceX = InputMoveX;
	MoveStartBlendSpaceY = InputMoveY;

	FVector2D TargetMoveLoopBlendSpace(GroundSpeedy, GroundSpeedx);
	if (bHasMovementInput)
	{
		FVector2D InputDirection(InputMoveX, InputMoveY);
		InputDirection.Normalize();
		TargetMoveLoopBlendSpace = InputDirection * GroundSpeed;
	}

	const float MoveLoopInterpSpeed = FMath::Max(MoveLoopBlendSpaceInterpSpeed, 0.f);
	if (MoveLoopInterpSpeed > 0.f)
	{
		MoveLoopBlendSpaceX = FMath::FInterpTo(MoveLoopBlendSpaceX, TargetMoveLoopBlendSpace.X, DeltaSeconds, MoveLoopInterpSpeed);
		MoveLoopBlendSpaceY = FMath::FInterpTo(MoveLoopBlendSpaceY, TargetMoveLoopBlendSpace.Y, DeltaSeconds, MoveLoopInterpSpeed);
	}
	else
	{
		MoveLoopBlendSpaceX = TargetMoveLoopBlendSpace.X;
		MoveLoopBlendSpaceY = TargetMoveLoopBlendSpace.Y;
	}

	if (GroundSpeed > MoveEndDirectionCacheMinSpeed)
	{
		MoveEndBlendSpaceX = GroundSpeedy;
		MoveEndBlendSpaceY = GroundSpeedx;
	}

	const float MaxBlendSpaceSpeed = FMath::Max(DirectionalBlendSpaceMaxSpeed, 1.f);
	const FVector2D LocalSpeedRange(-MaxBlendSpaceSpeed, MaxBlendSpaceSpeed);
	const FVector2D BlendSpaceRange(0.f, 100.f);
	const float TargetDirectionalBlendSpaceX = FMath::GetMappedRangeValueClamped(LocalSpeedRange, BlendSpaceRange, GroundSpeedy);
	const float TargetDirectionalBlendSpaceY = FMath::GetMappedRangeValueClamped(LocalSpeedRange, BlendSpaceRange, GroundSpeedx);
	const float BlendSpaceInterpSpeed = FMath::Max(DirectionalBlendSpaceInterpSpeed, 0.f);
	if (BlendSpaceInterpSpeed > 0.f)
	{
		DirectionalBlendSpaceX = FMath::FInterpTo(DirectionalBlendSpaceX, TargetDirectionalBlendSpaceX, DeltaSeconds, BlendSpaceInterpSpeed);
		DirectionalBlendSpaceY = FMath::FInterpTo(DirectionalBlendSpaceY, TargetDirectionalBlendSpaceY, DeltaSeconds, BlendSpaceInterpSpeed);
	}
	else
	{
		DirectionalBlendSpaceX = TargetDirectionalBlendSpaceX;
		DirectionalBlendSpaceY = TargetDirectionalBlendSpaceY;
	}

	VerticalVelocity = CurrentVelocity.Z;
	JumpCurrentCount = OwningCharacter->JumpCurrentCount;
	bIsFalling = OwningMovementComponent->IsFalling();
	
	bIsDoubleJumping = bIsFalling && JumpCurrentCount > 1 && VerticalVelocity > 0.f;
	bShouldEnterJumpState = bIsFalling && JumpCurrentCount > 0 && VerticalVelocity > 0.f;
	
	bShouldEnterFallingState = bIsFalling && VerticalVelocity <= 0.f;
	bShouldEnterLandState = !bIsFalling;
	bHasAcceleration = OwningMovementComponent->GetCurrentAcceleration().SizeSquared2D()>0.f;
	
	LocomotionDirection = UKismetAnimationLibrary::CalculateDirection(GroundVelocity,OwningCharacter->GetActorRotation());

	TurnInPlaceAngle = FMath::FindDeltaAngleDegrees(
		OwningCharacter->GetActorRotation().Yaw,
		OwningCharacter->GetBaseAimRotation().Yaw);
	const float AbsTurnInPlaceAngle = FMath::Abs(TurnInPlaceAngle);
	bShouldTurnInPlace =
		!bIsFalling &&
		!bHasAcceleration &&
		GroundSpeed <= TurnInPlaceMaxGroundSpeed &&
		AbsTurnInPlaceAngle >= TurnInPlaceMinAngle;

	if (!bShouldTurnInPlace)
	{
		TurnInPlaceDirection = EWarriorTurnInPlaceDirection::None;
	}
	else if (TurnInPlaceAngle <= -TurnInPlace180Angle)
	{
		TurnInPlaceDirection = EWarriorTurnInPlaceDirection::Left180;
	}
	else if (TurnInPlaceAngle < 0.f)
	{
		TurnInPlaceDirection = EWarriorTurnInPlaceDirection::Left90;
	}
	else if (TurnInPlaceAngle >= TurnInPlace180Angle)
	{
		TurnInPlaceDirection = EWarriorTurnInPlaceDirection::Right180;
	}
	else
	{
		TurnInPlaceDirection = EWarriorTurnInPlaceDirection::Right90;
	}
	
}
