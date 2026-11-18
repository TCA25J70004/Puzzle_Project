// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TriggerEffect.generated.h"

class ACutsceneTrigger;
class APawn;
class USoundBase;
class ACameraActor;

/*
 演出效果的基类。
 EditInlineNew + DefaultToInstanced：可以直接在触发框的 Details 面板里
 从下拉菜单中选择子类、新建实例并编辑参数。
 想加新的演出，只需要继承它并 override Execute()。
 */
UCLASS(Abstract, EditInlineNew, DefaultToInstanced, CollapseCategories)
class PUZZLE_PROJECT_API UTriggerEffect : public UObject
{
	GENERATED_BODY()

public:
	//触发后延迟多少秒才执行（用来安排多个效果的先后顺序）
	UPROPERTY(EditAnywhere, Category = "Effect", meta = (ClampMin = "0.0"))
	float Delay = 0.f;

	//执行演出。Trigger = 触发它的触发框，Player = 走进去的玩家
	virtual void Execute(ACutsceneTrigger* Trigger, APawn* Player) PURE_VIRTUAL(UTriggerEffect::Execute, );
};


/*
 开灯 / 熄灯
 可以指定任意 Actor：引擎自带的 PointLight / SpotLight，
 或者身上带有灯光组件的台灯等，它们身上所有的灯光组件都会被切换。
 */
UCLASS(meta = (DisplayName = "Light On/Off"))
class PUZZLE_PROJECT_API ULightSwitchEffect : public UTriggerEffect
{
	GENERATED_BODY()

public:
	//要控制的灯
	UPROPERTY(EditAnywhere, Category = "Effect")
	TArray<TObjectPtr<AActor>> Lights;

	//false = 熄灯，true = 开灯
	UPROPERTY(EditAnywhere, Category = "Effect")
	bool bTurnOn = false;

	virtual void Execute(ACutsceneTrigger* Trigger, APawn* Player) override;
};


/**
 * 播放音效
 * 指定了 SoundSource 时在它的位置播放 3D 音效，否则播放 2D 音效（直接在耳边响）
 */
UCLASS(meta = (DisplayName = "Play Sound"))
class PUZZLE_PROJECT_API UPlaySoundEffect : public UTriggerEffect
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Effect")
	TObjectPtr<USoundBase> Sound;

	//声音从哪里发出；留空 = 2D 音效
	UPROPERTY(EditAnywhere, Category = "Effect")
	TObjectPtr<AActor> SoundSource;

	UPROPERTY(EditAnywhere, Category = "Effect", meta = (ClampMin = "0.0"))
	float VolumeMultiplier = 1.f;

	virtual void Execute(ACutsceneTrigger* Trigger, APawn* Player) override;
};


/**
 * 切镜头：立刻切到指定摄像机，Duration 秒后立刻切回玩家
 * 多个切镜头效果用 Delay 错开，就能做出连续切换多个机位的演出
 */
UCLASS(meta = (DisplayName = "Camera Cut"))
class PUZZLE_PROJECT_API UCameraCutEffect : public UTriggerEffect
{
	GENERATED_BODY()

public:
	//要切到的摄像机
	UPROPERTY(EditAnywhere, Category = "Effect")
	TObjectPtr<ACameraActor> Camera;

	//镜头停留多少秒后切回玩家
	UPROPERTY(EditAnywhere, Category = "Effect", meta = (ClampMin = "0.1"))
	float Duration = 2.f;

	virtual void Execute(ACutsceneTrigger* Trigger, APawn* Player) override;
};