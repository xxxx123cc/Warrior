// Fill out your copyright notice in the Description page of Project Settings.


#include "WarriorTypes/WarriorCountDownAction.h"

void FWarriorCountDownAction::CancelAction()
{
	bNeedToCancel = true;
}

void FWarriorCountDownAction::UpdateOperation(FLatentResponse& Response)
{
	if (bNeedToCancel)
	{
		OutResult = EWarriorCountDownOutput::Cancelled; 
		Response.FinishAndTriggerIf(true,ExecutionFunction,OutputLink,CallbackTarget);
		return;
	}

	const float DeltaTime = Response.ElapsedTime();
	ElapsedTimeSinceStart += DeltaTime;
	ElapsedTimeInterval += DeltaTime;
	OutRemainingTime = FMath::Max(0.f, TotalCountDownTime - ElapsedTimeSinceStart);

	if (ElapsedTimeSinceStart>=TotalCountDownTime)
	{
		OutResult = EWarriorCountDownOutput::Completed;
		Response.FinishAndTriggerIf(true,ExecutionFunction,OutputLink,CallbackTarget);
		return;
	}

	if (ElapsedTimeInterval >= UpdateInterval)
	{
		OutResult = EWarriorCountDownOutput::Updated;
		Response.TriggerLink(ExecutionFunction,OutputLink,CallbackTarget);
		ElapsedTimeInterval=0.f;
	}
}
