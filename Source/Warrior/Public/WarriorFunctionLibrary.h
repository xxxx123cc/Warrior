// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "WarriorTypes/WarriorEnumTypes.h"
#include "WarriorFunctionLibrary.generated.h"

struct FGameplayTag;
// 前向声明，避免循环依赖
class UWarriorAbilitySystemComponent;
class UPawnCombatComponent;



UCLASS()
class WARRIOR_API UWarriorFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()


public:
	/**
	 * @brief 从Actor获取WarriorAbilitySystemComponent（仅C++使用）
	 * @param InActor 目标Actor（不能为空）
	 * @return UWarriorAbilitySystemComponent* 能力系统组件指针
	 * @note 如果Actor没有ASC会触发check失败
	 */
	static UWarriorAbilitySystemComponent* NativeGetWarriorAscFromActor(AActor* InActor);

	/**
	 * @brief 如果Actor没有指定Tag，则添加该Tag
	 * @param InActor 目标Actor
	 * @param TagToAdd 要添加的GameplayTag
	 * @note 只在Actor当前不拥有该Tag时添加，避免重复
	 */
	UFUNCTION(BlueprintCallable,Category="Warrior|FunctionLibrary")
	static void AddGamePlayTagToActorIfNone(AActor* InActor, FGameplayTag TagToAdd);

	/**
	 * @brief 如果Actor拥有指定Tag，则移除该Tag
	 * @param InActor 目标Actor
	 * @param TagToRemove 要移除的GameplayTag
	 * @note 只在Actor当前拥有该Tag时移除
	 */
	UFUNCTION(BlueprintCallable,Category="Warrior|FunctionLibrary")
	static void RemoveGamePlayTagFromActorIfAny(AActor* InActor, FGameplayTag TagToRemove);

	/**
	 * @brief 检查Actor是否拥有指定的GameplayTag（仅C++使用）
	 * @param InActor 目标Actor
	 * @param TagToCheck 要检查的GameplayTag
	 * @return bool 是否拥有该Tag
	 */
	static bool NativeDoesActorHaveTag(AActor* InActor,FGameplayTag TagToCheck);

	/**
	 * @brief 检查Actor是否拥有指定的GameplayTag（Blueprint版本）
	 * @param InActor 目标Actor
	 * @param TagToCheck 要检查的GameplayTag
	 * @param OutConfirm 输出确认结果（Yes/No），用作Exec引脚
	 * @note 使用ExpandEnumAsExecs元数据将枚举转换为Blueprint的Exec引脚
	 */
	UFUNCTION(BlueprintCallable,Category="Warrior|FunctionLibrary",meta = (DisplayName="DoesActorHaveTag",ExpandEnumAsExecs = "OutConfirm"))
	static void BP_DoesActorHaveTag(AActor* InActor,FGameplayTag TagToCheck,EWarriorConfirmType& OutConfirm);

	static UPawnCombatComponent* NativeGetPawnCombatComponentFromActor(AActor* InActor);
	
	UFUNCTION(BlueprintCallable,Category="Warrior|FunctionLibrary",meta=(DisplayName="获取战斗组件",ExpandEnumAsExecs = "OutValid"))
	static  UPawnCombatComponent* BP_GetPawnCombatComponentFromActor(AActor* InActor,EWarriorValidType& OutValid);
};
