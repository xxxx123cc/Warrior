// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AbilitTask/AbilityTask_SpawnEnemies.h"
#include "AbilitySystemComponent.h"
#include "Engine/AssetManager.h"
#include "Warrior/Public/Characters/WarriorEnemyCharacter.h"
#include "NavigationSystem.h"
UAbilityTask_SpawnEnemies* UAbilityTask_SpawnEnemies::SpawnEnemies(UGameplayAbility* OwningAbility,
	FGameplayTag EventTag, TSoftClassPtr<AWarriorEnemyCharacter> SoftEnemyClass, int32 NumToSpawn,
	const FVector& SpawnOrigin, float RandomSpawnRadius)
{
	// 创建 AbilityTask 节点实例，并把蓝图传入的参数缓存起来，等待 Activate 后使用。
	UAbilityTask_SpawnEnemies * Node =NewAbilityTask<UAbilityTask_SpawnEnemies>(OwningAbility);
	
	Node->CachedWaitForEventTag = EventTag;
	Node->CachedEnemyClassToSpawn = SoftEnemyClass;
	Node->CachedNumToSpawn = NumToSpawn;
	Node->CachedSpawnOrigin = SpawnOrigin;
	Node->CachedRandomSpawnRadius = RandomSpawnRadius;
	
	
	return Node;
	
}

void UAbilityTask_SpawnEnemies::Activate()
{
	// 监听指定 GameplayTag 对应的 GameplayEvent，事件触发后执行刷怪逻辑。
	FGameplayEventMulticastDelegate& Delegate = AbilitySystemComponent->GenericGameplayEventCallbacks.FindOrAdd(CachedWaitForEventTag);
	
	CachedDelegateHandle = Delegate.AddUObject(this, &UAbilityTask_SpawnEnemies::OnGameplayEventReceived);
}

void UAbilityTask_SpawnEnemies::OnDestroy(bool bInOwnerFinished)
{
	// 移除注册过的事件监听，防止任务销毁后仍然收到回调。
	FGameplayEventMulticastDelegate& Delegate = AbilitySystemComponent->GenericGameplayEventCallbacks.FindOrAdd(CachedWaitForEventTag);
	
	Delegate.Remove(CachedDelegateHandle);
	
	Super::OnDestroy(bInOwnerFinished);
}

void UAbilityTask_SpawnEnemies::OnGameplayEventReceived(const FGameplayEventData* InPayload)
{
	// 软类引用有效时异步加载敌人类，避免同步加载造成卡顿。
	if (ensure(!CachedEnemyClassToSpawn.IsNull()))
	{
		UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(CachedEnemyClassToSpawn.ToSoftObjectPath(), 
		FStreamableDelegate::CreateUObject(this, &ThisClass::OnEnemyLoaded));
		
	}
	else
	{
		// 没有可用的敌人类时直接通知蓝图本次生成失败。
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			DidNotSpawn.Broadcast(TArray<AWarriorEnemyCharacter*>());
		}
		EndTask();
	}
}
void UAbilityTask_SpawnEnemies::OnEnemyLoaded()
{
	// 异步加载结束后获取实际 UClass，并确认当前世界仍然有效。
	UClass* EnemyClass = CachedEnemyClassToSpawn.Get();
	UWorld* World = GetWorld();
	if (EnemyClass&&World)
	{
		// 收集成功生成的敌人，最后统一通过委托广播给蓝图。
		TArray<AWarriorEnemyCharacter*> SpawnedEnemy;
		
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		
		for (int i=1;i<=CachedNumToSpawn;i++)
		{
			// 在导航网格可到达范围内寻找随机点，避免敌人生成在不可行走区域。
			FVector SpawnLocation ;
			UNavigationSystemV1::K2_GetRandomReachablePointInRadius(this, CachedSpawnOrigin, SpawnLocation, CachedRandomSpawnRadius);
			
			// 稍微抬高生成位置，降低与地面或导航面重叠导致卡住的概率。
			SpawnLocation+=FVector(0.0f,0.0f,150.0f);
			// 使用技能拥有者当前朝向作为敌人的初始朝向。
			FRotator Rotator =AbilitySystemComponent->GetAvatarActor()->GetActorForwardVector().ToOrientationRotator();
			AWarriorEnemyCharacter* EnemyCharacter = World->SpawnActor<AWarriorEnemyCharacter>(EnemyClass, SpawnLocation, Rotator, SpawnParams);
			
			if (EnemyCharacter)
			{
				SpawnedEnemy.Add(EnemyCharacter);
			}
		}
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			// 只有至少一个敌人生成成功才广播成功，否则广播失败。
			if (!SpawnedEnemy.IsEmpty())
				OnSpawnFinished.Broadcast(SpawnedEnemy);
			else
			DidNotSpawn.Broadcast(TArray<AWarriorEnemyCharacter*>());
		}
	}
	else
	{
		// 类加载结果或世界无效时广播失败。
		if (ShouldBroadcastAbilityTaskDelegates())
		DidNotSpawn.Broadcast(TArray<AWarriorEnemyCharacter*>());
		
	}
	
	EndTask();
}
