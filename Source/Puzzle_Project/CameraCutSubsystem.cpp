// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraCutSubsystem.h"
#include "Camera/CameraActor.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

bool UCameraCutSubsystem::StartCameraCut(ACameraActor* Camera, UObject* Requester, float Duration)
{
	UWorld* World = GetWorld();
	if (!Camera || !Requester || !World)
	{
		return false;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	if (!PC)
	{
		return false;
	}

	// 第一次进入切镜头状态时禁用输入；
	// 如果是覆盖正在进行的切镜头，输入已经是禁用的，不需要重复禁用
	if (!bIsActive)
	{
		if (APawn* Pawn = PC->GetPawn())
		{
			Pawn->DisableInput(PC);
			DisabledPawn = Pawn;
		}
	}

	bIsActive = true;
	CurrentRequester = Requester;

	// 混合时间 0 = 瞬间切换
	PC->SetViewTargetWithBlend(Camera, 0.f);

	// 清掉上一个切镜头可能留下的计时器
	World->GetTimerManager().ClearTimer(DurationTimer);
	if (Duration > 0.f)
	{
		World->GetTimerManager().SetTimer(
			DurationTimer, this, &UCameraCutSubsystem::HandleDurationElapsed, Duration, false);
	}

	return true;
}

void UCameraCutSubsystem::EndCameraCut(UObject* Requester)
{
	// 只有当前的请求者才能结束，旧请求者的结束调用会被忽略
	if (!bIsActive || CurrentRequester.Get() != Requester)
	{
		return;
	}

	FinishCameraCut();
}

void UCameraCutSubsystem::HandleDurationElapsed()
{
	if (bIsActive)
	{
		FinishCameraCut();
	}
}

void UCameraCutSubsystem::FinishCameraCut()
{
	bIsActive = false;
	CurrentRequester.Reset();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	World->GetTimerManager().ClearTimer(DurationTimer);

	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	if (PC)
	{
		if (APawn* Pawn = PC->GetPawn())
		{
			PC->SetViewTargetWithBlend(Pawn, 0.f);  // 瞬间切回玩家视角
		}
	}

	if (APawn* Pawn = DisabledPawn.Get())
	{
		Pawn->EnableInput(PC);
	}
	DisabledPawn.Reset();
}