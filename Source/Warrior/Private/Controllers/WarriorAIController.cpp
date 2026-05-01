// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/WarriorAIController.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/CrowdFollowingComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISenseConfig_Sight.h"
#include "TimerManager.h"

namespace
{
	const FName TargetActorKeyName(TEXT("TargetActor"));
	constexpr float TargetActorMemoryDuration = 5.f;
}

AWarriorAIController::AWarriorAIController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>("PathFollowingComponent"))
{
	// 如果默认路径跟随组件已被替换为 CrowdFollowingComponent，这里可以继续做构造期检查或初始化。
	if (UCrowdFollowingComponent* CrowdFollowingComponent = Cast<UCrowdFollowingComponent>(GetPathFollowingComponent()))
	{
	}

	// 创建视觉感知配置，并只让 AI 感知敌对阵营目标。
	WarriorAISenseConfig_Sight = CreateDefaultSubobject<UAISenseConfig_Sight>("EnemySenseConfig_Sight");
	WarriorAISenseConfig_Sight->DetectionByAffiliation.bDetectEnemies = true;
	WarriorAISenseConfig_Sight->DetectionByAffiliation.bDetectFriendlies = false;
	WarriorAISenseConfig_Sight->DetectionByAffiliation.bDetectNeutrals = false;
	WarriorAISenseConfig_Sight->SightRadius = 5000.f;
	WarriorAISenseConfig_Sight->LoseSightRadius = 7000.f;
	WarriorAISenseConfig_Sight->PeripheralVisionAngleDegrees = 360.f;

	// 创建 AI 感知组件，注册视觉配置，并在目标集合变化时回调更新黑板。
	WarriorAIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>("EnemyPerceptionComponent");
	WarriorAIPerceptionComponent->ConfigureSense(*WarriorAISenseConfig_Sight);
	WarriorAIPerceptionComponent->SetDominantSense(UAISenseConfig_Sight::StaticClass());
	WarriorAIPerceptionComponent->OnPerceptionUpdated.AddUniqueDynamic(this, &ThisClass::OnPerceptionUpdated);

	// 当前控制器归属到队伍 1，用于和玩家或其他阵营做敌友识别。
	SetGenericTeamId(FGenericTeamId(1));
}

ETeamAttitude::Type AWarriorAIController::GetTeamAttitudeTowards(const AActor& Other) const
{
	const APawn* PawnToCheck = Cast<const APawn>(&Other);
	const IGenericTeamAgentInterface* OtherTeamAgent = Cast<IGenericTeamAgentInterface>(PawnToCheck->GetController());
	if (OtherTeamAgent && OtherTeamAgent->GetGenericTeamId() < GetGenericTeamId())
	{
		return ETeamAttitude::Hostile;
	}
	return ETeamAttitude::Friendly;
}

void AWarriorAIController::BeginPlay()
{
	Super::BeginPlay();

	// 在运行时配置 CrowdFollowingComponent，避免直接在构造阶段依赖运行态参数。
	if (UCrowdFollowingComponent* CrowdFollowingComponent = Cast<UCrowdFollowingComponent>(GetPathFollowingComponent()))
	{
		CrowdFollowingComponent->SetCrowdSimulationState(bEnableDetourAvoidance ? ECrowdSimulationState::Enabled : ECrowdSimulationState::Disabled);
		switch (DetourCrowdAvoidanceQuality)
		{
		case 1:
			CrowdFollowingComponent->SetCrowdAvoidanceQuality(ECrowdAvoidanceQuality::Low);
			break;
		case 2:
			CrowdFollowingComponent->SetCrowdAvoidanceQuality(ECrowdAvoidanceQuality::Medium);
			break;
		case 3:
			CrowdFollowingComponent->SetCrowdAvoidanceQuality(ECrowdAvoidanceQuality::Good);
			break;
		case 4:
			CrowdFollowingComponent->SetCrowdAvoidanceQuality(ECrowdAvoidanceQuality::High);
			break;
		default:
			break;
		}

		// 让该 AI 既参与避让分组，也主动避开同组成员。
		CrowdFollowingComponent->SetAvoidanceGroup(1);
		CrowdFollowingComponent->SetGroupsToAvoid(1);
		CrowdFollowingComponent->SetCrowdCollisionQueryRange(CollisionQueryRange);
	}
}

void AWarriorAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	UBlackboardComponent* BlackboardComponent = GetBlackboardComponent();
	if (!BlackboardComponent || !WarriorAIPerceptionComponent)
	{
		return;
	}

	for (AActor* Actor : UpdatedActors)
	{
		if (!Actor)
		{
			continue;
		}

		FActorPerceptionBlueprintInfo PerceptionInfo;
		WarriorAIPerceptionComponent->GetActorsPerception(Actor, PerceptionInfo);

		const bool bSuccessfullySensed = PerceptionInfo.LastSensedStimuli.ContainsByPredicate(
			[](const FAIStimulus& Stimulus)
			{
				return Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>() && Stimulus.WasSuccessfullySensed();
			}
		);

		if (bSuccessfullySensed)
		{
			// Refresh the current target and cancel any pending sight-memory clear.
			GetWorldTimerManager().ClearTimer(ClearTargetActorTimerHandle);
			RememberedTargetActor.Reset();
			BlackboardComponent->SetValueAsObject(TargetActorKeyName, Actor);
		}
		else if (BlackboardComponent->GetValueAsObject(TargetActorKeyName) == Actor)
		{
			// Keep the last seen target briefly before clearing it from the blackboard.
			StartTargetActorMemory(Actor);
		}
	}
}

void AWarriorAIController::StartTargetActorMemory(AActor* ActorToRemember)
{
	if (!ActorToRemember)
	{
		return;
	}

	RememberedTargetActor = ActorToRemember;

	if (TargetActorMemoryDuration <= 0.f)
	{
		ClearRememberedTargetActor();
		return;
	}

	GetWorldTimerManager().SetTimer(
		ClearTargetActorTimerHandle,
		this,
		&ThisClass::ClearRememberedTargetActor,
		TargetActorMemoryDuration,
		false
	);
}

void AWarriorAIController::ClearRememberedTargetActor()
{
	UBlackboardComponent* BlackboardComponent = GetBlackboardComponent();
	AActor* ActorToForget = RememberedTargetActor.Get();

	if (BlackboardComponent && (!ActorToForget || BlackboardComponent->GetValueAsObject(TargetActorKeyName) == ActorToForget))
	{
		BlackboardComponent->ClearValue(TargetActorKeyName);
	}

	RememberedTargetActor.Reset();
}
