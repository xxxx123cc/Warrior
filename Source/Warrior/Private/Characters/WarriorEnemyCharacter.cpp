// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/WarriorEnemyCharacter.h"

#include "components/CapsuleComponent.h"
#include "Components/Combat/EnemyCombatComponent.h"
#include "DataAssets/StartUpData/DataAsset_StartUpDataBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/AssetManager.h"
#include "Components/UI/EnemyUIComponent.h"
#include "Components/WidgetComponent.h"
#include "Widgets/WarriorWidgetBase.h"
#include "Warrior/Public/DataAssets/StartUpData/DataAsset_EnemyStartUpData.h"

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
