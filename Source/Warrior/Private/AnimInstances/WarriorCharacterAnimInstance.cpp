// Fill out your copyright notice in the Description page of Project Settings.



#include "AnimInstances/WarriorCharacterAnimInstance.h"
#include "Characters/WarriorBaseCharacter.h"
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

	const float MaxBlendSpaceSpeed = FMath::Max(DirectionalBlendSpaceMaxSpeed, 1.f);
	const FVector2D LocalSpeedRange(-MaxBlendSpaceSpeed, MaxBlendSpaceSpeed);
	const FVector2D BlendSpaceRange(0.f, 100.f);
	DirectionalBlendSpaceX = FMath::GetMappedRangeValueClamped(LocalSpeedRange, BlendSpaceRange, GroundSpeedy);
	DirectionalBlendSpaceY = FMath::GetMappedRangeValueClamped(LocalSpeedRange, BlendSpaceRange, GroundSpeedx);

	VerticalVelocity = CurrentVelocity.Z;
	JumpCurrentCount = OwningCharacter->JumpCurrentCount;
	bIsFalling = OwningMovementComponent->IsFalling();
	
	bIsDoubleJumping = bIsFalling && JumpCurrentCount > 1 && VerticalVelocity > 0.f;
	bShouldEnterJumpState = bIsFalling && JumpCurrentCount > 0 && VerticalVelocity > 0.f;
	
	bShouldEnterFallingState = bIsFalling && VerticalVelocity <= 0.f;
	bShouldEnterLandState = !bIsFalling;
	bHasAcceleration = OwningMovementComponent->GetCurrentAcceleration().SizeSquared2D()>0.f;
	
	LocomotionDirection = UKismetAnimationLibrary::CalculateDirection(GroundVelocity,OwningCharacter->GetActorRotation());
	
}
