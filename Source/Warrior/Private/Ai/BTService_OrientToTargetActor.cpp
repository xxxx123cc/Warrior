// Fill out your copyright notice in the Description page of Project Settings.


#include "Ai/BTService_OrientToTargetActor.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardData.h"
#include "Kismet/KismetMathLibrary.h"

UBTService_OrientToTargetActor::UBTService_OrientToTargetActor()
{
	NodeName = TEXT("Orient To Target Actor");
	
	INIT_SERVICE_NODE_NOTIFY_FLAGS();
	
	RotationSpeed = 5.f;
	Interval = 0.f; 
	RandomDeviation = 0.f; // 不需要随机偏差，保持朝向稳定
	//TargetActorKey 只能选择 Blackboard 中类型为 AActor/Object(AActor子类) 的 Key
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, TargetActorKey), AActor::StaticClass());
	
	
	
}

void UBTService_OrientToTargetActor::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	
	if (UBlackboardData* BlackboardData= GetBlackboardAsset())
	{
		TargetActorKey.ResolveSelectedKey(*BlackboardData);
	}
}

FString UBTService_OrientToTargetActor::GetStaticDescription() const
{
	const FString KeyDescription = TargetActorKey.SelectedKeyName.ToString();
	
	return  FString::Printf(TEXT(" Rotate towards to %s Key %s"), *KeyDescription, *GetStaticServiceDescription());
	
	 

}

void UBTService_OrientToTargetActor::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	
	UObject* ActorObject = OwnerComp.GetBlackboardComponent()->GetValueAsObject(TargetActorKey.SelectedKeyName);
	AActor* TargetActor = Cast<AActor>(ActorObject);

	APawn* OwningPawn = OwnerComp.GetAIOwner()->GetPawn();

	if (OwningPawn && TargetActor)
	{
		
		const FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(OwningPawn->GetActorLocation(), TargetActor->GetActorLocation());
		const FRotator TargetRot = FMath::RInterpTo(OwningPawn->GetActorRotation(), LookAtRot, DeltaSeconds, RotationSpeed);
		
		OwningPawn->SetActorRotation(TargetRot);
	}
}
