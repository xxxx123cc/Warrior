// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GameplayEffect/DamageTaken.h"
#include "AbilitySystem/WarriorAttributeSet.h"
#include "Warrior/Public/WarriorGameplayTags.h"
#include "WarriorTypes/WarriorStructTypes.h"
//快速伤害捕获
struct  FWarriorDamageCapture
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(AttackPower);//攻击力
	DECLARE_ATTRIBUTE_CAPTUREDEF(DefensePower);//防御力
	DECLARE_ATTRIBUTE_CAPTUREDEF(DamageTaken);//承受的伤害
	FWarriorDamageCapture()
	{   //捕获伤害来源的攻击力
		DEFINE_ATTRIBUTE_CAPTUREDEF(UWarriorAttributeSet, AttackPower, Source, false);
		//捕获攻击对象的防御力
		DEFINE_ATTRIBUTE_CAPTUREDEF(UWarriorAttributeSet, DefensePower, Target, false);
		//捕获对敌方造成的伤害
		DEFINE_ATTRIBUTE_CAPTUREDEF(UWarriorAttributeSet, DamageTaken, Target, false);
	}
};
static const FWarriorDamageCapture&GetWarriorDamageCapture()
{   //实例化对象
	static FWarriorDamageCapture DamageCapture;
	return DamageCapture;
}
UDamageTaken::UDamageTaken()
{//找到攻击力（慢的方法）
	// FProperty* AttackPowerProperty = FindFieldChecked<FProperty>(UWarriorAttributeSet::StaticClass(), GET_MEMBER_NAME_CHECKED(UWarriorAttributeSet, AttackPower));
 //     
	// FGameplayEffectAttributeCaptureDefinition AttackPowerCaptureDefinition(
	// 	AttackPowerProperty, 
	// 	EGameplayEffectAttributeCaptureSource::Source, //
	// 	false // 捕获瞬时属性
	// 	);
	// RelevantAttributesToCapture.Add(AttackPowerCaptureDefinition);
	
	
	//把捕获任务单加进去，GAS 在执行计算之前会提前把这些属性值都准备好，你在后面的 Execute_Implementation 函数里就能直接取出来用。
	RelevantAttributesToCapture.Add(GetWarriorDamageCapture().AttackPowerDef);
	RelevantAttributesToCapture.Add(GetWarriorDamageCapture().DefensePowerDef);
	RelevantAttributesToCapture.Add(GetWarriorDamageCapture().DamageTakenDef);
}

void UDamageTaken::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	Super::Execute_Implementation(ExecutionParams, OutExecutionOutput);
	 const FGameplayEffectSpec& EffectSpec= ExecutionParams.GetOwningSpec();
	// EffectSpec.GetContext().GetSourceObject();
	// EffectSpec.GetContext().GetAbility();
	// EffectSpec.GetContext().GetInstigator();
	// EffectSpec.GetContext().GetEffectCauser();
	//创建一个评估参数
	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = EffectSpec.CapturedSourceTags.GetAggregatedTags();
	EvaluationParameters.TargetTags = EffectSpec.CapturedTargetTags.GetAggregatedTags();
	float AttackPower = 0.f;
	float DefensePower = 0.f;
	float DamageTaken = 0.f;
	//从捕获的属性中取出攻击力和防御力的数值
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(GetWarriorDamageCapture().AttackPowerDef, EvaluationParameters, AttackPower);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(GetWarriorDamageCapture().DefensePowerDef, EvaluationParameters, DefensePower);
	float BaseDamage = 0.f;
	int32 UsedLightAttackComboCount = 0;
	int32 UsedHeavyAttackComboCount = 0;
	for (const TPair<FGameplayTag,float>& TagMagnitude :EffectSpec.SetByCallerTagMagnitudes)
	{
		if (TagMagnitude.Key.MatchesTagExact(WarriorGameplayTags::Shared_SetByCaller_BaseDamage))
		{
			 BaseDamage = TagMagnitude.Value;
			
		}
		if (TagMagnitude.Key.MatchesTagExact(WarriorGameplayTags::Player_SetByCaller_AttackType_Light))
		{
			 UsedLightAttackComboCount = TagMagnitude.Value;
			
		}
		if (TagMagnitude.Key.MatchesTagExact(WarriorGameplayTags::Player_SetByCaller_AttackType_Heavy))
		{
			UsedHeavyAttackComboCount = TagMagnitude.Value;
			
		}
	}
	if (UsedLightAttackComboCount>0)
	{
	   const float DamageIncreasePercentLight = (UsedLightAttackComboCount-1)*0.05f+1.f;
		BaseDamage *= DamageIncreasePercentLight;
	}
	if (UsedHeavyAttackComboCount>0)
	{
		const float DamageIncreasePercentHeavy = (UsedHeavyAttackComboCount)*0.2f+1.f;
		BaseDamage *= DamageIncreasePercentHeavy;
	}
	const float FinalDamageDone=BaseDamage*AttackPower/DefensePower;
	if (FinalDamageDone >0.f)
	{//将计算出的最终伤害值作为一个属性修改操作，添加到Gameplay Effect的执行输出中。
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(GetWarriorDamageCapture().DamageTakenProperty, EGameplayModOp::Override, FinalDamageDone));
		
	}
	
}
