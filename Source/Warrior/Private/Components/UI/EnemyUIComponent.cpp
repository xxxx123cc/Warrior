// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/UI/EnemyUIComponent.h"

#include "Widgets/WarriorWidgetBase.h"

void UEnemyUIComponent::RegisterEnemyDrawWidget(UWarriorWidgetBase* InWidgetToRegister)
{
	EnemyWidgets.Add(InWidgetToRegister);
}

void UEnemyUIComponent::RemoveWidgetIfAny()
{
	if (EnemyWidgets.IsEmpty())
	{
		return;
	}
	for (UWarriorWidgetBase* Widget : EnemyWidgets)
	{
		if (Widget)
		{
			Widget->RemoveFromParent();
		}
		
	}
	
}
