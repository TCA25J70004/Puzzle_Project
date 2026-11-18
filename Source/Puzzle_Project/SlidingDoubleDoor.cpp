// Fill out your copyright notice in the Description page of Project Settings.


#include "SlidingDoubleDoor.h"
//#include "MySwitchActor.h"
#include "SwitchBase.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/CollisionProfile.h"
#include "UObject/ConstructorHelpers.h"
#include "GameMessageSubsystem.h"

#include "FlagSubsystem.h"
#include "Engine/GameInstance.h"

#include "Camera/CameraActor.h"                 //新增，开门演出所需
#include "CameraCutSubsystem.h"
#include "TimerManager.h"

// Sets default values
ASlidingDoubleDoor::ASlidingDoubleDoor()
{
	// 需要 Tick 来播放动画，但平时不需要，所以默认关闭，只在门运动时开启
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	DoorRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DoorRoot"));
	RootComponent = DoorRoot;

	LeftDoor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftDoor"));
	LeftDoor->SetupAttachment(DoorRoot);

	RightDoor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightDoor"));
	RightDoor->SetupAttachment(DoorRoot);

	// 暂时用引擎自带的立方体做门板（100×100×100 厘米，原点在中心）
	static ConstructorHelpers::FObjectFinder<UStaticMesh>CubeFinder(
		TEXT("/Engine/BasicShapes/Cube.Cube")
	);

	for (UStaticMeshComponent* Panel : { LeftDoor.Get(), RightDoor.Get() })
	{
		if (CubeFinder.Succeeded())
		{
			Panel->SetStaticMesh(CubeFinder.Object);
		}
		Panel->SetMobility(EComponentMobility::Movable);
		Panel->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
		// 每扇门板：厚 20、宽 100、高 250 厘米
		Panel->SetRelativeScale3D(FVector(0.2f, 1.f, 2.5f));
	}

	// 两扇门板并排放置，合起来 200 厘米宽，底部贴地
	LeftDoor->SetRelativeLocation(FVector(0.f, -50.f, 125.f));
	RightDoor->SetRelativeLocation(FVector(0.f, 50.f, 125.f));

}

// Called when the game starts or when spawned
void ASlidingDoubleDoor::BeginPlay()
{
	Super::BeginPlay();

	LeftClosedLocation = LeftDoor->GetRelativeLocation();
	RightClosedLocation = RightDoor->GetRelativeLocation();

	/*
	if (LinkedSwitch)
	{
		// 订阅开关的广播
		SwitchHandle = LinkedSwitch->OnSwitchToggled.AddUObject(
			this, &ASlidingDoubleDoor::HandleSwitchToggled
		);

		// 立即同步开关的初始状态（不播放动画）
		Alpha = TargetAlpha = LinkedSwitch->IsOn() ? 1.f : 0.f;
		ApplyAlpha(Alpha);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s has no LinkedSwitch assigned."), *GetName());
	}
	*/

	if (LinkedSwitches.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s has no LinkedSwitches assigned."), *GetName());
	}

	for (ASwitchBase* Switch : LinkedSwitches)
	{
		if (Switch)
		{
			Switch->OnSwitchToggled.AddUObject(this, &ASlidingDoubleDoor::HandleSwitchToggled);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s has an empty slot in LinkedSwitches."), *GetName());
		}
	}

	// 根据所有开关的初始状态，立即同步门的状态（不播放动画、不发消息）
	Alpha = TargetAlpha = AreAllSwitchesOn() ? 1.f : 0.f;
	ApplyAlpha(Alpha);

	// 新增：开局就已经是打开状态的门，视为"已经开过"，之后不再播放演出
	if (Alpha >= 1.f)
	{
		bHasPlayedCutscene = true;
	}

}

