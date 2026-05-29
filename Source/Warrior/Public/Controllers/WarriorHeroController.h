// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GenericTeamAgentInterface.h"
#include "WarriorHeroController.generated.h"

class UTeamManagerComponent;
class UDataAsset_TeamConfig;

/**

 */
UCLASS()
class WARRIOR_API AWarriorHeroController : public APlayerController,public IGenericTeamAgentInterface
{
	GENERATED_BODY()
public:
	AWarriorHeroController ();
	//IGenericTeamAgentInterface接口函数
	virtual FGenericTeamId GetGenericTeamId() const override;

protected:
	virtual void BeginPlay() override;

private:
	FGenericTeamId HeroTeamId ; // 默认玩家团队 ID，确保玩家角色被 AI 识别为友好目标。

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Team", meta = (AllowPrivateAccess = "true"))
	UTeamManagerComponent* TeamManagerComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Team", meta = (AllowPrivateAccess = "true"))
	UDataAsset_TeamConfig* TeamConfig;
};