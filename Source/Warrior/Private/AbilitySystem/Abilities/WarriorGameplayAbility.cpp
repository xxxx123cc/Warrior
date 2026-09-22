// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/WarriorGameplayAbility.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "AbilitySystem/WarriorAttributeSet.h"
#include "Components/Combat/PawnCombatComponent.h"
#include "Components/UI/PawnUIComponent.h"
#include "Interfaces/PawnUIInterface.h"
#include "WarriorDebugHelper.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "WarriorFunctionLibrary.h"
#include "WarriorGameplayTags.h"

// OnGiven 策略下，能力一旦授予即自动激活（常用于被动/初始化能力）。
void UWarriorGameplayAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	bMoveCancelBoundForCurrentActivation = false;
	MoveCancelEventTask = nullptr;

	if (ShouldUseMoveCancelForActivation(Handle, ActorInfo))
	{
		AddAttackMoveCancelTags(ActorInfo);
		StartMoveCancelListener(ActorInfo);
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UWarriorGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);
	
	if (AbilityActivationPolicy==EwarriorAbilityActivationPolicy::OnGiven)
	{
		if (ActorInfo && !Spec.IsActive())
		ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle);
		
	}
}

void UWarriorGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (bMoveCancelBoundForCurrentActivation)
	{
		RemoveAttackMoveCancelTags(ActorInfo);
		MoveCancelEventTask = nullptr;
		bMoveCancelBoundForCurrentActivation = false;
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	
	if (AbilityActivationPolicy==EwarriorAbilityActivationPolicy::OnGiven)
	{
		if (ActorInfo )
		{
			ActorInfo->AbilitySystemComponent->ClearAbility(Handle);
		}
		
	}
	
}

// 从 Avatar 身上快速定位战斗组件，供派生技能复用。
void UWarriorGameplayAbility::StartMoveCancelListener(const FGameplayAbilityActorInfo* ActorInfo)
{
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		return;
	}

	MoveCancelEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		WarriorGameplayTags::Player_Event_MoveCancel,
		ActorInfo->AvatarActor.Get(),
		true,
		true);

	if (!MoveCancelEventTask)
	{
		return;
	}

	MoveCancelEventTask->EventReceived.AddDynamic(this, &ThisClass::OnMoveCancelEventReceived);
	MoveCancelEventTask->ReadyForActivation();
	bMoveCancelBoundForCurrentActivation = true;
}

bool UWarriorGameplayAbility::ShouldUseMoveCancelForActivation(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo) const
{
	if (!bEnableAttackMoveCancel || !ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
	{
		return false;
	}

	const FGameplayAbilitySpec* Spec = ActorInfo->AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
	if (Spec)
	{
		for (const FGameplayTag& SourceTag : Spec->GetDynamicSpecSourceTags())
		{
			if (IsAttackInputTag(SourceTag))
			{
				return true;
			}
		}
	}

	const FGameplayTagContainer& AssetAbilityTags = GetAssetTags();
	return AssetAbilityTags.HasTagExact(WarriorGameplayTags::Player_Ability_Attack_Light_Axe) ||
		AssetAbilityTags.HasTagExact(WarriorGameplayTags::Player_Ability_Attack_Heavy_Axe);
}

bool UWarriorGameplayAbility::IsAttackInputTag(FGameplayTag InputTag)
{
	return InputTag.MatchesTagExact(WarriorGameplayTags::InputTag_LightAttack_Axe) ||
		InputTag.MatchesTagExact(WarriorGameplayTags::InputTag_LightAttack) ||
		InputTag.MatchesTagExact(WarriorGameplayTags::InputTag_HeavyAttack_Axe) ||
		InputTag.MatchesTagExact(WarriorGameplayTags::InputTag_HeavyAttack) ||
		InputTag.MatchesTagExact(WarriorGameplayTags::InputTag_HeavyAttack_Axe_Air) ||
		InputTag.MatchesTagExact(WarriorGameplayTags::InputTag_LightAttack_Axe_Rage_Ground) ||
		InputTag.MatchesTagExact(WarriorGameplayTags::InputTag_HeavyAttack_Axe_Rage_Ground) ||
		InputTag.MatchesTagExact(WarriorGameplayTags::InputTag_LightAttack_Axe_Rage_Air);
}

void UWarriorGameplayAbility::AddAttackMoveCancelTags(const FGameplayAbilityActorInfo* ActorInfo) const
{
	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		UWarriorFunctionLibrary::AddGamePlayTagToActorIfNone(
			ActorInfo->AvatarActor.Get(),
			WarriorGameplayTags::Player_Status_Attacking);
	}
}

void UWarriorGameplayAbility::RemoveAttackMoveCancelTags(const FGameplayAbilityActorInfo* ActorInfo) const
{
	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		UWarriorFunctionLibrary::RemoveGamePlayTagFromActorIfAny(
			ActorInfo->AvatarActor.Get(),
			WarriorGameplayTags::Player_Status_CanMoveCancel);
		UWarriorFunctionLibrary::RemoveGamePlayTagFromActorIfAny(
			ActorInfo->AvatarActor.Get(),
			WarriorGameplayTags::Player_Status_Attacking);
	}
}

