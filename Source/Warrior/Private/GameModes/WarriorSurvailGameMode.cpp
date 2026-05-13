// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/WarriorSurvailGameMode.h"
#include "Engine/AssetManager.h"
#include "Warrior/Public/Characters/WarriorEnemyCharacter.h"
#include  "Kismet/GameplayStatics.h"
#include "Engine/TargetPoint.h"
#include "NavigationSystem.h"
#include "Items/Weapons/WarriorWeaponBase.h"
#include "WarriorFunctionLibrary.h"
void AWarriorSurvailGameMode::BeginPlay()
{
	Super::BeginPlay();
	checkf(EnemyWaveSpawnerDataTable, TEXT("EnemyWaveSpawnerDataTable is not assigned in the editor! Please assign it to a valid DataTable asset."));
	
	SetCurrentState(EWarriorSurvailGameModeState::WaitSpawnNewWave);
	
	TotalWavesToSpawn = EnemyWaveSpawnerDataTable->GetRowNames().Num();
	
	PreLoadNextWaveEnemies();
}

void AWarriorSurvailGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (CurrentGameModeState == EWarriorSurvailGameModeState::WaitSpawnNewWave)
	{
		TimePassedSinceStart+= DeltaSeconds;
		if (TimePassedSinceStart >= SpawnNewWaveWaitTime)
		{
			
			SetCurrentState(EWarriorSurvailGameModeState::SpawningNewWave);
			TimePassedSinceStart = 0.f;
		}
	}
	if (CurrentGameModeState == EWarriorSurvailGameModeState::SpawningNewWave)
	{
	TimePassedSinceStart+= DeltaSeconds;
		
		if (TimePassedSinceStart >= SpawnEnemiesDelayTime)
		{
			CurrentSpawnedEnemiesCounter += TrySpawnEnemies();
			SetCurrentState(EWarriorSurvailGameModeState::InProgress);
			TimePassedSinceStart = 0.f;
		}
		
	}
	if (CurrentGameModeState == EWarriorSurvailGameModeState::WaveCompleted)
	{
		TimePassedSinceStart+= DeltaSeconds;
		if (TimePassedSinceStart >= WaveCompletedWaitTime)
		{
			if (HasFinishedAllWaves())
			{
				SetCurrentState(EWarriorSurvailGameModeState::AllWaveDone);
			}
			else
			{
				SetCurrentState(EWarriorSurvailGameModeState::WaitSpawnNewWave);
				CurrentWaveIndex ++;
				PreLoadNextWaveEnemies();
			}
			
			TimePassedSinceStart = 0.f;
		}
		
	}
	
}

void AWarriorSurvailGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	
	WarriorDifficulty SavedGameDifficulty;
	
	UWarriorFunctionLibrary::TryLoadSaveGameDifficulty(SavedGameDifficulty);
	
	if (UWarriorFunctionLibrary::TryLoadSaveGameDifficulty(SavedGameDifficulty))
	{
		GameDifficulty = SavedGameDifficulty;
		
	}
}

void AWarriorSurvailGameMode::SetCurrentState(EWarriorSurvailGameModeState InNewState)
{
	CurrentGameModeState = InNewState;
	OnSurvialGameModeStateChanged.Broadcast(CurrentGameModeState);
}

bool AWarriorSurvailGameMode::HasFinishedAllWaves() const
{
	return CurrentWaveIndex >= TotalWavesToSpawn;
	
}

void AWarriorSurvailGameMode::PreLoadNextWaveEnemies()
{
	if (CurrentWaveIndex > TotalWavesToSpawn)
	{
		return;
	}
	LoadedEnemyClassMap.Empty();
	for (const FWarriorEnemyWaveSpawnerInfo& SpawnerInfo: GetCurrentWaveSpawnerData()->EnemySpawners)
	{
		if (SpawnerInfo.EnemyToSpawn.IsNull())
		{
			continue;
		}
		
		UAssetManager::GetStreamableManager().RequestAsyncLoad(
		SpawnerInfo.EnemyToSpawn.ToSoftObjectPath(),
		FStreamableDelegate::CreateLambda([SpawnerInfo,this]()
		{
			UClass* LoadedEnemyClass = SpawnerInfo.EnemyToSpawn.Get();
			if (LoadedEnemyClass)
			{
				LoadedEnemyClassMap.Emplace(SpawnerInfo.EnemyToSpawn, LoadedEnemyClass);
				
			}
			
		})
		);	
		
		
	}

	
}

FWarriorEnemyWaveSpawnerTableRow* AWarriorSurvailGameMode::GetCurrentWaveSpawnerData() const
{
	const FName CurrentWaveRowName = FName(TEXT("Wave") + FString::FromInt(CurrentWaveIndex));
	
	FWarriorEnemyWaveSpawnerTableRow* FoundRow = EnemyWaveSpawnerDataTable->FindRow<FWarriorEnemyWaveSpawnerTableRow>(CurrentWaveRowName,TEXT("GetCurrentWaveSpawnerData"));
	
	checkf(FoundRow,TEXT("Could not find row %s in EnemyWaveSpawnerDataTable! Please ensure the DataTable contains a row with this name."), *CurrentWaveRowName.ToString());
	
	return FoundRow;
}

