// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/WarriorHeroCharacter.h"
#include "WarriorDebugHelper.h"
#include "Camera/CameraComponent.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include"EnhancedInputSubsystems.h"
#include"DataAssets/Input/DataAsset_InputConfig.h"
#include"Components/Input/WarriorInputComponent.h"
#include "WarriorGameplayTags.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "AbilitySystem/WarriorAttributeSet.h"
#include "GameFramework/PlayerController.h"
#include "DataAssets/StartUpData/DataAsset_Hero_StartUpData.h"
#include "Components/Combat/HeroCombatComponent.h"
AWarriorHeroCharacter::AWarriorHeroCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f,96.f);
	//设置胶囊体组件
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
	//弹簧臂
	CameraBoom=CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength=200.f;
	CameraBoom->SocketOffset= FVector(0.f,55.f,65.f);
	CameraBoom->bUsePawnControlRotation = true;
	//摄像机
	FollowCamera=CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom,USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	//角色移动组件
	GetCharacterMovement()->bOrientRotationToMovement=true;
	GetCharacterMovement()->RotationRate=FRotator(0.f,500.f,0.f);
	GetCharacterMovement()->MaxWalkSpeed=600.f;
	GetCharacterMovement()->BrakingDecelerationWalking=2000.f;
	
	HeroCombatComponent = CreateDefaultSubobject<UHeroCombatComponent>(TEXT("HeroCombatComponent"));
	
}

UPawnCombatComponent* AWarriorHeroCharacter::GetPawnCombatComponent() const
{
	return HeroCombatComponent;
}

void AWarriorHeroCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	if (!CharacterStartUpData.IsNull())
	{
	if (UDataAsset_StartUpDataBase* LoadedData = CharacterStartUpData.LoadSynchronous())
	{LoadedData->GivenToAbilitySystemComponent(WarriorAbilitySystemComponent,1);
		
	}
	}
}

void AWarriorHeroCharacter::BeginPlay()
{
	Super::BeginPlay();

}

void AWarriorHeroCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{   // 确保已设置用于输入绑定的配置数据资产
    checkf(InputConfigDataAsset,TEXT("找不到有效的数据资产"));  
	
	// 调用父类以初始化基础输入绑定
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	// 从控制器获取本地玩家（用于访问本地子系统，如增强输入子系统）
	ULocalPlayer* LocalPlayer=GetController<APlayerController>()->GetLocalPlayer();
	
	// 通过本地玩家获取 Enhanced Input 的本地子系统
	UEnhancedInputLocalPlayerSubsystem*Subsystem= ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
	// 确保子系统有效
	check(Subsystem);
	
	// 将默认的 Mapping Context 添加到增强输入子系统（优先级 0）
	Subsystem->AddMappingContext(InputConfigDataAsset->DefaultMappingContext,0);
	
	// 将通用的 UInputComponent 转换为项目自定义的 UWarriorInputComponent（若失败会断言）
	UWarriorInputComponent* WarriorInputComponent= CastChecked<UWarriorInputComponent>(PlayerInputComponent);
	
	// 绑定原生输入动作：将 DataAsset 中标记为 Move 的动作，触发时调用本类的 Input_Move
	WarriorInputComponent->BindNativeInputAction(InputConfigDataAsset,WarriorGameplayTags::InputTag_Move,ETriggerEvent::Triggered,this,&ThisClass::Input_Move);
	
	WarriorInputComponent->BindNativeInputAction(InputConfigDataAsset,WarriorGameplayTags::InputTag_Look,ETriggerEvent::Triggered,this,&ThisClass::Input_Look);
	
	WarriorInputComponent->BindAbilityInputAction(InputConfigDataAsset,this,&ThisClass::Input_AbilityInputPressed,&ThisClass::Input_AbilityInputReleased);
}

void AWarriorHeroCharacter::Input_Move(const FInputActionValue& InputActionValue)
{				
	const FVector2D MovementVector = InputActionValue.Get<FVector2D>();
	
    const FRotator  MovementRotator(0.f,Controller->GetControlRotation().Yaw,0.f);
	
	if (MovementVector.Y!=0.f)
	{
		const FVector ForwardDirection =MovementRotator.RotateVector(FVector::ForwardVector) ;
		
		AddMovementInput(ForwardDirection,MovementVector.Y);
		
	}
	if (MovementVector.X!=0.f)
	{
		const FVector RightDirection = MovementRotator.RotateVector(FVector::RightVector);
		
		AddMovementInput(RightDirection,MovementVector.X);
	
	}
	
	// 处理移动输入的回调。InputActionValue 中会包含轴向值或按键状态
	// 例如：如果移动动作是二维向量，可以这样获取：
	// FVector2D MoveValue = InputActionValue.Get<FVector2D>();
	// 使用 MoveValue 来驱动角色移动，例如 AddMovementInput
}

void AWarriorHeroCharacter::Input_Look(const FInputActionValue& InputActionValue)
{
	const FVector2D LookAxisVector = InputActionValue.Get<FVector2D>();
	
	if (LookAxisVector.X!=0.f)
	{
		AddControllerYawInput(LookAxisVector.X);
	}
	
	if (LookAxisVector.Y!=0.f)
	{
		AddControllerPitchInput(LookAxisVector.Y);
	}
	// 处理视角输入的回调。InputActionValue 中会包含轴向值或按键状态
}

void AWarriorHeroCharacter::Input_AbilityInputPressed(FGameplayTag Input_Tag)
{
	WarriorAbilitySystemComponent->OnAbilityInputPressed(Input_Tag);
	
}

void AWarriorHeroCharacter::Input_AbilityInputReleased(FGameplayTag Input_Tag)
{
	WarriorAbilitySystemComponent->OnAbilityInputReleased(Input_Tag);
}
