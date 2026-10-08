// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/WarriorBaseCharacter.h"
#include "Components/Widget.h"
#include "WarriorEnemyCharacter.generated.h"

class UBoxComponent;
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

	virtual void Tick(float DeltaSeconds) override;
	
	virtual UPawnCombatComponent* GetPawnCombatComponent() const override ;
	
	virtual UPawnUIComponent* GetPawnUIComponent() const override ;
	
	virtual UEnemyUIComponent* GetEnemyUIComponent() const override;
	
 #if WITH_EDITOR	
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	

protected:
	//初始化角色(采用异步加载）
	virtual void PossessedBy(AController* NewController) override;
	
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Contact")
	bool bStopMovementWhenTouchingTarget = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Contact", meta=(ClampMin="0.0", EditCondition="bStopMovementWhenTouchingTarget"))
	float TargetContactStopBuffer = 8.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Contact", meta=(ClampMin="0.0", EditCondition="bStopMovementWhenTouchingTarget"))
	float TargetContactReleaseBuffer = 24.f;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="combat")
	UEnemyCombatComponent* EnemyCombatComponent;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="UI")
	UEnemyUIComponent* EnemyUIComponent;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="UI")
	UWidgetComponent* EnemyHealthBarWidget;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Combat")
	FName LeftHandCollisionBoxBoneName;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Combat")
	FName RightHandCollisionBoxBoneName;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Collision")
	UBoxComponent* LeftBoxComponent;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Colliosn")
	UBoxComponent* RightBoxComponent;
	
	UFUNCTION()
	virtual void  OnBodyCollisionBoxBeginOverlap( UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult) ;
	
	UFUNCTION()
	virtual void OnBodyCollisionBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp ,int32 OtherBodyIndex);
	
private:
	void InitEnemyStartUpData();

	void UpdateTargetContactMovement();
	void SetTargetContactMovementHeld(bool bShouldHold);
	AActor* GetCurrentTargetActor() const;
	float GetTargetContactDistance(const AActor* TargetActor) const;

	bool bMovementHeldByTargetContact = false;
	float MaxWalkSpeedBeforeTargetContactHold = 0.f;
	
	 
	
public:
	FORCEINLINE UEnemyCombatComponent* GetEnemyCombatComponent() const { return EnemyCombatComponent; }
	FORCEINLINE UBoxComponent* GetLeftBoxComponent() const { return LeftBoxComponent; }
	FORCEINLINE UBoxComponent* GetRightBoxComponent() const { return RightBoxComponent; }
	
	
};
