// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CrosshairHUD.generated.h"

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
	
};
