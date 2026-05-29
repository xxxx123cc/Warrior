#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WarriorTypes/WarriorTeamTypes.h"
#include "DataAsset_TeamConfig.generated.h"

UCLASS()
class WARRIOR_API UDataAsset_TeamConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Team")
	TArray<FWarriorTeamMemberEntry> TeamMembers;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Team")
	float SwitchCooldown = 2.f;
};
