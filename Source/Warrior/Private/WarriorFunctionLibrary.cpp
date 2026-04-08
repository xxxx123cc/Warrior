// Fill out your copyright notice in the Description page of Project Settings.


#include "WarriorFunctionLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Warrior/Public/AbilitySystem/WarriorAbilitySystemComponent.h"
#include "BlueprintGameplayTagLibrary.h"

/**
 * @brief 从Actor获取WarriorAbilitySystemComponent
 * @param InActor 目标Actor
 * @return UWarriorAbilitySystemComponent* Warrior自定义的能力系统组件
 * @note 使用CastChecked确保类型安全，如果获取失败会触发断言
 */
UWarriorAbilitySystemComponent* UWarriorFunctionLibrary::NativeGetWarriorAscFromActor(AActor* InActor)
{
	// 确保传入的Actor指针有效
	check(InActor);
	
	// 通过AbilitySystemBlueprintLibrary获取ASC，并强制转换为Warrior自定义类型
	return CastChecked<UWarriorAbilitySystemComponent> (UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InActor));
}

/**
 * @brief 如果Actor没有指定Tag，则添加该Tag
 * @param InActor 目标Actor
 * @param TagToAdd 要添加的GameplayTag
 */
void UWarriorFunctionLibrary::AddGamePlayTagToActorIfNone(AActor* InActor, FGameplayTag TagToAdd)
{
	// 获取Actor的ASC
	UWarriorAbilitySystemComponent* ASC = NativeGetWarriorAscFromActor(InActor);
	
	// 检查ASC有效性，并确保Actor当前不拥有该Tag
	if (ASC && !ASC->HasMatchingGameplayTag(TagToAdd))
	{
		// 添加为LooseGameplayTag（非GE授予的自由标签）
		ASC->AddLooseGameplayTag(TagToAdd);
	}
}

/**
 * @brief 如果Actor拥有指定Tag，则移除该Tag
 * @param InActor 目标Actor
 * @param TagToRemove 要移除的GameplayTag
 */
void UWarriorFunctionLibrary::RemoveGamePlayTagFromActorIfAny(AActor* InActor, FGameplayTag TagToRemove)
{
	// 获取Actor的ASC
	UWarriorAbilitySystemComponent* ASC = NativeGetWarriorAscFromActor(InActor);
	
	// 检查ASC有效性，并确保Actor当前拥有该Tag
	if (ASC && ASC->HasMatchingGameplayTag(TagToRemove))
	{
		// 移除LooseGameplayTag
		ASC->RemoveLooseGameplayTag(TagToRemove);
	}
}

/**
 * @brief 检查Actor是否拥有指定的GameplayTag（C++版本）
 * @param InActor 目标Actor
 * @param TagToCheck 要检查的GameplayTag
 * @return bool 是否拥有该Tag
 */
bool UWarriorFunctionLibrary::NativeDoesActorHaveTag(AActor* InActor, FGameplayTag TagToCheck)
{
	// 获取Actor的ASC
	UWarriorAbilitySystemComponent* ASC = NativeGetWarriorAscFromActor(InActor);
	
	// 如果ASC有效，检查是否拥有指定Tag
	if (ASC)
	{
		return ASC->HasMatchingGameplayTag(TagToCheck);
	}
	
	// ASC无效时返回false（注意：原代码缺少返回值）
	return false;
}

/**
 * @brief 检查Actor是否拥有指定的GameplayTag（Blueprint版本）
 * @param InActor 目标Actor
 * @param TagToCheck 要检查的GameplayTag
 * @param OutConfirm 输出确认结果，用于Blueprint的Exec引脚分支
 */
void UWarriorFunctionLibrary::BP_DoesActorHaveTag(AActor* InActor, FGameplayTag TagToCheck,
	EWarriorConfirmType& OutConfirm)
{
	// 根据Tag检查结果设置输出枚举值
	// 使用三元运算符：如果拥有Tag则输出Yes，否则输出No
	NativeDoesActorHaveTag(InActor, TagToCheck) ? OutConfirm = EWarriorConfirmType::Yes : OutConfirm = EWarriorConfirmType::No;
}

