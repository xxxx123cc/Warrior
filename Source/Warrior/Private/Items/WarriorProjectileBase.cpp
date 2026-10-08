// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/WarriorProjectileBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffect.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraComponent.h"
#include "WarriorFunctionLibrary.h"
#include "Warrior/Public/WarriorGameplayTags.h"

AWarriorProjectileBase::AWarriorProjectileBase()
{
	PrimaryActorTick.bCanEverTick = false;
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ProjectileCollisionBox = CreateDefaultSubobject<UBoxComponent>(FName("BoxComponent"));
	SetRootComponent(ProjectileCollisionBox);
	ProjectileCollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ProjectileCollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	ProjectileCollisionBox->SetCollisionResponseToChannel(ECC_Pawn,ECR_Block);
	ProjectileCollisionBox->SetCollisionResponseToChannel(ECC_WorldDynamic,ECR_Block);
	ProjectileCollisionBox->SetCollisionResponseToChannel(ECC_WorldStatic,ECR_Block);
	ProjectileCollisionBox->SetNotifyRigidBodyCollision(true);
	ProjectileCollisionBox->OnComponentHit.AddUniqueDynamic(this,&ThisClass::OnProjectileHit);
	ProjectileCollisionBox->OnComponentBeginOverlap.AddUniqueDynamic(this,&ThisClass::OnProjectileOverlap);

	ProjectileNiagaraComponent= CreateDefaultSubobject<UNiagaraComponent>(FName("NiagaraSystem"));
	ProjectileNiagaraComponent->SetupAttachment(ProjectileCollisionBox);

	ProjectileMovementComponent=CreateDefaultSubobject<UProjectileMovementComponent>(FName("ProjectileMovementComponent"));
	ProjectileMovementComponent->InitialSpeed = 700.f;
	ProjectileMovementComponent->MaxSpeed = 900.f;
	ProjectileMovementComponent->Velocity = FVector(1.0f, 0.0f, 0.0f);
	ProjectileMovementComponent->bInitialVelocityInLocalSpace = true;
	ProjectileMovementComponent->ProjectileGravityScale = 0.f;

	InitialLifeSpan = 4.f;
}

void AWarriorProjectileBase::SetProjectileDamageHandle(const FGameplayEffectSpecHandle& InProjectileDamageHandle)
{
	ProjectileDamageHandle = InProjectileDamageHandle;
}

void AWarriorProjectileBase::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* ProjectileOwner = GetOwner())
	{
		ProjectileCollisionBox->IgnoreActorWhenMoving(ProjectileOwner, true);
	}

	if (APawn* ProjectileInstigator = GetInstigator())
	{
		ProjectileCollisionBox->IgnoreActorWhenMoving(ProjectileInstigator, true);
	}

	if (ProjectileDamagePolicy == EProjectileDamagePolicy::OnBeginOverlap)
	{
		ProjectileCollisionBox->SetCollisionResponseToChannel(ECC_Pawn,ECR_Overlap);
	}
}

void AWarriorProjectileBase::OnProjectileHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (OtherActor == GetOwner() || OtherActor == GetInstigator())
	{
		return;
	}

	BP_OnSpawnProjectileHitFx(Hit.ImpactPoint);

	APawn* HitedPawn = Cast<APawn>(OtherActor);
	if (!HitedPawn || !UWarriorFunctionLibrary::IsTargetPawnHostile(GetInstigator(), HitedPawn))
	{
		Destroy();
		return;
	}

	FGameplayEventData Data;
	Data.Instigator = this;
	Data.Target = HitedPawn;

	if (TryHandleProjectileDefense(HitedPawn, Data))
	{
		Destroy();
		return;
	}

	HandleApplyProjectileEffect(HitedPawn, Data);
	Destroy();
}

