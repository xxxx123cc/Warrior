// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/PickUps/WarriorPickUpBase.h"
#include "WarriorStoneBase.generated.h"


class UGameplayEffect;
class UWarriorAbilitySystemComponent;
/**
 * 
 */
UCLASS()
class WARRIOR_API AWarriorStoneBase : public AWarriorPickUpBase
{
	GENERATED_BODY()
protected:
	virtual void OnPickUpMeshBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

	UPROPERTY(EditDefaultsOnly,Category="WarriorStone")
	TSubclassOf<UGameplayEffect> StoneEffectClass;

	UFUNCTION(BlueprintImplementableEvent,meta= (DisplayName="On Stones Consumed"))
    void BP_OnStonesConsumed();

public:
	void Consume(UWarriorAbilitySystemComponent*AbilitySystemComponent,int32 ApplyLevel);
	
	
	
};
