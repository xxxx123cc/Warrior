// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "BTTask_ActivateAbilityAndWait.generated.h"

class UWarriorAbilitySystemComponent;
struct FGameplayAbilitySpec;

struct FActivateAbilityAndWaitTaskMemory
{
	TWeakObjectPtr<UWarriorAbilitySystemComponent> AbilitySystemComponent;
	FGameplayAbilitySpecHandle AbilitySpecHandle;
	bool bActivatedAbility = false;

	bool IsValid() const
	{
		return AbilitySystemComponent.IsValid() && AbilitySpecHandle.IsValid();
	}

	void Reset()
	{
		AbilitySystemComponent.Reset();
		AbilitySpecHandle = FGameplayAbilitySpecHandle();
		bActivatedAbility = false;
	}
};

/**
 * Activates a Gameplay Ability by tag and keeps the behavior tree task running
 * until that ability is no longer active.
 */
UCLASS()
class WARRIOR_API UBTTask_ActivateAbilityAndWait : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_ActivateAbilityAndWait();

	virtual uint16 GetInstanceMemorySize() const override;
	virtual FString GetStaticDescription() const override;
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	FGameplayAbilitySpec* FindAbilitySpec(UWarriorAbilitySystemComponent* AbilitySystemComponent) const;
	void FinishWaitingTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type Result) const;

	UPROPERTY(EditAnywhere, Category = "Ability")
	FGameplayTag AbilityTagToActivate;

	UPROPERTY(EditAnywhere, Category = "Ability")
	bool bWaitForAlreadyActiveAbility = true;

	UPROPERTY(EditAnywhere, Category = "Ability")
	bool bCancelAbilityOnAbort = true;
};
