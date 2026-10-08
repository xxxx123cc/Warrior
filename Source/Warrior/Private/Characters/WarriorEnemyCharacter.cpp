// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/WarriorEnemyCharacter.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "WarriorFunctionLibrary.h"
#include "WarriorGameplayTags.h"
#include "components/CapsuleComponent.h"
#include "Components/Combat/EnemyCombatComponent.h"
#include "DataAssets/StartUpData/DataAsset_StartUpDataBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/AssetManager.h"
#include "Components/UI/EnemyUIComponent.h"
#include "Components/WidgetComponent.h"
#include "Widgets/WarriorWidgetBase.h"
#include "Components/BoxComponent.h"
#include "GameModes/WarriorBaseGameMode.h"
#include "Misc/MapErrors.h"

namespace
{
	const FName TargetActorKeyName(TEXT("TargetActor"));
}

AWarriorEnemyCharacter::AWarriorEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	SetActorHiddenInGame(true);

	//ai
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
	//角色移动组件
	GetCharacterMovement()->bUseControllerDesiredRotation=false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate=FRotator(0.f,180.f,0.f);
	GetCharacterMovement()->MaxWalkSpeed = 300.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 1000.f;
	
	EnemyCombatComponent= CreateDefaultSubobject<UEnemyCombatComponent>("UEnemyCombatComponent");
	
	EnemyUIComponent= CreateDefaultSubobject<UEnemyUIComponent>("UEnemyUIComponent");
	
	LeftBoxComponent= CreateDefaultSubobject<UBoxComponent>("LeftBoxComponent");
	LeftBoxComponent->SetupAttachment(GetMesh());
	RightBoxComponent= CreateDefaultSubobject<UBoxComponent>("RightBoxComponent");
	RightBoxComponent->SetupAttachment(GetMesh());
	
	LeftBoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LeftBoxComponent->OnComponentBeginOverlap.AddDynamic(this,&ThisClass::OnBodyCollisionBoxBeginOverlap);
	LeftBoxComponent->OnComponentEndOverlap.AddDynamic(this,&ThisClass::OnBodyCollisionBoxEndOverlap);
	RightBoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RightBoxComponent->OnComponentBeginOverlap.AddDynamic(this,&ThisClass::OnBodyCollisionBoxBeginOverlap);
	RightBoxComponent->OnComponentEndOverlap.AddDynamic(this,&ThisClass::OnBodyCollisionBoxEndOverlap);
	
	
	EnemyHealthBarWidget= CreateDefaultSubobject<UWidgetComponent>("EnemyHealthBarWidget");
	EnemyHealthBarWidget->SetupAttachment(GetMesh());
}

void AWarriorEnemyCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateTargetContactMovement();
}

UPawnCombatComponent* AWarriorEnemyCharacter::GetPawnCombatComponent() const
{
	return EnemyCombatComponent;
}

UPawnUIComponent* AWarriorEnemyCharacter::GetPawnUIComponent() const
{
	
	return EnemyUIComponent;
}

UEnemyUIComponent* AWarriorEnemyCharacter::GetEnemyUIComponent() const
{

	return EnemyUIComponent;
}
#if WITH_EDITOR
void AWarriorEnemyCharacter::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	if (PropertyChangedEvent.GetMemberPropertyName()==GET_MEMBER_NAME_CHECKED(ThisClass,LeftHandCollisionBoxBoneName))
	{
		LeftBoxComponent->AttachToComponent(GetMesh(),FAttachmentTransformRules::SnapToTargetNotIncludingScale,LeftHandCollisionBoxBoneName);
	}
	
	
	if (PropertyChangedEvent.GetMemberPropertyName()==GET_MEMBER_NAME_CHECKED(ThisClass,RightHandCollisionBoxBoneName))
	{
		RightBoxComponent->AttachToComponent(GetMesh(),FAttachmentTransformRules::SnapToTargetNotIncludingScale,RightHandCollisionBoxBoneName);
	}
	
	
}
#endif

void AWarriorEnemyCharacter::PossessedBy(AController* NewController)
{ 
	Super::PossessedBy(NewController);
	InitEnemyStartUpData();
}

void AWarriorEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	UWarriorWidgetBase* EnemyHealthWidget = Cast<UWarriorWidgetBase>(EnemyHealthBarWidget->GetUserWidgetObject());
	{
		if (EnemyHealthWidget)
		{
			EnemyHealthWidget->InitEnemyWidget(this);
		}
	}
}

void AWarriorEnemyCharacter::OnBodyCollisionBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	const FHitResult& SweepResult)
{
	check(OtherActor);
	APawn* Target = Cast<APawn>(OtherActor);
	
	if (Target)
	{
		if (UWarriorFunctionLibrary::IsTargetPawnHostile(this,Target))
		{
		EnemyCombatComponent->OnHitTargetActor(Target);
		}
	}
	
	
}

void AWarriorEnemyCharacter::OnBodyCollisionBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	
	
}

