// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AbilitTask/AbilityTask_ExecuteTaskOnTick.h"


UAbilityTask_ExecuteTaskOnTick::UAbilityTask_ExecuteTaskOnTick()
{
	// 显式开启 Tick，否则 TickTask 不会被调用。
	bTickingTask = true;
}

UAbilityTask_ExecuteTaskOnTick* UAbilityTask_ExecuteTaskOnTick::ExecuteTaskOnTick(UGameplayAbility* OwningAbility)
{
	// 通过 GAS 的工厂方法创建任务实例，并挂到所属 Ability 上。
	UAbilityTask_ExecuteTaskOnTick * Node =NewAbilityTask<UAbilityTask_ExecuteTaskOnTick>(OwningAbility);
	return Node;
}

void UAbilityTask_ExecuteTaskOnTick::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);

	// 只有任务仍处于可广播状态时，才继续向外触发 Tick。
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		// 把当前帧的 DeltaTime 传给蓝图或其他绑定方。
		OnTick.Broadcast(DeltaTime);
	}
	else
	{
		// 无法继续广播时，主动结束任务，避免无效 Tick 持续运行。
		EndTask();
	}
}
