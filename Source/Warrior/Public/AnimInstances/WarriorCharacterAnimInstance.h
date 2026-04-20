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
	
	// 标记角色当前是否正在加速
	UPROPERTY(VisibleDefaultsOnly,BlueprintReadOnly,Category="AnimData|Locomotion")
	bool bHasAcceleration;
	
	UPROPERTY(VisibleDefaultsOnly,BlueprintReadOnly,Category="AnimData|Locomotion")
	float LocomotionDirection;
	
	UPROPERTY(VisibleDefaultsOnly,BlueprintReadOnly,Category="AnimData|Locomotion")
	bool bShouldStrafing;
	
};
