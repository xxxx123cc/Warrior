// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/WarriorGameplayAbility.h"
#include "WarriorHeroGameplayAbility.generated.h"

class AWarriorHeroCharacter;
class AWarriorHeroController;
class UHeroCombatComponent;
/**
 * 
 */
UCLASS()
class WARRIOR_API UWarriorHeroGameplayAbility : public UWarriorGameplayAbility
{
	GENERATED_BODY()
	
public:
	//获取玩家角色
	UFUNCTION(BlueprintPure,Category="Warrior|Ability")
	AWarriorHeroCharacter* GetHeroCharacterFromActorInfo();
	//获取玩家控制器
	UFUNCTION(BlueprintPure,Category="Warrior|Ability")
	AWarriorHeroController* GetHeroControllerFromActorInfo();
	//获取玩家战斗组件
	UFUNCTION(BlueprintPure,Category="Warrior|Ability")
	UHeroCombatComponent* GetHeroCombatComponentFromActorInfo();
	//
	UFUNCTION(BlueprintPure,Category="Warrior|Ability")
	FGameplayEffectSpecHandle HeroDamageEffectHandle(TSubclassOf<UGameplayEffect>EffectClass,float InWeaponBaseDamage,FGameplayTag InCurrentAttackTypeTag,int32 InUsedComboCount);
	
private:
	//弱指针引用缓存角色，指向 UObject/AActor 的弱指针，不会阻止对象被 GC/销毁；对象没了它会自动变成无效（不会变成悬空野指针）。-4.1
	TWeakObjectPtr<AWarriorHeroCharacter> CachedHeroCharacter;
	//弱指针引用缓存控制器
	TWeakObjectPtr<AWarriorHeroController> CachedHeroController;
	//弱指针
	TWeakObjectPtr<UHeroCombatComponent> CacheHeroCombatComponent;
};
