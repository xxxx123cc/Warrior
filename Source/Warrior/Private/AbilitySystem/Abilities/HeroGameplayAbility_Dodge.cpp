// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/HeroGameplayAbility_Dodge.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "Characters/WarriorHeroCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "TimerManager.h"
#include "WarriorGameplayTags.h"

UHeroGameplayAbility_Dodge::UHeroGameplayAbility_Dodge(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	FGameplayTagContainer DodgeAbilityTags;
	DodgeAbilityTags.AddTag(WarriorGameplayTags::Player_Ability_Roll);
	SetAssetTags(DodgeAbilityTags);

	CooldownTag = WarriorGameplayTags::Player_CoolDown_Roll;
	DodgeActiveTags.AddTag(WarriorGameplayTags::Player_Status_Rolling);
}

bool UHeroGameplayAbility_Dodge::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const AWarriorHeroCharacter* HeroCharacter =
		ActorInfo ? Cast<AWarriorHeroCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	const UWarriorAbilitySystemComponent* WarriorASC =
		ActorInfo ? Cast<UWarriorAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get()) : nullptr;

	if (!HeroCharacter || !WarriorASC)
	{
		return false;
	}

	if (WarriorASC->HasMatchingGameplayTag(WarriorGameplayTags::Player_Status_Rolling) ||
		WarriorASC->HasMatchingGameplayTag(WarriorGameplayTags::Shared_Status_Death) ||
		(CooldownTag.IsValid() && WarriorASC->HasMatchingGameplayTag(CooldownTag)))
	{
		return false;
	}

	const UCharacterMovementComponent* MovementComponent = HeroCharacter->GetCharacterMovement();
	if (!MovementComponent || MovementComponent->MovementMode == MOVE_None ||
		MovementComponent->IsSwimming() || MovementComponent->IsFlying())
	{
		return false;
	}

	return bAllowDodgeWhileFalling || !MovementComponent->IsFalling();
}

void UHeroGameplayAbility_Dodge::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		return;
	}

	AWarriorHeroCharacter* HeroCharacter =
		ActorInfo ? Cast<AWarriorHeroCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (!HeroCharacter)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	HeroCharacter->SetRunning(true);

	const bool bHasMovementInput = HasDodgeMovementInput(*HeroCharacter);
	const FVector DodgeDirection = ResolveDodgeDirection(*HeroCharacter, bHasMovementInput);
	UAnimMontage* MontageToPlay = SelectDodgeMontage(bHasMovementInput);

	AddDodgeTags(ActorInfo);
	StartCooldown(ActorInfo);
	StartSuccessfulDodgeListener(ActorInfo);

	if (bIgnoreMoveInputDuringDodge)
	{
		if (AController* Controller = HeroCharacter->GetController())
		{
			Controller->SetIgnoreMoveInput(true);
			BlockedMoveInputController = Controller;
			bBlockedMoveInput = true;
		}
	}

	StartDodgeMovement(*HeroCharacter, DodgeDirection, bHasMovementInput);

	if (MontageToPlay)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask =
			UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
				this,
				NAME_None,
				MontageToPlay,
				MontagePlayRate);

		MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnDodgeFinished);
		MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnDodgeFinished);
		MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnDodgeFinished);
		MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnDodgeFinished);
		MontageTask->ReadyForActivation();
		return;
	}

	if (DodgeDuration <= 0.f)
	{
		OnDodgeFinished();
		return;
	}

	UAbilityTask_WaitDelay* DelayTask = UAbilityTask_WaitDelay::WaitDelay(this, DodgeDuration);
	DelayTask->OnFinish.AddDynamic(this, &ThisClass::OnDodgeFinished);
	DelayTask->ReadyForActivation();
}

void UHeroGameplayAbility_Dodge::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	RemoveDodgeTags(ActorInfo);

	if (bBlockedMoveInput && BlockedMoveInputController.IsValid())
	{
		BlockedMoveInputController->SetIgnoreMoveInput(false);
	}

	bBlockedMoveInput = false;
	BlockedMoveInputController.Reset();
	SuccessfulDodgeEventTask = nullptr;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UHeroGameplayAbility_Dodge::HasDodgeMovementInput(const AWarriorHeroCharacter& HeroCharacter) const
{
	const UCharacterMovementComponent* MovementComponent = HeroCharacter.GetCharacterMovement();

	FVector MovementInput = HeroCharacter.GetPendingMovementInputVector();
	if (MovementInput.IsNearlyZero())
	{
		MovementInput = HeroCharacter.GetLastMovementInputVector();
	}

	if (MovementInput.IsNearlyZero() && MovementComponent)
	{
		MovementInput = MovementComponent->GetCurrentAcceleration();
	}

	if (MovementInput.IsNearlyZero() && MovementComponent)
	{
		MovementInput = MovementComponent->Velocity;
	}

	MovementInput.Z = 0.f;
	return !MovementInput.IsNearlyZero();
}

