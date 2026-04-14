// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/PawnExtensionComponentBase.h"
#include "PawnCombatComponent.generated.h"
UENUM  (BlueprintType)
enum class EToggleDamageType :uint8
{
	CurrentEquippedWeapon,
	RightHand,
	LeftHand
	
};

class AWarriorWeaponBase;
/**
 * 
 */
UCLASS()
class WARRIOR_API UPawnCombatComponent : public UPawnExtensionComponentBase
{
	GENERATED_BODY()
private://定义一个Map来存储武器标签和对应的武器实例，已经拥有的武器-4.1
	 TMap<FGameplayTag,AWarriorWeaponBase*> CarriedWeaponsMap;
public:
	//在Map里注册已装备武器标签，是否为已装备武器初始时为false-4.1
	UFUNCTION(BlueprintCallable,Category="Warrior|Combat")
	void RegisterSpawnedWeapon(FGameplayTag InWeaponTagToRegister,AWarriorWeaponBase* InWeaponToRegister,bool bRegisterEquippedWeapon = false);
	//通过标签获取已装备武器-4.1 
	UFUNCTION(BlueprintCallable,Category="Warrior|Combat")
	AWarriorWeaponBase* GetCarriedWeaponByTag(FGameplayTag InWeaponTagToGet) const;
	//定义当前武器标签-4.1
	UPROPERTY(BlueprintReadWrite,Category="Warrior|Combat")
	FGameplayTag CurrentEquippedWeaponTag;
	//通过武器标签获取当前武器-4.1
	UFUNCTION(BlueprintCallable,Category="Warrior|Combat")
	AWarriorWeaponBase* GetCurrentEquippedWeapon() const;
	//控制武器碰撞 
	UFUNCTION(BlueprintCallable,Category="Weapon")
	void ToggleWeaponCollision(bool bEnableCollision,EToggleDamageType ToggleDamageType=EToggleDamageType::CurrentEquippedWeapon);

	virtual  void OnHitTargetActor(AActor* HitActor) ;
	
	virtual void OnWeaponEndOverlapTarget (AActor* EndOverlapActor) ;
protected:
	TArray<AActor*> OverlapActors;
	
	
};
