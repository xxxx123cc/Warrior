// Fill out your copyright notice in the Description page of Project Settings.


#include "WarriorFunctionLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Warrior/Public/AbilitySystem/WarriorAbilitySystemComponent.h"
//#include "BlueprintGameplayTagLibrary.h"
//#include "Warrior/Public/Characters/WarriorBaseCharacter.h"
#include"Warrior/Public/Interfaces/PawnCombatInterface.h"
#include "GenericTeamAgentInterface.h"
#include "MeshPaintVisualize.h"
#include "WarriorDebugHelper.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/Combat/PawnCombatComponent.h"
#include "Items/Weapons/WarriorWeaponBase.h"
#include "WarriorGameplayTags.h"
/**
 * @brief 从Actor获取WarriorAbilitySystemComponent
 * @param InActor 目标Actor
 * @return UWarriorAbilitySystemComponent* Warrior自定义的能力系统组件
 * @note 使用CastChecked确保类型安全，如果获取失败会触发断言
 */
UWarriorAbilitySystemComponent* UWarriorFunctionLibrary::NativeGetWarriorAscFromActor(AActor* InActor)
{
	// 确保传入的Actor指针有效
	check(InActor);
	
	// 通过AbilitySystemBlueprintLibrary获取ASC，并强制转换为Warrior自定义类型
	return CastChecked<UWarriorAbilitySystemComponent> (UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InActor));
}

/**
 * @brief 如果Actor没有指定Tag，则添加该Tag
 * @param InActor 目标Actor
 * @param TagToAdd 要添加的GameplayTag
 */
void UWarriorFunctionLibrary::AddGamePlayTagToActorIfNone(AActor* InActor, FGameplayTag TagToAdd)
{
	// 获取Actor的ASC
	UWarriorAbilitySystemComponent* ASC = NativeGetWarriorAscFromActor(InActor);
	
	// 检查ASC有效性，并确保Actor当前不拥有该Tag
	if (ASC && !ASC->HasMatchingGameplayTag(TagToAdd))
	{
		// 添加为LooseGameplayTag（非GE授予的自由标签）
		ASC->AddLooseGameplayTag(TagToAdd);
	}
}

/**
 * @brief 如果Actor拥有指定Tag，则移除该Tag
 * @param InActor 目标Actor
 * @param TagToRemove 要移除的GameplayTag
 */
void UWarriorFunctionLibrary::RemoveGamePlayTagFromActorIfAny(AActor* InActor, FGameplayTag TagToRemove)
{
	// 获取Actor的ASC
	UWarriorAbilitySystemComponent* ASC = NativeGetWarriorAscFromActor(InActor);
	
	// 检查ASC有效性，并确保Actor当前拥有该Tag
	if (ASC && ASC->HasMatchingGameplayTag(TagToRemove))
	{
		// 移除LooseGameplayTag
		ASC->RemoveLooseGameplayTag(TagToRemove);
	}
}

/**
 * @brief 检查Actor是否拥有指定的GameplayTag（C++版本）
 * @param InActor 目标Actor
 * @param TagToCheck 要检查的GameplayTag
 * @return bool 是否拥有该Tag
 */
bool UWarriorFunctionLibrary::NativeDoesActorHaveTag(AActor* InActor, FGameplayTag TagToCheck)
{
	// 获取Actor的ASC
	UWarriorAbilitySystemComponent* ASC = NativeGetWarriorAscFromActor(InActor);
	
	// 如果ASC有效，检查是否拥有指定Tag
	if (ASC)
	{
		return ASC->HasMatchingGameplayTag(TagToCheck);
	}
	
	// ASC无效时返回false（注意：原代码缺少返回值）
	return false;
}

/**
 * @brief 检查Actor是否拥有指定的GameplayTag（Blueprint版本）
 * @param InActor 目标Actor
 * @param TagToCheck 要检查的GameplayTag
 * @param OutConfirm 输出确认结果，用于Blueprint的Exec引脚分支
 */
void UWarriorFunctionLibrary::BP_DoesActorHaveTag(AActor* InActor, FGameplayTag TagToCheck,
	EWarriorConfirmType& OutConfirm)
{
	// 根据Tag检查结果设置输出枚举值
	// 使用三元运算符：如果拥有Tag则输出Yes，否则输出No
	NativeDoesActorHaveTag(InActor, TagToCheck) ? OutConfirm = EWarriorConfirmType::Yes : OutConfirm = EWarriorConfirmType::No;
}

UPawnCombatComponent* UWarriorFunctionLibrary::NativeGetPawnCombatComponentFromActor(AActor* InActor)
{
	check(InActor)
	if (IPawnCombatInterface*PawnCombatInterface= Cast<IPawnCombatInterface>(InActor))
	{
	return PawnCombatInterface->GetPawnCombatComponent();	
	}
	return nullptr;
}

