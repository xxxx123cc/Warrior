// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PawnExtensionComponentBase.h"
#include "WarriorTypes/WarriorEnergyTypes.h"
#include "HeroEnergyComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHeroEnergyChangedDelegate, float, CurrentEnergy, float, MaxEnergy);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHeroEnergyPercentChangedDelegate, float, CurrentEnergyPercent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHeroEnergyPhaseChangedDelegate, EHeroEnergyPhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHeroEnergySkillPointsChangedDelegate, int32, CurrentSkillPoints, int32, RequiredSkillPoints);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHeroEnergyBoolStateChangedDelegate, bool, bIsActive);

/**
 * Drives the hero's staged energy loop:
 * Phase 1 damage fills energy, first ultimate spends it and moves to phase 2.
 * Phase 2 damage fills energy, empowered heavy drains it into skill points.
 * Three skill points unlock the second ultimate.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WARRIOR_API UHeroEnergyComponent : public UPawnExtensionComponentBase
{
	GENERATED_BODY()

public:
	UHeroEnergyComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category="Warrior|HeroEnergy")
	void AddEnergyFromDamage(float DamageAmount);

	UFUNCTION(BlueprintCallable, Category="Warrior|HeroEnergy")
	void AddEnergy(float EnergyAmount);

	UFUNCTION(BlueprintCallable, Category="Warrior|HeroEnergy")
	void SetEnergy(float NewEnergy);

	UFUNCTION(BlueprintCallable, Category="Warrior|HeroEnergy")
	void ResetEnergyState();

	UFUNCTION(BlueprintPure, Category="Warrior|HeroEnergy")
	bool CanActivatePhaseOneUltimate() const;

	UFUNCTION(BlueprintCallable, Category="Warrior|HeroEnergy")
	bool TryConsumePhaseOneUltimateEnergy();

	UFUNCTION(BlueprintPure, Category="Warrior|HeroEnergy")
	bool CanStartEmpoweredHeavy() const;

	UFUNCTION(BlueprintCallable, Category="Warrior|HeroEnergy")
	bool TryStartEmpoweredHeavy();

	UFUNCTION(BlueprintCallable, Category="Warrior|HeroEnergy")
	void StopEmpoweredHeavy();

	UFUNCTION(BlueprintCallable, Category="Warrior|HeroEnergy")
	float ConsumeEnergyForEmpoweredHeavy(float EnergyToConsume);

	UFUNCTION(BlueprintPure, Category="Warrior|HeroEnergy")
	bool CanActivateSecondUltimate() const;

	UFUNCTION(BlueprintCallable, Category="Warrior|HeroEnergy")
	bool TryConsumeSecondUltimate();

	UFUNCTION(BlueprintPure, Category="Warrior|HeroEnergy")
	float GetCurrentEnergy() const { return CurrentEnergy; }

	UFUNCTION(BlueprintPure, Category="Warrior|HeroEnergy")
	float GetMaxEnergy() const { return MaxEnergy; }

	UFUNCTION(BlueprintPure, Category="Warrior|HeroEnergy")
	float GetEnergyPercent() const;

	UFUNCTION(BlueprintPure, Category="Warrior|HeroEnergy")
	EHeroEnergyPhase GetEnergyPhase() const { return CurrentPhase; }

	UFUNCTION(BlueprintPure, Category="Warrior|HeroEnergy")
	int32 GetCurrentSkillPoints() const { return CurrentSkillPoints; }

	UFUNCTION(BlueprintPure, Category="Warrior|HeroEnergy")
	int32 GetRequiredSkillPoints() const { return RequiredSkillPointsForSecondUltimate; }

	UFUNCTION(BlueprintPure, Category="Warrior|HeroEnergy")
	bool IsEmpoweredHeavyActive() const { return bIsEmpoweredHeavyActive; }

	UPROPERTY(BlueprintAssignable, Category="Warrior|HeroEnergy")
	FOnHeroEnergyChangedDelegate OnEnergyChanged;

	UPROPERTY(BlueprintAssignable, Category="Warrior|HeroEnergy")
	FOnHeroEnergyPercentChangedDelegate OnEnergyPercentChanged;

	UPROPERTY(BlueprintAssignable, Category="Warrior|HeroEnergy")
	FOnHeroEnergyPhaseChangedDelegate OnEnergyPhaseChanged;

	UPROPERTY(BlueprintAssignable, Category="Warrior|HeroEnergy")
	FOnHeroEnergySkillPointsChangedDelegate OnSkillPointsChanged;

	UPROPERTY(BlueprintAssignable, Category="Warrior|HeroEnergy")
	FOnHeroEnergyBoolStateChangedDelegate OnPhaseOneUltimateReadyChanged;

	UPROPERTY(BlueprintAssignable, Category="Warrior|HeroEnergy")
	FOnHeroEnergyBoolStateChangedDelegate OnEmpoweredHeavyReadyChanged;

	UPROPERTY(BlueprintAssignable, Category="Warrior|HeroEnergy")
	FOnHeroEnergyBoolStateChangedDelegate OnEmpoweredHeavyActiveChanged;

	UPROPERTY(BlueprintAssignable, Category="Warrior|HeroEnergy")
	FOnHeroEnergyBoolStateChangedDelegate OnSecondUltimateUnlockedChanged;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Warrior|HeroEnergy", meta=(ClampMin="1.0"))
	float MaxEnergy = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Warrior|HeroEnergy", meta=(ClampMin="0.0"))
	float EnergyGainPerDamage = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Warrior|HeroEnergy", meta=(ClampMin="1"))
	int32 RequiredSkillPointsForSecondUltimate = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Warrior|HeroEnergy", meta=(ClampMin="0.0"))
	float EmpoweredHeavyEnergyDrainPerSecond = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Warrior|HeroEnergy", meta=(ClampMin="1.0"))
	float EnergyPerSkillPoint = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Warrior|HeroEnergy")
	bool bResetToPhaseOneAfterSecondUltimate = true;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Warrior|HeroEnergy")
	float CurrentEnergy = 0.f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Warrior|HeroEnergy")
	EHeroEnergyPhase CurrentPhase = EHeroEnergyPhase::PhaseOne;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Warrior|HeroEnergy")
	int32 CurrentSkillPoints = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Warrior|HeroEnergy")
	float EnergyConvertedTowardNextSkillPoint = 0.f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Warrior|HeroEnergy")
	bool bIsEmpoweredHeavyActive = false;

private:
	void SetPhase(EHeroEnergyPhase NewPhase);
	void SetSkillPoints(int32 NewSkillPoints);
	void AddConvertedEnergyTowardSkillPoints(float ConvertedEnergy);
	void BroadcastAllState();
	void BroadcastEnergy();
	void BroadcastReadinessChanges();
	void UpdateStatusTags();

	bool bLastPhaseOneUltimateReady = false;
	bool bLastEmpoweredHeavyReady = false;
	bool bLastSecondUltimateUnlocked = false;
};
