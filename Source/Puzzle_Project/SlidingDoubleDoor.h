// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SlidingDoubleDoor.generated.h"

class AMySwitchActor;
class UStaticMeshComponent;

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

	//控制这扇门的开关，在关卡中为每个门实例单独指定
	UPROPERTY(EditInstanceOnly, Category = "Door")
	TObjectPtr<AMySwitchActor> LinkedSwitch;

	//每扇门向外滑动的距离（厘米）
	UPROPERTY(EditAnywhere, Category = "Door", meta = (ClampMin = "0"))
	float SlideDistance = 100.f;

	//从完全关闭到完全打开所需的秒数
	UPROPERTY(EditAnywhere, Category = "Door", meta = (ClampMin = "0.01"))
	float OpenDuration = 1.0f;

private:
	void HandleSwitchToggled(AMySwitchActor* Switch, bool bIsOn);

	//根据 Alpha 摆放两扇门板
	void ApplyAlpha(float InAlpha);

	float Alpha = 0.f; // 当前开启程度
	float TargetAlpha = 0.f; // 目标开启程度

	FVector LeftClosedLocation;
	FVector RightClosedLocation;

	FDelegateHandle SwitchHandle;


protected:

	//显示在提示中的门名称，例如 门A
	UPROPERTY(EditInstanceOnly, Category = "Door")
	FText DoorDisplayName;


private:
	void PostStateMessage(bool bOpen);

};
