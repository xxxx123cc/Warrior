// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WarriorWidgetBase.generated.h"
class UHeroUIComponent;
class UEnemyUIComponent;
/**
 *
 */
UCLASS()
class WARRIOR_API UWarriorWidgetBase : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized()override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintImplementableEvent,meta = (DisPlayName="OnOwningHeroUIComponentInitialized"))
	void BP_OnOwningHeroUIComponentInitialized(UHeroUIComponent* HeroUIComponent);
	UFUNCTION(BlueprintImplementableEvent,meta = (DisPlayName="OnOwningEnemyUIComponentInitialized"))
	void BP_OnOwningEnemyUIComponentInitialized(UEnemyUIComponent* EnemyUIComponent);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "RefreshInitialUIValues"))
	void BP_RefreshInitialUIValues(UHeroUIComponent* HeroUIComponent);
public:
	UFUNCTION(BlueprintCallable)
	void InitEnemyWidget(AActor* EnemyActor);

	UFUNCTION(BlueprintCallable)
	void RefreshHeroUIComponent();

	UFUNCTION()
	void OnControllerPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);
};