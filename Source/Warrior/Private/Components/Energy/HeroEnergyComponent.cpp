// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/Energy/HeroEnergyComponent.h"

#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Components/UI/HeroUIComponent.h"
#include "Interfaces/PawnUIInterface.h"
#include "WarriorGameplayTags.h"

namespace
{
	UHeroUIComponent* GetHeroUIComponentFromActor(AActor* InActor)
	{
		IPawnUIInterface* PawnUIInterface = Cast<IPawnUIInterface>(InActor);
		return PawnUIInterface ? PawnUIInterface->GetHeroUIComponent() : nullptr;
	}
}

UHeroEnergyComponent::UHeroEnergyComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UHeroEnergyComponent::BeginPlay()
{
	Super::BeginPlay();

	EnergyPerSkillPoint = EnergyPerSkillPoint > 0.f ? EnergyPerSkillPoint : MaxEnergy;
	SetEnergy(0.f);
	SetSkillPoints(0);
	SetPhase(EHeroEnergyPhase::PhaseOne);
	BroadcastAllState();
	UpdateStatusTags();
}

void UHeroEnergyComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsEmpoweredHeavyActive || DeltaTime <= 0.f || EmpoweredHeavyEnergyDrainPerSecond <= 0.f)
	{
		return;
	}

	ConsumeEnergyForEmpoweredHeavy(EmpoweredHeavyEnergyDrainPerSecond * DeltaTime);
}

void UHeroEnergyComponent::AddEnergyFromDamage(float DamageAmount)
{
	if (DamageAmount <= 0.f || EnergyGainPerDamage <= 0.f)
	{
		return;
	}

	AddEnergy(DamageAmount * EnergyGainPerDamage);
}

void UHeroEnergyComponent::AddEnergy(float EnergyAmount)
{
	if (EnergyAmount <= 0.f || CanActivateSecondUltimate())
	{
		return;
	}

	SetEnergy(CurrentEnergy + EnergyAmount);
}

void UHeroEnergyComponent::SetEnergy(float NewEnergy)
{
	const float ClampedMaxEnergy = FMath::Max(MaxEnergy, 1.f);
	const float NewClampedEnergy = FMath::Clamp(NewEnergy, 0.f, ClampedMaxEnergy);

	if (FMath::IsNearlyEqual(CurrentEnergy, NewClampedEnergy))
	{
		BroadcastReadinessChanges();
		UpdateStatusTags();
		return;
	}

	CurrentEnergy = NewClampedEnergy;
	BroadcastEnergy();
	BroadcastReadinessChanges();
	UpdateStatusTags();
}

void UHeroEnergyComponent::ResetEnergyState()
{
	StopEmpoweredHeavy();
	EnergyConvertedTowardNextSkillPoint = 0.f;
	SetSkillPoints(0);
	SetPhase(EHeroEnergyPhase::PhaseOne);
	SetEnergy(0.f);
}

bool UHeroEnergyComponent::CanActivatePhaseOneUltimate() const
{
	return CurrentPhase == EHeroEnergyPhase::PhaseOne && GetEnergyPercent() >= 1.f;
}

bool UHeroEnergyComponent::TryConsumePhaseOneUltimateEnergy()
{
	if (!CanActivatePhaseOneUltimate())
	{
		return false;
	}

	StopEmpoweredHeavy();
	SetEnergy(0.f);
	SetSkillPoints(0);
	EnergyConvertedTowardNextSkillPoint = 0.f;
	SetPhase(EHeroEnergyPhase::PhaseTwo);
	return true;
}

bool UHeroEnergyComponent::CanStartEmpoweredHeavy() const
{
	return CurrentPhase == EHeroEnergyPhase::PhaseTwo &&
		GetEnergyPercent() >= 1.f &&
		!CanActivateSecondUltimate();
}

bool UHeroEnergyComponent::TryStartEmpoweredHeavy()
{
	if (!CanStartEmpoweredHeavy())
	{
		return false;
	}

	if (!bIsEmpoweredHeavyActive)
	{
		bIsEmpoweredHeavyActive = true;
		OnEmpoweredHeavyActiveChanged.Broadcast(true);
		if (UHeroUIComponent* HeroUIComponent = GetHeroUIComponentFromActor(GetOwner()))
		{
			HeroUIComponent->OnEmpoweredHeavyActiveChanged.Broadcast(true);
		}
		UpdateStatusTags();
	}

	return true;
}

void UHeroEnergyComponent::StopEmpoweredHeavy()
{
	if (!bIsEmpoweredHeavyActive)
	{
		return;
	}

	bIsEmpoweredHeavyActive = false;
	OnEmpoweredHeavyActiveChanged.Broadcast(false);
	if (UHeroUIComponent* HeroUIComponent = GetHeroUIComponentFromActor(GetOwner()))
	{
		HeroUIComponent->OnEmpoweredHeavyActiveChanged.Broadcast(false);
	}
	UpdateStatusTags();
}

