// Fill out your copyright notice in the Description page of Project Settings.

#include "Items/WarriorMovingRectangleDamageArea.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Pawn.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "WarriorFunctionLibrary.h"
#include "WarriorGameplayTags.h"

AWarriorMovingRectangleDamageArea::AWarriorMovingRectangleDamageArea()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	bReplicates = false;
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	DamageBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("DamageBox"));
	DamageBoxComponent->SetupAttachment(SceneRoot);
	DamageBoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DamageBoxComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	DamageBoxComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	DamageBoxComponent->SetGenerateOverlapEvents(true);
	DamageBoxComponent->SetHiddenInGame(true);

	WarningAreaMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WarningArea"));
	WarningAreaMeshComponent->SetupAttachment(SceneRoot);
	WarningAreaMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WarningAreaMeshComponent->SetGenerateOverlapEvents(false);
	WarningAreaMeshComponent->SetCastShadow(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMeshFinder(TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (PlaneMeshFinder.Succeeded())
	{
		WarningAreaMeshComponent->SetStaticMesh(PlaneMeshFinder.Object);
	}
}

void AWarriorMovingRectangleDamageArea::InitializeDamageArea(
	AActor* InDamageSource,
	const FGameplayEffectSpecHandle& InDamageSpecHandle,
	const FWarriorMovingRectangleDamageAreaConfig& InConfig)
{
	DamageSourceActor = InDamageSource;
	DamageSpecHandle = InDamageSpecHandle;
	Config = InConfig;
	bInitialized = true;
}

void AWarriorMovingRectangleDamageArea::BeginPlay()
{
	Super::BeginPlay();

	if (!bInitialized)
	{
		bInitialized = true;
	}

	ConfigureComponents();

	if (Config.WarningDuration <= KINDA_SMALL_NUMBER)
	{
		StartDamageMovement();
	}
}

void AWarriorMovingRectangleDamageArea::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bInitialized || bFinished || DeltaSeconds <= 0.f)
	{
		return;
	}

	if (!bDamageMovementStarted)
	{
		WarningElapsedTime += DeltaSeconds;
		if (WarningElapsedTime >= Config.WarningDuration)
		{
			StartDamageMovement();
		}
		return;
	}

	DamageElapsedTime += DeltaSeconds;
	UpdateDamageBoxLocation();
	CollectAndDamageOverlappingPawns();

	if (Config.bDrawDebugDamageBox && DamageBoxComponent)
	{
		DrawDebugBox(
			GetWorld(),
			DamageBoxComponent->GetComponentLocation(),
			DamageBoxComponent->GetScaledBoxExtent(),
			DamageBoxComponent->GetComponentQuat(),
			FColor::Red,
			false,
			0.f,
			0,
			2.f);
	}

	if (DamageElapsedTime >= GetMoveDuration())
	{
		FinishDamageArea();
	}
}

void AWarriorMovingRectangleDamageArea::ConfigureComponents()
{
	const float GroundRelativeZ = ResolveGroundRelativeZ();
	ConfigureDamageBox(GroundRelativeZ);
	ConfigureWarningArea(GroundRelativeZ);
}

void AWarriorMovingRectangleDamageArea::ConfigureDamageBox(float GroundRelativeZ)
{
	const FVector BoxSize(
		FMath::Max(1.f, Config.DamageBoxSize.X),
		FMath::Max(1.f, Config.DamageBoxSize.Y),
		FMath::Max(1.f, Config.DamageBoxSize.Z));

	DamageBoxComponent->SetBoxExtent(BoxSize * 0.5f, true);
	DamageBoxComponent->SetRelativeLocation(
		FVector(
			BoxSize.X * 0.5f,
			0.f,
			GroundRelativeZ + BoxSize.Z * 0.5f + Config.DamageBoxGroundOffset));
}

void AWarriorMovingRectangleDamageArea::ConfigureWarningArea(float GroundRelativeZ)
{
	const float WarningLength = GetResolvedWarningAreaLength();
	const float WarningWidth = GetResolvedWarningAreaWidth();

	WarningAreaMeshComponent->SetRelativeLocation(
		FVector(WarningLength * 0.5f, 0.f, GroundRelativeZ + Config.WarningGroundOffset));
	WarningAreaMeshComponent->SetRelativeRotation(FRotator::ZeroRotator);
	WarningAreaMeshComponent->SetRelativeScale3D(
		FVector(WarningLength / 100.f, WarningWidth / 100.f, 1.f));
	WarningAreaMeshComponent->SetVisibility(true, true);

	ApplyWarningAreaMaterial();
}

