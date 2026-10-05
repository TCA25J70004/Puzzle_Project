// Copyright Epic Games, Inc. All Rights Reserved.

#include "Puzzle_ProjectGameMode.h"
#include "CrosshairHUD.h" //自己写的 HUD 类

APuzzle_ProjectGameMode::APuzzle_ProjectGameMode()
{
	// stub
	// 指定这个游戏模式使用的 HUD 类
	HUDClass = ACrosshairHUD::StaticClass();
}
