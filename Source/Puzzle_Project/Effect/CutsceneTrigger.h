// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CutsceneTrigger.generated.h"

class UBoxComponent;
class UTriggerEffect;

/**
 * 演出触发框：玩家进入区域时，依次执行 Effects 里的所有演出效果。
 * 触发框只负责"什么时候触发"，"触发后做什么"交给各个 UTriggerEffect。
 */
UCLASS()
class PUZZLE_PROJECT_API ACutsceneTrigger : public AActor
{
	GENERATED_BODY()

public:
	ACutsceneTrigger();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Trigger")
	TObjectPtr<UBoxComponent> TriggerBox;

	//只触发一次（演出一般都只需要一次）
	UPROPERTY(EditAnywhere, Category = "Trigger")
	bool bTriggerOnce = true;

	//要执行的演出效果；点 "+" 后从下拉菜单选择效果类型
	UPROPERTY(EditAnywhere, Instanced, Category = "Trigger")
	TArray<TObjectPtr<UTriggerEffect>> Effects;

	//是否已经触发过（运行时可在 Details 中查看，方便调试）
	UPROPERTY(VisibleInstanceOnly, Category = "Trigger")
	bool bHasTriggered = false;

private:
	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void RunEffects(APawn* Player);
};