void AWarriorMovingRectangleDamageArea::ApplyWarningAreaMaterial()
{
	if (!WarningAreaMaterial || !WarningAreaMeshComponent)
	{
		return;
	}

	DynamicWarningAreaMaterial = UMaterialInstanceDynamic::Create(WarningAreaMaterial, this);
	if (!DynamicWarningAreaMaterial)
	{
		WarningAreaMeshComponent->SetMaterial(0, WarningAreaMaterial);
		return;
	}

	FLinearColor WarningColor = Config.WarningColor;
	WarningColor.A = Config.WarningOpacity;
	DynamicWarningAreaMaterial->SetVectorParameterValue(TEXT("WarningColor"), WarningColor);
	DynamicWarningAreaMaterial->SetVectorParameterValue(TEXT("BaseColor"), WarningColor);
	DynamicWarningAreaMaterial->SetScalarParameterValue(TEXT("Opacity"), Config.WarningOpacity);
	WarningAreaMeshComponent->SetMaterial(0, DynamicWarningAreaMaterial);
}

void AWarriorMovingRectangleDamageArea::StartDamageMovement()
{
	if (bDamageMovementStarted || bFinished)
	{
		return;
	}

	bDamageMovementStarted = true;
	DamageElapsedTime = 0.f;

	if (Config.bHideWarningWhenDamageStarts && WarningAreaMeshComponent)
	{
		WarningAreaMeshComponent->SetVisibility(false, true);
	}

	DamageBoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	UpdateDamageBoxLocation();
	DamageBoxComponent->UpdateOverlaps();
	CollectAndDamageOverlappingPawns();
	BP_OnDamageMovementStarted();

	if (GetMoveDuration() <= KINDA_SMALL_NUMBER)
	{
		FinishDamageArea();
	}
}

void AWarriorMovingRectangleDamageArea::UpdateDamageBoxLocation()
{
	if (!DamageBoxComponent)
	{
		return;
	}

	const float MoveDuration = GetMoveDuration();
	const float MoveAlpha = MoveDuration > KINDA_SMALL_NUMBER
		? FMath::Clamp(DamageElapsedTime / MoveDuration, 0.f, 1.f)
		: 1.f;

	FVector RelativeLocation = DamageBoxComponent->GetRelativeLocation();
	RelativeLocation.X = GetResolvedDamageBoxLength() * 0.5f + FMath::Max(0.f, Config.TravelDistance) * MoveAlpha;
	DamageBoxComponent->SetRelativeLocation(RelativeLocation, false, nullptr, ETeleportType::TeleportPhysics);
	DamageBoxComponent->UpdateOverlaps();
}

void AWarriorMovingRectangleDamageArea::CollectAndDamageOverlappingPawns()
{
	if (!DamageBoxComponent)
	{
		return;
	}

	TArray<AActor*> OverlappingActors;
	DamageBoxComponent->GetOverlappingActors(OverlappingActors, APawn::StaticClass());

	for (AActor* OverlappingActor : OverlappingActors)
	{
		TryApplyDamageToActor(OverlappingActor);
	}
}

