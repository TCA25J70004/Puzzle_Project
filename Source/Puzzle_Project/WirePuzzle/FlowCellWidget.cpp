#include "FlowCellWidget.h"
#include "Components/Image.h"

void UFlowCellWidget::SetVisual(const FLinearColor& Color, bool bIsEndpoint, uint8 ArmMask)
{
	ShowImage(Image_Dot, bIsEndpoint, Color);
	ShowImage(Image_Up, (ArmMask & FlowArm::Up) != 0, Color);
	ShowImage(Image_Down, (ArmMask & FlowArm::Down) != 0, Color);
	ShowImage(Image_Left, (ArmMask & FlowArm::Left) != 0, Color);
	ShowImage(Image_Right, (ArmMask & FlowArm::Right) != 0, Color);
}

void UFlowCellWidget::ClearVisual()
{
	SetVisual(FLinearColor::Transparent, false, 0);
}

void UFlowCellWidget::ShowImage(UImage* Image, bool bShow, const FLinearColor& Color)
{
	if (!Image) return;
	// HitTestInvisible：可见但不拦截鼠标，点击会落到 Image_BG 上，再冒泡给棋盘
	Image->SetVisibility(bShow ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	if (bShow) Image->SetColorAndOpacity(Color);
}