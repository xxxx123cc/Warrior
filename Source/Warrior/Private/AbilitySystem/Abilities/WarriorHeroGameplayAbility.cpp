// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/WarriorHeroGameplayAbility.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "Characters/WarriorHeroCharacter.h"
#include "Controllers/WarriorHeroController.h"
#include "WarriorGameplayTags.h"

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

FGameplayEffectSpecHandle UWarriorHeroGameplayAbility::HeroDamageEffectHandle(TSubclassOf<UGameplayEffect> EffectClass,
	float InWeaponBaseDamage, FGameplayTag InCurrentAttackTypeTag, int32 InUsedComboCount)
{
	check(EffectClass)
	FGameplayEffectContextHandle EffectContextHandle= GetWarriorASCFromActorInfo()->MakeEffectContext();
	EffectContextHandle.SetAbility(this);
	EffectContextHandle.AddSourceObject(GetAvatarActorFromActorInfo());
	EffectContextHandle.AddInstigator(GetAvatarActorFromActorInfo(),GetAvatarActorFromActorInfo());
	//创建 EffectSpec，MakeOutgoingSpec， 相当于生成一个“伤害包”，里面包含了伤害的各种信息（伤害数值、伤害类型、伤害来源等等），这个“伤害包”会被传递到后续的计算和应用过程中。
	FGameplayEffectSpecHandle EffectSpecHandle= GetWarriorASCFromActorInfo()->MakeOutgoingSpec(EffectClass,GetAbilityLevel(),EffectContextHandle);
	//SetSetByCallerMagnitude(...)往“伤害包”里放：基础伤害连击数 / 攻击类型
	EffectSpecHandle.Data->SetSetByCallerMagnitude(WarriorGameplayTags::Shared_SetByCaller_BaseDamage,InWeaponBaseDamage);
	
	if (InCurrentAttackTypeTag.IsValid())
	{
		EffectSpecHandle.Data->SetSetByCallerMagnitude(InCurrentAttackTypeTag,InUsedComboCount);
	}
	return EffectSpecHandle;
}
