// Fill out your copyright notice in the Description page of Project Settings.
//2026.3.30 创建武器静态网格体，碰撞盒体
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "WarriorWeaponBase.generated.h"

class UBoxComponent;

DECLARE_DELEGATE_OneParam(FOnTargetInteractedDelegate,AActor*)


UCLASS()
class WARRIOR_API AWarriorWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWarriorWeaponBase();

	FOnTargetInteractedDelegate OnWeaponHitTarget;
	
	FOnTargetInteractedDelegate OnWeaponEndOverlapTarget;
protected:
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Weapons")
	UStaticMeshComponent* WeaponMesh;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Box")
	UBoxComponent* WeaponCollisionBox;
	UFUNCTION()
	virtual void OnWeaponOverlapBegin( UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	UFUNCTION()
 	virtual void OnWeaponOverlapEnd( UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp ,int32 OtherBodyIndex);

public:
	FORCEINLINE UBoxComponent* GetWeaponCollisionMesh() const { return WeaponCollisionBox; }

	

};
