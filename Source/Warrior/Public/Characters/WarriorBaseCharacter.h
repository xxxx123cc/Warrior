// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "WarriorBaseCharacter.generated.h"


class UWarriorAbilitySystemComponent;
class UWarriorAttributeSet;
class UDataAsset_StartUpDataBase;

UCLASS()
class WARRIOR_API AWarriorBaseCharacter : public ACharacter,public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AWarriorBaseCharacter();
	//~ Begin IAbilitySystemInterface Interface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~ End IAbilitySystemInterface Interface
protected:
	//~ Begin APawn Interface
	// 说明：PossessedBy 是 APawn 的生命周期钩子（Engine 在 possession 时调用），通常不应该
	//       被外部类直接调用或作为公共 API 暴露出给游戏逻辑。因此保持 protected（或 private）
	//       是合适的，表明这是一个由引擎驱动的重载点，而非外部调用的接口。
	virtual void PossessedBy(AController* NewController) override;
	//~ End APawn Interface
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="AbilitySystem");
	UWarriorAbilitySystemComponent* WarriorAbilitySystemComponent;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="AbilitySystem");
	UWarriorAttributeSet* WarriorAttributeSet;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="StartUpData")
	TSoftObjectPtr<UDataAsset_StartUpDataBase> CharacterStartUpData;
public:
	FORCEINLINE UWarriorAbilitySystemComponent* GetWarriorAbilitySystemComponent() const { return WarriorAbilitySystemComponent; }
	FORCEINLINE UWarriorAttributeSet* GetWarriorAttributeSet() const { return WarriorAttributeSet; }
	
	
};