void AWarriorProjectileBase::OnProjectileOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (OtherActor == GetOwner() || OtherActor == GetInstigator())
	{
		return;
	}

	if (OverlapActors.Contains(OtherActor))
	{
		return;
	}
	OverlapActors.AddUnique(OtherActor);

	if (APawn* HitPawn = Cast<APawn>(OtherActor))
	{
		FGameplayEventData Data;
		Data.Instigator = GetInstigator();
		Data.Target = HitPawn;

		if (!GetInstigator())
		{
			UE_LOG(LogTemp, Warning, TEXT("Projectile overlap ignored: %s has no Instigator."), *GetNameSafe(this));
			return;
		}

		if (!UWarriorFunctionLibrary::IsTargetPawnHostile(GetInstigator(), HitPawn))
		{
			UE_LOG(LogTemp, Warning, TEXT("Projectile overlap ignored: %s is not hostile to %s."),
				*GetNameSafe(HitPawn),
				*GetNameSafe(GetInstigator()));
			return;
		}

		if (TryHandleProjectileDefense(HitPawn, Data))
		{
			UE_LOG(LogTemp, Warning, TEXT("Projectile damage blocked/dodged by %s."), *GetNameSafe(HitPawn));
			return;
		}

		HandleApplyProjectileEffect(HitPawn, Data);
	}
}

void AWarriorProjectileBase::HandleApplyProjectileEffect(APawn* InHitPawn,const FGameplayEventData& Data)
{
	checkf(ProjectileDamageHandle.IsValid(),TEXT("Forget Assign valid spec handle to projectile"));

	if (!ProjectileDamageHandle.Data.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Projectile damage skipped: invalid damage spec data on %s."), *GetNameSafe(this));
		return;
	}

	FGameplayEffectSpecHandle DamageHandle = ProjectileDamageHandle;
	UWarriorFunctionLibrary::SetAttackImpactDataToEffectSpecHandle(DamageHandle, ResolveAttackImpactData());

	const float BaseDamage = DamageHandle.Data->GetSetByCallerMagnitude(
		WarriorGameplayTags::Shared_SetByCaller_BaseDamage,
		false,
		0.f);
	if (BaseDamage <= 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Projectile damage spec has no positive base damage. Projectile=%s Target=%s BaseDamage=%.2f"),
			*GetNameSafe(this),
			*GetNameSafe(InHitPawn),
			BaseDamage);
	}

	const bool bWasApplied =
		UWarriorFunctionLibrary::ApplyGameplayEffectHandleToTarget(GetInstigator(), InHitPawn, DamageHandle);

	if (bWasApplied)
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			InHitPawn,
			WarriorGameplayTags::Shared_Event_HitReact,
			Data);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Projectile damage GE failed to apply. Projectile=%s Instigator=%s Target=%s"),
			*GetNameSafe(this),
			*GetNameSafe(GetInstigator()),
			*GetNameSafe(InHitPawn));
	}
}

FWarriorAttackImpactData AWarriorProjectileBase::ResolveAttackImpactData() const
{
	FWarriorAttackImpactData AttackImpactData = DefaultAttackImpactData;
	AttackImpactData.BlockCost = SuccessfulBlockCost;

	return UWarriorFunctionLibrary::GetAttackImpactDataFromEffectSpecHandle(
		ProjectileDamageHandle,
		AttackImpactData);
}

bool AWarriorProjectileBase::TryHandleProjectileDefense(APawn* InHitPawn, FGameplayEventData& Data)
{
	if (!InHitPawn)
	{
		return false;
	}

	const FWarriorAttackImpactData AttackImpactData = ResolveAttackImpactData();
	Data.EventMagnitude = AttackImpactData.BlockCost;
	UWarriorFunctionLibrary::AddAttackImpactDataToGameplayEventData(Data, AttackImpactData);

	if (AttackImpactData.bCanBeDodged && UWarriorFunctionLibrary::IsActorInDodgeIFrame(InHitPawn))
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			InHitPawn,
			WarriorGameplayTags::Player_Event_SuccessDodge,
			Data);
		return true;
	}

	const bool bIsPlayerBlocking =
		UWarriorFunctionLibrary::NativeDoesActorHaveTag(InHitPawn, WarriorGameplayTags::Player_Status_Blocking);
	if (AttackImpactData.bCanBeBlocked &&
		bIsPlayerBlocking &&
		UWarriorFunctionLibrary::IsValidBlock(this, InHitPawn))
	{
		UWarriorFunctionLibrary::HandleSuccessfulBlock(InHitPawn, this, AttackImpactData.BlockCost, Data);
		return true;
	}

	return false;
}