int32 AWarriorSurvailGameMode::TrySpawnEnemies()
{
	if (TargetPoint.Num() == 0)
	{
		UGameplayStatics::GetAllActorsOfClass(this,ATargetPoint::StaticClass(),TargetPoint);
	}
	checkf(!TargetPoint.IsEmpty(),TEXT("No spawn points found in the level! Please ensure there are actors of class ATargetPoint placed in the level for enemy spawning."));
	
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	uint32 EnemiesSpawnedThisTime = 0;
	
	for (FWarriorEnemyWaveSpawnerInfo&WarriorEnemyWaveSpawnerInfo:GetCurrentWaveSpawnerData()->EnemySpawners)
	{
		if (WarriorEnemyWaveSpawnerInfo.EnemyToSpawn.IsNull())
		{
			continue;
		}
		const int32 NumToSpawn = FMath::RandRange(WarriorEnemyWaveSpawnerInfo.MinPerSpawnCount, WarriorEnemyWaveSpawnerInfo.MaxPerSpawnCount);
		
		UClass* LoadedClass = nullptr;
		if (UClass** FoundLoadedClass = LoadedEnemyClassMap.Find(WarriorEnemyWaveSpawnerInfo.EnemyToSpawn))
		{
			LoadedClass = *FoundLoadedClass;
		}
		else
		{
			LoadedClass = WarriorEnemyWaveSpawnerInfo.EnemyToSpawn.LoadSynchronous();
			if (LoadedClass)
			{
				LoadedEnemyClassMap.Emplace(WarriorEnemyWaveSpawnerInfo.EnemyToSpawn, LoadedClass);
			}
		}

		if (!LoadedClass)
		{
			continue;
		}
		
		for (int32 i=0; i<NumToSpawn; i++)
		{
			const int32 RandomTargetPointIndex = FMath::RandRange(0, TargetPoint.Num() - 1);
			FVector SpawnLocation = TargetPoint[RandomTargetPointIndex]->GetActorLocation();
			FRotator SpawnRotation = TargetPoint[RandomTargetPointIndex]->GetActorForwardVector().ToOrientationRotator();	
			FVector RandomLocation;
			UNavigationSystemV1::K2_GetRandomLocationInNavigableRadius(this, SpawnLocation, RandomLocation, 400.f);
			RandomLocation.Z+=150.f;
			
			const FTransform SpawnTransform(SpawnRotation, RandomLocation);
			AWarriorEnemyCharacter* EnemyCharacter = GetWorld()->SpawnActorDeferred<AWarriorEnemyCharacter>(
				LoadedClass,
				SpawnTransform,
				nullptr,
				nullptr,
				SpawnParams.SpawnCollisionHandlingOverride);

			if (EnemyCharacter)
			{
				EnemyCharacter->SetActorHiddenInGame(true);
				EnemyCharacter->FinishSpawning(SpawnTransform);
				EnemyCharacter->OnDestroyed.AddDynamic(this,&ThisClass::OnEnemyDestroed);
				EnemiesSpawnedThisTime++;
				TotalSpawnedEnemiesThisWaveCounter++;
				
			}
			if (!ShouldKeepSpawnEnemies())
				return EnemiesSpawnedThisTime;
		}
		
	}
	return EnemiesSpawnedThisTime;
}

bool AWarriorSurvailGameMode::ShouldKeepSpawnEnemies() const
{
	return TotalSpawnedEnemiesThisWaveCounter<GetCurrentWaveSpawnerData()->TotalEnemyToSpawnThisWave;
	
}

void AWarriorSurvailGameMode::OnEnemyDestroed(AActor* DestroyedActor)
{
	CurrentSpawnedEnemiesCounter --;
	
	if (ShouldKeepSpawnEnemies())
	{
		CurrentSpawnedEnemiesCounter+=TrySpawnEnemies();
	}
	
	if (CurrentSpawnedEnemiesCounter <= 0)
	{
		TotalSpawnedEnemiesThisWaveCounter = 0;
		CurrentSpawnedEnemiesCounter = 0 ;
		
		SetCurrentState(EWarriorSurvailGameModeState::WaveCompleted);
	}
}

void AWarriorSurvailGameMode::RegisterSpawnedEnemies(const TArray<AWarriorEnemyCharacter*>& SpawnedEnemies)
{
	for (AWarriorEnemyCharacter* SpawnedEnemy : SpawnedEnemies)
	{
		if (SpawnedEnemy)
		{
			CurrentSpawnedEnemiesCounter++;
			SpawnedEnemy->OnDestroyed.AddDynamic(this, &ThisClass::OnEnemyDestroed);
		}
	}
	
	
}
