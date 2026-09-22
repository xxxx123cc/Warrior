// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/WarriorWidgetBase.h"

#include "Components/UI/EnemyUIComponent.h"
#include "Components/UI/HeroUIComponent.h"
#include "Warrior/Public/Interfaces/PawnUIInterface.h"
#include "Warrior/Public/Components/UI/PawnUIComponent.h"
#include "Characters/WarriorBaseCharacter.h"
#include "Characters/WarriorHeroCharacter.h"
#include "AbilitySystem/WarriorAttributeSet.h"

void UWarriorWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	RefreshHeroUIComponent();

	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->OnPossessedPawnChanged.AddDynamic(this, &UWarriorWidgetBase::OnControllerPossessedPawnChanged);
	}
}

void UWarriorWidgetBase::NativeDestruct()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->OnPossessedPawnChanged.RemoveDynamic(this, &UWarriorWidgetBase::OnControllerPossessedPawnChanged);
	}

	Super::NativeDestruct();
}

void UWarriorWidgetBase::RefreshHeroUIComponent()
{
	if (IPawnUIInterface* PawnUIInterface = Cast<IPawnUIInterface>(GetOwningPlayerPawn()))
	{
		if (UHeroUIComponent* HeroUIComponent = PawnUIInterface->GetHeroUIComponent())
		{
			BP_OnOwningHeroUIComponentInitialized(HeroUIComponent);

			// Broadcast current attribute values so UI shows correct state immediately
			if (AWarriorHeroCharacter* Hero = Cast<AWarriorHeroCharacter>(GetOwningPlayerPawn()))
			{
				if (const UWarriorAttributeSet* AS = Hero->GetWarriorAttributeSet())
				{
					const float MaxHealth = AS->GetMaxHealth();
					const float MaxRage = AS->GetMaxRage();
					const float MaxBlockValue = AS->GetMaxBlockValue();

					if (MaxHealth > 0.f)
					{
						PawnUIInterface->GetPawnUIComponent()->OnCurrentHealthChanged.Broadcast(
							AS->GetCurrentHealth() / MaxHealth);
					}
					if (MaxRage > 0.f)
					{
						HeroUIComponent->OnCurrentRageChanged.Broadcast(
							AS->GetCurrentRage() / MaxRage);
					}
					if (MaxBlockValue > 0.f)
					{
						HeroUIComponent->OnCurrentBlockValueChanged.Broadcast(
							AS->GetCurrentBlockValue() / MaxBlockValue);
					}
				}
			}

			BP_RefreshInitialUIValues(HeroUIComponent);
		}
	}
}

void UWarriorWidgetBase::InitEnemyWidget(AActor* EnemyActor)
{
	if (IPawnUIInterface* PawnUIInterface = Cast<IPawnUIInterface>(EnemyActor))
	{
		if (UEnemyUIComponent* EnemyUIComponent = PawnUIInterface->GetEnemyUIComponent())
		{
			BP_OnOwningEnemyUIComponentInitialized(EnemyUIComponent);

			if (AWarriorBaseCharacter* EnemyCharacter = Cast<AWarriorBaseCharacter>(EnemyActor))
			{
				if (const UWarriorAttributeSet* AS = EnemyCharacter->GetWarriorAttributeSet())
				{
					const float MaxHealth = AS->GetMaxHealth();
					const float MaxBossPoise = AS->GetMaxBossPoise();

					if (MaxHealth > 0.f)
					{
						EnemyUIComponent->OnCurrentHealthChanged.Broadcast(
							AS->GetCurrentHealth() / MaxHealth);
					}
					if (MaxBossPoise > 0.f)
					{
						EnemyUIComponent->OnCurrentBossPoiseChanged.Broadcast(
							AS->GetCurrentBossPoise() / MaxBossPoise);
					}
				}
			}
		}
	}
}

void UWarriorWidgetBase::OnControllerPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	RefreshHeroUIComponent();
}
