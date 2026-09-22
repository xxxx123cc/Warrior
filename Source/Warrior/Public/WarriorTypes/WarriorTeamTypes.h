#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "WarriorTeamTypes.generated.h"

class AWarriorHeroCharacter;
class UTexture2D;

USTRUCT(BlueprintType)
struct FWarriorTeamMemberEntry
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Team")
	TSubclassOf<AWarriorHeroCharacter> CharacterClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Team")
	TSoftObjectPtr<UTexture2D> PortraitIcon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Team")
	FText CharacterName;
};

USTRUCT(BlueprintType)
struct FWarriorTeamMemberState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Team")
	float CurrentHealth = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Team")
	float MaxHealth = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Team")
	float CurrentRage = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Team")
	float MaxRage = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Team")
	float CurrentBlockValue = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Team")
	float MaxBlockValue = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Team")
	float AttackPower = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Team")
	float DefensePower = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Team")
	FTransform LastTransform;

	UPROPERTY(BlueprintReadOnly, Category = "Team")
	FGameplayTag EquippedWeaponTag;

	UPROPERTY(BlueprintReadOnly, Category = "Team")
	bool bIsDead = false;
};