float UHeroEnergyComponent::ConsumeEnergyForEmpoweredHeavy(float EnergyToConsume)
{
	if (CurrentPhase != EHeroEnergyPhase::PhaseTwo || EnergyToConsume <= 0.f || CurrentEnergy <= 0.f)
	{
		StopEmpoweredHeavy();
		return 0.f;
	}

	const float ConsumedEnergy = FMath::Min(EnergyToConsume, CurrentEnergy);
	SetEnergy(CurrentEnergy - ConsumedEnergy);
	AddConvertedEnergyTowardSkillPoints(ConsumedEnergy);

	if (CurrentEnergy <= 0.f || CanActivateSecondUltimate())
	{
		StopEmpoweredHeavy();
	}

	return ConsumedEnergy;
}

bool UHeroEnergyComponent::CanActivateSecondUltimate() const
{
	return CurrentPhase == EHeroEnergyPhase::PhaseTwo &&
		CurrentSkillPoints >= RequiredSkillPointsForSecondUltimate;
}

bool UHeroEnergyComponent::TryConsumeSecondUltimate()
{
	if (!CanActivateSecondUltimate())
	{
		return false;
	}

	StopEmpoweredHeavy();

	if (bResetToPhaseOneAfterSecondUltimate)
	{
		ResetEnergyState();
	}
	else
	{
		SetSkillPoints(0);
		EnergyConvertedTowardNextSkillPoint = 0.f;
		SetEnergy(0.f);
	}

	return true;
}

float UHeroEnergyComponent::GetEnergyPercent() const
{
	const float ClampedMaxEnergy = FMath::Max(MaxEnergy, 1.f);
	return FMath::Clamp(CurrentEnergy / ClampedMaxEnergy, 0.f, 1.f);
}

void UHeroEnergyComponent::SetPhase(EHeroEnergyPhase NewPhase)
{
	if (CurrentPhase == NewPhase)
	{
		BroadcastReadinessChanges();
		UpdateStatusTags();
		return;
	}

	CurrentPhase = NewPhase;
	OnEnergyPhaseChanged.Broadcast(CurrentPhase);
	if (UHeroUIComponent* HeroUIComponent = GetHeroUIComponentFromActor(GetOwner()))
	{
		HeroUIComponent->OnHeroEnergyPhaseChanged.Broadcast(CurrentPhase);
	}
	BroadcastReadinessChanges();
	UpdateStatusTags();
}

void UHeroEnergyComponent::SetSkillPoints(int32 NewSkillPoints)
{
	const int32 ClampedRequiredSkillPoints = FMath::Max(RequiredSkillPointsForSecondUltimate, 1);
	const int32 NewClampedSkillPoints = FMath::Clamp(NewSkillPoints, 0, ClampedRequiredSkillPoints);

	if (CurrentSkillPoints == NewClampedSkillPoints)
	{
		BroadcastReadinessChanges();
		UpdateStatusTags();
		return;
	}

	CurrentSkillPoints = NewClampedSkillPoints;
	OnSkillPointsChanged.Broadcast(CurrentSkillPoints, ClampedRequiredSkillPoints);
	if (UHeroUIComponent* HeroUIComponent = GetHeroUIComponentFromActor(GetOwner()))
	{
		HeroUIComponent->OnHeroEnergySkillPointsChanged.Broadcast(CurrentSkillPoints, ClampedRequiredSkillPoints);
	}
	BroadcastReadinessChanges();
	UpdateStatusTags();
}

void UHeroEnergyComponent::AddConvertedEnergyTowardSkillPoints(float ConvertedEnergy)
{
	if (ConvertedEnergy <= 0.f || CanActivateSecondUltimate())
	{
		return;
	}

	const float SafeEnergyPerSkillPoint = FMath::Max(EnergyPerSkillPoint, 1.f);
	EnergyConvertedTowardNextSkillPoint += ConvertedEnergy;

	while (EnergyConvertedTowardNextSkillPoint >= SafeEnergyPerSkillPoint && !CanActivateSecondUltimate())
	{
		EnergyConvertedTowardNextSkillPoint -= SafeEnergyPerSkillPoint;
		SetSkillPoints(CurrentSkillPoints + 1);
	}
}

