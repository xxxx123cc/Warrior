// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/WarriorEnemyGameplayAbility.h"

#include "WarriorGameplayTags.h"
#include "Characters/WarriorEnemyCharacter.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
AWarriorEnemyCharacter* UWarriorEnemyGameplayAbility::GetEnemyCharacterFromActorInfor()
{
	if (!CachedEnemyCharacter.IsValid())
	CachedEnemyCharacter = Cast<AWarriorEnemyCharacter>(GetAvatarActorFromActorInfo());
	return CachedEnemyCharacter.IsValid() ? CachedEnemyCharacter.Get() : nullptr;
	
}

UEnemyCombatComponent* UWarriorEnemyGameplayAbility::GetEnemyCombatComponent()
{
	return GetEnemyCharacterFromActorInfor()->GetEnemyCombatComponent();
}



FGameplayEffectSpecHandle UWarriorEnemyGameplayAbility::EnemyDamageEffectHandle(
	TSubclassOf<UGameplayEffect> EffectClass,const FScalableFloat& InDamageScalableFloat)
{
	check(EffectClass);
	// 1) 构建 EffectContext，标记能力、来源对象和施加者，供后续执行计算读取。
	FGameplayEffectContextHandle EffectContextHandle = GetWarriorASCFromActorInfo()->MakeEffectContext();
	EffectContextHandle.SetAbility(this);
	EffectContextHandle.AddSourceObject(GetAvatarActorFromActorInfo());
	EffectContextHandle.AddInstigator(GetAvatarActorFromActorInfo(), GetAvatarActorFromActorInfo());
	
	FGameplayEffectSpecHandle EffectSpecHandle = GetWarriorASCFromActorInfo()->MakeOutgoingSpec(EffectClass, GetAbilityLevel(), EffectContextHandle);
	
	EffectSpecHandle.Data->SetSetByCallerMagnitude(WarriorGameplayTags::Shared_SetByCaller_BaseDamage, InDamageScalableFloat.GetValueAtLevel(GetAbilityLevel()));
	
	return EffectSpecHandle;
}
