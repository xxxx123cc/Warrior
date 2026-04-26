// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Combat/EnemyCombatComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "WarriorGameplayTags.h"
#include "Abilities/GameplayAbilityTypes.h"
#include  "WarriorDebugHelper.h"
#include "WarriorFunctionLibrary.h"
void UEnemyCombatComponent::OnHitTargetActor(AActor* HitActor)
{Debug::print(TEXT("EnemyCombatComponent::OnHitTargetActor"));
	//敌人攻击玩家时，发送一个事件给玩家，玩家接到事件后播放受击动画
	if (OverlapActors.Contains(HitActor))
	{	
		return;
	}
	OverlapActors.AddUnique(HitActor);
	
	 bool bIsValidBlock=false;
	const bool bIsPlayerBlocking =UWarriorFunctionLibrary::NativeDoesActorHaveTag(HitActor,WarriorGameplayTags::Player_Status_Blocking);
	//此攻击角色不可格挡
	const bool bIsMyAttackUnblockable = false;
	FGameplayEventData EventData;
	EventData.Instigator = GetOwningPawn();
	EventData.Target = HitActor;
	if (bIsPlayerBlocking&&!bIsMyAttackUnblockable)
	{//实现格挡检测
		if (UWarriorFunctionLibrary::IsValidBlock(GetOwningPawn(),HitActor))
		{
			bIsValidBlock=true;	
			
		}
	}
	if (bIsValidBlock)
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(HitActor,WarriorGameplayTags::Player_Event_SuccessBlock,EventData);
	}
	else 
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetOwningPawn(),WarriorGameplayTags::Shared_Event_MeleeHit,EventData);
	

	
	
}

void UEnemyCombatComponent::OnWeaponEndOverlapTarget(AActor* EndOverlapActor)
{
	
}
