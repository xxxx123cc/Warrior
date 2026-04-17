// Fill out your copyright notice in the Description page of Project Settings.

// 文件说明（中文注释）
// - Pawn: 表示可被控制的“身体”（继承自 AActor），可以被 Controller possess。
// - Character: 是常用的 Pawn 子类，带有 CharacterMovementComponent、SkeletalMesh 等，适合实现角色行为。
// - Controller: 控制 Pawn/Character 的“脑”，分为 APlayerController（玩家）和 AAIController（AI）。
// 在 PossessedBy 中使用 this 时，this 指向当前的 AWarriorBaseCharacter 实例（即 ACharacter 子类），
// 因此可以隐式转换为 AActor*/APawn* 用于函数调用。


#include "Characters/WarriorBaseCharacter.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"

#include "AbilitySystem/WarriorAttributeSet.h"
#include "Interfaces/PawnCombatInterface.h"
// Sets default values
AWarriorBaseCharacter::AWarriorBaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	GetMesh()->bReceivesDecals = false;

	WarriorAbilitySystemComponent = CreateDefaultSubobject<UWarriorAbilitySystemComponent>(TEXT("WarriorAbilitySystemComponent"));
	WarriorAttributeSet = CreateDefaultSubobject<UWarriorAttributeSet>(TEXT("WarriorAttributeSet"));

    
}

UAbilitySystemComponent* AWarriorBaseCharacter::GetAbilitySystemComponent() const
{
    // 返回角色持有的 AbilitySystemComponent，供 GAS（Gameplay Ability System） 查询
    return GetWarriorAbilitySystemComponent();
    
}

UPawnCombatComponent* AWarriorBaseCharacter::GetPawnCombatComponent() const
{
	// Base 角色不一定有战斗组件；子类（Hero/Enemy）会返回各自的 CombatComponent
	return nullptr;
}

UPawnUIComponent* AWarriorBaseCharacter::GetPawnUIComponent() const
{
	return nullptr;
}


void AWarriorBaseCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    // 在服务端（Server）中，当 Controller possess 这个 Pawn/Character 时会调用 PossessedBy
    // 这里初始化 AbilitySystemComponent 的 ActorInfo：InitAbilityActorInfo(OwnerActor, AvatarActor)
    // 语义说明：
    // - OwnerActor（第一个参数）代表“谁拥有/代表这套能力系统/身份的归属”，通常用于权限/来源判断。
    // - AvatarActor（第二个参数）代表“能力实际作用的化身/外显对象”，技能执行通常依赖 Avatar 的位置、组件等。
    // 当 AbilitySystemComponent 挂在 Character/Pawn 本身时，常见做法是把 Owner 和 Avatar 都设置为 this：InitAbilityActorInfo(this, this);
    // 如果 ASC 挂在 PlayerState（用于跨生存保留数据），则应该调用 InitAbilityActorInfo(PlayerState, Pawn)

    if (WarriorAbilitySystemComponent)
    {
        // 这里的 this 是 AWarriorBaseCharacter*，传入函数会被当作 AActor* 使用
        WarriorAbilitySystemComponent->InitAbilityActorInfo(this, this);
    	
    	
    	ensureMsgf(!CharacterStartUpData.IsNull(),TEXT("请确保在 %s 的蓝图类中设置了有效的 StartUpData"),*GetName());
    
    }

    // 注意客户端初始化：PossessedBy 在服务端被调用，客户端可能需要在 OnRep_PlayerState / OnRep_Controller 或 BeginPlay 中也做一次 InitAbilityActorInfo
    // 以确保客户端的 AbilitySystemComponent 有正确的 Owner/Avatar 信息，避免客户端读到空的 ActorInfo。
}
