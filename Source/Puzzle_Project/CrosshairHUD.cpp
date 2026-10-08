// Fill out your copyright notice in the Description page of Project Settings.


#include "CrosshairHUD.h"
#include "Engine/Canvas.h"

#include "GameMessageSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "CanvasItem.h"

ACrosshairHUD::ACrosshairHUD()
{
	// 如果之后需要中文字体，在这里加载（见下文"关于中文字体"）
}

void ACrosshairHUD::BeginPlay()
{
	Super::BeginPlay();

	if (UGameMessageSubsystem* MessageSubsystem=GetWorld()->GetSubsystem<UGameMessageSubsystem>())
	{
		MessageHandle = MessageSubsystem->OnGameMessage.AddUObject(this, &ACrosshairHUD::HandleGameMessage);
	}
}

void ACrosshairHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UGameMessageSubsystem* MessageSubsystem = GetWorld()->GetSubsystem<UGameMessageSubsystem>())
	{
		MessageSubsystem->OnGameMessage.Remove(MessageHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void ACrosshairHUD::HandleGameMessage(const FText& Message)
{
	Messages.Add({ Message, GetWorld()->GetTimeSeconds() });

	// 超过上限时丢弃最旧的消息
	while(Messages.Num()>MaxMessages)
	{
		Messages.RemoveAt(0);
	}
}

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

	DrawMessages();

}

void ACrosshairHUD::DrawMessages()
{
	if (Messages.Num()==0)
	{
		return;
	}

	const double Now = GetWorld()->GetTimeSeconds(); // float → double

	// 移除已经过期的消息
	Messages.RemoveAll([this, Now](const FHUDMessage& Msg)
		{
			return Now - Msg.StartTime >= MessageDuration;
		}

	);

	UFont* Font = MessageFont ? MessageFont.Get() : GEngine->GetLargeFont();
	if (!Font)
	{
		return;
	}

	const float Scale = Canvas->ClipY / 1080.f * MessageScale;
	const float LineHeight = Font->GetMaxCharHeight() * Scale * 1.2f;
	float Y = Canvas->ClipY * MessageYRatio;

	for (const FHUDMessage& Msg: Messages)
	{
		// 两个时间戳相减得到的时间差很小，转回 float 是安全的
		const float Elapsed = static_cast<float>(Now - Msg.StartTime);
		const float Remaining = MessageDuration - Elapsed;
		const float Opacity = FMath::Clamp(Remaining / FadeOutTime, 0.f, 1.f);

		FCanvasTextItem TextItem(
			FVector2D(Canvas->ClipX * 0.5f, Y),
			Msg.Text,
			Font,
			FLinearColor(1.f, 1.f, 1.f, Opacity)
		);

		TextItem.bCentreX = true; // 以 X 坐标为中心水平居中
		TextItem.Scale = FVector2D(Scale, Scale);
		TextItem.EnableShadow(FLinearColor(0.f, 0.f, 0.f, Opacity * 0.8f));
		TextItem.BlendMode = SE_BLEND_Translucent; // 让透明度生效

		Canvas->DrawItem(TextItem);
		Y += LineHeight;

	}
	
}

