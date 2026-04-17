// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/WarriorBaseCharacter.h"
#include "Components/Widget.h"
#include "WarriorEnemyCharacter.generated.h"

class UEnemyCombatComponent;
class UWidgetComponent;
/**
 * 
 */
UCLASS()
class WARRIOR_API AWarriorEnemyCharacter : public AWarriorBaseCharacter
{
	GENERATED_BODY()
public:
	AWarriorEnemyCharacter();
	
	virtual UPawnCombatComponent* GetPawnCombatComponent() const override ;
	
	virtual UPawnUIComponent* GetPawnUIComponent() const override ;
	
	virtual UEnemyUIComponent* GetEnemyUIComponent() const override;
protected:
	//初始化角色(采用异步加载）
	virtual void PossessedBy(AController* NewController) override;
	
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="combat")
	UEnemyCombatComponent* EnemyCombatComponent;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="UI")
	UEnemyUIComponent* EnemyUIComponent;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="UI")
	UWidgetComponent* EnemyHealthBarWidget;
	
private:
	void InitEnemyStartUpData();
	
	 
	
public:
	FORCEINLINE UEnemyCombatComponent* GetEnemyCombatComponent() const { return EnemyCombatComponent; }
};
