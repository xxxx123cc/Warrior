// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimInstances/WarriorBaseAnimInstance.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

bool UWarriorBaseAnimInstance::DoesOwningActorHaveTag(FGameplayTag TagToCheck) const
{
	if (!TagToCheck.IsValid())
	{
		return false;
	}

	AActor* OwningActor = GetOwningActor();
	if (!OwningActor)
	{
		return false;
	}

	const UAbilitySystemComponent* AbilitySystemComponent =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwningActor);

	return AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(TagToCheck);
}

