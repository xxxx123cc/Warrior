// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/WarriorProjectileBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffect.h"
#include "Components/BoxComponent.h"
#include "Characters/WarriorHeroCharacter.h"
#include "EngineUtils.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "WarriorFunctionLibrary.h"
#include "Warrior/Public/WarriorGameplayTags.h"

AWarriorProjectileBase::AWarriorProjectileBase()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
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
	ProjectileCollisionBox->OnComponentEndOverlap.AddUniqueDynamic(this,&ThisClass::OnProjectileEndOverlap);

	ProjectileNiagaraComponent= CreateDefaultSubobject<UNiagaraComponent>(FName("NiagaraSystem"));
	ProjectileNiagaraComponent->SetupAttachment(ProjectileCollisionBox);

	ProjectileMovementComponent=CreateDefaultSubobject<UProjectileMovementComponent>(FName("ProjectileMovementComponent"));
	ProjectileMovementComponent->InitialSpeed = 0.f;
	ProjectileMovementComponent->MaxSpeed = DefaultFireSpeed;
	ProjectileMovementComponent->Velocity = FVector::ZeroVector;
	ProjectileMovementComponent->bInitialVelocityInLocalSpace = true;
	ProjectileMovementComponent->ProjectileGravityScale = 0.f;
	ProjectileMovementComponent->bAutoActivate = false;

	InitialLifeSpan = ProjectileLifeSpan;
}

void AWarriorProjectileBase::SetProjectileDamageHandle(const FGameplayEffectSpecHandle& InProjectileDamageHandle)
{
	ProjectileDamageHandle = InProjectileDamageHandle;
}

void AWarriorProjectileBase::FireForward(float FireSpeed, float BaseDamageOverride)
{
	PrepareProjectileMovementStart(EProjectileMovementMode::FireForward);
	bIsDashing = false;
	ActiveDashDistance = 0.f;
	ActiveDashSpeed = 0.f;
	ActiveDashTraveledDistance = 0.f;
	DashTargetActor.Reset();
	ActiveBaseDamageOverride = BaseDamageOverride > 0.f ? BaseDamageOverride : FireForwardBaseDamageOverride;
	SetProjectileVelocityForward(FireSpeed > 0.f ? FireSpeed : DefaultFireSpeed);
	HandleCurrentOverlapsAfterMovementStart();
	RefreshProjectileTickEnabled();
}

void AWarriorProjectileBase::DashForward(float DashDistance, float DashSpeed, float BaseDamageOverride)
{
	const float ResolvedDashDistance = DashDistance > 0.f ? DashDistance : DefaultDashDistance;
	const float ResolvedDashSpeed = DashSpeed > 0.f ? DashSpeed : DefaultDashSpeed;
	if (ResolvedDashDistance <= 0.f || ResolvedDashSpeed <= 0.f)
	{
		StopProjectileMovement();
		return;
	}

	PrepareProjectileMovementStart(EProjectileMovementMode::DashForward);
	ActiveBaseDamageOverride = BaseDamageOverride > 0.f ? BaseDamageOverride : DashForwardBaseDamageOverride;
	DashStartLocation = GetActorLocation();
	LastDashLocation = DashStartLocation;
	ActiveDashTraveledDistance = 0.f;
	ActiveDashSpeed = ResolvedDashSpeed;
	DashTargetActor = ResolveDashTarget();
	DashDirection = ResolveDashFallbackDirection();
	if (DashTargetActor.IsValid())
	{
		UpdateDashDirectionTowardTarget(0.f);
	}
	if (DashDirection.IsNearlyZero())
	{
		DashDirection = FVector::ForwardVector;
	}

	ActiveDashDistance = ResolvedDashDistance;
	bIsDashing = true;
	SetProjectileVelocityInDirection(DashDirection, ResolvedDashSpeed);
	HandleCurrentOverlapsAfterMovementStart();
	RefreshProjectileTickEnabled();
}

void AWarriorProjectileBase::StopProjectileMovement()
{
	bIsDashing = false;
	ActiveMovementMode = EProjectileMovementMode::None;
	ActiveDashDistance = 0.f;
	ActiveDashSpeed = 0.f;
	ActiveDashTraveledDistance = 0.f;
	DashTargetActor.Reset();

	if (ProjectileMovementComponent)
	{
		ProjectileMovementComponent->StopMovementImmediately();
		ProjectileMovementComponent->Deactivate();
		ProjectileMovementComponent->SetComponentTickEnabled(false);
	}

	RefreshProjectileTickEnabled();
}

void AWarriorProjectileBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateDashMovement(DeltaSeconds);

	if (!bApplyDamageRepeatedlyWhileOverlapping || DeltaSeconds <= 0.f)
	{
		return;
	}

	RepeatedOverlapDamageElapsedTime += DeltaSeconds;
	if (RepeatedOverlapDamageElapsedTime < FMath::Max(0.01f, RepeatedOverlapDamageInterval))
	{
		return;
	}

	RepeatedOverlapDamageElapsedTime = 0.f;
	ApplyRepeatedOverlapDamage();
}

void AWarriorProjectileBase::BeginPlay()
{
	Super::BeginPlay();

	SetLifeSpan(ProjectileLifeSpan);

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

		if (bIgnoreWorldCollisionWhenUsingOverlapPolicy)
		{
			ProjectileCollisionBox->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
			ProjectileCollisionBox->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);
		}
	}

	if (bStartStationary)
	{
		StopProjectileMovement();
	}
	else
	{
		FireForward(DefaultFireSpeed);
	}

	RefreshProjectileTickEnabled();
}

void AWarriorProjectileBase::OnProjectileHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (!IsProjectileCollisionActive())
	{
		return;
	}

	if (OtherActor == GetOwner() || OtherActor == GetInstigator())
	{
		return;
	}

	SpawnProjectileImpactFeedback(ResolveImpactFeedbackLocation(Hit, OtherActor));

	APawn* HitedPawn = Cast<APawn>(OtherActor);
	if (!HitedPawn || !UWarriorFunctionLibrary::IsTargetPawnHostile(GetInstigator(), HitedPawn))
	{
		if (ProjectileDamagePolicy == EProjectileDamagePolicy::OnBeginOverlap)
		{
			StopProjectileMovement();
			return;
		}

		Destroy();
		return;
	}

	FGameplayEventData Data;
	Data.Instigator = this;
	Data.Target = HitedPawn;

	if (TryHandleProjectileDefense(HitedPawn, Data))
	{
		if (ShouldDestroyFireForwardForActor(HitedPawn))
		{
			Destroy();
			return;
		}

		if (ShouldStopDashForActor(HitedPawn))
		{
			StopProjectileMovement();
			return;
		}
		Destroy();
		return;
	}

	HandleApplyProjectileEffect(HitedPawn, Data);

	if (ShouldDestroyFireForwardForActor(HitedPawn))
	{
		Destroy();
		return;
	}

	if (ShouldStopDashForActor(HitedPawn))
	{
		StopProjectileMovement();
	}

}

void AWarriorProjectileBase::OnProjectileOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!IsProjectileCollisionActive())
	{
		return;
	}

	if (OtherActor == GetOwner() || OtherActor == GetInstigator())
	{
		return;
	}

	if (OverlapActors.Contains(OtherActor))
	{
		return;
	}

	if (APawn* HitPawn = Cast<APawn>(OtherActor))
	{
		const bool bShouldDestroyFireForward = ShouldDestroyFireForwardForActor(HitPawn);
		const FVector ImpactFeedbackLocation = ResolveImpactFeedbackLocation(SweepResult, OtherActor);

		if (bApplyDamageRepeatedlyWhileOverlapping)
		{
			TrackRepeatedOverlapPawn(HitPawn);

			if (bApplyDamageImmediatelyOnOverlap)
			{
				TryApplyProjectileOverlapDamage(HitPawn, true);
			}

			if (bShouldDestroyFireForward)
			{
				SpawnProjectileImpactFeedback(ImpactFeedbackLocation);
				Destroy();
				return;
			}

			if (ShouldStopDashForActor(HitPawn))
			{
				StopProjectileMovement();
			}
			return;
		}

		if (TryApplyProjectileOverlapDamage(HitPawn, true))
		{
			OverlapActors.AddUnique(OtherActor);

			if (bShouldDestroyFireForward)
			{
				SpawnProjectileImpactFeedback(ImpactFeedbackLocation);
				Destroy();
				return;
			}
		}

		if (ShouldStopDashForActor(HitPawn))
		{
			StopProjectileMovement();
		}
	}
}

void AWarriorProjectileBase::OnProjectileEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (!IsProjectileCollisionActive())
	{
		return;
	}

	RemoveRepeatedOverlapActor(OtherActor);
}

