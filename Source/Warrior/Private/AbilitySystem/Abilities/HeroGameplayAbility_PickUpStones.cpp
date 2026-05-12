// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/HeroGameplayAbility_PickUpStones.h"
#include "Components/UI/HeroUIComponent.h"
#include "Items/PickUps/WarriorStoneBase.h"
#include "Kismet/KismetsystemLibrary.h"
#include "Warrior/Public/Characters/WarriorHeroCharacter.h"
void UHeroGameplayAbility_PickUpStones::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	UHeroUIComponent* HeroUIComponent = GetHeroCharacterFromActorInfo()->GetHeroUIComponent();
	
	HeroUIComponent->OnStoneInteracted.Broadcast(true);
	
}

void UHeroGameplayAbility_PickUpStones::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	UHeroUIComponent* HeroUIComponent = GetHeroCharacterFromActorInfo()->GetHeroUIComponent();
	
	HeroUIComponent->OnStoneInteracted.Broadcast(false);
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	
}

void UHeroGameplayAbility_PickUpStones::PickUpStones()
{
	PickedUpStones.Empty();
	TArray<FHitResult> HitResults;
	UKismetSystemLibrary::BoxTraceMultiForObjects(
		GetHeroCharacterFromActorInfo(),
		GetHeroCharacterFromActorInfo()->GetActorLocation(),
		GetHeroCharacterFromActorInfo()->GetActorLocation()+GetHeroCharacterFromActorInfo()->GetActorForwardVector()*BoxTraceDistance,
		BoxTraceSize/2,
		(-GetAvatarActorFromActorInfo()->GetActorUpVector()).ToOrientationRotator(),
		StonesToPickUp,
		false,
		TArray<AActor*>(),
		bDrawDebugShape?EDrawDebugTrace::ForOneFrame:EDrawDebugTrace::None,
		HitResults,
		true
	);
	for (const FHitResult& HitResult : HitResults)
	{
		if (AWarriorStoneBase* FoundStone = Cast<AWarriorStoneBase>(HitResult.GetActor()))
		{
			PickedUpStones.AddUnique(FoundStone);
		}
		
	}
	if (PickedUpStones.IsEmpty())
	{
		CancelAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true);
	}
	
	
}

void UHeroGameplayAbility_PickUpStones::ConsumeStones()
{
	if (PickedUpStones.IsEmpty())
	{
		CancelAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true);
	return;
	}
	for (AWarriorStoneBase* Stone : PickedUpStones)
	{
		Stone->Consume(GetWarriorASCFromActorInfo(),GetAbilityLevel());
	}
	
	
}
