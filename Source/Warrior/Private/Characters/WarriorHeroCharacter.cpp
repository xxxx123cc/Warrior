// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/WarriorHeroCharacter.h"
#include "WarriorDebugHelper.h"
#include "Camera/CameraComponent.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "components/CapsuleComponent.h"
#include "Engine/World.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include"EnhancedInputSubsystems.h"
#include"DataAssets/Input/DataAsset_InputConfig.h"
#include"Components/Input/WarriorInputComponent.h"
#include "WarriorGameplayTags.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/PlayerController.h"
#include "DataAssets/StartUpData/DataAsset_Hero_StartUpData.h"
#include "Components/Combat/HeroCombatComponent.h"
#include "Components/UI/HeroUIComponent.h"
#include "WarriorFunctionLibrary.h"
#include "GameModes/WarriorBaseGameMode.h"

namespace
{
	constexpr float JumpFloorProbeStartOffset = 6.f;
	constexpr float JumpFloorProbeDistance = 34.f;
	constexpr float JumpFloorProbeRadiusScale = 0.45f;
	constexpr float CameraCollisionProbeSize = 12.f;
}

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
	CameraBoom->bDoCollisionTest = true;
	CameraBoom->ProbeChannel = ECC_Camera;
	CameraBoom->ProbeSize = CameraCollisionProbeSize;
	//摄像机
	FollowCamera=CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom,USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	//角色移动组件
	GetCharacterMovement()->bOrientRotationToMovement=true;
	GetCharacterMovement()->RotationRate=FRotator(0.f,500.f,0.f);
	GetCharacterMovement()->MaxWalkSpeed=600.f;
	GetCharacterMovement()->BrakingDecelerationWalking=2000.f;
	JumpMaxCount = 2;
	
	HeroCombatComponent = CreateDefaultSubobject<UHeroCombatComponent>(TEXT("HeroCombatComponent"));
	
	HeroUIComponent = CreateDefaultSubobject<UHeroUIComponent>(TEXT("HeroUIComponent"));
}

UPawnCombatComponent* AWarriorHeroCharacter::GetPawnCombatComponent() const
{
	return HeroCombatComponent;
}

UPawnUIComponent* AWarriorHeroCharacter::GetPawnUIComponent() const
{
	
	return HeroUIComponent;
}

UHeroUIComponent* AWarriorHeroCharacter::GetHeroUIComponent() const
{

	return HeroUIComponent;
}

void AWarriorHeroCharacter::Jump()
{
	if (!Super::CanJumpInternal_Implementation() && HasJumpableFloor())
	{
		JumpCurrentCount = 0;
		JumpCurrentCountPreJump = 0;
	}

	Super::Jump();
}

void AWarriorHeroCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	if (!CharacterStartUpData.IsNull())
	{
	if (UDataAsset_StartUpDataBase* LoadedData = CharacterStartUpData.LoadSynchronous())
	{
		int32 AbilityApplyLevel = 1;
		if (AWarriorBaseGameMode* BaseGameMode = GetWorld()->GetAuthGameMode<AWarriorBaseGameMode>())
		{
			switch (BaseGameMode->GetGameDifficulty())
			{
				case WarriorDifficulty::Easy:
					AbilityApplyLevel = 4;
					break;
				case WarriorDifficulty::Normal:
					AbilityApplyLevel = 3;
					break;
				case WarriorDifficulty::Medium:
					AbilityApplyLevel = 2;
					break;	
				case WarriorDifficulty::Hard:
				    AbilityApplyLevel = 1;
				default:
					break;
				
			};
			
			
		}
		LoadedData->GivenToAbilitySystemComponent(WarriorAbilitySystemComponent,AbilityApplyLevel);
		
	}
	}
}

void AWarriorHeroCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (CameraBoom)
	{
		CameraBoom->bDoCollisionTest = true;
		CameraBoom->ProbeChannel = ECC_Camera;
		CameraBoom->ProbeSize = CameraCollisionProbeSize;
	}
}

void AWarriorHeroCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	FGameplayEventData Data;
	Data.EventTag = WarriorGameplayTags::Player_Event_Landed;
	Data.Instigator = this;
	Data.Target = this;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, WarriorGameplayTags::Player_Event_Landed, Data);
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
	
	WarriorInputComponent->BindNativeInputAction(InputConfigDataAsset,WarriorGameplayTags::InputTag_SwitchTarget,ETriggerEvent::Triggered,this,&ThisClass::Input_SwitchTargetTriggered);
	
	WarriorInputComponent->BindNativeInputAction(InputConfigDataAsset,WarriorGameplayTags::InputTag_SwitchTarget,ETriggerEvent::Completed,this,&ThisClass::Input_SwitchTargetCompleted);
	
	WarriorInputComponent->BindNativeInputAction(InputConfigDataAsset,WarriorGameplayTags::InputTag_PickUp_Stones,ETriggerEvent::Started,this,&ThisClass::Input_PickUpStonesStarted);
	
	WarriorInputComponent->BindAbilityInputAction(InputConfigDataAsset,this,&ThisClass::Input_AbilityInputPressed,&ThisClass::Input_AbilityInputReleased);
	
	
}

