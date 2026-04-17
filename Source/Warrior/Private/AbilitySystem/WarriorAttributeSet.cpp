// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/WarriorAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "WarriorDebugHelper.h"
#include "WarriorFunctionLibrary.h"
#include "WarriorGameplayTags.h"
#include "Chaos/Deformable/MuscleActivationConstraints.h"
#include "Components/UI/HeroUIComponent.h"
#include "Interfaces/PawnUIInterface.h"
#include "Components/UI/PawnUIComponent.h"
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
	//获取UI组件
	if (!CachedPawnUIInterface.IsValid())
	{
		//CachedPawnUIInterface = Cast<IPawnUIInterface>(Data.Target.GetAvatarActor());
		CachedPawnUIInterface = TWeakInterfacePtr<IPawnUIInterface>(Data.Target.GetAvatarActor());
	}
	//IPawnUIInterface* PawnUIInterface = Cast<IPawnUIInterface>( Data.Target.GetAvatarActor());
	checkf(CachedPawnUIInterface.IsValid(),TEXT("PawnUIInterface is not valid!"));
	UPawnUIComponent* PawnUIComponent = CachedPawnUIInterface->GetPawnUIComponent();
	checkf(PawnUIComponent, TEXT("PawnUIComponent is not valid!"));
	if ( Data.EvaluatedData.Attribute == GetCurrentHealthAttribute())
	{
		const float NewCurrentHealth = FMath::Clamp(GetCurrentHealth(), 0.f, GetMaxHealth());
		
		SetCurrentHealth(NewCurrentHealth);
		PawnUIComponent->OnCurrentHealthChanged.Broadcast(NewCurrentHealth / GetMaxHealth());
	}  
	if ( Data.EvaluatedData.Attribute == GetCurrentRageAttribute())
	{
		const float NewCurrentRage = FMath::Clamp(GetCurrentRage(), 0.f, GetMaxRage());
		
		SetCurrentRage(NewCurrentRage);
		//如果是玩家的话,更新怒气值UI
		if ( UHeroUIComponent* HeroUIComponent=CachedPawnUIInterface->GetHeroUIComponent())
		{
			HeroUIComponent->OnCurrentRageChanged.Broadcast(NewCurrentRage / GetMaxRage());
		}
		
	}  
	
	if ( Data.EvaluatedData.Attribute == GetDamageTakenAttribute())
	{
		const float OldHealth = GetCurrentHealth();
		const float DamageDone=GetDamageTaken();
		const float NewCurrentHealth = FMath::Clamp(OldHealth - DamageDone, 0.f, GetMaxHealth());
		SetCurrentHealth(NewCurrentHealth);
		PawnUIComponent->OnCurrentHealthChanged.Broadcast(NewCurrentHealth / GetMaxHealth());
		//const FString DebugString= FString::Printf(TEXT("DamageTaken: %f, OldHealth: %f, NewHealth: %f"),DamageDone,OldHealth,NewCurrentHealth);
		//Debug::print(DebugString);
		//处理角色死亡
		if (GetCurrentHealth()==0.f)
		{
			//Debug::print("die");
			UWarriorFunctionLibrary::AddGamePlayTagToActorIfNone(Data.Target.GetAvatarActor(),WarriorGameplayTags::Shared_Status_Death);
		}
	}  
	
}
