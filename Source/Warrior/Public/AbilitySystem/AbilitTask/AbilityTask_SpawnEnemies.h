// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_SpawnEnemies.generated.h"
class AWarriorEnemyCharacter;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSpawnEnemiesDelegate,const TArray<AWarriorEnemyCharacter*>&,SpawnEnemies);

/**
 * 等待指定 GameplayEvent 触发后，在目标范围内异步加载并生成敌人的 AbilityTask。
 */

UCLASS()
class WARRIOR_API UAbilityTask_SpawnEnemies : public UAbilityTask
{
	GENERATED_BODY()
	
public:
	// 蓝图节点入口：等待 EventTag 对应的事件，然后在 SpawnOrigin 周围随机生成指定数量的敌人。
	UFUNCTION(
		BlueprintCallable, 
		Category = "WarriorAbility|Tasks",
		meta = (
		DisplayName = "Wait Gameplay Event And Spawn Enemies",
		HidePin = "OwningAbility",
		DefaultToSelf="OwningAbility",
		BlueprintInternalUseOnly = "true",
		NumToSpawn="1",
		RadomSpawnRadius="200.0"))
	static UAbilityTask_SpawnEnemies* SpawnEnemies(UGameplayAbility* OwningAbility, 
		FGameplayTag EventTag,
		TSoftClassPtr<AWarriorEnemyCharacter> SoftEnemyClass,
		int32 NumToSpawn,const FVector&SpawnOrigin,float RandomSpawnRadius
		);
	
	// 成功生成至少一个敌人时广播，参数为本次生成的敌人数组。
	UPROPERTY(BlueprintAssignable)
	FSpawnEnemiesDelegate OnSpawnFinished;
	// 敌人类无效、加载失败或没有成功生成敌人时广播。
	UPROPERTY(BlueprintAssignable)
	FSpawnEnemiesDelegate DidNotSpawn;
	
	// AbilityTask 激活时注册 GameplayEvent 监听。
	virtual void Activate() override;
	
	// AbilityTask 销毁时移除 GameplayEvent 监听，避免委托残留。
	virtual void OnDestroy(bool bInOwnerFinished)override;
	
	
	
private:
	// 需要等待的 GameplayEvent 标签。
	FGameplayTag CachedWaitForEventTag;
	// 待生成敌人的软类引用，触发事件后异步加载。
	TSoftClassPtr<AWarriorEnemyCharacter> CachedEnemyClassToSpawn;
	// 计划生成的敌人数量。
	int32 CachedNumToSpawn;
	// 随机生成位置的中心点。
	FVector CachedSpawnOrigin;
	// 随机生成位置的半径。
	float CachedRandomSpawnRadius;
	
	
	// 注册到 GameplayEvent 委托后的句柄，用于销毁时解除绑定。
	FDelegateHandle CachedDelegateHandle;
	
	// 接收到指定 GameplayEvent 后开始异步加载敌人类。
	void OnGameplayEventReceived(const FGameplayEventData* InPayload);
	
	// 敌人类加载完成后的实际生成逻辑。
	void  OnEnemyLoaded();
};
