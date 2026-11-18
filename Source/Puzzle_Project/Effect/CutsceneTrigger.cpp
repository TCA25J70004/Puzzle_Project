// Fill out your copyright notice in the Description page of Project Settings.


#include "CutsceneTrigger.h"
#include "TriggerEffect.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"

ACutsceneTrigger::ACutsceneTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	RootComponent = TriggerBox;

	TriggerBox->SetBoxExtent(FVector(100.f, 100.f, 100.f));
	// "Trigger" 预设：只检测重叠，不阻挡任何东西
	TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
	// 编辑器里用醒目的颜色显示；游戏中默认不可见
	TriggerBox->ShapeColor = FColor(255, 120, 0);
	TriggerBox->SetLineThickness(2.f);
}

void ACutsceneTrigger::BeginPlay()
{
	Super::BeginPlay();

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ACutsceneTrigger::HandleBeginOverlap);

	if (Effects.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s has no Effects assigned."), *GetName());
	}
}

void ACutsceneTrigger::HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 只响应玩家控制的角色（避免被推动的物体、NPC 等误触发）
	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn || !Pawn->IsPlayerControlled())
	{
		return;
	}

	if (bTriggerOnce && bHasTriggered)
	{
		return;
	}
	bHasTriggered = true;

	RunEffects(Pawn);
}

void ACutsceneTrigger::RunEffects(APawn* Player)
{
	for (UTriggerEffect* Effect : Effects)
	{
		if (!Effect)
		{
			continue;
		}

		if (Effect->Delay <= 0.f)
		{
			Effect->Execute(this, Player);
			continue;
		}

		// 有延迟的效果交给计时器。
		// 用弱指针捕获：如果等待期间对象被销毁，就不会访问到无效内存
		TWeakObjectPtr<UTriggerEffect> WeakEffect = Effect;
		TWeakObjectPtr<APawn> WeakPlayer = Player;

		FTimerHandle Handle;
		GetWorldTimerManager().SetTimer(
			Handle,
			FTimerDelegate::CreateWeakLambda(this, [this, WeakEffect, WeakPlayer]()
				{
					if (UTriggerEffect* E = WeakEffect.Get())
					{
						E->Execute(this, WeakPlayer.Get());
					}
				}),
			Effect->Delay,
			false);
	}
}