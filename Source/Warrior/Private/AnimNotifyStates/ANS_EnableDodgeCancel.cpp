// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyStates/ANS_EnableDodgeCancel.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "WarriorGameplayTags.h"

namespace
{
	UWarriorAbilitySystemComponent* GetWarriorASCFromDodgeCancelMeshOwner(const USkeletalMeshComponent* MeshComp)
	{
		AActor* OwnerActor = MeshComp ? MeshComp->GetOwner() : nullptr;
		return OwnerActor
			? Cast<UWarriorAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerActor))
			: nullptr;
	}
}

UANS_EnableDodgeCancel::UANS_EnableDodgeCancel()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(55, 150, 255);
#endif
}

void UANS_EnableDodgeCancel::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (UWarriorAbilitySystemComponent* WarriorASC = GetWarriorASCFromDodgeCancelMeshOwner(MeshComp))
	{
		if (!WarriorASC->HasMatchingGameplayTag(WarriorGameplayTags::Player_Status_CanDodgeCancel))
		{
			WarriorASC->AddLooseGameplayTag(WarriorGameplayTags::Player_Status_CanDodgeCancel);
		}
	}
}

void UANS_EnableDodgeCancel::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!bRemoveDodgeCancelTagOnEnd || !MeshComp)
	{
		return;
	}

	if (UWarriorAbilitySystemComponent* WarriorASC = GetWarriorASCFromDodgeCancelMeshOwner(MeshComp))
	{
		if (WarriorASC->HasMatchingGameplayTag(WarriorGameplayTags::Player_Status_CanDodgeCancel))
		{
			WarriorASC->RemoveLooseGameplayTag(WarriorGameplayTags::Player_Status_CanDodgeCancel);
		}
	}
}

FString UANS_EnableDodgeCancel::GetNotifyName_Implementation() const
{
	return TEXT("Enable Dodge Cancel");
}
