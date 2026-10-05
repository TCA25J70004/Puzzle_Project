// Fill out your copyright notice in the Description page of Project Settings.


#include "CrosshairHUD.h"
#include "Engine/Canvas.h"

void ACrosshairHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas)
	{
		return;
	}

	// ClipX   ClipY 是当前视口的像素尺寸
	const float CenterX = Canvas->ClipX * 0.5f;
	const float CenterY = Canvas->ClipY * 0.5f;

	// 按分辨率缩放：1080p 下为 1 倍，4K 下为 2 倍，保证点的视觉大小一致
	const float Scale = Canvas->ClipY / 1080.f;
	const float Size = DotSize * Scale;
	const float Outline = OutlineThickness * Scale;

	// 先画稍大的深色方块作为描边
	const float OuterSize = Size + Outline * 2.f;
	DrawRect(OutlineColor, CenterX - OuterSize * 0.5f, CenterY - OuterSize * 0.5f, OuterSize, OuterSize);

	// 再在上面画白色的点
	DrawRect(DotColor, CenterX - Size * 0.5f, CenterY - Size * 0.5f, Size, Size);


}

