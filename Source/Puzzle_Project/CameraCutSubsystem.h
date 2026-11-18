// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CameraCutSubsystem.generated.h"

class ACameraActor;
class APawn;

/**
 * 统一管理"镜头切到某台摄像机，再切回玩家"的子系统。
 * 同一时刻只有一个切镜头生效：
 *  - 新的 StartCameraCut 会覆盖正在进行的切镜头
 *  - EndCameraCut 只有当前的请求者调用才有效，避免把别人的镜头切走
 */
UCLASS()
class PUZZLE_PROJECT_API UCameraCutSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * 立刻切到指定摄像机，并禁用玩家输入。
	 * @param Camera     要切到的摄像机
	 * @param Requester  请求者（通常传 this），用于之后结束
	 * @param Duration   > 0：这么多秒后自动切回；<= 0：需要请求者手动调用 EndCameraCut
	 * @return 是否成功切换
	 */
	bool StartCameraCut(ACameraActor* Camera, UObject* Requester, float Duration = 0.f);

	//立刻切回玩家视角并恢复输入；Requester 不是当前请求者时不做任何事
	void EndCameraCut(UObject* Requester);

	bool IsCameraCutActive() const { return bIsActive; }

private:
	void HandleDurationElapsed();
	void FinishCameraCut();

	bool bIsActive = false;
	TWeakObjectPtr<UObject> CurrentRequester;
	TWeakObjectPtr<APawn> DisabledPawn;   // 被禁用输入的角色，结束时恢复
	FTimerHandle DurationTimer;
};