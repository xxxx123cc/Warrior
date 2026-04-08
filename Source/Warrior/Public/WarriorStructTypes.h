// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "WarriorStructTypes.generated.h"


class UWarriorHeroLinkedAnimLayer;
class UWarriorHeroGameplayAbility;
class UInputMappingContext;
/**
 * 
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
	TSubclassOf<UWarriorHeroGameplayAbility> AbilityToGrant;
	
	/**
	 * 检查配置是否有效
	 * @return 如果InputTag和AbilityToGrant都有效则返回true
	 */
	bool IsValid() const;
};



USTRUCT(BlueprintType)
struct FWarriorHeroWeaponData
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="WeaponData")
	TSubclassOf<UWarriorHeroLinkedAnimLayer> WeaponAnimLayerToLink;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="WeaponData")
	UInputMappingContext* WeaponInputMappingContext;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,meta=(TitleProperty = "InpuTag"))
	TArray<FWarriorHeroAbilitySets> DefaultWeaponAbilities;
};
