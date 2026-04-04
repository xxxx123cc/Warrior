// Fill out your copyright notice in the Description page of Project Settings.
//2026.3.30 创建一个基础的 GameplayAbility 类，添加一个枚举来控制技能的激活时机（OnTriggered 或 OnGiven）。在 OnGiveAbility 和 EndAbility 中根据激活策略自动尝试激活或清除技能。
#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "WarriorGameplayAbility.generated.h"

class UPawnCombatComponent;
class UWarriorAbilitySystemComponent;
UENUM(BlueprintType)//2026.3.30
enum class EwarriorAbilityActivationPolicy :uint8
{
	OnTriggered,
	OnGiven
};

/**
 * 
 */
UCLASS()
class WARRIOR_API UWarriorGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
protected:
	//~Begin UGameplayAbility Interface
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	//~ End UGameplayAbility Interface
	
	UPROPERTY(EditDefaultsOnly,category = "WarriorAbility")
	EwarriorAbilityActivationPolicy AbilityActivationPolicy= EwarriorAbilityActivationPolicy::OnTriggered;

	UFUNCTION(BlueprintPure,Category="Ability|Combat")
    UPawnCombatComponent* GetCombatComponentFromActorInfo() const;
	
	UFUNCTION(BlueprintPure,Category="Ability|Combat")
	UWarriorAbilitySystemComponent* GetWarriorASCFromActorInfo() const;

};
