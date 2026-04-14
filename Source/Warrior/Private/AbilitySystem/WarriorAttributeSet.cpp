// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/WarriorAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "WarriorDebugHelper.h"
#include "WarriorFunctionLibrary.h"
#include "WarriorGameplayTags.h"
//初始化数值
UWarriorAttributeSet::UWarriorAttributeSet()
{
	InitCurrentHealth(1.f);
	InitMaxHealth(1.f);
	InitMaxRage(1.f);
	InitCurrentRage(1.f);
	InitAttackPower(1.f);
	InitDefensePower(1.f);
}
//游戏效果执行后
void UWarriorAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	if ( Data.EvaluatedData.Attribute == GetCurrentHealthAttribute())
	{
		const float NewCurrentHealth = FMath::Clamp(GetCurrentHealth(), 0.f, GetMaxHealth());
		
		SetCurrentHealth(NewCurrentHealth);
	}  
	if ( Data.EvaluatedData.Attribute == GetCurrentRageAttribute())
	{
		const float NewCurrentRage = FMath::Clamp(GetCurrentRage(), 0.f, GetMaxRage());
		
		SetCurrentRage(NewCurrentRage);
	}  
	
	if ( Data.EvaluatedData.Attribute == GetDamageTakenAttribute())
	{
		const float OldHealth = GetCurrentHealth();
		const float DamageDone=GetDamageTaken();
		const float NewCurrentHealth = FMath::Clamp(OldHealth - DamageDone, 0.f, GetMaxHealth());
		SetCurrentHealth(NewCurrentHealth);
		//const FString DebugString= FString::Printf(TEXT("DamageTaken: %f, OldHealth: %f, NewHealth: %f"),DamageDone,OldHealth,NewCurrentHealth);
		//Debug::print(DebugString);
		//处理角色死亡
		if (NewCurrentHealth==0.f)
		{
			//Debug::print("die");
			UWarriorFunctionLibrary::AddGamePlayTagToActorIfNone(Data.Target.GetAvatarActor(),WarriorGameplayTags::Shared_Status_Death);
		}
	}  
	
}
