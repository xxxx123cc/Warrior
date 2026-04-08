// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataAssets/StartUpData/DataAsset_StartUpDataBase.h"
#include "DataAsset_EnemyStartUpData.generated.h"
class UWarriorEnemyGameplayAbility;
/**
 * 
 */
UCLASS()
class WARRIOR_API UDataAsset_EnemyStartUpData : public UDataAsset_StartUpDataBase
{
	GENERATED_BODY()
	
public:
	virtual void GivenToAbilitySystemComponent(UWarriorAbilitySystemComponent* InAscToGive,int32 ApplyLevel = 1)override;
private:
	UPROPERTY(EditDefaultsOnly,Category="StartUpData",meta=(TitleProperty = "InputTag"))
	TArray<TSubclassOf< UWarriorEnemyGameplayAbility>>EnemyCombatAbilities;
	
};