FVector UHeroGameplayAbility_Dodge::ResolveDodgeDirection(
	const AWarriorHeroCharacter& HeroCharacter,
	bool bHasMovementInput) const
{
	const UCharacterMovementComponent* MovementComponent = HeroCharacter.GetCharacterMovement();

	if (!bHasMovementInput)
	{
		FVector BackwardDirection = -HeroCharacter.GetActorForwardVector();
		BackwardDirection.Z = 0.f;
		BackwardDirection.Normalize();
		return BackwardDirection;
	}

	FVector DodgeDirection = HeroCharacter.GetPendingMovementInputVector();
	if (DodgeDirection.IsNearlyZero())
	{
		DodgeDirection = HeroCharacter.GetLastMovementInputVector();
	}

	if (DodgeDirection.IsNearlyZero() && MovementComponent)
	{
		DodgeDirection = MovementComponent->GetCurrentAcceleration();
	}

	if (DodgeDirection.IsNearlyZero() && MovementComponent)
	{
		DodgeDirection = MovementComponent->Velocity;
	}

	DodgeDirection.Z = 0.f;
	if (!DodgeDirection.Normalize())
	{
		DodgeDirection = HeroCharacter.GetActorForwardVector();
		DodgeDirection.Z = 0.f;
		DodgeDirection.Normalize();
	}

	return DodgeDirection;
}

UAnimMontage* UHeroGameplayAbility_Dodge::SelectDodgeMontage(bool bHasMovementInput) const
{
	UAnimMontage* SelectedMontage = bHasMovementInput ? ForwardDodgeMontage.Get() : BackwardDodgeMontage.Get();
	return SelectedMontage ? SelectedMontage : DodgeMontage.Get();
}

void UHeroGameplayAbility_Dodge::StartDodgeMovement(
	AWarriorHeroCharacter& HeroCharacter,
	const FVector& DodgeDirection,
	bool bHasMovementInput) const
{
	if (bRotateToDodgeDirection && bHasMovementInput)
	{
		FRotator TargetRotation = DodgeDirection.ToOrientationRotator();
		TargetRotation.Pitch = 0.f;
		TargetRotation.Roll = 0.f;
		HeroCharacter.SetActorRotation(TargetRotation);
	}

	HeroCharacter.LaunchCharacter(DodgeDirection * DodgeLaunchStrength, true, false);
}

void UHeroGameplayAbility_Dodge::AddDodgeTags(const FGameplayAbilityActorInfo* ActorInfo)
{
	UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	if (!ASC || bAppliedDodgeTags)
	{
		return;
	}

	for (const FGameplayTag& Tag : DodgeActiveTags)
	{
		if (Tag.IsValid())
		{
			ASC->AddLooseGameplayTag(Tag);
		}
	}

	bAppliedDodgeTags = true;
}

void UHeroGameplayAbility_Dodge::RemoveDodgeTags(const FGameplayAbilityActorInfo* ActorInfo)
{
	UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	if (!ASC || !bAppliedDodgeTags)
	{
		return;
	}

	for (const FGameplayTag& Tag : DodgeActiveTags)
	{
		if (Tag.IsValid())
		{
			ASC->RemoveLooseGameplayTag(Tag);
		}
	}

	bAppliedDodgeTags = false;
}

void UHeroGameplayAbility_Dodge::StartCooldown(const FGameplayAbilityActorInfo* ActorInfo) const
{
	UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	AActor* AvatarActor = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr;
	UWorld* World = AvatarActor ? AvatarActor->GetWorld() : nullptr;
	if (!ASC || !World || !CooldownTag.IsValid() || DodgeCooldown <= 0.f)
	{
		return;
	}

	ASC->AddLooseGameplayTag(CooldownTag);

	TWeakObjectPtr<UAbilitySystemComponent> WeakASC = ASC;
	const FGameplayTag CooldownTagToRemove = CooldownTag;
	FTimerHandle CooldownTimerHandle;
	World->GetTimerManager().SetTimer(
		CooldownTimerHandle,
		FTimerDelegate::CreateLambda([WeakASC, CooldownTagToRemove]()
		{
			if (WeakASC.IsValid())
			{
				WeakASC->RemoveLooseGameplayTag(CooldownTagToRemove);
			}
		}),
		DodgeCooldown,
		false);
}

void UHeroGameplayAbility_Dodge::StartSuccessfulDodgeListener(const FGameplayAbilityActorInfo* ActorInfo)
{
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		return;
	}

	SuccessfulDodgeEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		WarriorGameplayTags::Player_Event_SuccessDodge,
		ActorInfo->AvatarActor.Get(),
		true,
		true);

	if (!SuccessfulDodgeEventTask)
	{
		return;
	}

	SuccessfulDodgeEventTask->EventReceived.AddDynamic(this, &ThisClass::OnSuccessfulDodgeEventReceived);
	SuccessfulDodgeEventTask->ReadyForActivation();
}

void UHeroGameplayAbility_Dodge::OnDodgeFinished()
{
	if (!IsActive())
	{
		return;
	}

	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void UHeroGameplayAbility_Dodge::OnSuccessfulDodgeEventReceived(FGameplayEventData Payload)
{
	BP_OnSuccessfulDodge(Payload);
}
