#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FlowTypes.h"
#include "FlowBoardWidget.generated.h"

class UUniformGridPanel;
class UFlowCellWidget;
class UFlowPuzzleState;
class AFlowPuzzleTerminal;

UCLASS(Abstract)
class PUZZLE_PROJECT_API UFlowBoardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFlowBoardWidget(const FObjectInitializer& ObjectInitializer);

	// 由终端在 CreateWidget 之后调用
	bool InitPuzzle(const FPuzzleLevel& Level, AFlowPuzzleTerminal* InTerminal);

protected:
	virtual void NativeOnInitialized() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseCaptureLost(const FCaptureLostEvent& CaptureLostEvent) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUniformGridPanel> GridPanel;

	// 在 WBP_FlowBoard 的 Class Defaults 里指定为 WBP_FlowCell
	UPROPERTY(EditDefaultsOnly, Category = "Flow")
	TSubclassOf<UFlowCellWidget> CellClass;

	// 颜色由美术调整，不写死在格子里
	UPROPERTY(EditDefaultsOnly, Category = "Flow")
	TMap<EFlowColor, FLinearColor> ColorPalette;

private:
	bool GetCellUnderMouse(const FPointerEvent& MouseEvent, FIntPoint& OutCell, bool bClamp) const;
	bool DragTowards(FIntPoint Target);
	void FinishStroke();
	void ClosePuzzle();
	void Redraw();

	UPROPERTY()
	TObjectPtr<UFlowPuzzleState> PuzzleState;

	UPROPERTY()
	TArray<TObjectPtr<UFlowCellWidget>> Cells;   // 下标 = Y * GridSize + X

	TWeakObjectPtr<AFlowPuzzleTerminal> OwnerTerminal;
	bool bSolved = false;
};