// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SlidingDoubleDoor.generated.h"

//class AMySwitchActor;
class ASwitchBase;
class UStaticMeshComponent;

class ACameraActor; //摄像机演出功能新增

UCLASS()
class PUZZLE_PROJECT_API ASlidingDoubleDoor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASlidingDoubleDoor();

	virtual void Tick(float DeltaTime)override;

	//让门开始打开或关闭
	void SetOpen(bool bOpen);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason)override;

	UPROPERTY(VisibleAnywhere, Category = "Door")
	TObjectPtr<USceneComponent> DoorRoot;

	UPROPERTY(VisibleAnywhere, Category = "Door")
	TObjectPtr<UStaticMeshComponent> LeftDoor;

	UPROPERTY(VisibleAnywhere, Category = "Door")
	TObjectPtr<UStaticMeshComponent> RightDoor;

	/*
	//控制这扇门的开关，在关卡中为每个门实例单独指定
	UPROPERTY(EditInstanceOnly, Category = "Door")
	TObjectPtr<AMySwitchActor> LinkedSwitch;
	*/

	//控制这扇门的开关；全部处于 ON 状态时门才会打开
	UPROPERTY(EditInstanceOnly, Category = "Door")
	//TArray<TObjectPtr<AMySwitchActor>> LinkedSwitches;
	TArray<TObjectPtr<ASwitchBase>> LinkedSwitches;

	//每扇门向外滑动的距离（厘米）
	UPROPERTY(EditAnywhere, Category = "Door", meta = (ClampMin = "0"))
	float SlideDistance = 100.f;

	//从完全关闭到完全打开所需的秒数
	UPROPERTY(EditAnywhere, Category = "Door", meta = (ClampMin = "0.01"))
	float OpenDuration = 1.0f;

private:
	//void HandleSwitchToggled(AMySwitchActor* Switch, bool bIsOn);
	void HandleSwitchToggled(ASwitchBase* Switch, bool bIsOn);

	//根据 Alpha 摆放两扇门板
	void ApplyAlpha(float InAlpha);

	float Alpha = 0.f; // 当前开启程度
	float TargetAlpha = 0.f; // 目标开启程度

	FVector LeftClosedLocation;
	FVector RightClosedLocation;

	//FDelegateHandle SwitchHandle;

	//是否所有关联的开关都处于 ON 状态
	bool AreAllSwitchesOn() const;



protected:

	//显示在提示中的门名称，例如 门A
	UPROPERTY(EditInstanceOnly, Category = "Door")
	FText DoorDisplayName;


private:
	void PostStateMessage(bool bOpen);

protected:
	UPROPERTY(EditInstanceOnly, Category = "Door") //新增，旗标系统所需
	FName FlagId;


// 新增：首次开门演出
protected:
	//开门演出用的摄像机；在关卡中为每个门实例单独指定，不指定则不播放演出
	UPROPERTY(EditInstanceOnly, Category = "Door|Cutscene")
	TObjectPtr<ACameraActor> CutsceneCamera;

	//是否启用首次开门演出
	UPROPERTY(EditAnywhere, Category = "Door|Cutscene")
	bool bPlayCutsceneOnFirstOpen = true;

	//门完全打开后，镜头再停留多少秒才切回玩家（0 = 立刻切回）
	UPROPERTY(EditAnywhere, Category = "Door|Cutscene", meta = (ClampMin = "0.0"))
	float CutsceneHoldTime = 0.f;

	//这扇门的演出是否已经播放过（运行时可在 Details 中查看，方便调试）
	UPROPERTY(VisibleInstanceOnly, Category = "Door|Cutscene")
	bool bHasPlayedCutscene = false;

private:
	//镜头切到门那边
	void StartOpenCutscene();
	//镜头切回玩家
	void EndOpenCutscene();
	//门完全打开的那一刻调用
	void OnDoorFullyOpened();

	bool bIsPlayingCutscene = false;
	FTimerHandle CutsceneTimerHandle;

};
