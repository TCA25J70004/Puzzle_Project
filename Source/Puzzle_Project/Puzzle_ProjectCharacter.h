// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Puzzle_ProjectCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

class UPhysicsHandleComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A basic first person character
 */
UCLASS(abstract)
class APuzzle_ProjectCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: first person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FirstPersonMesh;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

protected:

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	class UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	class UInputAction* MouseLookAction;
	
public:
	APuzzle_ProjectCharacter();

protected:

	/** Called from Input Actions for movement input */
	void MoveInput(const FInputActionValue& Value);

	/** Called from Input Actions for looking input */
	void LookInput(const FInputActionValue& Value);

	/** Handles aim inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoAim(float Yaw, float Pitch);

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles jump start inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump end inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

protected:
	//交互输入动作
	UPROPERTY(EditAnywhere, Category = "Input")
	class UInputAction* InteractAction;

	//交互距离cm
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float InteractDistance = 300.f;

	//按下交互键时调用
	void Interact();


protected:
	/** 从摄像机沿视线方向发射射线 */
	bool TraceFromCamera(float Distance, FHitResult& OutHit) const;


protected:

	/** Set up input action bindings */
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	

public:

	/** Returns the first person mesh **/
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }

	/** Returns first person camera component **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }


public:
	virtual void Tick(float DeltaSeconds) override;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Grab")
	TObjectPtr<UPhysicsHandleComponent> PhysicsHandle;


	/** 拿在手中时，物体中心距离摄像机多远 */
	UPROPERTY(EditDefaultsOnly, Category = "Grab")
	float HoldDistance = 200.f;

	/** 能拾取的最大质量（kg） */
	UPROPERTY(EditDefaultsOnly, Category = "Grab")
	float MaxGrabMass = 50.f;

	/** 物体被卡住、离目标点超过这个距离时自动脱手 */
	UPROPERTY(EditDefaultsOnly, Category = "Grab")
	float BreakDistance = 150.f;

	/** 尝试拾取指定组件，成功则返回 true */
	bool TryGrabComponent(UPrimitiveComponent* Component);
	void ReleaseGrabbed();

	FVector GetHoldLocation() const;
	FQuat GetViewYawQuat() const;

private:
	/** 拾起时，物体相对于玩家水平朝向的旋转 */
	FQuat HeldRelativeRotation = FQuat::Identity;

	/** 物体是否已经到达过手中的位置 */
	bool bHoldSettled = false;

public:
		//当前对准物体的提示文字，供 HUD 显示
		const FText& GetFocusPrompt() const { return FocusPrompt; }

private:
	void UpdateFocus();

	FText FocusPrompt;

};

