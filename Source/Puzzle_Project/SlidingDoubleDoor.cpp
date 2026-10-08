// Fill out your copyright notice in the Description page of Project Settings.


#include "SlidingDoubleDoor.h"
#include "MySwitchActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/CollisionProfile.h"
#include "UObject/ConstructorHelpers.h"

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
	
}

void ASlidingDoubleDoor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 取消订阅
	if (LinkedSwitch)
	{
		LinkedSwitch->OnSwitchToggled.Remove(SwitchHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void ASlidingDoubleDoor::HandleSwitchToggled(AMySwitchActor* Switch, bool bIsOn)
{
	SetOpen(bIsOn);
}

void ASlidingDoubleDoor::SetOpen(bool bOpen)
{
	TargetAlpha = bOpen ? 1.f : 0.f;

	if (!FMath::IsNearlyEqual(Alpha, TargetAlpha))
	{
		SetActorTickEnabled(true);
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

