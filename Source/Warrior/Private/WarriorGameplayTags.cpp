// Fill out your copyright notice in the Description page of Project Settings.


#include "WarriorGameplayTags.h"

namespace WarriorGameplayTags
{//创建游戏标签，移动标签
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Move,"InputTag.Move");//移动标签
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Look,"InputTag.Look");//视角标签
	//装备的拿和收-4.2
	UE_DEFINE_GAMEPLAY_TAG(InputTag_EquipAxe,"InputTag.EquipAxe");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_UnEquipAxe,"InputTag.UnEquipAxe");
	//玩法相关标签//4.1
	UE_DEFINE_GAMEPLAY_TAG(Player_Weapon_Axe,"Player.Weapon.Axe");
	
	UE_DEFINE_GAMEPLAY_TAG(Player_Event_Equip_Axe,"Player.Event.Equip.Axe");
		
	UE_DEFINE_GAMEPLAY_TAG(Player_Event_Unequip_Axe,"Player.Event.Unequip.Axe");
	
}