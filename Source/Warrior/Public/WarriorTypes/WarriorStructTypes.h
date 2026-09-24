// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ScalableFloat.h"
#include "WarriorStructTypes.generated.h"

class UWarriorHeroLinkedAnimLayer;
class UWarriorHeroGameplayAbility;
class UInputMappingContext;
class UTexture2D;
/**
 * 
 */
USTRUCT(BlueprintType)
struct FWarriorAttackImpactData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Defense")
	bool bCanBeBlocked = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Defense")
	bool bCanBeDodged = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Defense", meta=(ClampMin="0.0"))
	float BlockCost = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Impact", meta=(ClampMin="0.0"))
	float HorizontalLaunchStrength = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Impact", meta=(ClampMin="0.0"))
	float VerticalLaunchStrength = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Impact", meta=(ClampMin="-89.0", ClampMax="89.0"))
	float MeshTiltAngle = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Impact", meta=(ClampMin="0.0"))
	float MeshTiltDuration = 0.18f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HitReact", meta=(Categories="Shared.Status.HitReact"))
	FGameplayTagContainer HitReactTags;

	bool HasLaunchImpact() const
	{
		return !FMath::IsNearlyZero(HorizontalLaunchStrength) || !FMath::IsNearlyZero(VerticalLaunchStrength);
	}

	bool HasMeshTiltImpact() const
	{
		return !FMath::IsNearlyZero(MeshTiltAngle) && MeshTiltDuration > 0.f;
	}
};

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

USTRUCT(Blueprintable)
struct FWarriorHeroSpecialAbilitySets: public FWarriorHeroAbilitySets
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	TSoftObjectPtr<UMaterialInterface> SoftAbilityIconMaterial;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,meta=(Categories = "Player.CoolDown"))
	FGameplayTag AbilityCooldownTag;
	
};


USTRUCT(BlueprintType)
struct FWarriorHeroWeaponData
{
	GENERATED_BODY()
	//动画层
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="WeaponData")
	TSubclassOf<UWarriorHeroLinkedAnimLayer> WeaponAnimLayerToLink;
	//武器攻击上下文映射
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="WeaponData")
	UInputMappingContext* WeaponInputMappingContext;
	//武器的能力
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,meta=(TitleProperty = "InpuTag"))
	TArray<FWarriorHeroAbilitySets> DefaultWeaponAbilities;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,meta=(TitleProperty = "InpuTag"))
	TArray<FWarriorHeroSpecialAbilitySets> SpecialWeaponSpecialAbilities;
	//基础伤害，后续可以根据武器类型、英雄属性等进行调整
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="WeaponData")
	TSoftObjectPtr< UTexture2D> SoftWeaponIconTexture;
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,meta=(TitleProperty = "InpuTag"))
	FScalableFloat WeaponBaseDamage;
	
	
};