void AWarriorEnemyCharacter::UpdateTargetContactMovement()
{
	if (!bStopMovementWhenTouchingTarget)
	{
		SetTargetContactMovementHeld(false);
		return;
	}

	const AActor* TargetActor = GetCurrentTargetActor();
	const float ContactDistance = GetTargetContactDistance(TargetActor);
	if (!TargetActor || ContactDistance <= 0.f)
	{
		SetTargetContactMovementHeld(false);
		return;
	}

	const float StopDistance = ContactDistance + FMath::Max(0.f, TargetContactStopBuffer);
	const float ReleaseDistance = ContactDistance + FMath::Max(TargetContactReleaseBuffer, TargetContactStopBuffer);
	const float DistanceThreshold = bMovementHeldByTargetContact ? ReleaseDistance : StopDistance;

	const float Distance2D = FVector::Dist2D(GetActorLocation(), TargetActor->GetActorLocation());
	SetTargetContactMovementHeld(Distance2D <= DistanceThreshold);
}

void AWarriorEnemyCharacter::SetTargetContactMovementHeld(bool bShouldHold)
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (!MovementComponent)
	{
		bMovementHeldByTargetContact = false;
		MaxWalkSpeedBeforeTargetContactHold = 0.f;
		return;
	}

	if (bShouldHold)
	{
		if (!bMovementHeldByTargetContact)
		{
			MaxWalkSpeedBeforeTargetContactHold = MovementComponent->MaxWalkSpeed;
			bMovementHeldByTargetContact = true;
		}

		MovementComponent->MaxWalkSpeed = 0.f;
		MovementComponent->StopMovementImmediately();
		return;
	}

	if (bMovementHeldByTargetContact)
	{
		MovementComponent->MaxWalkSpeed = FMath::Max(0.f, MaxWalkSpeedBeforeTargetContactHold);
		bMovementHeldByTargetContact = false;
		MaxWalkSpeedBeforeTargetContactHold = 0.f;
	}
}

AActor* AWarriorEnemyCharacter::GetCurrentTargetActor() const
{
	const AAIController* AIController = Cast<AAIController>(GetController());
	const UBlackboardComponent* BlackboardComponent = AIController ? AIController->GetBlackboardComponent() : nullptr;

	return BlackboardComponent
		? Cast<AActor>(BlackboardComponent->GetValueAsObject(TargetActorKeyName))
		: nullptr;
}

float AWarriorEnemyCharacter::GetTargetContactDistance(const AActor* TargetActor) const
{
	if (!TargetActor || TargetActor == this)
	{
		return 0.f;
	}

	const UCapsuleComponent* EnemyCapsuleComponent = GetCapsuleComponent();
	const float EnemyRadius = EnemyCapsuleComponent ? EnemyCapsuleComponent->GetScaledCapsuleRadius() : 0.f;

	float TargetRadius = 0.f;
	if (const ACharacter* TargetCharacter = Cast<ACharacter>(TargetActor))
	{
		if (const UCapsuleComponent* TargetCapsuleComponent = TargetCharacter->GetCapsuleComponent())
		{
			TargetRadius = TargetCapsuleComponent->GetScaledCapsuleRadius();
		}
	}
	else if (const UCapsuleComponent* TargetCapsuleComponent = TargetActor->FindComponentByClass<UCapsuleComponent>())
	{
		TargetRadius = TargetCapsuleComponent->GetScaledCapsuleRadius();
	}

	return EnemyRadius + TargetRadius;
}

void AWarriorEnemyCharacter::InitEnemyStartUpData()
{
	if (CharacterStartUpData.IsNull())
	{
		SetActorHiddenInGame(false);
		return;
	}
	int32 AbilityApplyLevel = 1;
	if (AWarriorBaseGameMode* BaseGameMode = GetWorld()->GetAuthGameMode<AWarriorBaseGameMode>())
	{
		switch (BaseGameMode->GetGameDifficulty())
		{
		case WarriorDifficulty::Easy:
			AbilityApplyLevel = 1;
			break;
		case WarriorDifficulty::Normal:
			AbilityApplyLevel = 2;
			break;
		case WarriorDifficulty::Medium:
			AbilityApplyLevel = 3;
			break;	
		case WarriorDifficulty::Hard:
			AbilityApplyLevel = 4;
		default:
			break;
				
		};
			
			
	}
	UAssetManager::GetStreamableManager().RequestAsyncLoad(CharacterStartUpData.ToSoftObjectPath(), FStreamableDelegate::CreateLambda([this,AbilityApplyLevel]()
	{
		if (UDataAsset_StartUpDataBase*LoadedData = CharacterStartUpData.Get())
		{
			LoadedData->GivenToAbilitySystemComponent(WarriorAbilitySystemComponent,AbilityApplyLevel);
			SetActorHiddenInGame(false);
		}
		else
		{
			SetActorHiddenInGame(false);
		}
	}));

}
