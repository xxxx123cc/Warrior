// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/WarriorHeroGameplayAbility.h"
#include "WarriorTypes/WarriorStructTypes.h"
#include "HeroGameplayAbility_FireballBurst.generated.h"

class AWarriorProjectileBase;
class UGameplayEffect;

/**
 * Spawns a ring of projectile actors around the hero.
 */
UCLASS()
class WARRIOR_API UHeroGameplayAbility_FireballBurst : public UWarriorHeroGameplayAbility
{
	GENERATED_BODY()

public:
	UHeroGameplayAbility_FireballBurst(const FObjectInitializer& ObjectInitializer);

	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FireballBurst|Projectile")
	TSubclassOf<AWarriorProjectileBase> ProjectileClass = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FireballBurst|Projectile", meta=(ClampMin="1", UIMin="1"))
	int32 FireballCount = 12;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FireballBurst|Projectile", meta=(ClampMin="0.0"))
	float SpawnRadius = 80.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FireballBurst|Projectile")
	float SpawnHeightOffset = 80.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FireballBurst|Projectile")
	float RotationOffsetDegrees = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FireballBurst|Projectile", meta=(ClampMin="-89.0", ClampMax="89.0"))
	float PitchDegrees = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FireballBurst|Projectile")
	bool bUseOwnerForwardAsStartAngle = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FireballBurst|Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FireballBurst|Damage", meta=(ClampMin="0.0"))
	float BaseDamage = 20.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FireballBurst|Damage")
	FWarriorAttackImpactData AttackImpactData;

	UFUNCTION(BlueprintImplementableEvent, Category="FireballBurst")
	void BP_OnFireballBurstStarted(int32 SpawnedFireballCount);

private:
	FGameplayEffectSpecHandle MakeFireballDamageSpec();
	int32 SpawnFireballs(const FGameplayEffectSpecHandle& DamageSpecHandle);
	FVector GetHorizontalDirectionForIndex(int32 ProjectileIndex, int32 ProjectileCount) const;
	FVector GetSpawnLocationForDirection(const FVector& HorizontalDirection) const;
};
