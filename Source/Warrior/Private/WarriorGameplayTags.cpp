// Fill out your copyright notice in the Description page of Project Settings.


#include "WarriorGameplayTags.h"

namespace WarriorGameplayTags
{

	/**玩家标签**/
	//输入标签
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Move,"InputTag.Move");//移动标签
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Look,"InputTag.Look");//视角标签
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Roll,"InputTag.Roll");//视角标签
	UE_DEFINE_GAMEPLAY_TAG(InputTag_EquipAxe,"InputTag.EquipAxe");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_UnEquipAxe,"InputTag.UnEquipAxe");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_LightAttack_Axe,"InputTag.LightAttack.Axe");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_HeavyAttack_Axe,"InputTag.HeavyAttack.Axe");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_MustBeHeld,"InputTag.MustBeHeld");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_MustBeHeld_Block,"InputTag.MustBeHeld.Block");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_MustBeHeld_Jump,"InputTag.MustBeHeld.Jump");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Toggleable_Rage,"InputTag.Toggleable.Rage");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_SpecialWeaponAbility_Light,"InputTag.SpecialWeaponAbility.Light");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_SpecialWeaponAbility_Heavy,"InputTag.SpecialWeaponAbility.Heavy");
	//索敌
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Toggleable,"InputTag.Toggleable");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Toggleable_TargetLock,"InputTag.Toggleable.TargetLock");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_SwitchTarget,"InputTag.SwitchTarget");

	UE_DEFINE_GAMEPLAY_TAG(InputTag_PickUp_Stones,"InputTag.PickUp.Stones");

	UE_DEFINE_GAMEPLAY_TAG(InputTag_SwitchCharacter_1,"InputTag.SwitchCharacter.1");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_SwitchCharacter_2,"InputTag.SwitchCharacter.2");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_SwitchCharacter_3,"InputTag.SwitchCharacter.3");

	//玩法相关标签//4.1
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Equip_Axe,"Player.Ability.Equip.Axe");
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_UnequipAxe,"Player.Ability.Unequip.Axe");
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Attack_Light_Axe,"Player.Ability.Attack.Light.Axe");
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Attack_Heavy_Axe,"Player.Ability.Attack.Heavy.Axe");
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Attack_HitPause,"Player.Ability.Attack.HitPause");
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Jump,"Player.Ability.Jump");
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Roll,"Player.Ability.Roll");
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Block,"Player.Ability.Block");
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_TargetLock,"Player.Ability.TargetLock");
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Rage,"Player.Ability.Rage");
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_SpecialWeaponAbility_Light,"Player.Ability.SpecialWeaponAbility.Light");
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_SpecialWeaponAbility_Heavy,"Player.Ability.SpecialWeaponAbility.Heavy");
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_PickUp_Stones,"Player.Ability.PickUp.Stones");
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_SwitchCharacter,"Player.Ability.SwitchCharacter");


	UE_DEFINE_GAMEPLAY_TAG(Player_Weapon_Axe,"Player.Weapon.Axe");
	UE_DEFINE_GAMEPLAY_TAG(Player_Cooldown_SpecialWeaponAbility_Light,"Player.CoolDown.SpecialWeaponAbility_Light");
	UE_DEFINE_GAMEPLAY_TAG(Player_Cooldown_SpecialWeaponAbility_Heavy,"Player.CoolDown.SpecialWeaponAbility_Heavy");
	UE_DEFINE_GAMEPLAY_TAG(Player_CoolDown_SwitchCharacter,"Player.CoolDown.SwitchCharacter");

	//事件类
	UE_DEFINE_GAMEPLAY_TAG(Player_Event_Equip_Axe,"Player.Event.Equip.Axe");
	UE_DEFINE_GAMEPLAY_TAG(Player_Event_Unequip_Axe,"Player.Event.Unequip.Axe");
	UE_DEFINE_GAMEPLAY_TAG(Player_Event_HitPause,"Player.Event.HitPause");
	UE_DEFINE_GAMEPLAY_TAG(Player_Event_SuccessBlock,"Player.Event.SuccessBlock");
	UE_DEFINE_GAMEPLAY_TAG(Player_Event_SwitchTarget_Right,"Player.Event.SwitchTarget.Right");
	UE_DEFINE_GAMEPLAY_TAG(Player_Event_SwitchTarget_Left,"Player.Event.SwitchTarget.Left");
	UE_DEFINE_GAMEPLAY_TAG(Player_Event_ActivateRage,"Player.Event.ActivateRage");
	UE_DEFINE_GAMEPLAY_TAG(Player_Event_AOE,"Player.Event.AOE");
	UE_DEFINE_GAMEPLAY_TAG(Player_Event_ConsumeStones,"Player.Event.ConsumeStones");
	UE_DEFINE_GAMEPLAY_TAG(Player_Event_SwitchIn,"Player.Event.SwitchIn");
	UE_DEFINE_GAMEPLAY_TAG(Player_Event_SwitchOut,"Player.Event.SwitchOut");
	//状态类
	UE_DEFINE_GAMEPLAY_TAG(Player_Status_JumpToFinisher,"Player.Status.JumpToFinisher");
	UE_DEFINE_GAMEPLAY_TAG(Player_Status_Rolling,"Player.Status.Rolling");
	UE_DEFINE_GAMEPLAY_TAG(Player_Status_Blocking,"Player.Status.Blocking");
	UE_DEFINE_GAMEPLAY_TAG(Player_Status_TargetLock,"Player.Status.TargetLock");
	UE_DEFINE_GAMEPLAY_TAG(Player_Status_Rage_Activating,"Player.Status.Rage.Activating");
	UE_DEFINE_GAMEPLAY_TAG(Player_Status_Rage_Active,"Player.Status.Rage.Active");
	UE_DEFINE_GAMEPLAY_TAG(Player_Status_Rage_Full,"Player.Status.Rage.Full");
	UE_DEFINE_GAMEPLAY_TAG(Player_Status_Rage_None,"Player.Status.Rage.None");

	//自主设置
	UE_DEFINE_GAMEPLAY_TAG(Player_SetByCaller_AttackType_Light,"Player.SetByCaller.AttackType.Light");
	UE_DEFINE_GAMEPLAY_TAG(Player_SetByCaller_AttackType_Heavy,"Player.SetByCaller.AttackType.Heavy");



	/**敌人标签**/
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Weapon,"Enemy.Weapon");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Event_SpawnEnemies,"Enemy.Event.SpawnEnemies");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Ability_SpawnEnemies,"Enemy.Ability.SpawnEnemies");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Ability_Ranged,"Enemy.Ability.Ranged");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Ability_Melee,"Enemy.Ability.Melee");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Ability_SpawnStones,"Enemy.Ability.SpawnStones");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Status_Strafing,"Enemy.Status.Strafing");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Status_UnderAttack,"Enemy.Status.UnderAttack");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Status_UnBlockable,"Enemy.Status.UnBlockable");
	//共享标签
	//造成攻击
	UE_DEFINE_GAMEPLAY_TAG(Shared_Event_MeleeHit,"Shared.Event.MeleeHit");
	UE_DEFINE_GAMEPLAY_TAG(Shared_Event_WeaponClash,"Shared.Event.WeaponClash");
	UE_DEFINE_GAMEPLAY_TAG(Shared_Event_SpawnProjectile,"Shared.Event.SpawnProjectile");
	//受击
	UE_DEFINE_GAMEPLAY_TAG(Shared_Ability_HitReact,"Shared.Ability.HitReact");
	UE_DEFINE_GAMEPLAY_TAG(Shared_Ability_WeaponClash,"Shared.Ability.WeaponClash");
	UE_DEFINE_GAMEPLAY_TAG(Shared_Event_HitReact,"Shared.Event.HitReact");


	//死亡
	UE_DEFINE_GAMEPLAY_TAG(Shared_Ability_Death,"Shared.Ability.Death");
	UE_DEFINE_GAMEPLAY_TAG(Shared_Status_Death,"Shared.Status.Death");
	//状态
	UE_DEFINE_GAMEPLAY_TAG(Shared_Status_Clashing,"Shared.Status.Clashing");
	UE_DEFINE_GAMEPLAY_TAG(Shared_Status_HitReact_Front,"Shared.Status.HitReact.Front");
	UE_DEFINE_GAMEPLAY_TAG(Shared_Status_HitReact_Back,"Shared.Status.HitReact.Back");
	UE_DEFINE_GAMEPLAY_TAG(Shared_Status_HitReact_Left,"Shared.Status.HitReact.Left");
	UE_DEFINE_GAMEPLAY_TAG(Shared_Status_HitReact_Right,"Shared.Status.HitReact.Right");
	UE_DEFINE_GAMEPLAY_TAG(Shared_Status_Invincible,"Shared.Status.Invincible");

	UE_DEFINE_GAMEPLAY_TAG(Shared_SetByCaller_BaseDamage,"Shared.SetByCaller.BaseDamage");


	/**游戏数据标签**/
	UE_DEFINE_GAMEPLAY_TAG(GameData_Level_SurvialGameModeMap,"GameData.Level.SurvivalGameModeMap");
	UE_DEFINE_GAMEPLAY_TAG(GameData_Level_MainMenuMap,"GameData.Level.MainMenuMap");
	UE_DEFINE_GAMEPLAY_TAG(GameData_SaveGame_Slot_1,"GameData.SaveGame.Slot.1");
}
