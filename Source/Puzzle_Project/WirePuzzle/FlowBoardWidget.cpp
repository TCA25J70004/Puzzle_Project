#include "FlowBoardWidget.h"
#include "FlowCellWidget.h"
#include "FlowPuzzleState.h"
#include "FlowPuzzleTerminal.h"
#include "Components/UniformGridPanel.h"

UFlowBoardWidget::UFlowBoardWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 默认配色，可以在 WBP 的 Class Defaults 里覆盖
	ColorPalette.Add(EFlowColor::Red, FLinearColor(0.90f, 0.10f, 0.10f));
	ColorPalette.Add(EFlowColor::Green, FLinearColor(0.10f, 0.75f, 0.20f));
	ColorPalette.Add(EFlowColor::Blue, FLinearColor(0.15f, 0.35f, 0.95f));
	ColorPalette.Add(EFlowColor::Yellow, FLinearColor(0.95f, 0.85f, 0.10f));
	ColorPalette.Add(EFlowColor::Orange, FLinearColor(1.00f, 0.50f, 0.05f));
}

void UFlowBoardWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	SetIsFocusable(true); // 修掉那条 Non-Focusable 的 Error，键盘事件才能送到这里
}

bool UFlowBoardWidget::InitPuzzle(const FPuzzleLevel& Level, AFlowPuzzleTerminal* InTerminal)
{
	OwnerTerminal = InTerminal;
	bSolved = false;

	if (!GridPanel || !CellClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[FlowBoard] GridPanel or CellClass missing"));
		return false;
	}

	PuzzleState = NewObject<UFlowPuzzleState>(this);
	if (!PuzzleState->Init(Level)) return false;

	GridPanel->ClearChildren();
	Cells.Reset();

	const int32 N = PuzzleState->GetGridSize();
	for (int32 Y = 0; Y < N; ++Y)          // 外层：行
	{
		for (int32 X = 0; X < N; ++X)      // 内层：列
		{
			UFlowCellWidget* Cell = CreateWidget<UFlowCellWidget>(this, CellClass);
			GridPanel->AddChildToUniformGrid(Cell, /*Row*/ Y, /*Column*/ X);
			Cells.Add(Cell);               // 加入顺序保证下标 = Y * N + X
		}
	}

	Redraw();
	return true;
}

// ---------- 输入 ----------

FReply UFlowBoardWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// 右键退出（PIE 中 Esc 默认会结束运行，见下文）
	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		ClosePuzzle();
		return FReply::Handled();
	}

	if (bSolved || !PuzzleState || InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	FIntPoint Cell;
	if (GetCellUnderMouse(InMouseEvent, Cell, /*bClamp*/ false) && PuzzleState->BeginPath(Cell))
	{
		Redraw();
		// 捕获鼠标：拖动期间即使移出棋盘，Move/Up 事件也会继续送到这里
		return FReply::Handled().CaptureMouse(TakeWidget());
	}
	return FReply::Handled();
}

FReply UFlowBoardWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (PuzzleState && PuzzleState->GetActiveColor() != EFlowColor::None)
	{
		FIntPoint Cell;
		// bClamp = true：拖出棋盘时按最近的边缘格处理
		if (GetCellUnderMouse(InMouseEvent, Cell, /*bClamp*/ true) && DragTowards(Cell))
		{
			Redraw();
		}
		return FReply::Handled();
	}
	return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}

FReply UFlowBoardWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (PuzzleState && PuzzleState->GetActiveColor() != EFlowColor::None)
	{
		FinishStroke();
		return FReply::Handled().ReleaseMouseCapture();
	}
	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

void UFlowBoardWidget::NativeOnMouseCaptureLost(const FCaptureLostEvent& CaptureLostEvent)
{
	Super::NativeOnMouseCaptureLost(CaptureLostEvent);
	// Alt+Tab 等情况会在没有松开鼠标时丢失捕获，这里同样结束这一笔
	// 正常松开时 FinishStroke 已经把 ActiveColor 设为 None，这里不会重复执行
	if (PuzzleState && PuzzleState->GetActiveColor() != EFlowColor::None)
	{
		FinishStroke();
	}
}

FReply UFlowBoardWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		ClosePuzzle();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

// ---------- 逻辑衔接 ----------

