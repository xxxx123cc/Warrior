// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/WarriorEnemyCharacter.h"

#include "AbilitySystemBlueprintLibrary.h"
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
#include "Misc/MapErrors.h"

AWarriorEnemyCharacter::AWarriorEnemyCharacter()
{
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

void AWarriorEnemyCharacter::InitEnemyStartUpData()
{
	if (CharacterStartUpData.IsNull())
	{
		return;
	}
	UAssetManager::GetStreamableManager().RequestAsyncLoad(CharacterStartUpData.ToSoftObjectPath(), FStreamableDelegate::CreateLambda([this]()
	{
		if (UDataAsset_StartUpDataBase*LoadedData = CharacterStartUpData.Get())
		{
			LoadedData->GivenToAbilitySystemComponent(WarriorAbilitySystemComponent);
		}
	}));

}
