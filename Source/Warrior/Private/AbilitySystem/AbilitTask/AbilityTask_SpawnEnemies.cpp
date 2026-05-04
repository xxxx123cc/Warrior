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
	
	FGameplayEventMulticastDelegate& Delegate = AbilitySystemComponent->GenericGameplayEventCallbacks.FindOrAdd(CachedWaitForEventTag);
	
	CachedDelegateHandle = Delegate.AddUObject(this, &UAbilityTask_SpawnEnemies::OnGameplayEventReceived);
}

void UAbilityTask_SpawnEnemies::OnDestroy(bool bInOwnerFinished)
{
	FGameplayEventMulticastDelegate& Delegate = AbilitySystemComponent->GenericGameplayEventCallbacks.FindOrAdd(CachedWaitForEventTag);
	
	Delegate.Remove(CachedDelegateHandle);
	
	Super::OnDestroy(bInOwnerFinished);
}

void UAbilityTask_SpawnEnemies::OnGameplayEventReceived(const FGameplayEventData* InPayload)
{
	if (ensure(!CachedEnemyClassToSpawn.IsNull()))
	{
		UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(CachedEnemyClassToSpawn.ToSoftObjectPath(), 
		FStreamableDelegate::CreateUObject(this, &ThisClass::OnEnemyLoaded));
		
	}
	else
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			DidNotSpawn.Broadcast(TArray<AWarriorEnemyCharacter*>());
		}
		EndTask();
	}
}
void UAbilityTask_SpawnEnemies::OnEnemyLoaded()
{
	UClass* EnemyClass = CachedEnemyClassToSpawn.Get();
	UWorld* World = GetWorld();
	if (EnemyClass&&World)
	{
		TArray<AWarriorEnemyCharacter*> SpawnedEnemy;
		
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		
		for (int i=1;i<=CachedNumToSpawn;i++)
		{
			FVector SpawnLocation ;
			UNavigationSystemV1::K2_GetRandomReachablePointInRadius(this, CachedSpawnOrigin, SpawnLocation, CachedRandomSpawnRadius);
			
			SpawnLocation+=FVector(0.0f,0.0f,150.0f);
			FRotator Rotator =AbilitySystemComponent->GetAvatarActor()->GetActorForwardVector().ToOrientationRotator();
			AWarriorEnemyCharacter* EnemyCharacter = World->SpawnActor<AWarriorEnemyCharacter>(EnemyClass, SpawnLocation, Rotator, SpawnParams);
			
			if (EnemyCharacter)
			{
				SpawnedEnemy.Add(EnemyCharacter);
			}
		}
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			if (!SpawnedEnemy.IsEmpty())
				OnSpawnFinished.Broadcast(SpawnedEnemy);
			else
			DidNotSpawn.Broadcast(TArray<AWarriorEnemyCharacter*>());
		}
	}
	else
	{if (ShouldBroadcastAbilityTaskDelegates())
		DidNotSpawn.Broadcast(TArray<AWarriorEnemyCharacter*>());
		
	}
	
	EndTask();
}