void AWarriorHeroCharacter::Input_Move(const FInputActionValue& InputActionValue)
{				
	const FVector2D MovementVector = InputActionValue.Get<FVector2D>();

	if (!MovementVector.IsNearlyZero() &&
		UWarriorFunctionLibrary::NativeDoesActorHaveTag(this, WarriorGameplayTags::Player_Status_CanMoveCancel))
	{
		FGameplayEventData Data;
		Data.EventTag = WarriorGameplayTags::Player_Event_MoveCancel;
		Data.Instigator = this;
		Data.Target = this;

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, WarriorGameplayTags::Player_Event_MoveCancel, Data);
	}

	const FRotator MovementRotator(0.f, Controller->GetControlRotation().Yaw, 0.f);

	if (MovementVector.Y != 0.f)
	{
		const FVector ForwardDirection = MovementRotator.RotateVector(FVector::ForwardVector);
		
		AddMovementInput(ForwardDirection, MovementVector.Y);
		
	}
	if (MovementVector.X != 0.f)
	{
		const FVector RightDirection = MovementRotator.RotateVector(FVector::RightVector);
		
		AddMovementInput(RightDirection, MovementVector.X);
	
	}
	
	// 处理移动输入的回调。InputActionValue 中会包含轴向值或按键状态
	// 例如：如果移动动作是二维向量，可以这样获取：
	// FVector2D MoveValue = InputActionValue.Get<FVector2D>();
	// 使用 MoveValue 来驱动角色移动，例如 AddMovementInput
}

void AWarriorHeroCharacter::Input_Look(const FInputActionValue& InputActionValue)
{
	if (UWarriorFunctionLibrary::NativeDoesActorHaveTag(this, WarriorGameplayTags::Player_Status_TargetLock))
	{
		return;
	}

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

void AWarriorHeroCharacter::Input_SwitchTargetTriggered(const FInputActionValue& InputActionValue)
{
	SwitchDirection = InputActionValue.Get<FVector2D>();
}

void AWarriorHeroCharacter::Input_SwitchTargetCompleted(const FInputActionValue& InputActionValue)
{
	FGameplayEventData Data;
	
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this,
	SwitchDirection.X>0.f?WarriorGameplayTags::Player_Event_SwitchTarget_Right:WarriorGameplayTags::Player_Event_SwitchTarget_Left
	,Data);
	
}

void AWarriorHeroCharacter::Input_PickUpStonesStarted(const FInputActionValue& InputActionValue)
{ 
	FGameplayEventData Data;
	
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this,
	WarriorGameplayTags::Player_Event_ConsumeStones
	,Data);
}

void AWarriorHeroCharacter::Input_AbilityInputPressed(FGameplayTag Input_Tag)
{
	WarriorAbilitySystemComponent->OnAbilityInputPressed(Input_Tag);
	
}

void AWarriorHeroCharacter::Input_AbilityInputReleased(FGameplayTag Input_Tag)
{
	WarriorAbilitySystemComponent->OnAbilityInputReleased(Input_Tag);
}

bool AWarriorHeroCharacter::CanJumpInternal_Implementation() const
{
	if (Super::CanJumpInternal_Implementation())
	{
		return true;
	}

	const UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (!MovementComponent || bIsCrouched || JumpMaxCount <= 0)
	{
		return false;
	}

	if (MovementComponent->IsSwimming() || MovementComponent->IsFlying() || MovementComponent->MovementMode == MOVE_None)
	{
		return false;
	}

	return HasJumpableFloor();
}

bool AWarriorHeroCharacter::HasJumpableFloor() const
{
	const UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	const UCapsuleComponent* HeroCapsuleComponent = GetCapsuleComponent();
	const UWorld* World = GetWorld();
	if (!MovementComponent || !HeroCapsuleComponent || !World)
	{
		return false;
	}

	const float CapsuleRadius = HeroCapsuleComponent->GetScaledCapsuleRadius();
	const float CapsuleHalfHeight = HeroCapsuleComponent->GetScaledCapsuleHalfHeight();
	const float ProbeOffset = CapsuleRadius * JumpFloorProbeRadiusScale;
	const FVector FeetLocation = GetActorLocation() - FVector(0.f, 0.f, CapsuleHalfHeight);
	const FVector ProbeOffsets[] =
	{
		FVector::ZeroVector,
		GetActorForwardVector() * ProbeOffset,
		-GetActorForwardVector() * ProbeOffset,
		GetActorRightVector() * ProbeOffset,
		-GetActorRightVector() * ProbeOffset
	};

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(WarriorHeroJumpFloorProbe), false, this);
	QueryParams.AddIgnoredActor(this);

	const ECollisionChannel TraceChannel = HeroCapsuleComponent->GetCollisionObjectType();
	for (const FVector& ProbeOffsetVector : ProbeOffsets)
	{
		TArray<FHitResult> Hits;
		const FVector Start = FeetLocation + ProbeOffsetVector + FVector(0.f, 0.f, JumpFloorProbeStartOffset);
		const FVector End = FeetLocation + ProbeOffsetVector - FVector(0.f, 0.f, JumpFloorProbeDistance);

		if (!World->LineTraceMultiByChannel(Hits, Start, End, TraceChannel, QueryParams))
		{
			continue;
		}

		for (const FHitResult& Hit : Hits)
		{
			if (Hit.bBlockingHit && MovementComponent->IsWalkable(Hit))
			{
				return true;
			}
		}
	}

	return false;
}
