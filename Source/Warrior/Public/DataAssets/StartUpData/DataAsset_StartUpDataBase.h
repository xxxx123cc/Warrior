// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DataAsset_StartUpDataBase.generated.h"


class UWarriorGameplayAbility;
class UWarriorAbilitySystemComponent;
class UGameplayEffect;
/**
 * 启动数据资产基类
 * 
 * 用于配置角色或实体初始化时自动授予或激活的Gameplay能力。
 * 该数据资产可以被AbilitySystemComponent使用，在角色创建或能力系统初始化时
 * 自动授予指定的能力，并可选择性地立即激活某些能力。
 * 
 * 继承自UDataAsset，可以在编辑器中创建和配置不同的启动数据资产实例。
 */
UCLASS()
class WARRIOR_API UDataAsset_StartUpDataBase : public UDataAsset
{
	GENERATED_BODY()
public:
	/**
	 * 将配置的能力授予指定的能力系统组件
	 * 此函数会在能力系统初始化时被调用，用于将配置的激活能力和反应能力
	 * 授予目标能力系统组件。
	 */
	virtual void GivenToAbilitySystemComponent(UWarriorAbilitySystemComponent* InAscToGive,int32 ApplyLevel = 1);
	
	
protected:
	/**
	 * 授予时激活的能力列表
	 * 在能力被授予时会自动激活这些能力。
	 * 适用于需要在角色初始化时立即生效的被动或主动能力。
	 */
	UPROPERTY(EditDefaultsOnly,Category="StartUpData")
	TArray<TSubclassOf<UWarriorGameplayAbility>>ActivateOnGivenAbilities;
	/**
	 * 反应型能力列表
	 * 这些能力在被授予后不会立即激活，而是等待特定条件或事件触发时才激活。
	 * 适用于需要响应特定游戏事件的反应型能力。
	 */
	UPROPERTY(EditDefaultsOnly,Category="StartUpData")
	TArray<TSubclassOf<UWarriorGameplayAbility>>ReactiveAbilities;
	/**
	 * 授予能力到指定的能力系统组件 
	 * 内部辅助函数，用于遍历能力列表并逐个授予目标能力系统组件。
	 */
	UPROPERTY(EditDefaultsOnly,Category="StartUpData")
	TArray<TSubclassOf<UGameplayEffect>> StartUpGamePlayEffects;
	
	
	
	
	void GrantAbilities(TArray<TSubclassOf<UWarriorGameplayAbility>>& InAbilitiesToGive,UWarriorAbilitySystemComponent* InAscToGive  ,int32 ApplyLevel = 1);
	


};
