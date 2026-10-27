// Fill out your copyright notice in the Description page of Project Settings.


#include "CrosshairHUD.h"
#include "Engine/Canvas.h"

#include "GameMessageSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "CanvasItem.h"

#include "Puzzle_ProjectCharacter.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "GameFramework/PlayerController.h"

ACrosshairHUD::ACrosshairHUD()
{
	// 如果之后需要中文字体，在这里加载（见下文"关于中文字体"）

	static ConstructorHelpers::FObjectFinder<UInputAction> CloseActionFinder(
		TEXT("/Game/BP/Data/IA_CloseDocument.IA_CloseDocument")); // 换成实际路径
	if (CloseActionFinder.Succeeded())
	{
		CloseDocumentAction = CloseActionFinder.Object;
	}
}

void ACrosshairHUD::BeginPlay()
{
	Super::BeginPlay();

	if (UGameMessageSubsystem* MessageSubsystem=GetWorld()->GetSubsystem<UGameMessageSubsystem>())
	{
		MessageHandle = MessageSubsystem->OnGameMessage.AddUObject(this, &ACrosshairHUD::HandleGameMessage);
	}


	// 让 HUD 自己也能接收输入
	EnableInput(PlayerOwner);
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EIC->BindAction(CloseDocumentAction, ETriggerEvent::Started, this, &ACrosshairHUD::CloseDocument);
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

	if (bDocumentOpen)
	{
		DrawDocument();      // 阅读模式下不显示准星和提示
	}
	else
	{
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


		//绘制互动时的提示词
		DrawInteractPrompt();
	}

	

	DrawMessages(); //系统消息在两种模式下都显示

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


void ACrosshairHUD::OpenDocument(const FText& Title, const FText& Body)
{
	if (bDocumentOpen)
	{
		return;
	}

	DocumentTitle = Title;
	DocumentBody = Body;
	WrapCacheKey = FVector2D::ZeroVector;   // 内容变了，旧的换行结果作废
	bDocumentOpen = true;

	// 冻结角色的所有输入：移动、视角、交互
	if (APawn* Pawn = GetOwningPawn())
	{
		Pawn->DisableInput(PlayerOwner);
	}
}

void ACrosshairHUD::CloseDocument()
{
	if (!bDocumentOpen)
	{
		return;
	}

	bDocumentOpen = false;

	if (APawn* Pawn = GetOwningPawn())
	{
		Pawn->EnableInput(PlayerOwner);
	}
}

//字体获取：把之前的写法提取成一个函数，几处绘制都会用到
UFont* ACrosshairHUD::GetUIFont() const
{
	return MessageFont ? MessageFont.Get() : GEngine->GetLargeFont();
}

void ACrosshairHUD::DrawInteractPrompt()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(2, 0.f, FColor::Cyan,
			FString::Printf(TEXT("[HUD] DrawInteractPrompt, Pawn = %s, IsCharacter = %d"),
				*GetNameSafe(GetOwningPawn()),
				Cast<APuzzle_ProjectCharacter>(GetOwningPawn()) != nullptr));
	}

	const APuzzle_ProjectCharacter* Character = Cast<APuzzle_ProjectCharacter>(GetOwningPawn());
	if (!Character || Character->GetFocusPrompt().IsEmpty())
	{
		return;
	}

	const float UIScale = Canvas->ClipY / 1080.f;
	const float Scale = UIScale * PromptScale;

	// 显示在准星正下方
	FCanvasTextItem Item(
		FVector2D(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f + 30.f * UIScale),
		Character->GetFocusPrompt(),
		GetUIFont(),
		FLinearColor::White);
	Item.bCentreX = true;
	Item.Scale = FVector2D(Scale, Scale);
	Item.EnableShadow(FLinearColor(0.f, 0.f, 0.f, 0.8f));
	Canvas->DrawItem(Item);
}

void ACrosshairHUD::WrapText(const FString& Text, const UFont* Font, float Scale, float MaxWidth, TArray<FString>& OutLines) const
{
	OutLines.Reset();

	// 先按手动换行拆成段落（去掉 Windows 换行符中的 \r）
	TArray<FString> Paragraphs;
	Text.Replace(TEXT("\r"), TEXT("")).ParseIntoArray(Paragraphs, TEXT("\n"), false);

	for (const FString& Paragraph : Paragraphs)
	{
		FString Line;
		for (const TCHAR Ch : Paragraph)
		{
			FString Candidate = Line;
			Candidate.AppendChar(Ch);

			// 加上这个字会超出宽度，就把当前行存起来，从这个字开始新的一行
			if (!Line.IsEmpty() && Font->GetStringSize(*Candidate) * Scale > MaxWidth)
			{
				OutLines.Add(Line);
				Line = FString::Chr(Ch);
			}
			else
			{
				Line = MoveTemp(Candidate);
			}
		}
		OutLines.Add(Line);   // 空段落也保留一行，相当于空行
	}
}

void ACrosshairHUD::DrawDocument()
{
	UFont* Font = GetUIFont();
	if (!Font)
	{
		return;
	}

	// 1. 半透明黑色遮罩，覆盖整个屏幕
	DrawRect(OverlayColor, 0.f, 0.f, Canvas->ClipX, Canvas->ClipY);

	const float UIScale = Canvas->ClipY / 1080.f;
	const float TitleScale = UIScale * DocumentTitleScale;
	const float BodyScale = UIScale * DocumentBodyScale;
	const float PanelWidth = Canvas->ClipX * DocumentWidthRatio;
	const float Left = (Canvas->ClipX - PanelWidth) * 0.5f;
	float Y = Canvas->ClipY * 0.15f;

	// 2. 标题（居中，暖白色）
	FCanvasTextItem TitleItem(FVector2D(Canvas->ClipX * 0.5f, Y), DocumentTitle, Font, FLinearColor(1.f, 0.9f, 0.7f));
	TitleItem.bCentreX = true;
	TitleItem.Scale = FVector2D(TitleScale, TitleScale);
	Canvas->DrawItem(TitleItem);
	Y += Font->GetMaxCharHeight() * TitleScale * 1.8f;

	// 3. 正文：只有窗口尺寸变化或内容改变时才重新计算换行
	const FVector2D CacheKey(Canvas->ClipX, Canvas->ClipY);
	if (WrapCacheKey != CacheKey)
	{
		WrapText(DocumentBody.ToString(), Font, BodyScale, PanelWidth, WrappedLines);
		WrapCacheKey = CacheKey;
	}

	const float LineHeight = Font->GetMaxCharHeight() * BodyScale * 1.4f;
	for (const FString& Line : WrappedLines)
	{
		FCanvasTextItem LineItem(FVector2D(Left, Y), FText::FromString(Line), Font, FLinearColor::White);
		LineItem.Scale = FVector2D(BodyScale, BodyScale);
		Canvas->DrawItem(LineItem);
		Y += LineHeight;
	}

	// 4. 底部的关闭提示
	FCanvasTextItem HintItem(
		FVector2D(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.9f),
		NSLOCTEXT("Document", "CloseHint", "[ESC] 閉める"),
		Font,
		FLinearColor(0.6f, 0.6f, 0.6f));
	HintItem.bCentreX = true;
	HintItem.Scale = FVector2D(UIScale, UIScale);
	Canvas->DrawItem(HintItem);
}