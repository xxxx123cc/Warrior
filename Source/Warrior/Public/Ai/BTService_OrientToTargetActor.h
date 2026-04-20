
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_OrientToTargetActor.generated.h"

/**
 * 
 */
UCLASS()
class WARRIOR_API UBTService_OrientToTargetActor : public UBTService
{
	GENERATED_BODY()
	
	UBTService_OrientToTargetActor();
	
	public:
	
	virtual void  InitializeFromAsset(UBehaviorTree& Asset) override;

	virtual FString GetStaticDescription() const override;
	
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
	// 黑板中存储目标 Actor 的键
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey; 
	// 旋转速度，值越大旋转越快
	UPROPERTY(EditAnywhere, Category = "Rotation")
	float RotationSpeed ; 
	
	
};
