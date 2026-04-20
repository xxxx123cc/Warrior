#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "WarriorAIController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;

/**
 * 敌人 AI 控制器。
 * 负责初始化视觉感知、配置群体避让，并将感知结果同步到黑板。
 */
UCLASS()
class WARRIOR_API AWarriorAIController : public AAIController
{
	GENERATED_BODY()

public:
	// 使用 CrowdFollowingComponent 替换默认 PathFollowingComponent，便于启用群体避让。
	AWarriorAIController(const FObjectInitializer& ObjectInitializer);

	// 供感知系统根据团队 ID 判断目标是敌对还是友好。
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;

protected:
	// 在运行开始后应用 CrowdFollowingComponent 的避让参数。
	virtual void BeginPlay() override;

	// AI 感知组件，统一管理视觉等感知逻辑。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UAIPerceptionComponent* WarriorAIPerceptionComponent;

	// 视觉感知配置，定义感知范围和阵营过滤规则。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UAISenseConfig_Sight* WarriorAISenseConfig_Sight;

	// 当感知到的目标集合发生变化时触发，用于更新黑板中的 TargetActor。
	UFUNCTION()
	virtual void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

private:
	// 是否启用 Detour Crowd 群体避让。
	UPROPERTY(EditDefaultsOnly, Category = "detour crowd avoidance config")
	bool bEnableDetourAvoidance = true;

	// 群体避让质量，值越高效果越好但开销越大。
	UPROPERTY(EditDefaultsOnly, Category = "detour crowd avoidance config", meta = (EditCondition = "bEnableDetourAvoidance", UIMin = "1", UIMax = "4"))
	int32 DetourCrowdAvoidanceQuality = 4;

	// 群体避让查询范围，用于决定周围单位的检测半径。
	UPROPERTY(EditDefaultsOnly, Category = "detour crowd avoidance config", meta = (EditCondition = "bEnableDetourAvoidance"))
	float CollisionQueryRange = 600.f;
};
