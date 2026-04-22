// Fill out your copyright notice in the Description page of Project Settings.


#include "Ai/BTTask_RotateToFaceTarget.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "Kismet/KismetMathLibrary.h"
#include "Warrior/Public/Controllers/WarriorAIController.h"

UBTTask_RotateToFaceTarget::UBTTask_RotateToFaceTarget()
{
	// 行为树编辑器中显示的任务名称。
	NodeName="RotateToFaceTargetActor";

	// 默认允许 10 度以内的朝向误差；旋转速度越大，转向完成越快。
	AnglePrecision=10.f;
	RotationInterSpeed=5.f;

	// 开启 Tick，让角色按插值速度逐帧转向目标，而不是瞬间转向。
	bNotifyTick = true;
	bNotifyTaskFinished = true;
	
	INIT_TASK_NODE_NOTIFY_FLAGS();
	
	// 该任务只接受 Actor 类型的黑板对象作为目标。
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, TargetActorKey), AActor::StaticClass());
}

void UBTTask_RotateToFaceTarget::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	
	// 行为树资源加载完成后，黑板键选择器需要根据当前黑板数据解析出实际 Key ID。
	if (UBlackboardData* BlackboardData = GetBlackboardAsset())
	{
		// 将编辑器中选择的黑板键解析为运行时可用的键 ID。
		TargetActorKey.ResolveSelectedKey(*BlackboardData);
	}
	
}

uint16 UBTTask_RotateToFaceTarget::GetInstanceMemorySize() const
{
	// 行为树会根据该值为任务分配节点运行时内存。
	return sizeof (FRotateToFaceTargetTaskMemory);
}

FString UBTTask_RotateToFaceTarget::GetStaticDescription() const
{
	// 在节点描述中带上目标黑板键，方便在编辑器中快速确认配置。
	const FString KeyDescription = TargetActorKey.SelectedKeyName.ToString();
	
	return  FString::Printf(TEXT(" Rotate towards to %s Key "), *KeyDescription);
	
}

EBTNodeResult::Type UBTTask_RotateToFaceTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 从黑板读取本次任务要面向的目标对象；TargetActorKey 在 InitializeFromAsset 中已经完成解析。
	UObject* TargetObject = OwnerComp.GetBlackboardComponent()->GetValueAsObject(TargetActorKey.SelectedKeyName);
	AActor* TargetActor=Cast<AActor>(TargetObject);
	
	// 行为树组件由 AIController 驱动，真正需要旋转的是 Controller 当前控制的 Pawn。
	APawn* OwnerPawn = OwnerComp.GetAIOwner()->GetPawn();
	
	// 节点实例内存用于保存 Tick 阶段还需要继续访问的 Pawn 和 Target。
	FRotateToFaceTargetTaskMemory*Memory = CastInstanceNodeMemory< FRotateToFaceTargetTaskMemory>(NodeMemory);
	check(Memory);
	
	Memory->OwnerPawn = OwnerPawn;
	Memory->TargetActor = TargetActor;
	if (!Memory->IsValid())
	{
		return EBTNodeResult::Failed;
	}

	// 如果进入任务时已经面对目标，不需要进入 Tick，直接结束任务。
	if (HasReachedAnglePercision(OwnerPawn, TargetActor))
	{
		Memory->Reset();
		return EBTNodeResult::Succeeded;
	}
	return EBTNodeResult::InProgress;
}

void UBTTask_RotateToFaceTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	// 取回 ExecuteTask 中缓存的运行时对象；任务运行过程中对象可能已经被销毁，所以每帧都要校验。
	FRotateToFaceTargetTaskMemory*Memory = CastInstanceNodeMemory< FRotateToFaceTargetTaskMemory>(NodeMemory);
	if (!Memory->IsValid())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
	
	}
	
	if (HasReachedAnglePercision(Memory->OwnerPawn.Get(), Memory->TargetActor.Get()))
	{
		Memory->Reset();
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			
	}
	else
	{
		// 计算从 Pawn 当前位置看向目标位置所需的旋转，再按插值速度平滑靠近。
		const FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(Memory->OwnerPawn->GetActorLocation(), Memory->TargetActor->GetActorLocation());
		const FRotator TargetRotation = FMath::RInterpTo(Memory->OwnerPawn->GetActorRotation(), LookAtRotation, DeltaSeconds, RotationInterSpeed);
		
		Memory->OwnerPawn->SetActorRotation(TargetRotation);
	}
}

bool UBTTask_RotateToFaceTarget::HasReachedAnglePercision(APawn* QueryPawn, AActor* TargetActor) const
{   
	// 调用方在 ExecuteTask / TickTask 中已经通过 IsValid 做过对象检查。
	// 下面保留的是只比较水平 Yaw 的实现思路，适合完全忽略目标高度差的转身判定。
	// if (!QueryPawn || !TargetActor)
	// {
	// 	return false;
	// }
	//
	// const FVector ToTarget = TargetActor->GetActorLocation() - QueryPawn->GetActorLocation();
	// if (ToTarget.IsNearlyZero())
	// {
	// 	return true;
	// }
	//
	// const FRotator TargetRotation = ToTarget.Rotation();
	// const float YawDelta = FMath::Abs(FMath::FindDeltaAngleDegrees(QueryPawn->GetActorRotation().Yaw, TargetRotation.Yaw));
	//
	// return YawDelta <= AnglePrecision;
	
	
	// 当前实现使用 Pawn 的前向向量与 Pawn 到 Target 的方向向量做点积，再通过 Acos 转成角度。
	// 这种方式计算的是三维夹角：目标在高处或低处时，Pitch 也会影响 AngleDiff。
	const FVector OwnerForwardVector = QueryPawn->GetActorForwardVector();
	const FVector OwnerToTargetVector = (TargetActor->GetActorLocation() - QueryPawn->GetActorLocation()).GetSafeNormal();

	// DotProduct 越接近 1，表示两个方向越一致；DegAcos 将点积结果转换成 0 到 180 度的夹角。
	const float DotResult = FVector::DotProduct(OwnerForwardVector, OwnerToTargetVector);
	const float AngleDiff =  UKismetMathLibrary::DegAcos(DotResult);

	// 夹角小于等于允许误差时，认为 Pawn 已经面向目标。
	return AngleDiff <=AnglePrecision;
	
}
