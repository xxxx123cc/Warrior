// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameModes/WarriorBaseGameMode.h"
#include "WarriorSurvailGameMode.generated.h"

class AWarriorEnemyCharacter;

UENUM(BlueprintType)
enum class EWarriorSurvailGameModeState : uint8
{
	WaitSpawnNewWave,
	SpawningNewWave,
	InProgress,
	WaveCompleted,
	AllWaveDone,
	PlayerDied
	
};

USTRUCT(BlueprintType)
struct FWarriorEnemyWaveSpawnerInfo
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	TSoftClassPtr<AWarriorEnemyCharacter> EnemyToSpawn;
	
	UPROPERTY(EditAnywhere)
	int32 MinPerSpawnCount =1;

	
	UPROPERTY(EditAnywhere)
	int32 MaxPerSpawnCount = 3;
	
};

USTRUCT(BlueprintType)
struct FWarriorEnemyWaveSpawnerTableRow : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	TArray<FWarriorEnemyWaveSpawnerInfo> EnemySpawners;
	
	UPROPERTY(EditAnywhere)
	int32 TotalEnemyToSpawnThisWave = 1;
	
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSurvailGameModeStateChanged, EWarriorSurvailGameModeState, CurrentState);


/**
 *
 */
UCLASS()
class WARRIOR_API AWarriorSurvailGameMode : public AWarriorBaseGameMode
{
	GENERATED_BODY()
	
protected:
	
	virtual void BeginPlay() override;
	
	virtual void Tick(float DeltaSeconds) override;
	
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	
private:	
	
	void SetCurrentState(EWarriorSurvailGameModeState InNewState);
	
	bool HasFinishedAllWaves()const;
	
	void PreLoadNextWaveEnemies();
	
	FWarriorEnemyWaveSpawnerTableRow* GetCurrentWaveSpawnerData() const;
	
	int32 TrySpawnEnemies();
	
	bool ShouldKeepSpawnEnemies()const;
	
	UFUNCTION()
	void OnEnemyDestroed(AActor* DestroyedActor);
	
	UPROPERTY()
	EWarriorSurvailGameModeState CurrentGameModeState;
	
	UPROPERTY(BlueprintAssignable,BlueprintCallable)
	FOnSurvailGameModeStateChanged OnSurvialGameModeStateChanged;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="WaveDefinition",meta = (AllowPrivateAccess="true"))
	UDataTable* EnemyWaveSpawnerDataTable;
	
	UPROPERTY(VisibleDefaultsOnly,BlueprintReadOnly,Category="WaveDefinition",meta = (AllowPrivateAccess="true"))
	int32 TotalWavesToSpawn;
	
	UPROPERTY(VisibleDefaultsOnly,BlueprintReadOnly,Category="WaveDefinition",meta = (AllowPrivateAccess="true"))
	int32 CurrentWaveIndex =1;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="WaveDefinition",meta = (AllowPrivateAccess="true"))
	float SpawnNewWaveWaitTime= 5.f;
	
	UPROPERTY()
	int32 CurrentSpawnedEnemiesCounter=0;
	
	UPROPERTY()
	int32 TotalSpawnedEnemiesThisWaveCounter=0;
	
	UPROPERTY()
	TArray<AActor*> TargetPoint;
	
	UPROPERTY()
	float TimePassedSinceStart;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="WaveDefinition",meta = (AllowPrivateAccess="true"))
	float SpawnEnemiesDelayTime = 2.f;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="WaveDefinition",meta = (AllowPrivateAccess="true"))
	float WaveCompletedWaitTime = 5.f;
	
	UPROPERTY()
	TMap<TSoftClassPtr<AWarriorEnemyCharacter>, UClass*> LoadedEnemyClassMap;
	
public:
	UFUNCTION(BlueprintCallable)
	void RegisterSpawnedEnemies(const TArray<AWarriorEnemyCharacter*>& SpawnedEnemies);
	
};