void UWarriorGameplayAbility::OnMoveCancelEventReceived(FGameplayEventData Payload)
{
	if (!bMoveCancelBoundForCurrentActivation)
	{
		return;
	}

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor ||
		!UWarriorFunctionLibrary::NativeDoesActorHaveTag(AvatarActor, WarriorGameplayTags::Player_Status_CanMoveCancel))
	{
		return;
	}

	UWarriorFunctionLibrary::RemoveGamePlayTagFromActorIfAny(
		AvatarActor,
		WarriorGameplayTags::Player_Status_CanMoveCancel);

	MontageStop(MoveCancelMontageBlendOutTime);
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, true);
}

UPawnCombatComponent* UWarriorGameplayAbility::GetCombatComponentFromActorInfo() const
{
	return GetAvatarActorFromActorInfo()->FindComponentByClass<UPawnCombatComponent>();
	
}

// 返回项目自定义 ASC，避免每次在子类里重复 Cast。
UWarriorAbilitySystemComponent* UWarriorGameplayAbility::GetWarriorASCFromActorInfo() const
{
	return Cast<UWarriorAbilitySystemComponent>(CurrentActorInfo->AbilitySystemComponent);
	
}

// 对目标应用 GameplayEffectSpec：
// Spec 的构建由外部完成，这里仅负责执行投递。
FActiveGameplayEffectHandle UWarriorGameplayAbility::NativeApplyEffectSpecHandleToTarget(AActor* TargetActor,
	const FGameplayEffectSpecHandle& InEffectSpecHandle)
{
	check(InEffectSpecHandle.IsValid())
	check(TargetActor)
	
	
	UAbilitySystemComponent* TargetASC = TargetActor->FindComponentByClass<UAbilitySystemComponent>();
	

	FActiveGameplayEffectHandle EffectHandle = GetWarriorASCFromActorInfo()->ApplyGameplayEffectSpecToTarget(
		*InEffectSpecHandle.Data,
		TargetASC);

	return  EffectHandle;


}

FActiveGameplayEffectHandle UWarriorGameplayAbility::BP_ApplyEffectSpecHandleToTarget(AActor* TargetActor,
	const FGameplayEffectSpecHandle& InEffectSpecHandle, EWarriorSuccessful& OutSuccess)
{
	// 蓝图调用入口：返回句柄并同步给出成功/失败执行分支。
	
	FActiveGameplayEffectHandle EffectHandle= NativeApplyEffectSpecHandleToTarget(TargetActor,InEffectSpecHandle);
	OutSuccess= EffectHandle.WasSuccessfullyApplied()?EWarriorSuccessful::Successful:EWarriorSuccessful::Failed;
	return  EffectHandle;
}

void UWarriorGameplayAbility::ApplyGameplayEffectSpecHandleToHitResults(
	const FGameplayEffectSpecHandle& InEffectSpecHandle, const TArray<FHitResult>& HitResults)
{
	if (HitResults.IsEmpty()||!InEffectSpecHandle.IsValid())
	{
		return;
	}
	APawn*OwningPawn = Cast<APawn>(GetAvatarActorFromActorInfo());
	for (const FHitResult& HitResult : HitResults)
	{
		if (APawn *TargetPawn = Cast<APawn>(HitResult.GetActor()))
		{
			if (UWarriorFunctionLibrary::IsTargetPawnHostile(OwningPawn,TargetPawn))
			{
				FActiveGameplayEffectHandle ActiveGameplayEffectHandle =NativeApplyEffectSpecHandleToTarget(TargetPawn,InEffectSpecHandle);
				if (ActiveGameplayEffectHandle.WasSuccessfullyApplied())
				{
					FGameplayEventData EventData;
					EventData.Instigator = OwningPawn;
					EventData.Target = TargetPawn;

					if (UWarriorAbilitySystemComponent* TargetWarriorASC =
						Cast<UWarriorAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetPawn)))
					{
						UWarriorFunctionLibrary::ApplyBossPoiseDamage(
							TargetPawn,
							OwningPawn,
							TargetWarriorASC->GetBossPoiseDamageOnHit(),
							TargetWarriorASC->GetBossPoiseBreakStunDuration());
					}
					
					UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(TargetPawn, WarriorGameplayTags::Shared_Ability_HitReact, EventData);
					
				}
			}
		}
		
	}
	
	
}

void UWarriorGameplayAbility::BroadcastInitialCurrentHealth()
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor)
	{
		return;
	}

	IPawnUIInterface* PawnUIInterface = Cast<IPawnUIInterface>(AvatarActor);
	if (!PawnUIInterface)
	{
		return;
	}

	UPawnUIComponent* PawnUIComponent = PawnUIInterface->GetPawnUIComponent();
	const UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (!PawnUIComponent || !AbilitySystemComponent)
	{
		return;
	}

	const float MaxHealth = AbilitySystemComponent->GetNumericAttribute(UWarriorAttributeSet::GetMaxHealthAttribute());
	if (MaxHealth <= 0.f)
	{
		return;
	}

	const float CurrentHealth = FMath::Clamp(
		AbilitySystemComponent->GetNumericAttribute(UWarriorAttributeSet::GetCurrentHealthAttribute()),
		0.f,
		MaxHealth);
	PawnUIComponent->OnCurrentHealthChanged.Broadcast(CurrentHealth / MaxHealth);
}

// 标记连击输入已到达，供连击窗口逻辑在稍后消费。
void UWarriorGameplayAbility::OnComboInputPressed()
{
	bHasPendingComboInput = true;
	BP_OnComboInputPressed();
}