void AWarriorProjectileBase::SpawnProjectileImpactFeedback(const FVector& HitLocation)
{
	BP_OnSpawnProjectileHitFx(HitLocation);

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (ImpactNiagaraSystem)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			ImpactNiagaraSystem,
			HitLocation,
			GetActorRotation());
	}

	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			World,
			ImpactSound,
			HitLocation,
			FMath::Max(0.f, ImpactSoundVolumeMultiplier),
			FMath::Max(0.f, ImpactSoundPitchMultiplier));
	}
}

bool AWarriorProjectileBase::HandleApplyProjectileEffect(
	APawn* InHitPawn,
	const FGameplayEventData& Data,
	bool bSendHitReact)
{
	checkf(ProjectileDamageHandle.IsValid(),TEXT("Forget Assign valid spec handle to projectile"));

	if (!ProjectileDamageHandle.Data.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Projectile damage skipped: invalid damage spec data on %s."), *GetNameSafe(this));
		return false;
	}

	FGameplayEffectSpecHandle DamageHandle = ProjectileDamageHandle;
	const FWarriorAttackImpactData AttackImpactData = ResolveAttackImpactData();
	UWarriorFunctionLibrary::SetAttackImpactDataToEffectSpecHandle(DamageHandle, AttackImpactData);
	OverrideDamageHandleBaseDamageIfNeeded(DamageHandle);

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
		if (bSendHitReact)
		{
			FGameplayEventData HitReactData = Data;
			HitReactData.EventMagnitude = AttackImpactData.BlockCost;
			UWarriorFunctionLibrary::AddAttackImpactDataToGameplayEventData(HitReactData, AttackImpactData);

			UWarriorFunctionLibrary::SendHitReactEventToActor(
				InHitPawn,
				WarriorGameplayTags::Shared_Event_HitReact,
				HitReactData);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Projectile damage GE failed to apply. Projectile=%s Instigator=%s Target=%s"),
			*GetNameSafe(this),
			*GetNameSafe(GetInstigator()),
			*GetNameSafe(InHitPawn));
	}

	return bWasApplied;
}

FWarriorAttackImpactData AWarriorProjectileBase::ResolveAttackImpactData() const
{
	FWarriorAttackImpactData AttackImpactData = DefaultAttackImpactData;
	AttackImpactData.BlockCost = SuccessfulBlockCost;

	return UWarriorFunctionLibrary::GetAttackImpactDataFromEffectSpecHandle(
		ProjectileDamageHandle,
		AttackImpactData);
}

float AWarriorProjectileBase::ResolveActiveBaseDamageOverride() const
{
	return FMath::Max(0.f, ActiveBaseDamageOverride);
}

