// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_RotateToFaceTarget.generated.h"

class UBTTaskNode;

// 行为树任务的实例内存。
// BTTaskNode 默认可能被多个 AI 共享，运行时需要通过 NodeMemory 保存本次执行相关的对象引用。
struct FRotateToFaceTargetTaskMemory
{
	// 任务 Tick 过程中缓存的运行时对象。
	// 使用弱引用可以避免任务持有对象生命周期；Pawn 或目标被销毁后会自动失效。
	TWeakObjectPtr<APawn>OwnerPawn;
	TWeakObjectPtr<AActor>TargetActor;

	// 只有拥有者 Pawn 和目标 Actor 都有效时，任务才能继续执行。
	bool IsValid()const
	{
		return OwnerPawn.IsValid() && TargetActor.IsValid();
	}

	// 任务结束或对象失效时清理弱引用。
	void Reset()
	{
		OwnerPawn.Reset();
		TargetActor.Reset();
	}

};
/**
 * 行为树任务：让拥有者 Pawn 转向黑板中指定的目标 Actor。
 *
 * 执行流程：
 * 1. ExecuteTask 从黑板读取目标 Actor，并缓存拥有者 Pawn。
 * 2. 如果当前朝向已经满足 AnglePrecision，任务立即成功。
 * 3. 否则任务进入 InProgress，在 TickTask 中按 RotationInterSpeed 持续插值旋转。
 */
UCLASS()
class WARRIOR_API UBTTask_RotateToFaceTarget : public UBTTaskNode
{
	GENERATED_BODY()
	
	// 初始化任务默认参数，并限制黑板键类型。
	UBTTask_RotateToFaceTarget();
	
	// 根据当前行为树资源解析选中的黑板键。
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	
	// 返回该任务需要的运行时内存大小。
	virtual uint16 GetInstanceMemorySize() const override;
	
	// 在行为树编辑器中显示目标黑板键描述。
	virtual FString GetStaticDescription() const override;
	
	// 行为树进入该任务时调用：读取黑板目标、初始化节点内存，并决定任务立即成功或进入 Tick。
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	// 任务处于 InProgress 时逐帧调用：持续旋转拥有者 Pawn，直到达到角度精度或对象失效。
	virtual  void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
	// 判断 QueryPawn 当前朝向与目标方向之间的夹角是否已经小于 AnglePrecision。
	// 注意：当前 cpp 实现使用 ForwardVector 与目标方向的点积计算三维夹角。
	bool HasReachedAnglePercision(APawn* QueryPawn,AActor* TargetActor) const;
	
	// 可接受的最大朝向误差，单位为度。
	UPROPERTY(EditAnywhere, Category = "Rotation To Target")
	float AnglePrecision = 5.f;

	// 朝目标旋转时使用的插值速度。
	UPROPERTY(EditAnywhere, Category = "Rotation To Target")
	float RotationInterSpeed;

	// 保存目标 Actor 的黑板键。
	UPROPERTY(EditAnywhere, Category = "Rotation To Target")
	FBlackboardKeySelector TargetActorKey;
	
};
