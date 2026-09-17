// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyStates/ANS_EnableMoveCancel.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "WarriorGameplayTags.h"

namespace
{
	UWarriorAbilitySystemComponent* GetWarriorASCFromMeshOwner(const USkeletalMeshComponent* MeshComp)
	{
		AActor* OwnerActor = MeshComp ? MeshComp->GetOwner() : nullptr;
		return OwnerActor
			? Cast<UWarriorAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerActor))
			: nullptr;
	}
}

UANS_EnableMoveCancel::UANS_EnableMoveCancel()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(58, 196, 125);
#endif
}

void UANS_EnableMoveCancel::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (UWarriorAbilitySystemComponent* WarriorASC = GetWarriorASCFromMeshOwner(MeshComp))
	{
		if (!WarriorASC->HasMatchingGameplayTag(WarriorGameplayTags::Player_Status_CanMoveCancel))
		{
			WarriorASC->AddLooseGameplayTag(WarriorGameplayTags::Player_Status_CanMoveCancel);
		}
	}
}

void UANS_EnableMoveCancel::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!bRemoveMoveCancelTagOnEnd || !MeshComp)
	{
		return;
	}

	if (UWarriorAbilitySystemComponent* WarriorASC = GetWarriorASCFromMeshOwner(MeshComp))
	{
		if (WarriorASC->HasMatchingGameplayTag(WarriorGameplayTags::Player_Status_CanMoveCancel))
		{
			WarriorASC->RemoveLooseGameplayTag(WarriorGameplayTags::Player_Status_CanMoveCancel);
		}
	}
}

FString UANS_EnableMoveCancel::GetNotifyName_Implementation() const
{
	return TEXT("Enable Move Cancel");
}
