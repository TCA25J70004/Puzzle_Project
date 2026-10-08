// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CrosshairHUD.generated.h"

class UFont;

/**
 * 
 */
UCLASS()
class PUZZLE_PROJECT_API ACrosshairHUD : public AHUD // YOURPROJECT 换成你的模块名（创建类时自动生成的那个）
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

protected:
	// 以 1080p 为基准的点大小（像素）
	UPROPERTY(EditDefaultsOnly, Category = "Crosshair")
	float DotSize = 8.f;

	// 外圈描边宽度，让点在白色背景上也看得清
	UPROPERTY(EditDefaultsOnly, Category = "Crosshair")
	float OutlineThickness = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "Crosshair")
	FLinearColor DotColor = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, Category = "Crosshair")
	FLinearColor OutlineColor = FLinearColor(0.f, 0.f, 0.f, 0.6f);

public:
	ACrosshairHUD();

protected:
	virtual void BeginPlay()override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditDefaultsOnly, Category = "Message")
	TObjectPtr<UFont> MessageFont;

	//每条消息显示的总时长（秒）
	UPROPERTY(EditDefaultsOnly, Category = "Message")
	float MessageDuration = 2.5f;

	//最后多少秒用于淡出
	UPROPERTY(EditDefaultsOnly, Category = "Message")
	float FadeOutTime = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Message")
	float MessageScale = 1.5f;

	//消息的垂直位置（0 = 屏幕顶部，1 = 屏幕底部）
	UPROPERTY(EditDefaultsOnly, Category = "Message")
	float MessageYRatio = 0.65f;

	UPROPERTY(EditDefaultsOnly, Category = "Message")
	int32 MaxMessages = 4;

private:

	struct FHUDMessage
	{
		FText Text;
		double StartTime; // float → double
	};

	TArray<FHUDMessage> Messages;
	FDelegateHandle MessageHandle;

	void HandleGameMessage(const FText& Message);
	void DrawMessages();

};
