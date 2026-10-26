// Fill out your copyright notice in the Description page of Project Settings.


#include "PressurePlate.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/CollisionProfile.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

APressurePlate::APressurePlate()
{
	PrimaryActorTick.bCanEverTick = false;

	PlateRoot = CreateDefaultSubobject<USceneComponent>(TEXT("PlateRoot"));
	RootComponent = PlateRoot;

	PlateMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlateMesh"));
	PlateMesh->SetupAttachment(PlateRoot);
	PlateMesh->SetMobility(EComponentMobility::Movable); // 被压下时需要移动
	PlateMesh->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);

	// 路径 资产右键 → Copy Reference
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlateFinder(
		TEXT("/Game/Mesh/SM/SM_Pressure_Plate.SM_Pressure_Plate"));
	if (PlateFinder.Succeeded())
	{
		PlateMesh->SetStaticMesh(PlateFinder.Object);
	}

	// 触发盒挂在根组件上，而不是板子上，这样板子下沉时触发区域不会跟着动
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(PlateRoot);
	TriggerBox->SetCollisionProfileName(TEXT("Trigger")); // 只检测重叠，不阻挡任何东西
	TriggerBox->SetGenerateOverlapEvents(true);
}

void APressurePlate::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// 根据模型的实际大小，自动调整触发盒，使其刚好覆盖在板面上方
	const UStaticMesh* Mesh = PlateMesh->GetStaticMesh();
	if (!Mesh)
	{
		return;
	}

	const FBox Bounds = Mesh->GetBoundingBox();   // 模型自身空间的包围盒
	const FVector Scale = PlateMesh->GetRelativeScale3D();
	const FVector Extent = Bounds.GetExtent() * Scale;
	const FVector Center = Bounds.GetCenter() * Scale;
	const float TopZ = Bounds.Max.Z * Scale.Z;

	// 触发盒底部稍微伸进板面 2 厘米，确保刚好放在板面上的物体也能被检测到
	const float Bottom = TopZ - 2.f;
	const float HalfHeight = (TriggerHeight + 2.f) * 0.5f;

	TriggerBox->SetBoxExtent(FVector(Extent.X * TriggerInset, Extent.Y * TriggerInset, HalfHeight));
	TriggerBox->SetRelativeLocation(FVector(Center.X, Center.Y, Bottom + HalfHeight));
}

void APressurePlate::BeginPlay()
{
	Super::BeginPlay();

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &APressurePlate::OnTriggerBeginOverlap);
	TriggerBox->OnComponentEndOverlap.AddDynamic(this, &APressurePlate::OnTriggerEndOverlap);

	// 关卡开始时，板子上可能已经放着东西
	UpdatePressedState();
}

void APressurePlate::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UpdatePressedState();
}

void APressurePlate::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UpdatePressedState();
}

void APressurePlate::UpdatePressedState()
{
	FTimerManager& TimerManager = GetWorldTimerManager();

	if (HasQualifyingOverlap())
	{
		// 有东西压着：取消可能正在进行的弹起倒计时，立即压下
		TimerManager.ClearTimer(ReleaseTimerHandle);
		SetIsOn(true);
	}
	else if (bIsOn && !TimerManager.IsTimerActive(ReleaseTimerHandle))
	{
		// 东西离开了：不立即弹起，而是开始倒计时
		TimerManager.SetTimer(ReleaseTimerHandle, this, &APressurePlate::OnReleaseTimerExpired, ReleaseDelay, false);
	}
}

void APressurePlate::OnReleaseTimerExpired()
{
	// 倒计时结束时再确认一次，确实没有东西了才弹起
	if (!HasQualifyingOverlap())
	{
		SetIsOn(false);
	}
}

bool APressurePlate::HasQualifyingOverlap() const
{
	TArray<UPrimitiveComponent*> Overlaps;
	TriggerBox->GetOverlappingComponents(Overlaps);

	for (const UPrimitiveComponent* Component : Overlaps)
	{
		if (!Component || Component->GetOwner() == this)
		{
			continue;
		}

		// 玩家
		if (Cast<APawn>(Component->GetOwner()))
		{
			return true;
		}

		// 有一定重量的物理物体
		if (Component->IsSimulatingPhysics() && Component->GetMass() >= MinActivationMass)
		{
			return true;
		}
	}

	return false;
}

void APressurePlate::OnStateChanged()
{
	// 被压下时板面下沉
	PlateMesh->SetRelativeLocation(bIsOn ? FVector(0.f, 0.f, -PressDepth) : FVector::ZeroVector);
}