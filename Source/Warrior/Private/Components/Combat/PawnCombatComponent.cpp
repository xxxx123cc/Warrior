// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Combat/PawnCombatComponent.h"
#include "Items/Weapons/WarriorWeaponBase.h"
#include "Components/BoxComponent.h"
#include "WarriorDebugHelper.h"
#include "WarriorFunctionLibrary.h"
#include "WarriorGameplayTags.h"
void UPawnCombatComponent::RegisterSpawnedWeapon(FGameplayTag InWeaponTagToRegister,
	AWarriorWeaponBase* InWeaponToRegister, bool bRegisterEquippedWeapon)
{//检查武器标签不存在于Map里，检查要生成的武器是否有效-4.1
	if (!InWeaponToRegister)
	{
		UE_LOG(LogTemp, Warning, TEXT("RegisterSpawnedWeapon failed: %s has no weapon actor."), *InWeaponTagToRegister.ToString());
		return;
	}

	if (AWarriorWeaponBase** ExistingWeaponPtr = CarriedWeaponsMap.Find(InWeaponTagToRegister))
	{
		if (IsValid(*ExistingWeaponPtr))
		{
			if (*ExistingWeaponPtr != InWeaponToRegister)
			{
				UE_LOG(LogTemp, Warning, TEXT("RegisterSpawnedWeapon skipped duplicate weapon: %s."), *InWeaponTagToRegister.ToString());
				InWeaponToRegister->Destroy();
			}

			if (bRegisterEquippedWeapon)
			{
				CurrentEquippedWeaponTag = InWeaponTagToRegister;
			}
			return;
		}

		CarriedWeaponsMap.Remove(InWeaponTagToRegister);
	}
	
	//将武器标签添加进Map里面
	CarriedWeaponsMap.Emplace(InWeaponTagToRegister,InWeaponToRegister);
	//在武器初始化时，进行武器重叠时绑定回调函数
	InWeaponToRegister->OnWeaponHitTarget.BindUObject(this,&ThisClass::OnHitTargetActor);
	InWeaponToRegister->OnWeaponEndOverlapTarget.BindUObject(this,&ThisClass::OnWeaponEndOverlapTarget);
	//如果是已装备武器，设置当前武器标签为这个标签-4.1
	if (bRegisterEquippedWeapon)
	{
		CurrentEquippedWeaponTag= InWeaponTagToRegister;
		
	}
	
}

AWarriorWeaponBase* UPawnCombatComponent::GetCarriedWeaponByTag(FGameplayTag InWeaponTagToGet) const
{//检查当前武器是否被携带-4.1
	if (CarriedWeaponsMap.Contains(InWeaponTagToGet))
	{
		return CarriedWeaponsMap[InWeaponTagToGet];
	}
	
		return nullptr;
}

AWarriorWeaponBase* UPawnCombatComponent::GetCurrentEquippedWeapon() const
{//是否有已装备武器-4.1
	if (!CurrentEquippedWeaponTag.IsValid())
	{
		return nullptr;
	}
	return GetCarriedWeaponByTag(CurrentEquippedWeaponTag);
	
}

void UPawnCombatComponent::ToggleWeaponCollision(bool bEnableCollision, EToggleDamageType ToggleDamageType)
{
	if (ToggleDamageType==EToggleDamageType::CurrentEquippedWeapon)
	{
		ToggleCurrentWeaponCollision(bEnableCollision,ToggleDamageType);
	}
	else if (ToggleDamageType==EToggleDamageType::RightHand||ToggleDamageType==EToggleDamageType::LeftHand)
	{
		ToggleCurrentHandCollision(bEnableCollision,ToggleDamageType);
	}
	return;
	
}

void UPawnCombatComponent::OnHitTargetActor(AActor* HitActor)
{
	

}

void UPawnCombatComponent::OnWeaponEndOverlapTarget(AActor* EndOverlapActor)
{
	if (EndOverlapActor)
	{
		OverlapActors.RemoveSingleSwap(EndOverlapActor);
	}
	
}

bool UPawnCombatComponent::ShouldIgnoreHitDueToWeaponClash(AActor* HitActor) const
{
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (!OwningPawn || !HitActor)
	{
		return true;
	}

	return UWarriorFunctionLibrary::NativeDoesActorHaveTag(OwningPawn, WarriorGameplayTags::Shared_Status_Clashing) ||
		UWarriorFunctionLibrary::NativeDoesActorHaveTag(HitActor, WarriorGameplayTags::Shared_Status_Clashing);
}

void UPawnCombatComponent::ToggleCurrentWeaponCollision(bool bEnableCollision, EToggleDamageType ToggleDamageType)
{
	if (ToggleDamageType==EToggleDamageType::CurrentEquippedWeapon)
	{
		if(AWarriorWeaponBase* CurrentWeapon=GetCurrentEquippedWeapon())
		{
			UBoxComponent* WeaponCollisionBox = CurrentWeapon->GetWeaponCollisionMesh();
			if (!WeaponCollisionBox)
			{
				return;
			}

			if (bEnableCollision)
			{
				OverlapActors.Empty();
				WeaponCollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
				WeaponCollisionBox->UpdateOverlaps();
			}
			else
			{
				WeaponCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				OverlapActors.Empty();
			
		
			}
		}	
		
	}
	
	
}

void UPawnCombatComponent::CleanupAllWeapons()
{
	for (auto& Pair : CarriedWeaponsMap)
	{
		if (Pair.Value)
		{
			Pair.Value->Destroy();
		}
	}
	CarriedWeaponsMap.Empty();
}

void UPawnCombatComponent::ToggleCurrentHandCollision(bool bEnableCollision, EToggleDamageType ToggleDamageType)
{
	
}
