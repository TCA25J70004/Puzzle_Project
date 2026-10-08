// Fill out your copyright notice in the Description page of Project Settings.


#include "MySwitchActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/CollisionProfile.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"


// Sets default values
AMySwitchActor::AMySwitchActor()
{
	// 开关只在被点击时才工作，不需要每帧更新
	PrimaryActorTick.bCanEverTick = false;

	SwitchMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SwitchMesh"));
	RootComponent = SwitchMesh;

	// 必须阻挡 Visibility 通道，否则交互射线会直接穿过去
	SwitchMesh->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);

	// 默认加载 SM_SwitchCube，路径是资产右键 → Copy Reference
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshFinder(
		TEXT("/Game/Mesh/SM/SM_SwitchCube.SM_SwitchCube")
	);
	if (MeshFinder.Succeeded())
	{
		SwitchMesh->SetStaticMesh(MeshFinder.Object);
	}


}

// Called when the game starts or when spawned
void AMySwitchActor::BeginPlay()
{
	Super::BeginPlay();

	// 根据初始状态刷新一次外观
	UpdateVisual();
	
}

// Called every frame
void AMySwitchActor::Tick(float DeltaTime)
{

}

void AMySwitchActor::Interact(AActor* Interactor)
{
	bIsOn = !bIsOn;
	UpdateVisual();

	UE_LOG(LogTemp, Log, TEXT("%s toggled by %s -> %s"),
		*GetName(), *GetNameSafe(Interactor), bIsOn ? TEXT("ON") : TEXT("OFF")
		);

	// 通知所有订阅者；没人订阅时调用也是安全的
	OnSwitchToggled.Broadcast(this, bIsOn);
		

}

void AMySwitchActor::UpdateVisual()
{
	UMaterialInterface* Mat = bIsOn ? OnMaterial : OffMaterial;
	if (Mat)
	{
		SwitchMesh->SetMaterial(0, Mat);
	}
}