void AWarriorProjectileBase::OverrideDamageHandleBaseDamageIfNeeded(
	FGameplayEffectSpecHandle& InOutDamageHandle) const
{
	const float BaseDamageOverride = ResolveActiveBaseDamageOverride();
	if (BaseDamageOverride <= 0.f || !InOutDamageHandle.IsValid() || !InOutDamageHandle.Data.IsValid())
	{
		return;
	}

	InOutDamageHandle.Data->SetSetByCallerMagnitude(
		WarriorGameplayTags::Shared_SetByCaller_BaseDamage,
		BaseDamageOverride);
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

bool AWarriorProjectileBase::TryApplyProjectileOverlapDamage(APawn* InHitPawn, bool bSendHitReact)
{
	if (!InHitPawn)
	{
		return false;
	}

	FGameplayEventData Data;
	Data.Instigator = GetInstigator();
	Data.Target = InHitPawn;

	if (!GetInstigator())
	{
		UE_LOG(LogTemp, Warning, TEXT("Projectile overlap ignored: %s has no Instigator."), *GetNameSafe(this));
		return false;
	}

	if (!UWarriorFunctionLibrary::IsTargetPawnHostile(GetInstigator(), InHitPawn))
	{
		UE_LOG(LogTemp, Warning, TEXT("Projectile overlap ignored: %s is not hostile to %s."),
			*GetNameSafe(InHitPawn),
			*GetNameSafe(GetInstigator()));
		return false;
	}

	if (TryHandleProjectileDefense(InHitPawn, Data))
	{
		UE_LOG(LogTemp, Warning, TEXT("Projectile damage blocked/dodged by %s."), *GetNameSafe(InHitPawn));
		return true;
	}

	return HandleApplyProjectileEffect(InHitPawn, Data, bSendHitReact);
}

void AWarriorProjectileBase::TrackRepeatedOverlapPawn(APawn* InHitPawn)
{
	if (!InHitPawn)
	{
		return;
	}

	RepeatedOverlapPawns.AddUnique(InHitPawn);
}

void AWarriorProjectileBase::RemoveRepeatedOverlapActor(AActor* InActor)
{
	if (!InActor)
	{
		return;
	}

	RepeatedOverlapPawns.RemoveAllSwap(
		[InActor](const TWeakObjectPtr<APawn>& OverlapPawn)
		{
			return !OverlapPawn.IsValid() || OverlapPawn.Get() == InActor;
		});
}

void AWarriorProjectileBase::ApplyRepeatedOverlapDamage()
{
	if (!ProjectileCollisionBox || !IsProjectileCollisionActive())
	{
		return;
	}

	CollectCurrentOverlappingPawnsForRepeatedDamage();

	for (int32 Index = RepeatedOverlapPawns.Num() - 1; Index >= 0; --Index)
	{
		APawn* OverlapPawn = RepeatedOverlapPawns[Index].Get();
		if (!OverlapPawn || !ProjectileCollisionBox->IsOverlappingActor(OverlapPawn))
		{
			RepeatedOverlapPawns.RemoveAtSwap(Index);
			continue;
		}

		TryApplyProjectileOverlapDamage(OverlapPawn, bSendHitReactOnRepeatedOverlapDamage);
	}
}

void AWarriorProjectileBase::CollectCurrentOverlappingPawnsForRepeatedDamage()
{
	if (!ProjectileCollisionBox)
	{
		return;
	}

	TArray<AActor*> CurrentOverlappingActors;
	ProjectileCollisionBox->GetOverlappingActors(CurrentOverlappingActors, APawn::StaticClass());

	for (AActor* CurrentOverlappingActor : CurrentOverlappingActors)
	{
		TrackRepeatedOverlapPawn(Cast<APawn>(CurrentOverlappingActor));
	}
}

void AWarriorProjectileBase::SetProjectileVelocityForward(float Speed)
{
	SetProjectileVelocityInDirection(GetActorForwardVector(), Speed);
}

void AWarriorProjectileBase::SetProjectileVelocityInDirection(const FVector& Direction, float Speed)
{
	if (!ProjectileMovementComponent || Speed <= 0.f)
	{
		return;
	}

	FVector ResolvedDirection = Direction.GetSafeNormal();
	if (ResolvedDirection.IsNearlyZero())
	{
		ResolvedDirection = GetActorForwardVector().GetSafeNormal();
	}
	if (ResolvedDirection.IsNearlyZero())
	{
		ResolvedDirection = FVector::ForwardVector;
	}

	FRotator TargetRotation = ResolvedDirection.ToOrientationRotator();
	TargetRotation.Roll = 0.f;
	SetActorRotation(TargetRotation);

	ProjectileMovementComponent->SetUpdatedComponent(ProjectileCollisionBox);
	ProjectileMovementComponent->InitialSpeed = Speed;
	ProjectileMovementComponent->MaxSpeed = FMath::Max(ProjectileMovementComponent->MaxSpeed, Speed);
	ProjectileMovementComponent->Velocity = ResolvedDirection * Speed;
	ProjectileMovementComponent->Activate(true);
	ProjectileMovementComponent->SetComponentTickEnabled(true);
}

void AWarriorProjectileBase::PrepareProjectileMovementStart(EProjectileMovementMode MovementMode)
{
	ActiveMovementMode = MovementMode;
	OverlapActors.Empty();
	RepeatedOverlapPawns.Empty();
	RepeatedOverlapDamageElapsedTime = 0.f;
}

void AWarriorProjectileBase::HandleCurrentOverlapsAfterMovementStart()
{
	if (!ProjectileCollisionBox || !IsProjectileCollisionActive())
	{
		return;
	}

	ProjectileCollisionBox->UpdateOverlaps();

	TArray<AActor*> CurrentOverlappingActors;
	ProjectileCollisionBox->GetOverlappingActors(CurrentOverlappingActors, APawn::StaticClass());

	for (AActor* CurrentOverlappingActor : CurrentOverlappingActors)
	{
		if (!CurrentOverlappingActor ||
			CurrentOverlappingActor == GetOwner() ||
			CurrentOverlappingActor == GetInstigator())
		{
			continue;
		}

		OnProjectileOverlap(
			ProjectileCollisionBox,
			CurrentOverlappingActor,
			Cast<UPrimitiveComponent>(CurrentOverlappingActor->GetRootComponent()),
			INDEX_NONE,
			false,
			FHitResult());
	}
}

void AWarriorProjectileBase::UpdateDashMovement(float DeltaSeconds)
{
	if (!bIsDashing)
	{
		return;
	}

	if (ActiveDashDistance <= 0.f)
	{
		StopProjectileMovement();
		return;
	}

	const FVector CurrentLocation = GetActorLocation();
	ActiveDashTraveledDistance += FVector::Dist(LastDashLocation, CurrentLocation);
	LastDashLocation = CurrentLocation;

	if (bDashForwardContinuouslyTracksTarget)
	{
		UpdateDashDirectionTowardTarget(DeltaSeconds);
	}

	if (ActiveDashTraveledDistance < ActiveDashDistance)
	{
		return;
	}

	FVector TargetLocation = GetActorLocation();
	const float OvershootDistance = ActiveDashTraveledDistance - ActiveDashDistance;
	if (OvershootDistance > KINDA_SMALL_NUMBER && !DashDirection.IsNearlyZero())
	{
		TargetLocation -= DashDirection.GetSafeNormal() * OvershootDistance;
	}

	SetActorLocation(TargetLocation, false, nullptr, ETeleportType::TeleportPhysics);
	StopProjectileMovement();
}

AActor* AWarriorProjectileBase::ResolveDashTarget() const
{
	if (!bDashForwardTargetsEnemies)
	{
		return nullptr;
	}

	if (AActor* LockedTarget = ResolveLockedDashTarget())
	{
		if (IsValidDashTarget(LockedTarget, false))
		{
			return LockedTarget;
		}
	}

	if (AActor* NearestTarget = FindNearestDashTarget())
	{
		return NearestTarget;
	}

	return nullptr;
}

AActor* AWarriorProjectileBase::ResolveLockedDashTarget() const
{
	const AWarriorHeroCharacter* HeroInstigator = Cast<AWarriorHeroCharacter>(GetInstigator());
	return HeroInstigator ? HeroInstigator->GetCurrentLockedOnTarget() : nullptr;
}

AActor* AWarriorProjectileBase::FindNearestDashTarget() const
{
	UWorld* World = GetWorld();
	if (!World || !GetInstigator() || DashForwardTargetSearchRadius <= 0.f)
	{
		return nullptr;
	}

	const FVector SearchOrigin = GetActorLocation();
	const float SearchRadiusSquared = FMath::Square(DashForwardTargetSearchRadius);
	AActor* BestTargetActor = nullptr;
	float BestDistanceSquared = SearchRadiusSquared;

	for (TActorIterator<APawn> It(World); It; ++It)
	{
		APawn* CandidatePawn = *It;
		if (!IsValidDashTarget(CandidatePawn, true))
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared(SearchOrigin, CandidatePawn->GetActorLocation());
		if (DistanceSquared <= BestDistanceSquared)
		{
			BestTargetActor = CandidatePawn;
			BestDistanceSquared = DistanceSquared;
		}
	}

	return BestTargetActor;
}

bool AWarriorProjectileBase::IsValidDashTarget(AActor* CandidateActor, bool bRequireWithinSearchRadius) const
{
	APawn* InstigatorPawn = GetInstigator();
	APawn* CandidatePawn = Cast<APawn>(CandidateActor);
	if (!InstigatorPawn || !IsValid(CandidateActor) || !CandidatePawn || CandidatePawn == InstigatorPawn)
	{
		return false;
	}

	if (!UWarriorFunctionLibrary::IsTargetPawnHostile(InstigatorPawn, CandidatePawn))
	{
		return false;
	}

	if (UWarriorFunctionLibrary::NativeDoesActorHaveTag(CandidateActor, WarriorGameplayTags::Shared_Status_Death))
	{
		return false;
	}

	if (bRequireWithinSearchRadius && DashForwardTargetSearchRadius > 0.f)
	{
		return FVector::DistSquared(GetActorLocation(), CandidateActor->GetActorLocation()) <=
			FMath::Square(DashForwardTargetSearchRadius);
	}

	return !bRequireWithinSearchRadius;
}

bool AWarriorProjectileBase::UpdateDashDirectionTowardTarget(float DeltaSeconds)
{
	AActor* TargetActor = ResolveDashTarget();
	if (!TargetActor)
	{
		DashTargetActor.Reset();
		return false;
	}

	FVector DesiredDirection = TargetActor->GetActorLocation() - GetActorLocation();
	if (bDashForwardUseHorizontalTargeting)
	{
		DesiredDirection.Z = 0.f;
	}

	if (!DesiredDirection.Normalize())
	{
		return false;
	}

	DashTargetActor = TargetActor;
	FRotator TargetRotation = DesiredDirection.ToOrientationRotator();
	TargetRotation.Roll = 0.f;

	if (DashForwardTargetRotationInterpSpeed > 0.f && DeltaSeconds > 0.f)
	{
		TargetRotation = FMath::RInterpTo(
			GetActorRotation(),
			TargetRotation,
			DeltaSeconds,
			DashForwardTargetRotationInterpSpeed);
		TargetRotation.Roll = 0.f;

		DesiredDirection = TargetRotation.Vector();
		if (bDashForwardUseHorizontalTargeting)
		{
			DesiredDirection.Z = 0.f;
		}
		if (!DesiredDirection.Normalize())
		{
			return false;
		}
	}

	DashDirection = DesiredDirection;
	SetProjectileVelocityInDirection(DashDirection, ActiveDashSpeed > 0.f ? ActiveDashSpeed : DefaultDashSpeed);
	return true;
}

void AWarriorProjectileBase::RefreshProjectileTickEnabled()
{
	const bool bNeedsRepeatedOverlapTick =
		IsProjectileCollisionActive() &&
		ProjectileDamagePolicy == EProjectileDamagePolicy::OnBeginOverlap &&
		bApplyDamageRepeatedlyWhileOverlapping;

	SetActorTickEnabled(bIsDashing || bNeedsRepeatedOverlapTick);
}

bool AWarriorProjectileBase::IsProjectileCollisionActive() const
{
	return ActiveMovementMode != EProjectileMovementMode::None;
}

FVector AWarriorProjectileBase::ResolveDashFallbackDirection() const
{
	FVector FallbackDirection = FVector::ZeroVector;

	if (const APawn* InstigatorPawn = GetInstigator())
	{
		FallbackDirection = InstigatorPawn->GetActorForwardVector();
	}

	if (FallbackDirection.IsNearlyZero())
	{
		if (const AActor* OwnerActor = GetOwner())
		{
			FallbackDirection = OwnerActor->GetActorForwardVector();
		}
	}

	if (FallbackDirection.IsNearlyZero())
	{
		FallbackDirection = GetActorForwardVector();
	}

	if (bDashForwardUseHorizontalTargeting)
	{
		FallbackDirection.Z = 0.f;
	}

	return FallbackDirection.GetSafeNormal();
}

bool AWarriorProjectileBase::ShouldStopDashForActor(AActor* OtherActor) const
{
	if (!bIsDashing || !bStopDashWhenOverlappingEnemy || !OtherActor || !GetInstigator())
	{
		return false;
	}

	APawn* OtherPawn = Cast<APawn>(OtherActor);
	return OtherPawn && UWarriorFunctionLibrary::IsTargetPawnHostile(GetInstigator(), OtherPawn);
}

bool AWarriorProjectileBase::ShouldDestroyFireForwardForActor(AActor* OtherActor) const
{
	if (ActiveMovementMode != EProjectileMovementMode::FireForward ||
		!bDestroyFireForwardOnEnemyHit ||
		!OtherActor ||
		!GetInstigator())
	{
		return false;
	}

	APawn* OtherPawn = Cast<APawn>(OtherActor);
	return OtherPawn && UWarriorFunctionLibrary::IsTargetPawnHostile(GetInstigator(), OtherPawn);
}

FVector AWarriorProjectileBase::ResolveImpactFeedbackLocation(
	const FHitResult& Hit,
	AActor* FallbackActor) const
{
	if (!Hit.ImpactPoint.IsNearlyZero())
	{
		return Hit.ImpactPoint;
	}

	if (!Hit.Location.IsNearlyZero())
	{
		return Hit.Location;
	}

	return FallbackActor ? FallbackActor->GetActorLocation() : GetActorLocation();
}
