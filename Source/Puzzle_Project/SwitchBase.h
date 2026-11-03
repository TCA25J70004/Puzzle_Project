// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SwitchBase.generated.h"

class AMySwitchBase;
class UFlagSubsystem;

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


	//新增，旗标系统需要
public:
	virtual void PostInitializeComponents() override;

protected:
	//旗标名，例如 "Stage1.SwitchA"；留空则不记录
	UPROPERTY(EditInstanceOnly, Category = "Switch|Flag")
	FName FlagId;

	//加载关卡时是否从旗标恢复状态（压力板的状态取决于上面有没有东西，所以不恢复）
	virtual bool ShouldRestoreFromFlag() const { return true; }

	UFlagSubsystem* GetFlagSubsystem() const;

};
