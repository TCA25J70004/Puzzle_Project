// Fill out your copyright notice in the Description page of Project Settings.


#include "NoteActor.h"
#include "CrosshairHUD.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/CollisionProfile.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "UObject/ConstructorHelpers.h"

// Sets default values
ANoteActor::ANoteActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
	Mesh->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);

	// 暂时用压扁的立方体当作一张纸（30 × 21 × 1 厘米），以后可以换成自己的模型
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeFinder.Succeeded())
	{
		Mesh->SetStaticMesh(CubeFinder.Object);
	}
	Mesh->SetRelativeScale3D(FVector(0.3f, 0.21f, 0.01f));

	InteractPrompt = NSLOCTEXT("Note", "Prompt", "[E] 見る");
}

void ANoteActor::Interact(AActor* Interactor)
{
	// 找到"发起交互的那个玩家"的 HUD
	const APawn* Pawn = Cast<APawn>(Interactor);
	const APlayerController* PC = Pawn ? Pawn->GetController<APlayerController>() : nullptr;

	if (ACrosshairHUD* HUD = PC ? PC->GetHUD<ACrosshairHUD>() : nullptr)
	{
		HUD->OpenDocument(Title, Body);
	}
}