void ASlidingDoubleDoor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	/*
	// 取消订阅
	if (LinkedSwitch)
	{
		LinkedSwitch->OnSwitchToggled.Remove(SwitchHandle);
	}
	*/

	// 新增：万一在演出中途结束（切关卡等），把镜头和输入还给玩家
	EndOpenCutscene();

	//一次性取消所有订阅
	for (ASwitchBase* Switch : LinkedSwitches)
	{
		if (Switch)
		{
			// 移除这个委托中所有绑定到 this 的函数
			Switch->OnSwitchToggled.RemoveAll(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void ASlidingDoubleDoor::HandleSwitchToggled(ASwitchBase* Switch, bool bIsOn)
{
	//SetOpen(bIsOn);

	//任意一个开关变化时，重新判断整体条件
	SetOpen(AreAllSwitchesOn());
}

void ASlidingDoubleDoor::SetOpen(bool bOpen)
{
	/*
	* 旧版本，还没有加入开关门消息提示之前的代码
	TargetAlpha = bOpen ? 1.f : 0.f;

	if (!FMath::IsNearlyEqual(Alpha, TargetAlpha))
	{
		SetActorTickEnabled(true);
	}
	*/

	const float NewTarget = bOpen ? 1.f : 0.f;

	// 目标没变（比如已经在开门却又收到"开门"指令），什么都不做
	if (FMath::IsNearlyEqual(TargetAlpha, NewTarget))
	{
		return;
	}

	TargetAlpha = NewTarget;
	SetActorTickEnabled(true);
	PostStateMessage(bOpen);

	// 新增：开门演出
	if (bOpen)
	{
		StartOpenCutscene();     // 内部会判断是否第一次
	}
	else
	{
		EndOpenCutscene();       // 保险：演出中途门被关上，立即切回玩家
	}

	//新增，旗标系统所需
	if (UFlagSubsystem* FlagSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFlagSubsystem>() : nullptr)
	{
		FlagSubsystem->SetFlag(FlagId, bOpen);
	}
}



// Called every frame
void ASlidingDoubleDoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 以恒定速度把 Alpha 推向目标：每秒变化 1/OpenDuration
	Alpha = FMath::FInterpConstantTo(Alpha, TargetAlpha, DeltaTime, 1.f / OpenDuration);
	ApplyAlpha(Alpha);

	// 到达目标后关闭 Tick，不再消耗性能
	if (FMath::IsNearlyEqual(Alpha, TargetAlpha))
	{
		SetActorTickEnabled(false);

		// 新增：如果到达的是"完全打开"，通知演出结束
		if (TargetAlpha >= 1.f)
		{
			OnDoorFullyOpened();
		}
	}

}

void ASlidingDoubleDoor::ApplyAlpha(float InAlpha)
{
	// 缓入缓出：起步和停下时慢，中间快，比匀速移动自然得多
	const float Eased = FMath::InterpEaseInOut(0.f, 1.f, InAlpha, 2.f);
	const FVector Offset(0.f, SlideDistance * Eased, 0.f);

	LeftDoor->SetRelativeLocation(LeftClosedLocation - Offset);
	RightDoor->SetRelativeLocation(RightClosedLocation + Offset);
}

void ASlidingDoubleDoor::PostStateMessage(bool bOpen)
{
	UGameMessageSubsystem* MessageSubsystem = GetWorld()->GetSubsystem<UGameMessageSubsystem>();
	if (!MessageSubsystem)
	{
		return;
	}

	const FText Name = DoorDisplayName.IsEmpty()
		? FText::FromString(GetActorNameOrLabel())
		: DoorDisplayName;

	FFormatNamedArguments Args;
	Args.Add(TEXT("Door"), Name);

	const FText Format = bOpen
		? NSLOCTEXT("SlidingDoor", "Opened", "{Door}が開いています")
		: NSLOCTEXT("SlidingDoor", "Closed", "{Door}が閉まっています");

	MessageSubsystem->PostMessage(FText::Format(Format, Args));
}

//新增，多个开关全部 ON 才开门，所需的功能
bool ASlidingDoubleDoor::AreAllSwitchesOn() const
{
	// 没有关联任何开关时，门永远不会打开
	if (LinkedSwitches.Num() == 0)
	{
		return false;
	}

	for (const ASwitchBase* Switch : LinkedSwitches)
	{
		// 空槽位也视为"条件未满足"，避免漏填时门被意外打开
		if (!Switch || !Switch->IsOn())
		{
			return false;
		}
	}

	return true;
}


// ===== 新增：首次开门演出 =====

void ASlidingDoubleDoor::StartOpenCutscene()
{
	// 只在第一次开门时播放；没指定摄像机的门直接跳过
	if (!bPlayCutsceneOnFirstOpen || bHasPlayedCutscene || bIsPlayingCutscene || !CutsceneCamera)
	{
		return;
	}

	UCameraCutSubsystem* CameraCut = GetWorld()->GetSubsystem<UCameraCutSubsystem>();
	if (!CameraCut)
	{
		return;
	}

	// 切镜头、禁用输入都交给子系统；Duration 传 0 = 由门自己决定何时结束
	if (CameraCut->StartCameraCut(CutsceneCamera, this, 0.f))
	{
		bHasPlayedCutscene = true;
		bIsPlayingCutscene = true;
	}
}

void ASlidingDoubleDoor::EndOpenCutscene()
{
	if (!bIsPlayingCutscene)
	{
		return;
	}
	bIsPlayingCutscene = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CutsceneTimerHandle);
	}

	// 如果镜头已经被别的演出接管，子系统会忽略这次结束请求
	if (UWorld* World = GetWorld())
	{
		if (UCameraCutSubsystem* CameraCut = World->GetSubsystem<UCameraCutSubsystem>())
		{
			CameraCut->EndCameraCut(this);
		}
	}
}

void ASlidingDoubleDoor::OnDoorFullyOpened()
{
	if (!bIsPlayingCutscene)
	{
		return;
	}

	if (CutsceneHoldTime > 0.f)
	{
		// 停留一会儿再切回
		GetWorldTimerManager().SetTimer(
			CutsceneTimerHandle, this, &ASlidingDoubleDoor::EndOpenCutscene,
			CutsceneHoldTime, false);
	}
	else
	{
		EndOpenCutscene();
	}
}