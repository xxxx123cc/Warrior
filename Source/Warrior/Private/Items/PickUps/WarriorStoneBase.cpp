// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/PickUps/WarriorStoneBase.h"
#include "Warrior/Public/WarriorGameplayTags.h"
#include "Characters/WarriorHeroCharacter.h"
#include "Warrior/Public/AbilitySystem/WarriorAbilitySystemComponent.h"
void AWarriorStoneBase::OnPickUpMeshBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AWarriorHeroCharacter* HeroCharacter = Cast<AWarriorHeroCharacter>(OtherActor))
	{
		HeroCharacter->GetWarriorAbilitySystemComponent()->TryActivateAbilityByTag(WarriorGameplayTags::Player_Ability_PickUp_Stones);
		
	}
	
}

void AWarriorStoneBase::Consume(UWarriorAbilitySystemComponent* AbilitySystemComponent, int32 ApplyLevel)
{
	check(StoneEffectClass)
	AbilitySystemComponent->ApplyGameplayEffectToSelf(StoneEffectClass->GetDefaultObject<UGameplayEffect>(),ApplyLevel,AbilitySystemComponent->MakeEffectContext());
	 
	BP_OnStonesConsumed();
	
	
	
}
