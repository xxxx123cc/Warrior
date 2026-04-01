// 为该文件添加中文注释，说明类用途和各方法的行为
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PawnExtensionComponentBase.generated.h"


/**
 * UPawnExtensionComponentBase
 * - 这是一个供基于 Pawn 的 Actor 使用的扩展组件基类。
 * - 提供便捷的函数来获取拥有该组件的 Pawn 和该 Pawn 的 Controller。
 * - 注：这些函数为简化调用而设计，没有做运行时空指针保护（使用 CastChecked 时会在转换失败时断言）。
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WARRIOR_API UPawnExtensionComponentBase : public UActorComponent
{
	GENERATED_BODY()


protected:
	// 返回拥有该组件的 Pawn，并将其转换为指定的派生类型 T。
	// 模板约束：T 必须可以转换为 APawn（编译期通过 static_assert 检查）。
	// 注意：使用 CastChecked 会在类型不匹配时触发断言/崩溃（在非调试配置下可能表现为运行时错误），因此仅在你确信类型正确时使用此模板重载。
	template<class T>
	T* GetOwningPawn() const
	{
		static_assert(TPointerIsConvertibleFromTo<T, APawn>::Value, "T must be derived from APawn");
		return CastChecked<T>(GetOwner());
		
	}

	// 无模板重载：直接返回拥有该组件的 APawn*。
	// 这是一个方便的包装函数，等价于 GetOwningPawn<APawn>()。
	APawn* GetOwningPawn() const
	{
		return GetOwningPawn<APawn>();
	}
	
	// 返回拥有该组件的 Pawn 的 Controller，并将其转换为指定的 Controller 派生类型 T。
	// 模板约束：T 必须可以转换为 AController（通过 static_assert 编译期检查）。
	// 实现细节：先获取拥有的 Pawn（通过上面的 GetOwningPawn），然后从 Pawn 获取 Controller 并转换为 T。
	// 注意事项：如果 Pawn 为空或 Controller 为空，或类型不匹配，可能导致空指针访问或 CastChecked 断言失败（取决于底层实现）。
	template<class T>
	T* GetOwningController() const
	{
		static_assert(TPointerIsConvertibleFromTo<T, AController>::Value, "T must be derived from AController");
		return GetOwningPawn<APawn>()->GetController<T>();
		
	}
	
};
