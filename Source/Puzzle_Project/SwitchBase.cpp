// Fill out your copyright notice in the Description page of Project Settings.


#include "SwitchBase.h"
#include "FlagSubsystem.h"
#include "Engine/GameInstance.h"

/*
// Sets default values
ASwitchBase::ASwitchBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASwitchBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASwitchBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
*/

UFlagSubsystem* ASwitchBase::GetFlagSubsystem() const
{
	const UGameInstance* GameInstance = GetGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<UFlagSubsystem>() : nullptr;
}


void ASwitchBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	UFlagSubsystem* FLagSubsystem = GetFlagSubsystem();
	if (FlagId.IsNone()||!FLagSubsystem)
	{
		return;
	}

	if (ShouldRestoreFromFlag()&&FLagSubsystem->HasFlag(FlagId))
	{
		// 之前记录过 → 恢复状态（直接赋值，不广播，避免弹出"门A已开启"的提示）
		bIsOn = FLagSubsystem->GetFlag(FlagId);
		OnStateChanged();
	}
	else
	{
		// 第一次出现 → 把初始状态登记进去
		FLagSubsystem->SetFlag(FlagId, bIsOn);
	}
}

void ASwitchBase::SetIsOn(bool bNewIsOn)
{
	if (bIsOn == bNewIsOn)
	{
		return;
	}

	bIsOn = bNewIsOn;

	// 先写入旗标，再通知门，保证门在响应时查询到的旗标已经是最新的
	if (UFlagSubsystem* FlagSubsystem=GetFlagSubsystem())
	{
		FlagSubsystem->SetFlag(FlagId, bIsOn); // FlagId 为空时 SetFlag 内部会忽略
	}


	OnStateChanged();
	OnSwitchToggled.Broadcast(this, bIsOn);
}

