// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/WarriorHeroController.h"
#include "Components/Team/TeamManagerComponent.h"
#include "DataAssets/Team/DataAsset_TeamConfig.h"

AWarriorHeroController::AWarriorHeroController()
{
	HeroTeamId = FGenericTeamId(0); // 设置默认玩家团队 ID，确保玩家角色被 AI 识别为友好目标。

	TeamManagerComponent = CreateDefaultSubobject<UTeamManagerComponent>(TEXT("TeamManagerComponent"));
}

FGenericTeamId AWarriorHeroController::GetGenericTeamId() const
{
	return HeroTeamId;
}

void AWarriorHeroController::BeginPlay()
{
	Super::BeginPlay();

	if (TeamConfig && TeamManagerComponent)
	{
		TeamManagerComponent->InitializeTeam(TeamConfig);
	}
}
