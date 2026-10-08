// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameFramework/Actor.h"
#include "WarriorTypes/WarriorStructTypes.h"
#include "WarriorMovingRectangleDamageArea.generated.h"

class UBoxComponent;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class USceneComponent;
class UStaticMeshComponent;

USTRUCT(BlueprintType)
struct FWarriorMovingRectangleDamageAreaConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DamageArea|Damage", meta=(ClampMin="1.0"))
	FVector DamageBoxSize = FVector(300.f, 500.f, 160.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DamageArea|Movement", meta=(ClampMin="0.0"))
	float TravelDistance = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DamageArea|Movement", meta=(ClampMin="0.0"))
	float MoveSpeed = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DamageArea|Warning", meta=(ClampMin="0.0"))
	float WarningDuration = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DamageArea|Warning", meta=(ClampMin="0.0"))
	float WarningAreaLength = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DamageArea|Warning", meta=(ClampMin="0.0"))
	float WarningAreaWidth = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DamageArea|Warning", meta=(ClampMin="0.0"))
	float WarningGroundOffset = 4.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DamageArea|Warning")
	FLinearColor WarningColor = FLinearColor(1.f, 0.f, 0.f, 0.45f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DamageArea|Warning", meta=(ClampMin="0.0", ClampMax="1.0"))
	float WarningOpacity = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DamageArea|Warning")
	bool bHideWarningWhenDamageStarts = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DamageArea|Ground", meta=(ClampMin="0.0"))
	float GroundTraceDistance = 1200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DamageArea|Ground")
	float DamageBoxGroundOffset = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DamageArea|Damage")
	bool bDamageTargetsOnlyOnce = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DamageArea|Debug")
	bool bDrawDebugDamageBox = false;
};

UCLASS()
class WARRIOR_API AWarriorMovingRectangleDamageArea : public AActor
{
	GENERATED_BODY()

public:
	AWarriorMovingRectangleDamageArea();

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category="DamageArea")
	void InitializeDamageArea(
		AActor* InDamageSource,
		const FGameplayEffectSpecHandle& InDamageSpecHandle,
		const FWarriorMovingRectangleDamageAreaConfig& InConfig);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category="DamageArea")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category="DamageArea")
	TObjectPtr<UBoxComponent> DamageBoxComponent;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category="DamageArea")
	TObjectPtr<UStaticMeshComponent> WarningAreaMeshComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="DamageArea|Warning")
	TObjectPtr<UMaterialInterface> WarningAreaMaterial = nullptr;

	UFUNCTION(BlueprintImplementableEvent, Category="DamageArea")
	void BP_OnDamageMovementStarted();

	UFUNCTION(BlueprintImplementableEvent, Category="DamageArea")
	void BP_OnTargetDamaged(AActor* DamagedActor);

	UFUNCTION(BlueprintImplementableEvent, Category="DamageArea")
	void BP_OnDamageAreaFinished();

private:
	void ConfigureComponents();
	void ConfigureDamageBox(float GroundRelativeZ);
	void ConfigureWarningArea(float GroundRelativeZ);
	void ApplyWarningAreaMaterial();
	void StartDamageMovement();
	void UpdateDamageBoxLocation();
	void CollectAndDamageOverlappingPawns();
	void TryApplyDamageToActor(AActor* TargetActor);
	void FinishDamageArea();

	float ResolveGroundRelativeZ() const;
	float GetMoveDuration() const;
	float GetResolvedDamageBoxLength() const;
	float GetResolvedWarningAreaLength() const;
	float GetResolvedWarningAreaWidth() const;
	bool HasDamagedActor(AActor* TargetActor) const;
	void MarkActorDamaged(AActor* TargetActor);

	FWarriorMovingRectangleDamageAreaConfig Config;
	FGameplayEffectSpecHandle DamageSpecHandle;
	TWeakObjectPtr<AActor> DamageSourceActor;
	TArray<TWeakObjectPtr<AActor>> DamagedActors;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> DynamicWarningAreaMaterial = nullptr;

	float WarningElapsedTime = 0.f;
	float DamageElapsedTime = 0.f;
	bool bInitialized = false;
	bool bDamageMovementStarted = false;
	bool bFinished = false;
};
