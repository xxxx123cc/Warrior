// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WarriorEnergyTypes.generated.h"

UENUM(BlueprintType)
enum class EHeroEnergyPhase : uint8
{
	PhaseOne UMETA(DisplayName="Phase One"),
	PhaseTwo UMETA(DisplayName="Phase Two")
};
