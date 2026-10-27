// Copyright Epic Games, Inc. All Rights Reserved.

#include "Puzzle_ProjectCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Puzzle_Project.h"

#include "Interactable.h"
#include "DrawDebugHelpers.h"

#include "PhysicsEngine/PhysicsHandleComponent.h"

APuzzle_ProjectCharacter::APuzzle_ProjectCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	
	// Create the first person mesh that will be viewed only by this character's owner
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));

	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	// Create the Camera Component	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh, FName("head"));
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	// configure the character comps
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

	// Configure character movement
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;

	// 角色需要每帧更新物体的目标位置
	PrimaryActorTick.bCanEverTick = true;

	PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandle"));
}

void APuzzle_ProjectCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &APuzzle_ProjectCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &APuzzle_ProjectCharacter::DoJumpEnd);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APuzzle_ProjectCharacter::MoveInput);

		// Looking/Aiming
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APuzzle_ProjectCharacter::LookInput);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &APuzzle_ProjectCharacter::LookInput);

		//新增，互动
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &APuzzle_ProjectCharacter::Interact);

	}
	else
	{
		UE_LOG(LogPuzzle_Project, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}


void APuzzle_ProjectCharacter::MoveInput(const FInputActionValue& Value)
{
	// get the Vector2D move axis
	FVector2D MovementVector = Value.Get<FVector2D>();

	// pass the axis values to the move input
	DoMove(MovementVector.X, MovementVector.Y);

}

void APuzzle_ProjectCharacter::LookInput(const FInputActionValue& Value)
{
	// get the Vector2D look axis
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// pass the axis values to the aim input
	DoAim(LookAxisVector.X, LookAxisVector.Y);

}

void APuzzle_ProjectCharacter::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		// pass the rotation inputs
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void APuzzle_ProjectCharacter::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		// pass the move inputs
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void APuzzle_ProjectCharacter::DoJumpStart()
{
	// pass Jump to the character
	Jump();
}

void APuzzle_ProjectCharacter::DoJumpEnd()
{
	// pass StopJumping to the character
	StopJumping();
}

bool APuzzle_ProjectCharacter::TraceFromCamera(float Distance, FHitResult& OutHit) const
{
	const UCameraComponent* Camera = GetFirstPersonCameraComponent();
	if (!Camera)
	{
		return false;
	}

	const FVector Start = Camera->GetComponentLocation();
	const FVector End = Start + Camera->GetForwardVector() * Distance;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(CameraTrace), false, this);
	return GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, Params);
}

void APuzzle_ProjectCharacter::Interact()
{
	/*
	UCameraComponent* Camera = GetFirstPersonCameraComponent();
	if (!Camera)
	{
		return;
	}

	// 射线从摄像机位置出发，沿视线方向延伸 InteractDistance
	const FVector Start = Camera->GetComponentLocation();
	const FVector End = Start + Camera->GetForwardVector() * InteractDistance;

	FHitResult Hit;

	// 第三个参数 this：忽略玩家自己，防止射线打到自己的胶囊体
	FCollisionQueryParams Params(SCENE_QUERY_STAT(InteractTrace), false, this);

	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

#if ENABLE_DRAW_DEBUG

	// 调试用：命中画绿线，未命中画红线，持续 2 秒
	DrawDebugLine(GetWorld(), Start, bHit ? Hit.ImpactPoint : End, bHit ? FColor::Green : FColor::Red, false, 2.f, 0, 1.f);


#endif

	if (!bHit)
	{
		return;
	}

	AActor* HitActor = Hit.GetActor();
	UE_LOG(LogTemp, Log, TEXT("Interact trace hit: %s"), *GetNameSafe(HitActor));

	// 问这个物体：你实现了 IInteractable 吗？
	if (IInteractable* Interactable= Cast<IInteractable>(HitActor))
	{
		Interactable->Interact(this);
	}
	*/

	// 1. 手里有东西 → 放下
	if (PhysicsHandle->GetGrabbedComponent())
	{
		ReleaseGrabbed();
		return;
	}

	FHitResult Hit;
	if (!TraceFromCamera(InteractDistance, Hit))
	{
		return;
	}

	// 2. 可交互物体 → 交互
	if (IInteractable* Interactable = Cast<IInteractable>(Hit.GetActor()))
	{
		Interactable->Interact(this);
		return;
	}

	// 3. 物理物体 → 拾起
	TryGrabComponent(Hit.GetComponent());

}


