// Fill out your copyright notice in the Description page of Project Settings.

#include "Ai/BTTask_ActivateAbilityAndWait.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

UBTTask_ActivateAbilityAndWait::UBTTask_ActivateAbilityAndWait()
{
	NodeName = TEXT("Activate Ability And Wait");

	bNotifyTick = true;
	bNotifyTaskFinished = true;

	INIT_TASK_NODE_NOTIFY_FLAGS();
}

uint16 UBTTask_ActivateAbilityAndWait::GetInstanceMemorySize() const
{
	return sizeof(FActivateAbilityAndWaitTaskMemory);
}

FString UBTTask_ActivateAbilityAndWait::GetStaticDescription() const
{
	const FString AbilityTagDescription = AbilityTagToActivate.IsValid()
		? AbilityTagToActivate.ToString()
		: TEXT("None");

	return FString::Printf(TEXT("Activate ability with tag: %s"), *AbilityTagDescription);
}

EBTNodeResult::Type UBTTask_ActivateAbilityAndWait::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FActivateAbilityAndWaitTaskMemory* Memory =
		CastInstanceNodeMemory<FActivateAbilityAndWaitTaskMemory>(NodeMemory);
	check(Memory);
	Memory->Reset();

	if (!AbilityTagToActivate.IsValid())
	{
		return EBTNodeResult::Failed;
	}

	const AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* OwnerPawn = AIController ? AIController->GetPawn() : nullptr;
	if (!OwnerPawn)
	{
		return EBTNodeResult::Failed;
	}

	UWarriorAbilitySystemComponent* AbilitySystemComponent =
		Cast<UWarriorAbilitySystemComponent>(
			UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerPawn));
	if (!AbilitySystemComponent)
	{
		return EBTNodeResult::Failed;
	}

	FGameplayAbilitySpec* AbilitySpec = FindAbilitySpec(AbilitySystemComponent);
	if (!AbilitySpec)
	{
		return EBTNodeResult::Failed;
	}

	const bool bWasAlreadyActive = AbilitySpec->IsActive();
	if (bWasAlreadyActive && !bWaitForAlreadyActiveAbility)
	{
		return EBTNodeResult::Failed;
	}

	Memory->AbilitySystemComponent = AbilitySystemComponent;
	Memory->AbilitySpecHandle = AbilitySpec->Handle;
	Memory->bActivatedAbility = !bWasAlreadyActive;

	if (!bWasAlreadyActive && !AbilitySystemComponent->TryActivateAbility(AbilitySpec->Handle))
	{
		Memory->Reset();
		return EBTNodeResult::Failed;
	}

	AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(Memory->AbilitySpecHandle);
	if (!AbilitySpec || !AbilitySpec->IsActive())
	{
		Memory->Reset();
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::InProgress;
}

void UBTTask_ActivateAbilityAndWait::TickTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	FActivateAbilityAndWaitTaskMemory* Memory =
		CastInstanceNodeMemory<FActivateAbilityAndWaitTaskMemory>(NodeMemory);
	if (!Memory || !Memory->IsValid())
	{
		FinishWaitingTask(OwnerComp, NodeMemory, EBTNodeResult::Failed);
		return;
	}

	UWarriorAbilitySystemComponent* AbilitySystemComponent = Memory->AbilitySystemComponent.Get();
	FGameplayAbilitySpec* AbilitySpec =
		AbilitySystemComponent->FindAbilitySpecFromHandle(Memory->AbilitySpecHandle);
	if (!AbilitySpec || !AbilitySpec->IsActive())
	{
		FinishWaitingTask(OwnerComp, NodeMemory, EBTNodeResult::Succeeded);
	}
}

EBTNodeResult::Type UBTTask_ActivateAbilityAndWait::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FActivateAbilityAndWaitTaskMemory* Memory =
		CastInstanceNodeMemory<FActivateAbilityAndWaitTaskMemory>(NodeMemory);
	if (Memory && Memory->IsValid())
	{
		if (bCancelAbilityOnAbort && Memory->bActivatedAbility)
		{
			Memory->AbilitySystemComponent->CancelAbilityHandle(Memory->AbilitySpecHandle);
		}

		Memory->Reset();
	}

	return Super::AbortTask(OwnerComp, NodeMemory);
}

FGameplayAbilitySpec* UBTTask_ActivateAbilityAndWait::FindAbilitySpec(
	UWarriorAbilitySystemComponent* AbilitySystemComponent) const
{
	if (!AbilitySystemComponent || !AbilityTagToActivate.IsValid())
	{
		return nullptr;
	}

	TArray<FGameplayAbilitySpec*> FoundAbilitySpecs;
	AbilitySystemComponent->GetActivatableGameplayAbilitySpecsByAllMatchingTags(
		AbilityTagToActivate.GetSingleTagContainer(),
		FoundAbilitySpecs);

	if (FoundAbilitySpecs.IsEmpty())
	{
		return nullptr;
	}

	TArray<FGameplayAbilitySpec*> InactiveAbilitySpecs;
	TArray<FGameplayAbilitySpec*> ActiveAbilitySpecs;

	for (FGameplayAbilitySpec* AbilitySpec : FoundAbilitySpecs)
	{
		if (!AbilitySpec)
		{
			continue;
		}

		if (AbilitySpec->IsActive())
		{
			ActiveAbilitySpecs.Add(AbilitySpec);
		}
		else
		{
			InactiveAbilitySpecs.Add(AbilitySpec);
		}
	}

	if (!InactiveAbilitySpecs.IsEmpty())
	{
		const int32 AbilityIndex = FMath::RandRange(0, InactiveAbilitySpecs.Num() - 1);
		return InactiveAbilitySpecs[AbilityIndex];
	}

	return bWaitForAlreadyActiveAbility && !ActiveAbilitySpecs.IsEmpty()
		? ActiveAbilitySpecs[0]
		: nullptr;
}

void UBTTask_ActivateAbilityAndWait::FinishWaitingTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	EBTNodeResult::Type Result) const
{
	if (FActivateAbilityAndWaitTaskMemory* Memory =
		CastInstanceNodeMemory<FActivateAbilityAndWaitTaskMemory>(NodeMemory))
	{
		Memory->Reset();
	}

	FinishLatentTask(OwnerComp, Result);
}