UPawnCombatComponent* UWarriorFunctionLibrary::BP_GetPawnCombatComponentFromActor(AActor* InActor,
	EWarriorValidType& OutValid)
{
	UPawnCombatComponent*CombatComponent= NativeGetPawnCombatComponentFromActor(InActor) ;
     OutValid= CombatComponent?EWarriorValidType::Valid:EWarriorValidType::InValid;
	 return CombatComponent;
}

bool UWarriorFunctionLibrary::IsTargetPawnHostile(APawn* QueryPawn, APawn* TargetPawn)
{
	// 获取两个Pawn的TeamID
	FGenericTeamId QueryTeamID = Cast<IGenericTeamAgentInterface>(QueryPawn->GetController())->GetGenericTeamId();
	FGenericTeamId TargetTeamID = Cast<IGenericTeamAgentInterface>(TargetPawn->GetController())->GetGenericTeamId();
	
	if (QueryTeamID == TargetTeamID)
		return false;
	else
	{
		return true;
	}
}

FGameplayTag UWarriorFunctionLibrary::ComputeAttackDirectionTag(AActor* AttackerPawn, AActor* TargetPawn,
	float& OutAngleDifference)
{
	check(AttackerPawn&&TargetPawn)
	
	const FVector TargetForwardVector = TargetPawn->GetActorForwardVector();
	
	const FVector TargetToAttackerPawnNormalized = (AttackerPawn->GetActorLocation() - TargetPawn->GetActorLocation()).GetSafeNormal();
	
	const float DotResult = FVector::DotProduct(TargetForwardVector, TargetToAttackerPawnNormalized);
	
	OutAngleDifference = UKismetMathLibrary::DegAcos(DotResult);
	
	const FVector CrossResult = FVector::CrossProduct(TargetForwardVector, TargetToAttackerPawnNormalized);
	
	if (CrossResult.Z <0)
	{
		OutAngleDifference = -OutAngleDifference;
	}
	if (OutAngleDifference>=-45&&OutAngleDifference<=45)
	{
		return WarriorGameplayTags::Shared_Status_HitReact_Front;
	}
	if (OutAngleDifference>45&&OutAngleDifference<=135)
	{
		return WarriorGameplayTags::Shared_Status_HitReact_Right;
	}
	if (OutAngleDifference<-45&&OutAngleDifference>=-135)
	{
		return WarriorGameplayTags::Shared_Status_HitReact_Left;
	}	
	return WarriorGameplayTags::Shared_Status_HitReact_Back;
	
}


bool UWarriorFunctionLibrary::IsValidBlock(AActor* InAttacker, AActor* InDefender)
{
	check(InAttacker && InDefender);

	constexpr float CloseRangeThreshold = 150.f;
	constexpr float BlockHalfAngleDegrees = 70.f;
	const float BlockDotThreshold = FMath::Cos(FMath::DegreesToRadians(BlockHalfAngleDegrees));

	FVector AttackOrigin = InAttacker->GetActorLocation();

	if (UPawnCombatComponent* AttackerCombatComponent = NativeGetPawnCombatComponentFromActor(InAttacker))
	{
		if (AWarriorWeaponBase* CurrentWeapon = AttackerCombatComponent->GetCurrentEquippedWeapon())
		{
			if (UBoxComponent* WeaponCollisionBox = CurrentWeapon->GetWeaponCollisionMesh())
			{
				AttackOrigin = WeaponCollisionBox->GetComponentLocation();
			}
		}
	}

	FVector DefenderForward = InDefender->GetActorForwardVector();
	DefenderForward.Z = 0.f;
	DefenderForward = DefenderForward.GetSafeNormal();

	if (DefenderForward.IsNearlyZero())
	{
		return false;
	}

	FVector DefenderToAttackOrigin = AttackOrigin - InDefender->GetActorLocation();
	DefenderToAttackOrigin.Z = 0.f;

	float DotResult = -1.f;

	const float PositionDot = FVector::DotProduct(DefenderForward, DefenderToAttackOrigin.GetSafeNormal2D());

	// 近距离时武器盒子和角色根节点会快速穿插，朝向信息比纯位置更稳定
	if (DefenderToAttackOrigin.SizeSquared2D() <= FMath::Square(CloseRangeThreshold))
	{
		FVector AttackerForward = InAttacker->GetActorForwardVector();
		AttackerForward.Z = 0.f;
		AttackerForward = AttackerForward.GetSafeNormal();

		if (AttackerForward.IsNearlyZero())
		{
			return false;
		}

		const float FacingDot = FVector::DotProduct(DefenderForward, -AttackerForward);
		DotResult = FMath::Max(PositionDot, FacingDot);
	}
	else
	{
		DotResult = PositionDot;
	}

	Debug::print(FString::Printf(TEXT("BlockDot: %.2f"), DotResult), FColor::Red);

	return DotResult >= 0.6;
}