bool UFlowBoardWidget::GetCellUnderMouse(const FPointerEvent& MouseEvent, FIntPoint& OutCell, bool bClamp) const
{
	if (!PuzzleState || !GridPanel) return false;

	// 把屏幕坐标转换成 GridPanel 内部的局部坐标
	const FGeometry& Geo = GridPanel->GetCachedGeometry();
	const FVector2D Local = Geo.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	const FVector2D Size = Geo.GetLocalSize();
	if (Size.X <= 0.0 || Size.Y <= 0.0) return false;

	// UniformGridPanel 的每格大小相同，所以按比例就能算出第几格
	const int32 N = PuzzleState->GetGridSize();
	int32 X = FMath::FloorToInt(Local.X / Size.X * N);
	int32 Y = FMath::FloorToInt(Local.Y / Size.Y * N);
	if (bClamp)
	{
		X = FMath::Clamp(X, 0, N - 1);
		Y = FMath::Clamp(Y, 0, N - 1);
	}

	OutCell = FIntPoint(X, Y);
	return PuzzleState->IsInBounds(OutCell);
}

bool UFlowBoardWidget::DragTowards(FIntPoint Target)
{
	// 鼠标一帧可能跳过好几格，这里从路径末端一格一格走向目标
	bool bChanged = false;
	const FFlowPath* Path = PuzzleState->FindPath(PuzzleState->GetActiveColor());

	while (Path && Path->Points.Num() > 0)
	{
		const FIntPoint Current = Path->Points.Last();
		const FIntPoint Delta = Target - Current;
		if (Delta == FIntPoint::ZeroValue) break;

		// 优先沿差距较大的轴走一步
		const FIntPoint Step = (FMath::Abs(Delta.X) >= FMath::Abs(Delta.Y))
			? FIntPoint(FMath::Sign(Delta.X), 0)
			: FIntPoint(0, FMath::Sign(Delta.Y));

		if (!PuzzleState->ExtendPath(Current + Step)) break; // 被规则拒绝就停下
		bChanged = true;
	}
	return bChanged;
}

void UFlowBoardWidget::FinishStroke()
{
	PuzzleState->EndPath();
	Redraw();

	// 唯一的胜利判定点：每一笔结束时
	if (PuzzleState->IsSolved())
	{
		bSolved = true;
		if (AFlowPuzzleTerminal* Terminal = OwnerTerminal.Get())
		{
			Terminal->NotifySolved();
		}
	}
}

void UFlowBoardWidget::ClosePuzzle()
{
	if (AFlowPuzzleTerminal* Terminal = OwnerTerminal.Get())
	{
		Terminal->ClosePuzzle();
	}
}

// ---------- 绘制 ----------

void UFlowBoardWidget::Redraw()
{
	if (!PuzzleState) return;
	const int32 N = PuzzleState->GetGridSize();

	// 返回从 From 指向相邻格 To 的方向
	auto ArmToward = [](FIntPoint From, FIntPoint To) -> uint8
		{
			const FIntPoint D = To - From;
			if (D == FIntPoint(0, -1)) return FlowArm::Up;    // Y 向下增大，所以 Y-1 是上
			if (D == FIntPoint(0, 1)) return FlowArm::Down;
			if (D == FIntPoint(-1, 0)) return FlowArm::Left;
			if (D == FIntPoint(1, 0)) return FlowArm::Right;
			return 0;
		};

	// 1. 根据路径算出每格的臂：连向前一格和后一格
	TArray<uint8> Arms;
	Arms.Init(0, N * N);
	for (const TPair<EFlowColor, FFlowPath>& Entry : PuzzleState->GetPaths())
	{
		const TArray<FIntPoint>& P = Entry.Value.Points;
		for (int32 i = 0; i < P.Num(); ++i)
		{
			uint8& Mask = Arms[P[i].Y * N + P[i].X];
			if (i > 0)            Mask |= ArmToward(P[i], P[i - 1]);
			if (i + 1 < P.Num())  Mask |= ArmToward(P[i], P[i + 1]);
		}
	}

	// 2. 把结果交给每个格子显示
	for (int32 Y = 0; Y < N; ++Y)
	{
		for (int32 X = 0; X < N; ++X)
		{
			const FIntPoint C(X, Y);
			const int32 Idx = Y * N + X;
			const EFlowColor Color = PuzzleState->GetCellColor(C);

			if (Color == EFlowColor::None)
			{
				Cells[Idx]->ClearVisual();
				continue;
			}

			const FLinearColor* Found = ColorPalette.Find(Color);
			const FLinearColor LineColor = Found ? *Found : FLinearColor::White;
			const bool bIsEndpoint = PuzzleState->GetEndpointColor(C) != EFlowColor::None;
			Cells[Idx]->SetVisual(LineColor, bIsEndpoint, Arms[Idx]);
		}
	}
}