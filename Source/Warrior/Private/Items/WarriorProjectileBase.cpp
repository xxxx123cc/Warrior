// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/WarriorProjectileBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraComponent.h"
#include "WarriorFunctionLibrary.h"
#include "Warrior/Public/WarriorGameplayTags.h"

AWarriorProjectileBase::AWarriorProjectileBase()
{
	PrimaryActorTick.bCanEverTick = false;

	ProjectileCollisionBox = CreateDefaultSubobject<UBoxComponent>(FName("BoxComponent"));
	SetRootComponent(ProjectileCollisionBox);
	ProjectileCollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ProjectileCollisionBox->SetCollisionResponseToChannel(ECC_Pawn,ECR_Block);
	ProjectileCollisionBox->SetCollisionResponseToChannel(ECC_WorldDynamic,ECR_Block);
	ProjectileCollisionBox->SetCollisionResponseToChannel(ECC_WorldStatic,ECR_Block);
	ProjectileCollisionBox->OnComponentHit.AddUniqueDynamic(this,&ThisClass::OnProjectileHit);
	ProjectileCollisionBox->OnComponentBeginOverlap.AddUniqueDynamic(this,&ThisClass::OnProjectileOverlap);

	ProjectileNiagaraComponent= CreateDefaultSubobject<UNiagaraComponent>(FName("NiagaraSystem"));
	ProjectileNiagaraComponent->SetupAttachment(ProjectileCollisionBox);

	ProjectileMovementComponent=CreateDefaultSubobject<UProjectileMovementComponent>(FName("ProjectileMovementComponent"));
	ProjectileMovementComponent->InitialSpeed = 700.f;
	ProjectileMovementComponent->MaxSpeed = 900.f;
	ProjectileMovementComponent->Velocity = FVector(1.0f, 0.0f, 0.0f);
	ProjectileMovementComponent->ProjectileGravityScale = 0.f;

	InitialLifeSpan = 4.f;
}

void AWarriorProjectileBase::BeginPlay()
{
	Super::BeginPlay();

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

		if (UWarriorFunctionLibrary::IsTargetPawnHostile(GetInstigator(), HitPawn))
		{
			if (TryHandleProjectileDefense(HitPawn, Data))
			{
				return;
			}

			HandleApplyProjectileEffect(HitPawn, Data);
		}
	}
}

void AWarriorProjectileBase::HandleApplyProjectileEffect(APawn* InHitPawn,const FGameplayEventData& Data)
{
	checkf(ProjectileDamageHandle.IsValid(),TEXT("Forget Assign valid spec handle to projectile"));

	FGameplayEffectSpecHandle DamageHandle = ProjectileDamageHandle;
	UWarriorFunctionLibrary::SetAttackImpactDataToEffectSpecHandle(DamageHandle, ResolveAttackImpactData());

	const bool bWasApplied =
		UWarriorFunctionLibrary::ApplyGameplayEffectHandleToTarget(GetInstigator(), InHitPawn, DamageHandle);

	if (bWasApplied)
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			InHitPawn,
			WarriorGameplayTags::Shared_Event_HitReact,
			Data);
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
