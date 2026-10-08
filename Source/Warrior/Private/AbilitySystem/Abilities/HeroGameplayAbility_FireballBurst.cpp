// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/HeroGameplayAbility_FireballBurst.h"

#include "GameFramework/Pawn.h"
#include "Items/WarriorProjectileBase.h"
#include "Kismet/GameplayStatics.h"
#include "WarriorFunctionLibrary.h"
#include "WarriorGameplayTags.h"

UHeroGameplayAbility_FireballBurst::UHeroGameplayAbility_FireballBurst(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	bCancelAbilityOnInputRelease = false;

	FGameplayTagContainer FireballBurstTags;
	FireballBurstTags.AddTag(WarriorGameplayTags::Player_Ability_FireballBurst);
	SetAssetTags(FireballBurstTags);
}

bool UHeroGameplayAbility_FireballBurst::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	return ProjectileClass && DamageEffectClass;
}

void UHeroGameplayAbility_FireballBurst::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		return;
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!ProjectileClass || !DamageEffectClass)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	FGameplayEffectSpecHandle DamageSpecHandle = MakeFireballDamageSpec();
	if (!DamageSpecHandle.IsValid() || !DamageSpecHandle.Data.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const int32 SpawnedFireballCount = SpawnFireballs(DamageSpecHandle);
	BP_OnFireballBurstStarted(SpawnedFireballCount);

	EndAbility(Handle, ActorInfo, ActivationInfo, true, SpawnedFireballCount <= 0);
}

FGameplayEffectSpecHandle UHeroGameplayAbility_FireballBurst::MakeFireballDamageSpec()
{
	FGameplayEffectSpecHandle DamageSpecHandle =
		HeroDamageEffectHandle(DamageEffectClass, BaseDamage, FGameplayTag(), 0);

	UWarriorFunctionLibrary::SetAttackImpactDataToEffectSpecHandle(DamageSpecHandle, AttackImpactData);

	return DamageSpecHandle;
}

int32 UHeroGameplayAbility_FireballBurst::SpawnFireballs(const FGameplayEffectSpecHandle& DamageSpecHandle)
{
	APawn* OwningPawn = Cast<APawn>(GetAvatarActorFromActorInfo());
	UWorld* World = OwningPawn ? OwningPawn->GetWorld() : nullptr;
	if (!World || !OwningPawn || !ProjectileClass || !DamageSpecHandle.IsValid() || !DamageSpecHandle.Data.IsValid())
	{
		return 0;
	}

	const int32 ProjectileCount = FMath::Max(1, FireballCount);
	int32 SpawnedCount = 0;

	for (int32 ProjectileIndex = 0; ProjectileIndex < ProjectileCount; ++ProjectileIndex)
	{
		const FVector HorizontalDirection = GetHorizontalDirectionForIndex(ProjectileIndex, ProjectileCount);
		const FVector SpawnLocation = GetSpawnLocationForDirection(HorizontalDirection);
		FRotator SpawnRotation = HorizontalDirection.ToOrientationRotator();
		SpawnRotation.Pitch = FMath::Clamp(PitchDegrees, -89.f, 89.f);

		const FTransform SpawnTransform(SpawnRotation, SpawnLocation);
		AWarriorProjectileBase* Projectile =
			World->SpawnActorDeferred<AWarriorProjectileBase>(
				ProjectileClass,
				SpawnTransform,
				OwningPawn,
				OwningPawn,
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		if (!Projectile)
		{
			continue;
		}

		Projectile->SetProjectileDamageHandle(DamageSpecHandle);
		UGameplayStatics::FinishSpawningActor(Projectile, SpawnTransform);
		++SpawnedCount;
	}

	return SpawnedCount;
}

FVector UHeroGameplayAbility_FireballBurst::GetHorizontalDirectionForIndex(
	int32 ProjectileIndex,
	int32 ProjectileCount) const
{
	const AActor* AvatarActor = GetAvatarActorFromActorInfo();
	const float BaseYaw = bUseOwnerForwardAsStartAngle && AvatarActor ? AvatarActor->GetActorRotation().Yaw : 0.f;
	const float AngleStep = 360.f / static_cast<float>(FMath::Max(1, ProjectileCount));
	const float Yaw = BaseYaw + RotationOffsetDegrees + AngleStep * static_cast<float>(ProjectileIndex);

	return FRotator(0.f, Yaw, 0.f).Vector().GetSafeNormal();
}

FVector UHeroGameplayAbility_FireballBurst::GetSpawnLocationForDirection(const FVector& HorizontalDirection) const
{
	const AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor)
	{
		return FVector::ZeroVector;
	}

	FVector Direction = HorizontalDirection;
	Direction.Z = 0.f;
	Direction = Direction.GetSafeNormal();

	return AvatarActor->GetActorLocation() +
		Direction * FMath::Max(0.f, SpawnRadius) +
		FVector::UpVector * SpawnHeightOffset;
}
