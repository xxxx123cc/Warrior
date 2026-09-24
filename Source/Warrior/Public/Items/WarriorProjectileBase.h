// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameFramework/Actor.h"
#include "WarriorTypes/WarriorStructTypes.h"
#include "WarriorProjectileBase.generated.h"

struct FGameplayEventData;
class UNiagaraComponent;
class UBoxComponent;
class UProjectileMovementComponent;
UENUM(BlueprintType)
enum class EProjectileDamagePolicy : uint8
{
	OnHit,
	OnBeginOverlap
};


UCLASS()
class WARRIOR_API AWarriorProjectileBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWarriorProjectileBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleDefaultsOnly,BlueprintReadOnly,Category="Projectile")
	UBoxComponent* ProjectileCollisionBox;
	
	UPROPERTY(VisibleDefaultsOnly,BlueprintReadOnly,Category="Projectile")
	UNiagaraComponent* ProjectileNiagaraComponent;
	
	UPROPERTY(VisibleDefaultsOnly,BlueprintReadOnly,Category="Projectile")
	UProjectileMovementComponent* ProjectileMovementComponent;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Projectile")
	EProjectileDamagePolicy ProjectileDamagePolicy = EProjectileDamagePolicy::OnHit;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile|Block", meta=(ClampMin="0.0"))
	float SuccessfulBlockCost = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile|Attack")
	FWarriorAttackImpactData DefaultAttackImpactData;
	
	UPROPERTY(BlueprintReadOnly,Category="Projectile",meta = (ExposeOnSpawn = "true"))
	FGameplayEffectSpecHandle ProjectileDamageHandle;
	
	
	UFUNCTION()
	virtual  void OnProjectileHit( UPrimitiveComponent*HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) ;
	UFUNCTION()
	virtual void OnProjectileOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	
	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnSpawnProjectileHitFx(const FVector& HitLocation);
private:
	void HandleApplyProjectileEffect(APawn* InHitPawn,const FGameplayEventData& Data);
	FWarriorAttackImpactData ResolveAttackImpactData() const;
	bool TryHandleProjectileDefense(APawn* InHitPawn, FGameplayEventData& Data);
	
	TArray<AActor*> OverlapActors;

};
