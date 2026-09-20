// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyStates/ANS_DodgeIFrame.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "WarriorGameplayTags.h"

namespace
{
	UWarriorAbilitySystemComponent* GetWarriorASCFromDodgeIFrameMeshOwner(const USkeletalMeshComponent* MeshComp)
	{
		AActor* OwnerActor = MeshComp ? MeshComp->GetOwner() : nullptr;
		return OwnerActor
			? Cast<UWarriorAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerActor))
			: nullptr;
	}
}

UANS_DodgeIFrame::UANS_DodgeIFrame()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(112, 219, 255);
#endif
}

void UANS_DodgeIFrame::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (UWarriorAbilitySystemComponent* WarriorASC = GetWarriorASCFromDodgeIFrameMeshOwner(MeshComp))
	{
		WarriorASC->AddLooseGameplayTag(WarriorGameplayTags::Player_Status_DodgeIFrame);

		if (bGrantInvincibleTag)
		{
			WarriorASC->AddLooseGameplayTag(WarriorGameplayTags::Shared_Status_Invincible);
		}
	}
}

void UANS_DodgeIFrame::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (UWarriorAbilitySystemComponent* WarriorASC = GetWarriorASCFromDodgeIFrameMeshOwner(MeshComp))
	{
		WarriorASC->RemoveLooseGameplayTag(WarriorGameplayTags::Player_Status_DodgeIFrame);

		if (bGrantInvincibleTag)
		{
			WarriorASC->RemoveLooseGameplayTag(WarriorGameplayTags::Shared_Status_Invincible);
		}
	}
}

FString UANS_DodgeIFrame::GetNotifyName_Implementation() const
{
	return TEXT("Dodge IFrame");
}
