#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FlowCellWidget.generated.h"

class UImage;

// 四个方向的"臂"，用位掩码组合，例如 Up | Right 表示拐角
namespace FlowArm
{
	constexpr uint8 Up = 1 << 0;
	constexpr uint8 Down = 1 << 1;
	constexpr uint8 Left = 1 << 2;
	constexpr uint8 Right = 1 << 3;
}

// Abstract：这个 C++ 类本身没有布局，只能通过 WBP 子类使用
UCLASS(Abstract)
class PUZZLE_PROJECT_API UFlowCellWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetVisual(const FLinearColor& Color, bool bIsEndpoint, uint8 ArmMask);
	void ClearVisual();

protected:
	// 名字和类型必须与 WBP_FlowCell 里的控件完全一致
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UImage> Image_Up;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UImage> Image_Down;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UImage> Image_Left;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UImage> Image_Right;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UImage> Image_Dot;

private:
	static void ShowImage(UImage* Image, bool bShow, const FLinearColor& Color);
};