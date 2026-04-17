// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/WarriorWidgetBase.h"

#include "Components/UI/EnemyUIComponent.h"
#include "Warrior/Public/Interfaces/PawnUIInterface.h"
#include "Warrior/Public/Components/UI/PawnUIComponent.h"
void UWarriorWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (IPawnUIInterface* PawnUIInterface= Cast<IPawnUIInterface>(GetOwningPlayerPawn()))
	{
	if (UHeroUIComponent* HeroUIComponent = PawnUIInterface->GetHeroUIComponent())
	{
		BP_OnOwningHeroUIComponentInitialized(HeroUIComponent);
		
	}
		
	}
	
}

void UWarriorWidgetBase::InitEnemyWidget(AActor* EnemyActor)
{	
	if (IPawnUIInterface* PawnUIInterface= Cast<IPawnUIInterface>(EnemyActor))
	{
		if (UEnemyUIComponent* EnemyUIComponent = PawnUIInterface->GetEnemyUIComponent())
		{
			checkf(EnemyUIComponent,TEXT("UI生成失败"));
			BP_OnOwningEnemyUIComponentInitialized(EnemyUIComponent);
		}
		
	}
	
}
