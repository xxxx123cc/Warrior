// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Combat/EnemyCombatComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "WarriorGameplayTags.h"
#include "Abilities/GameplayAbilityTypes.h"

void UEnemyCombatComponent::OnHitTargetActor(AActor* HitActor)
{
	//敌人攻击玩家时，发送一个事件给玩家，玩家接到事件后播放受击动画
	if (OverlapActors.Contains(HitActor))
	{
		return;
	}
	OverlapActors.AddUnique(HitActor);
	
	 bool bIsValidBlock=false;
	const bool bIsPlayerBlocking = false;
	//此攻击角色不可格挡
	const bool bIsMyAttackUnblockable = false;
	FGameplayEventData EventData;
	EventData.Instigator = GetOwningPawn();
	EventData.Target = HitActor;
	if (bIsPlayerBlocking&&!bIsMyAttackUnblockable)
	{//实现格挡检测
		bIsValidBlock=true;	
	}
	if (bIsValidBlock)
	{
		
	}
	else
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetOwningPawn(),WarriorGameplayTags::Shared_Event_MeleeHit,EventData);
	}
	
	
	
}

void UEnemyCombatComponent::OnWeaponEndOverlapTarget(AActor* EndOverlapActor)
{
	
}
