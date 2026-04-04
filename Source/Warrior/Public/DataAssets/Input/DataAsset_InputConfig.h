// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "DataAsset_InputConfig.generated.h"

class UInputAction;
class UInputMappingContext;
USTRUCT(BlueprintType)
struct FWarriorInputActionConfig
{
	GENERATED_BODY()
	//Unreal Engine 的宏，用于启用反射系统，让结构体能被蓝图和垃圾回收系统识别
public://动作资源的标签(名字)
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
	//指向输入动作资源的指针
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	UInputAction* InputAction;
	
	bool IsVaild()const
	{
		if (InputTag.IsValid()&&InputAction)
			return true	;
		return false;
	}
};

/**
 * 
 */
UCLASS()
class WARRIOR_API UDataAsset_InputConfig : public UDataAsset
{
	GENERATED_BODY()
public://3.30移动输入
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	UInputMappingContext*DefaultMappingContext;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,meta = (TitleProperty = "InputTag"))
	TArray<FWarriorInputActionConfig>NativeInputActions;
	//辅助函数
	UInputAction* FindNativeInputActionByTag(const FGameplayTag&InInputTag)const;
	//武器装备输入-4.2
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,meta=(TitleProperty = "InputTag"))
	TArray<FWarriorInputActionConfig>AbilityInputActions;
	
};
