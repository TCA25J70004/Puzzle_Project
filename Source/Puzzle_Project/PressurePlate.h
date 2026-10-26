// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SwitchBase.h"
#include "PressurePlate.generated.h"

class UStaticMeshComponent;
class UBoxComponent;

UCLASS()
class PUZZLE_PROJECT_API APressurePlate : public ASwitchBase
{
	GENERATED_BODY()


public:
	APressurePlate();

	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	virtual void BeginPlay() override;
	virtual void OnStateChanged() override;

	UPROPERTY(VisibleAnywhere, Category = "Pressure Plate")
	TObjectPtr<USceneComponent> PlateRoot;

	UPROPERTY(VisibleAnywhere, Category = "Pressure Plate")
	TObjectPtr<UStaticMeshComponent> PlateMesh;

	UPROPERTY(VisibleAnywhere, Category = "Pressure Plate")
	TObjectPtr<UBoxComponent> TriggerBox;

	//触发区域的高度（厘米）//
	UPROPERTY(EditAnywhere, Category = "Pressure Plate")
	float TriggerHeight = 20.f;

	//触发区域相对于板面的水平比例，略小于 1 可以避免旁边的物体蹭到边缘时误触发 //
	UPROPERTY(EditAnywhere, Category = "Pressure Plate", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float TriggerInset = 0.9f;

	//物理物体至少要多重才能压下压力板（kg）//
	UPROPERTY(EditAnywhere, Category = "Pressure Plate")
	float MinActivationMass = 5.f;

	//被压下时板面下沉的距离（厘米）//
	UPROPERTY(EditAnywhere, Category = "Pressure Plate")
	float PressDepth = 3.f;

	//东西离开后，延迟多久才弹起（秒），防止物体弹跳造成的抖动 //
	UPROPERTY(EditAnywhere, Category = "Pressure Plate")
	float ReleaseDelay = 0.15f;

private:
	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void UpdatePressedState();
	void OnReleaseTimerExpired();
	bool HasQualifyingOverlap() const;

	FTimerHandle ReleaseTimerHandle;
	
};
