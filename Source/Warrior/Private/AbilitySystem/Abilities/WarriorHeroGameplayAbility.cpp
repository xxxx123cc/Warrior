// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/WarriorHeroGameplayAbility.h"
#include "Characters/WarriorHeroCharacter.h"
#include "Controllers/WarriorHeroController.h"

AWarriorHeroCharacter* UWarriorHeroGameplayAbility::GetHeroCharacterFromActorInfo()
{//如果缓存的角色指针无效，尝试从当前的 ActorInfo 中获取角色并缓存起来-4.1
   if (!CachedHeroCharacter.IsValid())
   {
       CachedHeroCharacter= Cast<AWarriorHeroCharacter>(CurrentActorInfo->AvatarActor);
   }
	//多次使用时不用每次都重新 Cast/查找，同时又不增加对象生命周期耦合。
	return CachedHeroCharacter.IsValid()?CachedHeroCharacter.Get():nullptr;
	
 
}

AWarriorHeroController* UWarriorHeroGameplayAbility::GetHeroControllerFromActorInfo()
{//获取控制器
	if (!CachedHeroController.IsValid())
	{
		CachedHeroController= Cast<AWarriorHeroController>(CurrentActorInfo->PlayerController);
	}

	return CachedHeroController.IsValid()?CachedHeroController.Get():nullptr;
}

UHeroCombatComponent* UWarriorHeroGameplayAbility::GetHeroCombatComponentFromActorInfo()
{//获取战斗组件
	return GetHeroCharacterFromActorInfo()->GetHeroCombatComponent();
}
