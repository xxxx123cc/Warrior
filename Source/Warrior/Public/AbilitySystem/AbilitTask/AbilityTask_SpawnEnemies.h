// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_SpawnEnemies.generated.h"
class AWarriorEnemyCharacter;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSpawnEnemiesDelegate,const TArray<AWarriorEnemyCharacter*>&,SpawnEnemies);

/**
 * 
 */

UCLASS()
class WARRIOR_API UAbilityTask_SpawnEnemies : public UAbilityTask
{
	GENERATED_BODY()
	
public:
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
	
	UPROPERTY(BlueprintAssignable)
	FSpawnEnemiesDelegate OnSpawnFinished;
	UPROPERTY(BlueprintAssignable)
	FSpawnEnemiesDelegate DidNotSpawn;
	
	virtual void Activate() override;
	
	virtual void OnDestroy(bool bInOwnerFinished)override;
	
	
	
private:
	FGameplayTag CachedWaitForEventTag;
	TSoftClassPtr<AWarriorEnemyCharacter> CachedEnemyClassToSpawn;
	int32 CachedNumToSpawn;
	FVector CachedSpawnOrigin;
	float CachedRandomSpawnRadius;
	
	
	FDelegateHandle CachedDelegateHandle;
	
	void OnGameplayEventReceived(const FGameplayEventData* InPayload);
	
	void  OnEnemyLoaded();
};
