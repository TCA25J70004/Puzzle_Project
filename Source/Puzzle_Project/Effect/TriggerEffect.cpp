// Fill out your copyright notice in the Description page of Project Settings.


#include "TriggerEffect.h"
#include "CutsceneTrigger.h"
#include "Components/LightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Camera/CameraActor.h"
#include "CameraCutSubsystem.h"

void ULightSwitchEffect::Execute(ACutsceneTrigger* Trigger, APawn* Player)
{
	for (AActor* LightActor : Lights)
	{
		if (!LightActor)
		{
			continue;
		}

		// 取出这个 Actor 身上所有的灯光组件，一起切换
		TArray<ULightComponent*> LightComponents;
		LightActor->GetComponents<ULightComponent>(LightComponents);

		if (LightComponents.Num() == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s has no light component."), *LightActor->GetName());
		}

		for (ULightComponent* Light : LightComponents)
		{
			// 引擎自带的 ALight::SetEnabled() 内部也是这样做的
			Light->SetVisibility(bTurnOn);
		}
	}
}

void UPlaySoundEffect::Execute(ACutsceneTrigger* Trigger, APawn* Player)
{
	if (!Sound || !Trigger)
	{
		return;
	}

	if (SoundSource)
	{
		UGameplayStatics::PlaySoundAtLocation(Trigger, Sound, SoundSource->GetActorLocation(), VolumeMultiplier);
	}
	else
	{
		UGameplayStatics::PlaySound2D(Trigger, Sound, VolumeMultiplier);
	}
}

void UCameraCutEffect::Execute(ACutsceneTrigger* Trigger, APawn* Player)
{
	if (!Camera || !Trigger)
	{
		return;
	}

	if (UCameraCutSubsystem* CameraCut = Trigger->GetWorld()->GetSubsystem<UCameraCutSubsystem>())
	{
		// 请求者传 this（效果自身），这样同一个触发框里的多个切镜头效果也能互相区分
		CameraCut->StartCameraCut(Camera, this, Duration);
	}
}