// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "DataAssets/Input/DataAsset_InputConfig.h"
#include "WarriorInputComponent.generated.h"

/**
 * UWarriorInputComponent
 * 基于 Enhanced Input 的项目专用输入组件扩展。
 * 提供便捷方法通过自定义 DataAsset（UDataAsset_InputConfig）和 GameplayTag 查找并绑定 UInputAction。
 */
UCLASS()
class WARRIOR_API UWarriorInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()
public:
	// 通过数据资产与 GameplayTag 绑定原生输入动作到给定回调（模板以支持任意对象与成员函数指针）
	template<class UserObject, typename CallbackFunc>
	void BindNativeInputAction(const UDataAsset_InputConfig* InInputConfig, const FGameplayTag& InInputTag, ETriggerEvent TriggerEvent, UserObject* ContextObject, CallbackFunc Func);

};

template <class UserObject, typename CallbackFunc>
inline void UWarriorInputComponent::BindNativeInputAction(const UDataAsset_InputConfig* InInputConfig,
	const FGameplayTag& InInputTag, ETriggerEvent TriggerEvent, UserObject* ContextObject, CallbackFunc Func)
{
	// 确保传入的数据资产有效
	checkf(InInputConfig, TEXT("Input config data asset is null, can not proceed with building"));

	// 在数据资产中通过 GameplayTag 查找对应的原生 UInputAction
	if (UInputAction* FoundAction = InInputConfig->FindNativeInputActionByTag(InInputTag))
	{
		// 使用 Enhanced Input 提供的 BindAction 将动作与回调绑定
		BindAction(FoundAction, TriggerEvent, ContextObject, Func);
	}
}

