// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/WarriorBaseCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameplayTagContainer.h"
#include "WarriorHeroCharacter.generated.h"
class USpringArmComponent;
class UCameraComponent;
class UDataAsset_InputConfig;
struct FInputActionValue;
class UHeroCombatComponent;
class UHeroUIComponent	;
/**
 
 */
UCLASS()
class WARRIOR_API AWarriorHeroCharacter : public AWarriorBaseCharacter
{
	GENERATED_BODY()
public:
	AWarriorHeroCharacter();
	virtual UPawnCombatComponent* GetPawnCombatComponent() const override ;
	
	virtual UPawnUIComponent* GetPawnUIComponent() const override ;
	virtual UHeroUIComponent* GetHeroUIComponent() const override ;
protected:
	//~ Begin APawn Interface
	virtual void PossessedBy(AController* NewController) override;
	//~ End APawn Interface
	
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
private:
#pragma region Components
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Camera", meta=(AllowPrivateAccess="true"))
	USpringArmComponent*CameraBoom;
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Camera", meta=(AllowPrivateAccess="true"))
	 UCameraComponent*FollowCamera;//2026.3.30-添加弹簧臂和摄影机
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Combat", meta=(AllowPrivateAccess="true"))
	UHeroCombatComponent* HeroCombatComponent;//2026.3.31-添加战斗组件
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Combat", meta=(AllowPrivateAccess="true"))
	UHeroUIComponent* HeroUIComponent;//2026-4.12添加UI组件
	
#pragma endregion//添加弹簧臂和摄影机

	
	#pragma region Inputs
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="CharacterData", meta=(AllowPrivateAccess="true"))
	UDataAsset_InputConfig* InputConfigDataAsset;
	
	void Input_Move(const FInputActionValue& InputActionValue);
	
	void Input_Look(const FInputActionValue& InputActionValue);
	
	void Input_AbilityInputPressed(FGameplayTag Input_Tag);
	
	void Input_AbilityInputReleased(FGameplayTag Input_Tag);
    #pragma endregion
	
public:
	FORCEINLINE UHeroCombatComponent* GetHeroCombatComponent() const{return HeroCombatComponent;}
	//2026.4.1-添加战斗组件的访问函数（内联函数减少调用开销）
	
	
};

