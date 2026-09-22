#include "Components/Team/TeamManagerComponent.h"
#include "Characters/WarriorHeroCharacter.h"
#include "DataAssets/Team/DataAsset_TeamConfig.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "AbilitySystem/WarriorAttributeSet.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "WarriorGameplayTags.h"
#include "Components/Combat/HeroCombatComponent.h"
#include "GameFramework/PlayerController.h"

UTeamManagerComponent::UTeamManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTeamManagerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UTeamManagerComponent::InitializeTeam(const UDataAsset_TeamConfig* TeamConfig)
{
	if (!TeamConfig || TeamConfig->TeamMembers.IsEmpty())
	{
		return;
	}

	TeamSlots = TeamConfig->TeamMembers;
	SwitchCooldown = TeamConfig->SwitchCooldown;

	BenchStates.SetNum(TeamSlots.Num());

	for (int32 i = 0; i < TeamSlots.Num(); ++i)
	{
		BenchStates[i] = FWarriorTeamMemberState();
		BenchStates[i].bIsDead = false;
	}

	const int32 FirstSlot = 0;
	const FWarriorTeamMemberEntry& FirstMember = TeamSlots[FirstSlot];
	if (!FirstMember.CharacterClass)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (!PC)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	APawn* OldPawn = PC->GetPawn();
	const FTransform SpawnTransform = OldPawn ? OldPawn->GetActorTransform() : FTransform::Identity;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AWarriorHeroCharacter* SpawnedCharacter = World->SpawnActor<AWarriorHeroCharacter>(
		FirstMember.CharacterClass,
		SpawnTransform,
		SpawnParams);

	if (!SpawnedCharacter)
	{
		return;
	}

	ActiveCharacter = SpawnedCharacter;
	ActiveSlotIndex = FirstSlot;
	BenchStates[FirstSlot].bIsDead = false;

	PC->Possess(SpawnedCharacter);
	if (OldPawn && OldPawn != SpawnedCharacter)
	{
		if (AWarriorHeroCharacter* OldHero = Cast<AWarriorHeroCharacter>(OldPawn))
		{
			if (UPawnCombatComponent* OldCombatComp = OldHero->GetPawnCombatComponent())
			{
				OldCombatComp->CleanupAllWeapons();
			}
		}
		OldPawn->Destroy();
	}

	BindActiveCharacterDeathEvent();

	OnTeamSlotSwitched.Broadcast(FirstSlot);
}

void UTeamManagerComponent::SwitchToSlot(int32 SlotIndex)
{
	if (bIsSwitching || SlotIndex == ActiveSlotIndex || SlotIndex < 0 || SlotIndex >= TeamSlots.Num())
	{
		return;
	}

	if (!IsSlotAlive(SlotIndex))
	{
		return;
	}

	if (!TeamSlots[SlotIndex].CharacterClass)
	{
		return;
	}

	bIsSwitching = true;

	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (!PC)
	{
		bIsSwitching = false;
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		bIsSwitching = false;
		return;
	}

	// Save current character state
	AWarriorHeroCharacter* OldCharacter = ActiveCharacter;
	FTransform SpawnTransform;

	if (OldCharacter)
	{
		SaveCurrentCharacterState();

		// Fire switch-out event on old character before unpossessing
		FGameplayEventData SwitchOutData;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OldCharacter, WarriorGameplayTags::Player_Event_SwitchOut, SwitchOutData);

		SpawnTransform = OldCharacter->GetActorTransform();
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AWarriorHeroCharacter* NewCharacter = World->SpawnActor<AWarriorHeroCharacter>(
		TeamSlots[SlotIndex].CharacterClass,
		SpawnTransform,
		SpawnParams);

	if (!NewCharacter)
	{
		bIsSwitching = false;
		return;
	}

	NewCharacter->TeamSlotIndex = SlotIndex;

	// Unbind old death event before destroying old character
	if (DeathTagDelegateHandle.IsValid() && OldCharacter)
	{
		UWarriorAbilitySystemComponent* OldASC = OldCharacter->GetWarriorAbilitySystemComponent();
		if (OldASC)
		{
			OldASC->RegisterGameplayTagEvent(WarriorGameplayTags::Shared_Status_Death, EGameplayTagEventType::NewOrRemoved).Remove(DeathTagDelegateHandle);
		}
		DeathTagDelegateHandle.Reset();
	}

	// Possess new character (triggers PossessedBy -> ASC init + startup data grant)
	PC->Possess(NewCharacter);

	// Restore state AFTER possess (ASC needs actor info initialized first)
	RestoreCharacterState(NewCharacter, BenchStates[SlotIndex]);

	ActiveCharacter = NewCharacter;
	ActiveSlotIndex = SlotIndex;

	// Destroy old character's weapons first, then the character itself
	if (OldCharacter)
	{
		if (UPawnCombatComponent* OldCombatComp = OldCharacter->GetPawnCombatComponent())
		{
			OldCombatComp->CleanupAllWeapons();
		}
		OldCharacter->Destroy();
	}

	BindActiveCharacterDeathEvent();

	// Fire switch events
	FGameplayEventData SwitchInData;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(NewCharacter, WarriorGameplayTags::Player_Event_SwitchIn, SwitchInData);

	OnTeamSlotSwitched.Broadcast(SlotIndex);
	bIsSwitching = false;
}

