#include "AbilitySystem/Abilities/HeroGameplayAbility_SwitchCharacter.h"
#include "Components/Team/TeamManagerComponent.h"
#include "WarriorGameplayTags.h"
#include "Characters/WarriorHeroCharacter.h"
#include "Controllers/WarriorHeroController.h"

void UHeroGameplayAbility_SwitchCharacter::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	const int32 TargetSlot = GetTargetSlotIndexFromSpec();
	if (TargetSlot == INDEX_NONE)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UTeamManagerComponent* TeamManager = GetTeamManagerComponent();
	if (!TeamManager)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (TeamManager->IsSwitching())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!TeamManager->IsSlotAlive(TargetSlot))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	TeamManager->SwitchToSlot(TargetSlot);

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

bool UHeroGameplayAbility_SwitchCharacter::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
	{
		return false;
	}

	AWarriorHeroController* Controller = Cast<AWarriorHeroController>(ActorInfo->PlayerController);
	if (!Controller)
	{
		return false;
	}

	UTeamManagerComponent* TeamManager = Controller->FindComponentByClass<UTeamManagerComponent>();
	if (!TeamManager || TeamManager->IsSwitching())
	{
		return false;
	}

	const FGameplayAbilitySpec* Spec = ActorInfo->AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
	if (!Spec)
	{
		return false;
	}

	int32 TargetSlot = INDEX_NONE;
	if (Spec->GetDynamicSpecSourceTags().HasTagExact(WarriorGameplayTags::InputTag_SwitchCharacter_1))
	{
		TargetSlot = 0;
	}
	else if (Spec->GetDynamicSpecSourceTags().HasTagExact(WarriorGameplayTags::InputTag_SwitchCharacter_2))
	{
		TargetSlot = 1;
	}
	else if (Spec->GetDynamicSpecSourceTags().HasTagExact(WarriorGameplayTags::InputTag_SwitchCharacter_3))
	{
		TargetSlot = 2;
	}

	if (TargetSlot == INDEX_NONE || TargetSlot == TeamManager->GetActiveSlotIndex())
	{
		return false;
	}

	return TeamManager->IsSlotAlive(TargetSlot);
}

int32 UHeroGameplayAbility_SwitchCharacter::GetTargetSlotIndexFromSpec() const
{
	const FGameplayAbilitySpec* Spec = GetCurrentAbilitySpec();
	if (!Spec)
	{
		return INDEX_NONE;
	}

	if (Spec->GetDynamicSpecSourceTags().HasTagExact(WarriorGameplayTags::InputTag_SwitchCharacter_1))
	{
		return 0;
	}
	if (Spec->GetDynamicSpecSourceTags().HasTagExact(WarriorGameplayTags::InputTag_SwitchCharacter_2))
	{
		return 1;
	}
	if (Spec->GetDynamicSpecSourceTags().HasTagExact(WarriorGameplayTags::InputTag_SwitchCharacter_3))
	{
		return 2;
	}

	return INDEX_NONE;
}

UTeamManagerComponent* UHeroGameplayAbility_SwitchCharacter::GetTeamManagerComponent() const
{
	AWarriorHeroController* Controller = GetHeroControllerFromActorInfo();
	if (!Controller)
	{
		return nullptr;
	}

	return Controller->FindComponentByClass<UTeamManagerComponent>();
}