void AWarriorMovingRectangleDamageArea::TryApplyDamageToActor(AActor* TargetActor)
{
	APawn* SourcePawn = Cast<APawn>(DamageSourceActor.Get());
	APawn* TargetPawn = Cast<APawn>(TargetActor);
	if (!SourcePawn || !TargetPawn || SourcePawn == TargetPawn)
	{
		return;
	}

	if (Config.bDamageTargetsOnlyOnce && HasDamagedActor(TargetPawn))
	{
		return;
	}

	if (!DamageSpecHandle.IsValid() ||
		!DamageSpecHandle.Data.IsValid() ||
		!UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(SourcePawn) ||
		!UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetPawn) ||
		!UWarriorFunctionLibrary::IsTargetPawnHostile(SourcePawn, TargetPawn) ||
		UWarriorFunctionLibrary::NativeDoesActorHaveTag(TargetPawn, WarriorGameplayTags::Shared_Status_Death))
	{
		return;
	}

	FGameplayEventData EventData;
	EventData.Instigator = SourcePawn;
	EventData.Target = TargetPawn;
	const FWarriorAttackImpactData AttackImpactData =
		UWarriorFunctionLibrary::GetAttackImpactDataFromEffectSpecHandle(
			DamageSpecHandle,
			FWarriorAttackImpactData());
	EventData.EventMagnitude = AttackImpactData.BlockCost;
	UWarriorFunctionLibrary::AddAttackImpactDataToGameplayEventData(EventData, AttackImpactData);

	if (AttackImpactData.bCanBeDodged && UWarriorFunctionLibrary::IsActorInDodgeIFrame(TargetPawn))
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			TargetPawn,
			WarriorGameplayTags::Player_Event_SuccessDodge,
			EventData);
		MarkActorDamaged(TargetPawn);
		return;
	}

	const bool bIsTargetBlocking =
		UWarriorFunctionLibrary::NativeDoesActorHaveTag(TargetPawn, WarriorGameplayTags::Player_Status_Blocking);
	const bool bWasBlocked =
		AttackImpactData.bCanBeBlocked &&
		bIsTargetBlocking &&
		UWarriorFunctionLibrary::IsValidBlock(SourcePawn, TargetPawn);

	if (bWasBlocked)
	{
		UWarriorFunctionLibrary::HandleSuccessfulBlock(
			TargetPawn,
			SourcePawn,
			AttackImpactData.BlockCost,
			EventData);
		MarkActorDamaged(TargetPawn);
		return;
	}

	const bool bApplied =
		UWarriorFunctionLibrary::ApplyGameplayEffectHandleToTarget(SourcePawn, TargetPawn, DamageSpecHandle);

	if (!bApplied)
	{
		return;
	}

	MarkActorDamaged(TargetPawn);

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		TargetPawn,
		WarriorGameplayTags::Shared_Event_HitReact,
		EventData);

	BP_OnTargetDamaged(TargetPawn);
}

void AWarriorMovingRectangleDamageArea::FinishDamageArea()
{
	if (bFinished)
	{
		return;
	}

	bFinished = true;
	DamageBoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BP_OnDamageAreaFinished();
	Destroy();
}

float AWarriorMovingRectangleDamageArea::ResolveGroundRelativeZ() const
{
	const UWorld* World = GetWorld();
	if (!World || Config.GroundTraceDistance <= 0.f)
	{
		return 0.f;
	}

	const FVector ActorLocation = GetActorLocation();
	const FVector TraceStart = ActorLocation + FVector::UpVector * 100.f;
	const FVector TraceEnd = ActorLocation - FVector::UpVector * Config.GroundTraceDistance;

	FHitResult HitResult;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(MovingRectangleDamageAreaGround), false, this);
	if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
	{
		return HitResult.ImpactPoint.Z - ActorLocation.Z;
	}

	return 0.f;
}

float AWarriorMovingRectangleDamageArea::GetMoveDuration() const
{
	const float TravelDistance = FMath::Max(0.f, Config.TravelDistance);
	const float MoveSpeed = FMath::Max(0.f, Config.MoveSpeed);
	return MoveSpeed > KINDA_SMALL_NUMBER ? TravelDistance / MoveSpeed : 0.f;
}

float AWarriorMovingRectangleDamageArea::GetResolvedDamageBoxLength() const
{
	return FMath::Max(1.f, Config.DamageBoxSize.X);
}

float AWarriorMovingRectangleDamageArea::GetResolvedWarningAreaLength() const
{
	return Config.WarningAreaLength > KINDA_SMALL_NUMBER
		? Config.WarningAreaLength
		: FMath::Max(1.f, Config.TravelDistance + GetResolvedDamageBoxLength());
}

float AWarriorMovingRectangleDamageArea::GetResolvedWarningAreaWidth() const
{
	return Config.WarningAreaWidth > KINDA_SMALL_NUMBER
		? Config.WarningAreaWidth
		: FMath::Max(1.f, Config.DamageBoxSize.Y);
}

bool AWarriorMovingRectangleDamageArea::HasDamagedActor(AActor* TargetActor) const
{
	return DamagedActors.ContainsByPredicate(
		[TargetActor](const TWeakObjectPtr<AActor>& DamagedActor)
		{
			return DamagedActor.Get() == TargetActor;
		});
}

void AWarriorMovingRectangleDamageArea::MarkActorDamaged(AActor* TargetActor)
{
	if (TargetActor && !HasDamagedActor(TargetActor))
	{
		DamagedActors.Add(TargetActor);
	}
}
