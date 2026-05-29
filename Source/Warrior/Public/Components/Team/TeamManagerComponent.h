#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WarriorTypes/WarriorTeamTypes.h"
#include "TeamManagerComponent.generated.h"

class UDataAsset_TeamConfig;
class AWarriorHeroCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTeamSlotSwitched, int32, NewSlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBenchHealthChanged, int32, SlotIndex, float, HealthPercent);

UCLASS(Blueprintable)
class WARRIOR_API UTeamManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTeamManagerComponent();

	void InitializeTeam(const UDataAsset_TeamConfig* TeamConfig);

	UFUNCTION(BlueprintCallable, Category = "Team")
	void SwitchToSlot(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Team")
	bool IsSlotAlive(int32 SlotIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Team")
	bool IsSwitching() const { return bIsSwitching; }

	UFUNCTION(BlueprintPure, Category = "Team")
	AWarriorHeroCharacter* GetActiveCharacter() const { return ActiveCharacter; }

	UFUNCTION(BlueprintPure, Category = "Team")
	int32 GetActiveSlotIndex() const { return ActiveSlotIndex; }

	UFUNCTION(BlueprintPure, Category = "Team")
	int32 GetTeamSize() const { return TeamSlots.Num(); }

	const FWarriorTeamMemberState& GetBenchState(int32 SlotIndex) const;

	UPROPERTY(BlueprintAssignable, Category = "Team")
	FOnTeamSlotSwitched OnTeamSlotSwitched;

	UPROPERTY(BlueprintAssignable, Category = "Team")
	FOnBenchHealthChanged OnBenchHealthChanged;

protected:
	virtual void BeginPlay() override;

private:
	void SaveCurrentCharacterState();
	void RestoreCharacterState(AWarriorHeroCharacter* Character, const FWarriorTeamMemberState& State);
	void OnActiveCharacterDied();
	int32 GetNextAliveSlot() const;
	void BindActiveCharacterDeathEvent();

	UPROPERTY()
	TArray<FWarriorTeamMemberEntry> TeamSlots;

	UPROPERTY()
	TArray<FWarriorTeamMemberState> BenchStates;

	UPROPERTY()
	int32 ActiveSlotIndex = INDEX_NONE;

	UPROPERTY()
	AWarriorHeroCharacter* ActiveCharacter = nullptr;

	UPROPERTY()
	float SwitchCooldown = 2.f;

	bool bIsSwitching = false;

	FDelegateHandle DeathTagDelegateHandle;
};
