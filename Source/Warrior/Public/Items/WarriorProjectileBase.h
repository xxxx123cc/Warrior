// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameFramework/Actor.h"
#include "WarriorTypes/WarriorStructTypes.h"
#include "WarriorProjectileBase.generated.h"

struct FGameplayEventData;
class UNiagaraComponent;
class UNiagaraSystem;
class USoundBase;
class UBoxComponent;
class UProjectileMovementComponent;
UENUM(BlueprintType)
enum class EProjectileDamagePolicy : uint8
{
	OnHit,
	OnBeginOverlap
};

UENUM(BlueprintType)
enum class EProjectileMovementMode : uint8
{
	None,
	FireForward,
	DashForward
};

UCLASS()
class WARRIOR_API AWarriorProjectileBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWarriorProjectileBase();

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category="Projectile")
	void SetProjectileDamageHandle(const FGameplayEffectSpecHandle& InProjectileDamageHandle);

	UFUNCTION(BlueprintCallable, Category="Projectile|Movement")
	void FireForward(float FireSpeed = 0.f, float BaseDamageOverride = 0.f);

	UFUNCTION(BlueprintCallable, Category="Projectile|Movement")
	void DashForward(float DashDistance = 0.f, float DashSpeed = 0.f, float BaseDamageOverride = 0.f);

	UFUNCTION(BlueprintCallable, Category="Projectile|Movement")
	void StopProjectileMovement();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleDefaultsOnly,BlueprintReadOnly,Category="Projectile")
	UBoxComponent* ProjectileCollisionBox;
	
	UPROPERTY(VisibleDefaultsOnly,BlueprintReadOnly,Category="Projectile")
	UNiagaraComponent* ProjectileNiagaraComponent;
	
	UPROPERTY(VisibleDefaultsOnly,BlueprintReadOnly,Category="Projectile")
	UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile", meta=(ClampMin="0.0"))
	float ProjectileLifeSpan = 4.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile")
	bool bIgnoreWorldCollisionWhenUsingOverlapPolicy = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile|Movement")
	bool bStartStationary = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile|Movement", meta=(ClampMin="0.0"))
	float DefaultFireSpeed = 700.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile|Movement", meta=(ClampMin="0.0"))
	float DefaultDashSpeed = 1200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile|Movement", meta=(ClampMin="0.0"))
	float DefaultDashDistance = 450.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile|Movement")
	bool bStopDashWhenOverlappingEnemy = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile|DashForward")
	bool bDashForwardTargetsEnemies = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile|DashForward", meta=(ClampMin="0.0"))
	float DashForwardTargetSearchRadius = 1200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile|DashForward")
	bool bDashForwardContinuouslyTracksTarget = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile|DashForward", meta=(ClampMin="0.0"))
	float DashForwardTargetRotationInterpSpeed = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile|DashForward")
	bool bDashForwardUseHorizontalTargeting = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile|FireForward")
	bool bDestroyFireForwardOnEnemyHit = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile|Damage", meta=(ClampMin="0.0"))
	float FireForwardBaseDamageOverride = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile|Damage", meta=(ClampMin="0.0"))
	float DashForwardBaseDamageOverride = 0.f;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Projectile")
	EProjectileDamagePolicy ProjectileDamagePolicy = EProjectileDamagePolicy::OnHit;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile|Block", meta=(ClampMin="0.0"))
	float SuccessfulBlockCost = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile|Block", meta=(ClampMin="0.0"))
    bool IsDestroyedAfterOverlap=true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile|Overlap Damage")
	bool bApplyDamageRepeatedlyWhileOverlapping = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile|Overlap Damage")
	bool bApplyDamageImmediatelyOnOverlap = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile|Overlap Damage")
	bool bSendHitReactOnRepeatedOverlapDamage = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile|Overlap Damage", meta=(ClampMin="0.01"))
	float RepeatedOverlapDamageInterval = 0.35f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile|Attack")
	FWarriorAttackImpactData DefaultAttackImpactData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile|Impact")
	UNiagaraSystem* ImpactNiagaraSystem = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile|Impact")
	USoundBase* ImpactSound = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile|Impact", meta=(ClampMin="0.0"))
	float ImpactSoundVolumeMultiplier = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile|Impact", meta=(ClampMin="0.0"))
	float ImpactSoundPitchMultiplier = 1.f;
	
	UPROPERTY(BlueprintReadOnly,Category="Projectile",meta = (ExposeOnSpawn = "true"))
	FGameplayEffectSpecHandle ProjectileDamageHandle;
	
	
	UFUNCTION()
	virtual  void OnProjectileHit( UPrimitiveComponent*HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) ;
	UFUNCTION()
	virtual void OnProjectileOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION()
	virtual void OnProjectileEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnSpawnProjectileHitFx(const FVector& HitLocation);
private:
	void SpawnProjectileImpactFeedback(const FVector& HitLocation);
	bool HandleApplyProjectileEffect(APawn* InHitPawn,const FGameplayEventData& Data, bool bSendHitReact = true);
	FWarriorAttackImpactData ResolveAttackImpactData() const;
	float ResolveActiveBaseDamageOverride() const;
	void OverrideDamageHandleBaseDamageIfNeeded(FGameplayEffectSpecHandle& InOutDamageHandle) const;
	bool TryHandleProjectileDefense(APawn* InHitPawn, FGameplayEventData& Data);
	bool TryApplyProjectileOverlapDamage(APawn* InHitPawn, bool bSendHitReact);
	void TrackRepeatedOverlapPawn(APawn* InHitPawn);
	void RemoveRepeatedOverlapActor(AActor* InActor);
	void ApplyRepeatedOverlapDamage();
	void CollectCurrentOverlappingPawnsForRepeatedDamage();
	void SetProjectileVelocityForward(float Speed);
	void SetProjectileVelocityInDirection(const FVector& Direction, float Speed);
	void PrepareProjectileMovementStart(EProjectileMovementMode MovementMode);
	void HandleCurrentOverlapsAfterMovementStart();
	void UpdateDashMovement(float DeltaSeconds);
	AActor* ResolveDashTarget() const;
	AActor* ResolveLockedDashTarget() const;
	AActor* FindNearestDashTarget() const;
	bool IsValidDashTarget(AActor* CandidateActor, bool bRequireWithinSearchRadius) const;
	bool UpdateDashDirectionTowardTarget(float DeltaSeconds);
	bool IsProjectileCollisionActive() const;
	FVector ResolveDashFallbackDirection() const;
	void RefreshProjectileTickEnabled();
	bool ShouldStopDashForActor(AActor* OtherActor) const;
	bool ShouldDestroyFireForwardForActor(AActor* OtherActor) const;
	FVector ResolveImpactFeedbackLocation(const FHitResult& Hit, AActor* FallbackActor) const;
	
	TArray<AActor*> OverlapActors;
	TArray<TWeakObjectPtr<APawn>> RepeatedOverlapPawns;
	float RepeatedOverlapDamageElapsedTime = 0.f;
	bool bIsDashing = false;
	EProjectileMovementMode ActiveMovementMode = EProjectileMovementMode::None;
	FVector DashStartLocation = FVector::ZeroVector;
	FVector LastDashLocation = FVector::ZeroVector;
	FVector DashDirection = FVector::ForwardVector;
	float ActiveDashDistance = 0.f;
	float ActiveDashSpeed = 0.f;
	float ActiveDashTraveledDistance = 0.f;
	float ActiveBaseDamageOverride = 0.f;
	TWeakObjectPtr<AActor> DashTargetActor;

};
