// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/WarriorBaseCharacter.h"
#include "WarriorEnemyCharacter.generated.h"

class UEnemyCombatComponent;
/**
 * 
 */
UCLASS()
class WARRIOR_API AWarriorEnemyCharacter : public AWarriorBaseCharacter
{
	GENERATED_BODY()
public:
	AWarriorEnemyCharacter();
	
protected:
	//初始化角色(采用异步加载）
	virtual void PossessedBy(AController* NewController) override;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="combat")
	UEnemyCombatComponent* EnemyCombatComponent;
private:
	void InitEnemyStartUpData();
	
	
public:
	FORCEINLINE UEnemyCombatComponent* GetEnemyCombatComponent() const { return EnemyCombatComponent; }
};
