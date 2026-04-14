// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAssets/StartUpData/DataAsset_StartUpDataBase.h"
#include  "Warrior/Public/AbilitySystem/Abilities/WarriorGameplayAbility.h"
#include "Warrior/Public/AbilitySystem/WarriorAbilitySystemComponent.h"
void UDataAsset_StartUpDataBase::GivenToAbilitySystemComponent(UWarriorAbilitySystemComponent* InAscToGive,
	int32 ApplyLevel)
{
	// ASC 必须有效，否则初始化能力/效果没有意义。
	check(InAscToGive);

	// 先授予主动与被动（反应型）能力。
	GrantAbilities(ActivateOnGivenAbilities, InAscToGive, ApplyLevel);
	GrantAbilities(ReactiveAbilities, InAscToGive, ApplyLevel);
	
	if (!StartUpGamePlayEffects.IsEmpty())
	{
		// 再应用开局就应生效的 GameplayEffect（例如基础属性修正）。
		for (const TSubclassOf<UGameplayEffect>& StartUpEffect : StartUpGamePlayEffects)
		{
			if (!StartUpEffect)
				continue;

			// 通过 CDO 获取效果模板，再按指定等级施加到自身。
			UGameplayEffect* EffectCDO=StartUpEffect->GetDefaultObject<UGameplayEffect>();
			
			{
				InAscToGive->ApplyGameplayEffectToSelf(EffectCDO,ApplyLevel,InAscToGive->MakeEffectContext());
			}
		}
		
		
	}
	
}

void  UDataAsset_StartUpDataBase::GrantAbilities(TArray<TSubclassOf<UWarriorGameplayAbility>>& InAbilitiesToGive,
	UWarriorAbilitySystemComponent* InAscToGive, int32 ApplyLevel)
{
	// 没有可授予能力时直接返回，避免无意义遍历。
	if (InAbilitiesToGive.IsEmpty())
	{
		return;
	}
	for (const TSubclassOf<UWarriorGameplayAbility>& Ability : InAbilitiesToGive)
	{
		if (!Ability)
			continue;

		// 构建能力规格：来源对象为 Avatar，等级由外部统一传入。
		FGameplayAbilitySpec AbilitySpec(Ability);
		AbilitySpec.SourceObject = InAscToGive->GetAvatarActor();
		AbilitySpec.Level = ApplyLevel;
		
		// 将能力注册到 ASC，后续由 GAS 生命周期管理。
		InAscToGive->GiveAbility(AbilitySpec);
	      
	
	}
}
