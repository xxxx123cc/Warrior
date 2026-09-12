// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AnimInstances/WarriorBaseAnimInstance.h"
#include "WarriorCharacterAnimInstance.generated.h"

class UCharacterMovementComponent;
class AWarriorBaseCharacter;
/**
 * 
 */
UCLASS()
class WARRIOR_API UWarriorCharacterAnimInstance : public UWarriorBaseAnimInstance
{
	GENERATED_BODY()
public:
	virtual void NativeInitializeAnimation() override;
	
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;
	
protected:
	// 拥有此动画实例的角色指针
	UPROPERTY()
	AWarriorBaseCharacter* OwningCharacter;
	
	// 拥有此动画实例的角色移动组件指针
	UPROPERTY()
	UCharacterMovementComponent* OwningMovementComponent;
	
	// 角色在地面上的移动速度
	UPROPERTY(VisibleDefaultsOnly,BlueprintReadOnly,Category="AnimData|LocalMotionData")
	float GroundSpeed;
	
	UPROPERTY(VisibleDefaultsOnly,BlueprintReadOnly,Category="AnimData|LocalMotionData")
	float GroundSpeedx;
	
	UPROPERTY(VisibleDefaultsOnly,BlueprintReadOnly,Category="AnimData|LocalMotionData")
	float GroundSpeedy;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="AnimData|LocalMotionData")
	float DirectionalBlendSpaceMaxSpeed = 600.f;

	UPROPERTY(VisibleDefaultsOnly,BlueprintReadOnly,Category="AnimData|LocalMotionData")
	float DirectionalBlendSpaceX = 50.f;

	UPROPERTY(VisibleDefaultsOnly,BlueprintReadOnly,Category="AnimData|LocalMotionData")
	float DirectionalBlendSpaceY = 50.f;
     //垂直速度
	UPROPERTY(VisibleDefaultsOnly,BlueprintReadOnly,Category="AnimData|Jump")
	float VerticalVelocity;

	UPROPERTY(VisibleDefaultsOnly,BlueprintReadOnly,Category="AnimData|Jump")
	int32 JumpCurrentCount;

	UPROPERTY(VisibleDefaultsOnly,BlueprintReadOnly,Category="AnimData|Jump")
	bool bIsFalling;

	UPROPERTY(VisibleDefaultsOnly,BlueprintReadOnly,Category="AnimData|Jump")
	bool bIsDoubleJumping;

	UPROPERTY(VisibleDefaultsOnly,BlueprintReadOnly,Category="AnimData|Jump")
	bool bShouldEnterJumpState;

	UPROPERTY(VisibleDefaultsOnly,BlueprintReadOnly,Category="AnimData|Jump")
	bool bShouldEnterFallingState;

	UPROPERTY(VisibleDefaultsOnly,BlueprintReadOnly,Category="AnimData|Jump")
	bool bShouldEnterLandState;
	// 标记角色当前是否正在加速
	UPROPERTY(VisibleDefaultsOnly,BlueprintReadOnly,Category="AnimData|Locomotion")
	bool bHasAcceleration;
	
	UPROPERTY(VisibleDefaultsOnly,BlueprintReadOnly,Category="AnimData|Locomotion")
	float LocomotionDirection;
	
	UPROPERTY(VisibleDefaultsOnly,BlueprintReadOnly,Category="AnimData|Locomotion")
	bool bShouldStrafing;
	
};