bool APuzzle_ProjectCharacter::TryGrabComponent(UPrimitiveComponent* Component)
{
	if (!Component || !Component->IsSimulatingPhysics())
	{
		return false;
	}

	if (Component->GetMass() > MaxGrabMass)
	{
		UE_LOG(LogTemp, Log, TEXT("%s is too heavy (%.1f kg)."), *GetNameSafe(Component->GetOwner()), Component->GetMass());
		return false;
	}

	HeldRelativeRotation = GetViewYawQuat().Inverse() * Component->GetComponentQuat();
	bHoldSettled = false;

	PhysicsHandle->GrabComponentAtLocationWithRotation(
		Component,
		NAME_None,
		Component->GetComponentLocation(),
		Component->GetComponentRotation());

	return true;
}

void APuzzle_ProjectCharacter::ReleaseGrabbed()
{
	PhysicsHandle->ReleaseComponent();
}

FVector APuzzle_ProjectCharacter::GetHoldLocation() const
{
	const UCameraComponent* Camera = GetFirstPersonCameraComponent();
	return Camera->GetComponentLocation() + Camera->GetForwardVector() * HoldDistance;
}

FQuat APuzzle_ProjectCharacter::GetViewYawQuat() const
{
	// 只取水平方向的旋转（Yaw），忽略抬头和低头（Pitch）
	return FRotator(0.f, GetControlRotation().Yaw, 0.f).Quaternion();
}

void APuzzle_ProjectCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateFocus();   // 新增

	UPrimitiveComponent* Held = PhysicsHandle->GetGrabbedComponent();
	if (!Held)
	{
		return;
	}

	const FVector HoldLocation = GetHoldLocation();
	const float Distance = FVector::Dist(Held->GetComponentLocation(), HoldLocation);

	if (Distance < BreakDistance)
	{
		bHoldSettled = true;
	}
	else if (bHoldSettled)
	{
		// 物体曾经到达过手中，现在却被拉开了很远，说明被墙之类的东西卡住了
		ReleaseGrabbed();
		return;
	}

	const FQuat TargetRotation = GetViewYawQuat() * HeldRelativeRotation;
	PhysicsHandle->SetTargetLocationAndRotation(HoldLocation, TargetRotation.Rotator());
}

void APuzzle_ProjectCharacter::UpdateFocus()
{
	FocusPrompt = FText::GetEmpty();

	// 手里拿着东西时，E 的含义是"放下"，此时不显示其他提示
	if (PhysicsHandle->GetGrabbedComponent())
	{
		return;
	}

	FHitResult Hit;
	if (!TraceFromCamera(InteractDistance, Hit))
	{
		return;
	}

	// 和 Interact() 中的判断顺序保持一致：可交互物体优先，其次是可拾取物体
	if (const IInteractable* Interactable = Cast<IInteractable>(Hit.GetActor()))
	{
		FocusPrompt = Interactable->GetInteractPrompt();
	}
	else if (const UPrimitiveComponent* Component = Hit.GetComponent();
		Component && Component->IsSimulatingPhysics() && Component->GetMass() <= MaxGrabMass)
	{
		FocusPrompt = NSLOCTEXT("Character", "GrabPrompt", "[E] 拾う");
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::Yellow,
			FString::Printf(TEXT("[Focus] Prompt = \"%s\""), *FocusPrompt.ToString()));
	}
}