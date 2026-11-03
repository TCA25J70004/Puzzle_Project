// Fill out your copyright notice in the Description page of Project Settings.


#include "FlagSwitch.h"
#include "FlagSubsystem.h"

AFlagSwitch::AFlagSwitch()
{
	PrimaryActorTick.bCanEverTick = false;

	// 纯逻辑物体，没有模型，只需要一个根组件来确定位置
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void AFlagSwitch::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// 和普通开关一样，在所有 BeginPlay 之前同步状态，确保门读到的是正确的值
	if (const UFlagSubsystem* FlagSubsystem = GetFlagSubsystem())
	{
		bIsOn = FlagSubsystem->GetFlag(WatchedFlagId);
	}
}

void AFlagSwitch::BeginPlay()
{
	Super::BeginPlay();

	if (UFlagSubsystem* FlagSubsystem = GetFlagSubsystem())
	{
		FlagSubsystem->OnFlagChanged.AddUObject(this, &AFlagSwitch::HandleFlagChanged);
	}
}

void AFlagSwitch::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UFlagSubsystem* FlagSubsystem = GetFlagSubsystem())
	{
		FlagSubsystem->OnFlagChanged.RemoveAll(this);
	}

	Super::EndPlay(EndPlayReason);
}

void AFlagSwitch::HandleFlagChanged(FName ChangedFlagId, bool bValue)
{
	// 旗标管理器会广播所有旗标的变化，这里只关心自己监视的那一个
	if (ChangedFlagId == WatchedFlagId)
	{
		SetIsOn(bValue);   // 父类负责：状态没变就不广播；变了就通知门
	}
}