void UTeamManagerComponent::SaveCurrentCharacterState()
{
	if (!ActiveCharacter)
	{
		return;
	}

	UWarriorAttributeSet* AS = ActiveCharacter->GetWarriorAttributeSet();
	if (!AS)
	{
		return;
	}

	FWarriorTeamMemberState& State = BenchStates[ActiveSlotIndex];
	State.CurrentHealth = AS->GetCurrentHealth();
	State.MaxHealth = AS->GetMaxHealth();
	State.CurrentRage = AS->GetCurrentRage();
	State.MaxRage = AS->GetMaxRage();
	State.CurrentBlockValue = AS->GetCurrentBlockValue();
	State.MaxBlockValue = AS->GetMaxBlockValue();
	State.AttackPower = AS->GetAttackPower();
	State.DefensePower = AS->GetDefensePower();
	State.LastTransform = ActiveCharacter->GetActorTransform();

	UHeroCombatComponent* CombatComp = ActiveCharacter->GetHeroCombatComponent();
	if (CombatComp)
	{
		State.EquippedWeaponTag = CombatComp->CurrentEquippedWeaponTag;
	}

	UWarriorAbilitySystemComponent* ASC = ActiveCharacter->GetWarriorAbilitySystemComponent();
	if (ASC)
	{
		State.bIsDead = ASC->HasMatchingGameplayTag(WarriorGameplayTags::Shared_Status_Death);
	}
}

void UTeamManagerComponent::RestoreCharacterState(AWarriorHeroCharacter* Character, const FWarriorTeamMemberState& State)
{
	if (!Character)
	{
		return;
	}

	UWarriorAbilitySystemComponent* ASC = Character->GetWarriorAbilitySystemComponent();
	UWarriorAttributeSet* AS = Character->GetWarriorAttributeSet();
	if (!ASC || !AS)
	{
		return;
	}

	ASC->SetNumericAttributeBase(UWarriorAttributeSet::GetCurrentHealthAttribute(), State.CurrentHealth);
	ASC->SetNumericAttributeBase(UWarriorAttributeSet::GetMaxHealthAttribute(), State.MaxHealth);
	ASC->SetNumericAttributeBase(UWarriorAttributeSet::GetCurrentRageAttribute(), State.CurrentRage);
	ASC->SetNumericAttributeBase(UWarriorAttributeSet::GetMaxRageAttribute(), State.MaxRage);
	ASC->SetNumericAttributeBase(UWarriorAttributeSet::GetCurrentBlockValueAttribute(), State.CurrentBlockValue);
	ASC->SetNumericAttributeBase(UWarriorAttributeSet::GetMaxBlockValueAttribute(), State.MaxBlockValue);
	ASC->SetNumericAttributeBase(UWarriorAttributeSet::GetAttackPowerAttribute(), State.AttackPower);
	ASC->SetNumericAttributeBase(UWarriorAttributeSet::GetDefensePowerAttribute(), State.DefensePower);

	if (State.bIsDead)
	{
		ASC->AddLooseGameplayTag(WarriorGameplayTags::Shared_Status_Death);
	}

	// Re-equip weapon if one was equipped
	if (State.EquippedWeaponTag.IsValid())
	{
		for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
		{
			if (Spec.GetDynamicSpecSourceTags().HasTagExact(WarriorGameplayTags::InputTag_EquipAxe))
			{
				ASC->TryActivateAbility(Spec.Handle);
				break;
			}
		}
	}
}

void UTeamManagerComponent::OnActiveCharacterDied()
{
	if (!ActiveCharacter)
	{
		return;
	}

	SaveCurrentCharacterState();
	BenchStates[ActiveSlotIndex].bIsDead = true;

	const int32 NextSlot = GetNextAliveSlot();
	if (NextSlot == INDEX_NONE)
	{
		// All dead — broadcast game over via survive game mode state change
		return;
	}

	SwitchToSlot(NextSlot);
}

int32 UTeamManagerComponent::GetNextAliveSlot() const
{
	for (int32 i = 0; i < BenchStates.Num(); ++i)
	{
		const int32 Index = (ActiveSlotIndex + 1 + i) % BenchStates.Num();
		if (!BenchStates[Index].bIsDead)
		{
			return Index;
		}
	}
	return INDEX_NONE;
}

bool UTeamManagerComponent::IsSlotAlive(int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= BenchStates.Num())
	{
		return false;
	}
	return !BenchStates[SlotIndex].bIsDead;
}

const FWarriorTeamMemberState& UTeamManagerComponent::GetBenchState(int32 SlotIndex) const
{
	static FWarriorTeamMemberState Empty;
	if (SlotIndex < 0 || SlotIndex >= BenchStates.Num())
	{
		return Empty;
	}
	return BenchStates[SlotIndex];
}

void UTeamManagerComponent::BindActiveCharacterDeathEvent()
{
	if (DeathTagDelegateHandle.IsValid())
	{
		// Unbind previous
		if (ActiveCharacter)
		{
			UWarriorAbilitySystemComponent* OldASC = ActiveCharacter->GetWarriorAbilitySystemComponent();
			if (OldASC)
			{
				OldASC->RegisterGameplayTagEvent(WarriorGameplayTags::Shared_Status_Death, EGameplayTagEventType::NewOrRemoved).Remove(DeathTagDelegateHandle);
			}
		}
		DeathTagDelegateHandle.Reset();
	}

	if (!ActiveCharacter)
	{
		return;
	}

	UWarriorAbilitySystemComponent* ASC = ActiveCharacter->GetWarriorAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	DeathTagDelegateHandle = ASC->RegisterGameplayTagEvent(
		WarriorGameplayTags::Shared_Status_Death,
		EGameplayTagEventType::NewOrRemoved)
		.AddWeakLambda(this, [this](const FGameplayTag Tag, int32 NewCount)
		{
			if (NewCount > 0 && !bIsSwitching)
			{
				OnActiveCharacterDied();
			}
		});
}
