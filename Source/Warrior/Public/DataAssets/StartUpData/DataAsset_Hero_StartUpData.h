// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataAssets/StartUpData/DataAsset_StartUpDataBase.h"
#include "GameplayTagContainer.h"
#include "DataAsset_Hero_StartUpData.generated.h"

/**
 * @struct FWarriorHeroAbilitySets
 * @brief 定义英雄启动时配置的技能集合
 * 
 * 该结构体用于配置英雄初始技能的输入标签和技能类，
 * 通常用于DataAsset中定义英雄的起始技能配置
 */
USTRUCT(Blueprintable)
struct FWarriorHeroAbilitySets
{
	GENERATED_BODY()
	
	/** 输入标签，用于标识技能绑定的输入操作（限制在InputTag分类下） */
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
	
	/** 要授予的能力类，指定授予英雄的具体技能类型 */
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	TSubclassOf<UWarriorGameplayAbility> AbilityToGrant;
	
	/**
	 * 检查配置是否有效
	 * @return 如果InputTag和AbilityToGrant都有效则返回true
	 */
	bool IsValid() const;
};

/**
 * @class UDataAsset_Hero_StartUpData
 * @brief 英雄启动数据资产类
 * 
 * 该类继承自UDataAsset_StartUpDataBase，专门用于配置英雄的初始数据，
 * 包括英雄的启动技能集合。通过DataAsset方式配置，便于策划在编辑器中调整
 */
UCLASS()
class WARRIOR_API UDataAsset_Hero_StartUpData : public UDataAsset_StartUpDataBase
{
	GENERATED_BODY()

public:
	/**
	 * 将配置的启动数据应用到指定的能力系统组件
	 * @param InAscToGive 目标能力系统组件
	 * @param ApplyLevel 应用等级，默认为1
	 */
	virtual void GivenToAbilitySystemComponent(UWarriorAbilitySystemComponent* InAscToGive,int32 ApplyLevel = 1)override;

private:
	/** 英雄启动技能集合数组，配置英雄初始时获得的技能 */
	UPROPERTY(EditDefaultsOnly,Category="StartUpData",meta=(TitleProperty = "InputTag"))
	TArray<FWarriorHeroAbilitySets>HeroStartUpAbilitySets;
};
