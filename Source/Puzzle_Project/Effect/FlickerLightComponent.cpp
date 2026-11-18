// Fill out your copyright notice in the Description page of Project Settings.


#include "FlickerLightComponent.h"
#include "Components/LightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

UFlickerLightComponent::UFlickerLightComponent()
{
	// 全部用计时器驱动，不需要 Tick
	PrimaryComponentTick.bCanEverTick = false;
}

void UFlickerLightComponent::BeginPlay()
{
	Super::BeginPlay();

	// 收集所在 Actor 身上的所有灯光组件，并记住它们原本的亮度
	TArray<ULightComponent*> Found;
	GetOwner()->GetComponents<ULightComponent>(Found);

	for (ULightComponent* Light : Found)
	{
		Lights.Add(Light);
		BaseIntensities.Add(Light->Intensity);
	}

	if (Lights.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: FlickerLightComponent found no light component."), *GetOwner()->GetName());
		return;
	}

	if (bStartActive)
	{
		SetFlickerActive(true);
	}
}

void UFlickerLightComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FlickerTimer);
	}

	Super::EndPlay(EndPlayReason);
}

void UFlickerLightComponent::SetFlickerActive(bool bActive)
{
	if (bIsActive == bActive)
	{
		return;
	}
	bIsActive = bActive;

	if (bIsActive)
	{
		ScheduleNextBurst();
	}
	else
	{
		// 停止时恢复正常亮度
		GetWorld()->GetTimerManager().ClearTimer(FlickerTimer);
		SetLightDim(false);
	}
}

void UFlickerLightComponent::ScheduleNextBurst()
{
	// Max 填得比 Min 小时，按 Min 处理，避免出错
	const float Wait = FMath::FRandRange(MinStableTime, FMath::Max(MinStableTime, MaxStableTime));

	GetWorld()->GetTimerManager().SetTimer(
		FlickerTimer, this, &UFlickerLightComponent::StartBurst, FMath::Max(Wait, 0.01f), false);
}

void UFlickerLightComponent::StartBurst()
{
	const int32 Count = FMath::RandRange(MinFlickerCount, FMath::Max(MinFlickerCount, MaxFlickerCount));

	// 每闪一下 = 变暗 + 变亮 两步；步数是偶数，所以最后一定停在"亮"的状态
	RemainingSteps = Count * 2;
	FlickerStep();
}

void UFlickerLightComponent::FlickerStep()
{
	if (RemainingSteps <= 0)
	{
		SetLightDim(false);
		ScheduleNextBurst();
		return;
	}

	SetLightDim(!bIsDim);
	--RemainingSteps;

	const float StepTime = FMath::FRandRange(MinStepTime, FMath::Max(MinStepTime, MaxStepTime));

	GetWorld()->GetTimerManager().SetTimer(
		FlickerTimer, this, &UFlickerLightComponent::FlickerStep, StepTime, false);
}

void UFlickerLightComponent::SetLightDim(bool bDim)
{
	bIsDim = bDim;

	for (int32 i = 0; i < Lights.Num(); ++i)
	{
		if (Lights[i])
		{
			Lights[i]->SetIntensity(BaseIntensities[i] * (bDim ? DimRatio : 1.f));
		}
	}

	if (bDim && FlickerSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FlickerSound, GetOwner()->GetActorLocation());
	}
}