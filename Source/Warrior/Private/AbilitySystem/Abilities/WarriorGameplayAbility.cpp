// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/WarriorGameplayAbility.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "Components/Combat/PawnCombatComponent.h"
#include "WarriorDebugHelper.h"
#include "AbilitySystemBlueprintLibrary.h"

// OnGiven 策略下，能力一旦授予即自动激活（常用于被动/初始化能力）。
void UWarriorGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);
	
	if (AbilityActivationPolicy==EwarriorAbilityActivationPolicy::OnGiven)
	{
		if (ActorInfo && !Spec.IsActive())
		ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle);
		
	}
}

void UWarriorGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	
	if (AbilityActivationPolicy==EwarriorAbilityActivationPolicy::OnGiven)
	{
		if (ActorInfo )
		{
			ActorInfo->AbilitySystemComponent->ClearAbility(Handle);
		}
		
	}
	
}

// 从 Avatar 身上快速定位战斗组件，供派生技能复用。
UPawnCombatComponent* UWarriorGameplayAbility::GetCombatComponentFromActorInfo() const
{
	return GetAvatarActorFromActorInfo()->FindComponentByClass<UPawnCombatComponent>();
	
}

// 返回项目自定义 ASC，避免每次在子类里重复 Cast。
UWarriorAbilitySystemComponent* UWarriorGameplayAbility::GetWarriorASCFromActorInfo() const
{
	return Cast<UWarriorAbilitySystemComponent>(CurrentActorInfo->AbilitySystemComponent);
	
}

// 对目标应用 GameplayEffectSpec：
// Spec 的构建由外部完成，这里仅负责执行投递。
FActiveGameplayEffectHandle UWarriorGameplayAbility::NativeApplyEffectSpecHandleToTarget(AActor* TargetActor,
	const FGameplayEffectSpecHandle& InEffectSpecHandle)
{
	check(InEffectSpecHandle.IsValid())
	check(TargetActor)
	
	
	UAbilitySystemComponent* TargetASC = TargetActor->FindComponentByClass<UAbilitySystemComponent>();
	

	FActiveGameplayEffectHandle EffectHandle = GetWarriorASCFromActorInfo()->ApplyGameplayEffectSpecToTarget(
		*InEffectSpecHandle.Data,
		TargetASC);

	return  EffectHandle;


}

FActiveGameplayEffectHandle UWarriorGameplayAbility::BP_ApplyEffectSpecHandleToTarget(AActor* TargetActor,
	const FGameplayEffectSpecHandle& InEffectSpecHandle, EWarriorSuccessful& OutSuccess)
{
	// 蓝图调用入口：返回句柄并同步给出成功/失败执行分支。
	
	FActiveGameplayEffectHandle EffectHandle= NativeApplyEffectSpecHandleToTarget(TargetActor,InEffectSpecHandle);
	OutSuccess= EffectHandle.WasSuccessfullyApplied()?EWarriorSuccessful::Successful:EWarriorSuccessful::Failed;
	return  EffectHandle;
}

// 标记连击输入已到达，供连击窗口逻辑在稍后消费。
void UWarriorGameplayAbility::OnComboInputPressed()
{
	bHasPendingComboInput = true;
}
