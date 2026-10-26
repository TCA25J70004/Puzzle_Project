// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SwitchBase.generated.h"

class AMySwitchBase;

// 委托从 SwitchActor.h 移到这里，参数类型改为 ASwitchBase*
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSwitchToggled, ASwitchBase* /*Switch*/, bool /*bIsOn*/);

UCLASS()
class PUZZLE_PROJECT_API ASwitchBase : public AActor
{
	GENERATED_BODY()
	
public:	
	bool IsOn() const { return bIsOn; }

	FOnSwitchToggled OnSwitchToggled;

protected:
	UPROPERTY(EditAnywhere, Category = "Switch")
	bool bIsOn = false;

	//修改状态的唯一入口：只有状态真正改变时才会广播
	void SetIsOn(bool bNewIsOn);

	//状态改变后调用，供子类更新外观
	virtual void OnStateChanged() {}

};
