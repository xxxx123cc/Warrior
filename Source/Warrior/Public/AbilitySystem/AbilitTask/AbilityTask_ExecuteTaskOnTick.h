// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_ExecuteTaskOnTick.generated.h"


// 每帧触发一次的动态多播委托，向外传出当前帧的 DeltaTime。
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityTaskTickDelegate,float,DeltaTime);

// 自定义 AbilityTask，用来把逐帧 Tick 能力暴露给 Ability。
// 常用于在 GameplayAbility 生命周期内执行持续逻辑。
UCLASS()
class WARRIOR_API UAbilityTask_ExecuteTaskOnTick : public UAbilityTask
{
	GENERATED_BODY()
public:
	UAbilityTask_ExecuteTaskOnTick();
	
	// 工厂函数：创建一个绑定到 OwningAbility 的 Tick Task。
	// BlueprintInternalUseOnly 表示它主要用于蓝图节点内部创建。
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility",DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"))
	static  UAbilityTask_ExecuteTaskOnTick* ExecuteTaskOnTick(UGameplayAbility* OwningAbility);
	
	// 任务激活期间由 AbilitySystem 每帧调用。
	virtual void TickTask(float DeltaTime) override;
	
	// 每帧广播一次 DeltaTime，供蓝图或其他监听方处理持续逻辑。
	UPROPERTY(BlueprintAssignable)
	FOnAbilityTaskTickDelegate OnTick;
};
