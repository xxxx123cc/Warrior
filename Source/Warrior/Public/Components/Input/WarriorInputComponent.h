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
   //绑定能力输入
	template<class UserObject, typename CallbackFunc>
	void BindAbilityInputAction(const UDataAsset_InputConfig* InInputConfig, UserObject* ContextObject, CallbackFunc PressedFunc,CallbackFunc ReleasedFunc);
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

    // 绑定技能输入动作的模板函数
    // 用于批量绑定所有技能输入动作，自动处理按下和释放事件
    // @tparam UserObject 用户对象类型
    // @tparam CallbackFunc 回调函数类型
    // @param InInputConfig 输入配置数据资源，包含所有技能输入动作配置
    // @param ContextObject 执行回调的上下文对象
    // @param PressedFunc 按键按下时触发的回调函数（ETriggerEvent::Started）
    // @param ReleasedFunc 按键释放时触发的回调函数（ETriggerEvent::Completed）
    template <class UserObject, typename CallbackFunc>
    inline void UWarriorInputComponent::BindAbilityInputAction(const UDataAsset_InputConfig* InInputConfig, UserObject* ContextObject, CallbackFunc PressedFunc, CallbackFunc ReleasedFunc)
    {
    	// 确保输入配置数据资源不为空
    	checkf(InInputConfig, TEXT("Input config data asset is null, can not proceed with building"));
    	
    	// 遍历输入配置中的所有技能输入动作
    	for (const FWarriorInputActionConfig& AbilityInputActionConfig : InInputConfig->AbilityInputActions)
		{
			// 跳过无效的配置项
			if (!AbilityInputActionConfig.IsVaild())
			{
				continue;
			}
			
			// 绑定按键按下事件（Started 触发）- 当玩家开始按下按键时调用 PressedFunc
			BindAction(AbilityInputActionConfig.InputAction, ETriggerEvent::Started, ContextObject, PressedFunc, AbilityInputActionConfig.InputTag);
			
			// 绑定按键释放事件（Completed 触发）- 当玩家释放按键时调用 ReleasedFunc
			BindAction(AbilityInputActionConfig.InputAction, ETriggerEvent::Completed, ContextObject, ReleasedFunc, AbilityInputActionConfig.InputTag);
		}
    }