// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/GameInstance.h"
#include "WarriorGameInstance.generated.h"

USTRUCT(BlueprintType)
struct FWarriorGameLevelSet
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag LevelSetTag;
	
	UPROPERTY(EditDefaultsOnly,meta=(Categories = "GameData.Level"))
	TSoftObjectPtr<UWorld> LevelToLoad;
	
	bool IsValid() const
	{
		
		return LevelSetTag.IsValid()&&!LevelToLoad.IsNull();
		
	}
};

/**
 *
 */
UCLASS()
class WARRIOR_API UWarriorGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	TArray<FWarriorGameLevelSet> GameLevelSets;
	
	virtual void Init() override;
	
	virtual void OnPreLoadMap(const FString& MapName);
	
	virtual void OnDestinationWorldLoaded(UWorld*LoadedWorld);
	
public:
	UFUNCTION(BlueprintPure,meta = (GameplayTagFilter = "GameData.Level"))
	TSoftObjectPtr<UWorld> GetLevelToLoadFromTag(FGameplayTag InputTag)const;
	
};