void UHeroEnergyComponent::BroadcastAllState()
{
	BroadcastEnergy();
	OnEnergyPhaseChanged.Broadcast(CurrentPhase);
	OnSkillPointsChanged.Broadcast(CurrentSkillPoints, FMath::Max(RequiredSkillPointsForSecondUltimate, 1));
	OnEmpoweredHeavyActiveChanged.Broadcast(bIsEmpoweredHeavyActive);
	if (UHeroUIComponent* HeroUIComponent = GetHeroUIComponentFromActor(GetOwner()))
	{
		HeroUIComponent->OnHeroEnergyPhaseChanged.Broadcast(CurrentPhase);
		HeroUIComponent->OnHeroEnergySkillPointsChanged.Broadcast(
			CurrentSkillPoints,
			FMath::Max(RequiredSkillPointsForSecondUltimate, 1));
		HeroUIComponent->OnEmpoweredHeavyActiveChanged.Broadcast(bIsEmpoweredHeavyActive);
	}
	BroadcastReadinessChanges();
}

void UHeroEnergyComponent::BroadcastEnergy()
{
	OnEnergyChanged.Broadcast(CurrentEnergy, FMath::Max(MaxEnergy, 1.f));
	OnEnergyPercentChanged.Broadcast(GetEnergyPercent());
	if (UHeroUIComponent* HeroUIComponent = GetHeroUIComponentFromActor(GetOwner()))
	{
		HeroUIComponent->OnCurrentHeroEnergyChanged.Broadcast(GetEnergyPercent());
	}
}

void UHeroEnergyComponent::BroadcastReadinessChanges()
{
	const bool bPhaseOneUltimateReady = CanActivatePhaseOneUltimate();
	const bool bEmpoweredHeavyReady = CanStartEmpoweredHeavy();
	const bool bSecondUltimateUnlocked = CanActivateSecondUltimate();

	if (bLastPhaseOneUltimateReady != bPhaseOneUltimateReady)
	{
		bLastPhaseOneUltimateReady = bPhaseOneUltimateReady;
		OnPhaseOneUltimateReadyChanged.Broadcast(bPhaseOneUltimateReady);
		if (UHeroUIComponent* HeroUIComponent = GetHeroUIComponentFromActor(GetOwner()))
		{
			HeroUIComponent->OnPhaseOneUltimateReadyChanged.Broadcast(bPhaseOneUltimateReady);
		}
	}

	if (bLastEmpoweredHeavyReady != bEmpoweredHeavyReady)
	{
		bLastEmpoweredHeavyReady = bEmpoweredHeavyReady;
		OnEmpoweredHeavyReadyChanged.Broadcast(bEmpoweredHeavyReady);
		if (UHeroUIComponent* HeroUIComponent = GetHeroUIComponentFromActor(GetOwner()))
		{
			HeroUIComponent->OnEmpoweredHeavyReadyChanged.Broadcast(bEmpoweredHeavyReady);
		}
	}

	if (bLastSecondUltimateUnlocked != bSecondUltimateUnlocked)
	{
		bLastSecondUltimateUnlocked = bSecondUltimateUnlocked;
		OnSecondUltimateUnlockedChanged.Broadcast(bSecondUltimateUnlocked);
		if (UHeroUIComponent* HeroUIComponent = GetHeroUIComponentFromActor(GetOwner()))
		{
			HeroUIComponent->OnSecondUltimateUnlockedChanged.Broadcast(bSecondUltimateUnlocked);
		}
	}
}

void UHeroEnergyComponent::UpdateStatusTags()
{
	AActor* OwnerActor = GetOwner();
	UWarriorAbilitySystemComponent* ASC =
		OwnerActor ? Cast<UWarriorAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerActor)) : nullptr;
	if (!ASC)
	{
		return;
	}

	const auto SetLooseTag = [ASC](const FGameplayTag& Tag, bool bShouldHaveTag)
	{
		if (!Tag.IsValid())
		{
			return;
		}

		if (bShouldHaveTag)
		{
			if (!ASC->HasMatchingGameplayTag(Tag))
			{
				ASC->AddLooseGameplayTag(Tag);
			}
		}
		else if (ASC->HasMatchingGameplayTag(Tag))
		{
			ASC->RemoveLooseGameplayTag(Tag);
		}
	};

	SetLooseTag(WarriorGameplayTags::Player_Status_Energy_PhaseOne, CurrentPhase == EHeroEnergyPhase::PhaseOne);
	SetLooseTag(WarriorGameplayTags::Player_Status_Energy_PhaseTwo, CurrentPhase == EHeroEnergyPhase::PhaseTwo);
	SetLooseTag(WarriorGameplayTags::Player_Status_Energy_PhaseOneUltimateReady, CanActivatePhaseOneUltimate());
	SetLooseTag(WarriorGameplayTags::Player_Status_Energy_EmpoweredHeavyReady, CanStartEmpoweredHeavy());
	SetLooseTag(WarriorGameplayTags::Player_Status_Energy_EmpoweredHeavyActive, bIsEmpoweredHeavyActive);
	SetLooseTag(WarriorGameplayTags::Player_Status_Energy_SecondUltimateUnlocked, CanActivateSecondUltimate());
}
