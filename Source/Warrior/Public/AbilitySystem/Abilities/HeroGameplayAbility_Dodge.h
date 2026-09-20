// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/WarriorHeroGameplayAbility.h"
#include "GameplayTagContainer.h"
#include "HeroGameplayAbility_Dodge.generated.h"

class AController;
class AWarriorHeroCharacter;
class UAnimMontage;
class UAbilityTask_WaitGameplayEvent;

/**
 * Player dodge ability.
 * Uses the current movement input when available, then falls back to velocity or actor forward.
 */
UCLASS()
class WARRIOR_API UHeroGameplayAbility_Dodge : public UWarriorHeroGameplayAbility
{
	GENERATED_BODY()

public:
	UHeroGameplayAbility_Dodge(const FObjectInitializer& ObjectInitializer);

	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge|Animation")
	TObjectPtr<UAnimMontage> ForwardDodgeMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge|Animation")
	TObjectPtr<UAnimMontage> BackwardDodgeMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge|Animation", meta = (DisplayName = "Fallback Dodge Montage"))
	TObjectPtr<UAnimMontage> DodgeMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge|Animation", meta = (ClampMin = "0.0"))
	float MontagePlayRate = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge", meta = (ClampMin = "0.0"))
	float DodgeLaunchStrength = 1200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge", meta = (ClampMin = "0.0"))
	float DodgeDuration = 0.45f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge", meta = (ClampMin = "0.0"))
	float DodgeCooldown = 0.45f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge")
	bool bAllowDodgeWhileFalling = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge")
	bool bRotateToDodgeDirection = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge")
	bool bIgnoreMoveInputDuringDodge = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge", meta = (Categories = "Player.CoolDown"))
	FGameplayTag CooldownTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge")
	FGameplayTagContainer DodgeActiveTags;

	UFUNCTION(BlueprintImplementableEvent, Category = "Dodge")
	void BP_OnSuccessfulDodge(const FGameplayEventData& Payload);

private:
	bool HasDodgeMovementInput(const AWarriorHeroCharacter& HeroCharacter) const;
	FVector ResolveDodgeDirection(const AWarriorHeroCharacter& HeroCharacter, bool bHasMovementInput) const;
	UAnimMontage* SelectDodgeMontage(bool bHasMovementInput) const;
	void StartDodgeMovement(AWarriorHeroCharacter& HeroCharacter, const FVector& DodgeDirection, bool bHasMovementInput) const;
	void AddDodgeTags(const FGameplayAbilityActorInfo* ActorInfo);
	void RemoveDodgeTags(const FGameplayAbilityActorInfo* ActorInfo);
	void StartCooldown(const FGameplayAbilityActorInfo* ActorInfo) const;
	void StartSuccessfulDodgeListener(const FGameplayAbilityActorInfo* ActorInfo);

	UFUNCTION()
	void OnDodgeFinished();

	UFUNCTION()
	void OnSuccessfulDodgeEventReceived(FGameplayEventData Payload);

	bool bAppliedDodgeTags = false;
	bool bBlockedMoveInput = false;

	TWeakObjectPtr<AController> BlockedMoveInputController;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitGameplayEvent> SuccessfulDodgeEventTask = nullptr;
};
