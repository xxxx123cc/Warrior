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
public:
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
	//动作资源的标签(名字)
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	UInputAction* InputAction;//指向输入动作资源的指针
};

/**
 * 
 */
UCLASS()
class WARRIOR_API UDataAsset_InputConfig : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	UInputMappingContext*DefaultMappingContext;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,meta = (TitleProperty = "InputTag"))
	TArray<FWarriorInputActionConfig>NativeInputActions;
	
	UInputAction* FindNativeInputActionByTag(const FGameplayTag&InInputTag)const;
	
};
