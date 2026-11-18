// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FlickerLightComponent.generated.h"

class ULightComponent;
class USoundBase;

/*
让所在 Actor 身上的灯"时不时闪烁"的组件。
可以加到关卡里任何一盏灯上（选中灯 → Details → Add → Flicker Light）。
循环：稳定亮一段随机时间 → 连续快速闪几下 → 回到稳定 → ……
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PUZZLE_PROJECT_API UFlickerLightComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFlickerLightComponent();

	//开始 / 停止闪烁（以后可以给演出触发框调用）
	void SetFlickerActive(bool bActive);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	//游戏开始时是否就在闪烁
	UPROPERTY(EditAnywhere, Category = "Flicker")
	bool bStartActive = true;

	//两次闪烁之间正常亮着的时间（秒），在此范围内随机
	UPROPERTY(EditAnywhere, Category = "Flicker|Timing", meta = (ClampMin = "0.0"))
	float MinStableTime = 2.f;

	UPROPERTY(EditAnywhere, Category = "Flicker|Timing", meta = (ClampMin = "0.0"))
	float MaxStableTime = 6.f;

	//每次闪烁时连续闪几下，在此范围内随机
	UPROPERTY(EditAnywhere, Category = "Flicker|Timing", meta = (ClampMin = "1"))
	int32 MinFlickerCount = 2;

	UPROPERTY(EditAnywhere, Category = "Flicker|Timing", meta = (ClampMin = "1"))
	int32 MaxFlickerCount = 5;

	//闪烁中每一次"暗"或"亮"持续的时间（秒），在此范围内随机
	UPROPERTY(EditAnywhere, Category = "Flicker|Timing", meta = (ClampMin = "0.01"))
	float MinStepTime = 0.03f;

	UPROPERTY(EditAnywhere, Category = "Flicker|Timing", meta = (ClampMin = "0.01"))
	float MaxStepTime = 0.12f;

	//暗下去时的亮度比例：0 = 完全熄灭，0.2 左右 = 忽明忽暗
	UPROPERTY(EditAnywhere, Category = "Flicker", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DimRatio = 0.f;

	//每次变暗时播放的声音（滋滋声、咔哒声等），可留空
	UPROPERTY(EditAnywhere, Category = "Flicker")
	TObjectPtr<USoundBase> FlickerSound;

private:
	void ScheduleNextBurst();   // 进入稳定期，安排下一次闪烁
	void StartBurst();          // 开始一轮闪烁
	void FlickerStep();         // 闪烁中的每一步：暗↔亮切换一次
	void SetLightDim(bool bDim);

	UPROPERTY()
	TArray<TObjectPtr<ULightComponent>> Lights;

	TArray<float> BaseIntensities;  // 每盏灯原本的亮度

	int32 RemainingSteps = 0;
	bool bIsDim = false;
	bool bIsActive = false;

	FTimerHandle FlickerTimer;
};