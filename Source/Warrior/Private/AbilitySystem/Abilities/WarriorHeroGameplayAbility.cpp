// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/WarriorHeroGameplayAbility.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "Characters/WarriorHeroCharacter.h"
#include "Controllers/WarriorHeroController.h"
#include "WarriorGameplayTags.h"

AWarriorHeroCharacter* UWarriorHeroGameplayAbility::GetHeroCharacterFromActorInfo() const
{
	// 懒加载缓存：首次调用时从 ActorInfo 取角色并缓存，后续复用，减少重复 Cast。
	// 使用 WeakPtr 避免延长对象生命周期；对象销毁后 IsValid() 会自然失效。
	if (!CachedHeroCharacter.IsValid() && CurrentActorInfo)
	{
		CachedHeroCharacter = Cast<AWarriorHeroCharacter>(CurrentActorInfo->AvatarActor);
	}

	return CachedHeroCharacter.IsValid() ? CachedHeroCharacter.Get() : nullptr;
}

AWarriorHeroController* UWarriorHeroGameplayAbility::GetHeroControllerFromActorInfo() const
{
	// 与角色缓存逻辑一致：按需缓存 Controller，避免频繁查找。
	if (!CachedHeroController.IsValid() && CurrentActorInfo)
	{
		CachedHeroController = Cast<AWarriorHeroController>(CurrentActorInfo->PlayerController);
	}

	return CachedHeroController.IsValid() ? CachedHeroController.Get() : nullptr;
}

UHeroCombatComponent* UWarriorHeroGameplayAbility::GetHeroCombatComponentFromActorInfo() const
{
	// 依赖角色对象拿战斗组件；调用侧应确保当前能力确实绑定在 Hero 身上。
	AWarriorHeroCharacter* HeroCharacter = GetHeroCharacterFromActorInfo();
	return HeroCharacter ? HeroCharacter->GetHeroCombatComponent() : nullptr;
}

bool UWarriorHeroGameplayAbility::IsHeroAirborne() const
{
	const UWarriorAbilitySystemComponent* WarriorASC = GetWarriorASCFromActorInfo();
	return WarriorASC && WarriorASC->IsAvatarAirborne();
}

bool UWarriorHeroGameplayAbility::IsHeroRageActive() const
{
	const UWarriorAbilitySystemComponent* WarriorASC = GetWarriorASCFromActorInfo();
	return WarriorASC && WarriorASC->IsRageActive();
}

bool UWarriorHeroGameplayAbility::IsHeroAttackInputBlocked(FGameplayTag InputTag) const
{
	const UWarriorAbilitySystemComponent* WarriorASC = GetWarriorASCFromActorInfo();
	return WarriorASC && WarriorASC->IsInputBlockedByAttackState(InputTag);
}

TArray<FGameplayTag> UWarriorHeroGameplayAbility::ResolveHeroAbilityInputTagPriority(FGameplayTag InputTag) const
{
	const UWarriorAbilitySystemComponent* WarriorASC = GetWarriorASCFromActorInfo();
	return WarriorASC ? WarriorASC->ResolveAbilityInputTagPriority(InputTag) : TArray<FGameplayTag>();
}

FGameplayEffectSpecHandle UWarriorHeroGameplayAbility::HeroDamageEffectHandle(TSubclassOf<UGameplayEffect> EffectClass,
	float InWeaponBaseDamage, FGameplayTag InCurrentAttackTypeTag, int32 InUsedComboCount)
{
	check(EffectClass)

	// 1) 构建 EffectContext，标记能力、来源对象和施加者，供后续执行计算读取。
	FGameplayEffectContextHandle EffectContextHandle = GetWarriorASCFromActorInfo()->MakeEffectContext();
	EffectContextHandle.SetAbility(this);
	EffectContextHandle.AddSourceObject(GetAvatarActorFromActorInfo());
	EffectContextHandle.AddInstigator(GetAvatarActorFromActorInfo(), GetAvatarActorFromActorInfo());

	// 2) 生成 Outgoing Spec（可理解为“待应用的伤害数据包”）。
	FGameplayEffectSpecHandle EffectSpecHandle = GetWarriorASCFromActorInfo()->MakeOutgoingSpec(EffectClass, GetAbilityLevel(), EffectContextHandle);

	// 3) 通过 SetByCaller 注入运行时参数：基础伤害始终写入，攻击类型按需写入。
	EffectSpecHandle.Data->SetSetByCallerMagnitude(WarriorGameplayTags::Shared_SetByCaller_BaseDamage, InWeaponBaseDamage);

	if (InCurrentAttackTypeTag.IsValid())
	{
		EffectSpecHandle.Data->SetSetByCallerMagnitude(InCurrentAttackTypeTag, InUsedComboCount);
	}

	// 4) 返回 Spec 供外部应用到目标（ApplyGameplayEffectSpecToTarget/Actor 等）。
	return EffectSpecHandle;
}

bool UWarriorHeroGameplayAbility::GetAbilityRemainingCooldownByTag(FGameplayTag CooldownTag, float& TimeRemaining,
	float& TotalCooldownTime)
{
	check(CooldownTag.IsValid());
	
	FGameplayEffectQuery EffectQuery = FGameplayEffectQuery::MakeQuery_MatchAllOwningTags(CooldownTag.GetSingleTagContainer());
	TArray<TPair<float,float>>TimeRemainingAndDuration= GetAbilitySystemComponentFromActorInfo()->GetActiveEffectsTimeRemainingAndDuration(EffectQuery);
	
	if (!TimeRemainingAndDuration.IsEmpty())
	{
		TotalCooldownTime = TimeRemainingAndDuration[0].Value;
		TimeRemaining = TimeRemainingAndDuration[0].Key;
		
	}
	
	return TimeRemaining > 0.0f;
	
}
