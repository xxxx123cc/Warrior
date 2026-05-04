// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/WarriorProjectileBase.h"

#include "Components/BoxComponent.h"
#include "NiagaraComponent.h"
#include "WarriorFunctionLibrary.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Warrior/Public/WarriorGameplayTags.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Chaos/Deformable/MuscleActivationConstraints.h"
// Sets default values
AWarriorProjectileBase::AWarriorProjectileBase()
{
 	
	PrimaryActorTick.bCanEverTick = false;
//发射物碰撞设置
	ProjectileCollisionBox = CreateDefaultSubobject<UBoxComponent>(FName("BoxComponent"));
	SetRootComponent(ProjectileCollisionBox);
	ProjectileCollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ProjectileCollisionBox->SetCollisionResponseToChannel(ECC_Pawn,ECR_Block);
	ProjectileCollisionBox->SetCollisionResponseToChannel(ECC_WorldDynamic,ECR_Block);
	ProjectileCollisionBox->SetCollisionResponseToChannel(ECC_WorldStatic,ECR_Block);
	ProjectileCollisionBox->OnComponentHit.AddUniqueDynamic(this,&ThisClass::OnProjectileHit);
	ProjectileCollisionBox->OnComponentBeginOverlap.AddUniqueDynamic(this,&ThisClass::OnProjectileOverlap);
//NiagaraComponent设置	
	ProjectileNiagaraComponent= CreateDefaultSubobject<UNiagaraComponent>(FName("NiagaraSystem"));
	ProjectileNiagaraComponent->SetupAttachment(ProjectileCollisionBox);
//发射物移动设置	
	ProjectileMovementComponent=CreateDefaultSubobject<UProjectileMovementComponent>(FName("ProjectileMovementComponent"));
	ProjectileMovementComponent->InitialSpeed = 700.f;
	ProjectileMovementComponent->MaxSpeed = 900.f;
	ProjectileMovementComponent->Velocity = FVector(1.0f, 0.0f, 0.0f);
	ProjectileMovementComponent->ProjectileGravityScale = 0.f;
	//生命周期
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

void AWarriorProjectileBase::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	
	BP_OnSpawnProjectileHitFx(Hit.ImpactPoint);
	//受击者
	
	
	APawn* HitedPawn = Cast<APawn>(OtherActor);
	if (!HitedPawn||!UWarriorFunctionLibrary::IsTargetPawnHostile(GetInstigator(),HitedPawn))
	{
		Destroy();
		return;
	}
	
	bool bIsValidBlock = false;
	const bool bIsPlayerBlocking =UWarriorFunctionLibrary::NativeDoesActorHaveTag(HitedPawn,WarriorGameplayTags::Player_Status_Blocking);
	FGameplayEventData Data;
	Data.Instigator =this;
	Data.Target = HitedPawn;
	
	if (bIsPlayerBlocking)
	{
		bIsValidBlock =UWarriorFunctionLibrary::IsValidBlock(this,HitedPawn);
	}
	if (bIsValidBlock)
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(HitedPawn,WarriorGameplayTags::Player_Event_SuccessBlock,Data);
		
	}
	else
	{
		HandleApplyProjectileEffect(HitedPawn,Data);
		
	}
	Destroy();
}

void AWarriorProjectileBase::OnProjectileOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
	
}

void AWarriorProjectileBase::HandleApplyProjectileEffect(APawn* InHitPawn,const FGameplayEventData& Data)
{
	checkf(ProjectileDamageHandle.IsValid(),TEXT("Forget Assign valid spec handle to projectile"));
	
	const bool bWasApplied =  UWarriorFunctionLibrary::ApplyGameplayEfectHandleToTarget(GetInstigator(),InHitPawn,ProjectileDamageHandle);
	
	if (bWasApplied)
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(InHitPawn,WarriorGameplayTags::Shared_Event_HitReact,Data);
	}
}



