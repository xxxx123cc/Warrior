// Fill out your copyright notice in the Description page of Project Settings.
//2026.3.30 创建一个基础的 GameplayAbility 类，添加一个枚举来控制技能的激活时机（OnTriggered 或 OnGiven）。在 OnGiveAbility 和 EndAbility 中根据激活策略自动尝试激活或清除技能。
#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "WarriorTypes/WarriorEnumTypes.h"
#include "WarriorGameplayAbility.generated.h"

class UPawnCombatComponent;
class AActor;
class UWarriorAbilitySystemComponent;
// 控制技能在被授予后的触发时机：
// - OnTriggered: 仅在外部输入/事件触发时激活
// - OnGiven: 技能授予后立即自动激活
UENUM(BlueprintType)//2026.3.30
enum class EwarriorAbilityActivationPolicy :uint8
{
	OnTriggered,
	OnGiven
};

/**
 * 战士项目的基础 GameplayAbility。
 * 提供统一的激活策略、常用组件获取方法，以及效果应用封装。
 */
UCLASS()
class WARRIOR_API UWarriorGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
protected:
	//~Begin UGameplayAbility Interface
	// 技能被授予时，根据激活策略决定是否立即尝试激活。
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	// 技能结束时，如果是 OnGiven 一次性技能，则清理该能力句柄。
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	//~ End UGameplayAbility Interface
	
	// 激活策略默认使用 OnTriggered，避免授予时自动触发。
	UPROPERTY(EditDefaultsOnly,category = "WarriorAbility")
	EwarriorAbilityActivationPolicy AbilityActivationPolicy= EwarriorAbilityActivationPolicy::OnTriggered;

	// 从当前 Avatar 上获取战斗组件，供技能读取武器/连击等战斗状态。
	UFUNCTION(BlueprintPure,Category="Ability|Combat")
    UPawnCombatComponent* GetCombatComponentFromActorInfo() const;
	
	// 获取项目自定义 ASC，便于调用扩展能力系统接口。
	UFUNCTION(BlueprintPure,Category="Ability|Combat")
	UWarriorAbilitySystemComponent* GetWarriorASCFromActorInfo() const;
	
	// C++ 侧实际执行：把已构建好的 GameplayEffectSpec 应用到目标 Actor。
	FActiveGameplayEffectHandle NativeApplyEffectSpecHandleToTarget(AActor* TargetActor,const FGameplayEffectSpecHandle& InEffectSpecHandle);
	
	// 蓝图包装：执行效果应用，并通过 OutSuccess 输出是否应用成功。
	UFUNCTION(BlueprintCallable,Category="Ability|ApplyEffect",meta = (DisplayName="ApplyEffectSpecHandleToTarget", ExpandEnumAsExecs = "OutSuccess"))
	FActiveGameplayEffectHandle BP_ApplyEffectSpecHandleToTarget(AActor* TargetActor,const FGameplayEffectSpecHandle& InEffectSpecHandle,EWarriorSuccessful& OutSuccess );
	
public:
	// 连击预输入缓存：当窗口未开启时先记下输入，窗口开启后再消费。
	UPROPERTY(BlueprintReadWrite, Category = "Warrior|Ability")
	bool bHasPendingComboInput = false;

	// 记录一次连击输入（通常由输入事件调用）。
	void OnComboInputPressed();

};
