// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/LatentActionManager.h"
#include "WarriorTypes/WarriorEnumTypes.h"
/**
 * 
 */
class FWarriorCountDownAction :public FPendingLatentAction
{
	public:
	FWarriorCountDownAction(float InTotalCountDownTime,float InUpdateInterval,float& InOutRemainingTime,EWarriorCountDownOutput& InOutResult,const FLatentActionInfo& LatentActionInfo )
		:bNeedToCancel(false)
	,TotalCountDownTime(FMath::Max(0.f, InTotalCountDownTime))
	,UpdateInterval(FMath::Max(0.f, InUpdateInterval))
	,OutRemainingTime(InOutRemainingTime)
	,OutResult(InOutResult)
	,ExecutionFunction(LatentActionInfo.ExecutionFunction)
	,OutputLink(LatentActionInfo.Linkage)
	,CallbackTarget(LatentActionInfo.CallbackTarget)
	,ElapsedTimeSinceStart(0.f)
	,ElapsedTimeInterval(0.f)
	{
		OutRemainingTime = TotalCountDownTime;
	};
	void CancelAction();
	
	virtual void UpdateOperation(FLatentResponse& Response) override;
	
private:
	bool bNeedToCancel;
	float TotalCountDownTime;
	float UpdateInterval;
	float& OutRemainingTime;
	EWarriorCountDownOutput& OutResult;
	FName ExecutionFunction;
	int32 OutputLink;
	FWeakObjectPtr CallbackTarget;
	//开始后已过去时间
	float ElapsedTimeSinceStart;
	//已经经过的间隔
	float ElapsedTimeInterval;
	
};
