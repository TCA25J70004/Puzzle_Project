#include "FlowPuzzleState.h"

bool UFlowPuzzleState::Init(const FPuzzleLevel& Level)
{
	GridSize = Level.GridSize;
	const int32 NumCells = GridSize * GridSize;

	Endpoints.Init(EFlowColor::None, NumCells);
	Occupancy.Init(EFlowColor::None, NumCells);
	Paths.Reset();
	PairsByColor.Reset();
	ActiveColor = EFlowColor::None;

	for (const FFlowPair& Pair : Level.Pairs)
	{
		if (Pair.Color == EFlowColor::None)
		{
			UE_LOG(LogTemp, Error, TEXT("[FlowState] Pair has color None"));
			return false;
		}
		if (PairsByColor.Contains(Pair.Color))
		{
			UE_LOG(LogTemp, Error, TEXT("[FlowState] Color %c appears twice"), ColorToChar(Pair.Color));
			return false;
		}

		for (const FIntPoint& P : { Pair.PointA, Pair.PointB })
		{
			if (!IsInBounds(P))
			{
				UE_LOG(LogTemp, Error, TEXT("[FlowState] Endpoint (%d,%d) out of bounds"), P.X, P.Y);
				return false;
			}
			if (Endpoints[ToIndex(P)] != EFlowColor::None)
			{
				// PointA == PointB 也会在这里被拦下
				UE_LOG(LogTemp, Error, TEXT("[FlowState] Endpoint (%d,%d) overlaps another"), P.X, P.Y);
				return false;
			}
			Endpoints[ToIndex(P)] = Pair.Color;
		}

		PairsByColor.Add(Pair.Color, Pair);
		Paths.Add(Pair.Color, FFlowPath());
	}

	RebuildOccupancy();
	return true;
}

bool UFlowPuzzleState::BeginPath(FIntPoint Cell)
{
	if (!IsInBounds(Cell)) return false;
	const int32 Idx = ToIndex(Cell);

	if (Endpoints[Idx] != EFlowColor::None)
	{
		// 按下端点：这条线从头开始
		ActiveColor = Endpoints[Idx];
		FFlowPath& Path = Paths.FindChecked(ActiveColor);
		Path.Points.Reset();
		Path.Points.Add(Cell);
	}
	else if (Occupancy[Idx] != EFlowColor::None)
	{
		// 按下线的中间：截断到这里，接着画
		ActiveColor = Occupancy[Idx];
		FFlowPath& Path = Paths.FindChecked(ActiveColor);
		const int32 Found = Path.Points.IndexOfByKey(Cell);
		Path.Points.SetNum(Found + 1);
	}
	else
	{
		return false; // 空格子，不能从这里开始
	}

	RebuildOccupancy();
	return true;
}

bool UFlowPuzzleState::ExtendPath(FIntPoint Cell)
{
	if (ActiveColor == EFlowColor::None || !IsInBounds(Cell)) return false;

	FFlowPath& Path = Paths.FindChecked(ActiveColor);
	if (Path.Points.Num() == 0) return false;

	const FIntPoint Last = Path.Points.Last();
	if (Cell == Last || !AreAdjacent(Last, Cell)) return false;

	// 1. 拖回自己的路径：回退
	const int32 Existing = Path.Points.IndexOfByKey(Cell);
	if (Existing != INDEX_NONE)
	{
		Path.Points.SetNum(Existing + 1);
		RebuildOccupancy();
		return true;
	}

	// 2. 已经连通：只能回退，不能再往外延伸
	if (IsColorConnected(ActiveColor)) return false;

	const int32 Idx = ToIndex(Cell);

	// 3. 别的颜色的端点：不能进入
	if (Endpoints[Idx] != EFlowColor::None && Endpoints[Idx] != ActiveColor) return false;

	// 4. 别的颜色的线：从这个格子切断对方
	const EFlowColor Other = Occupancy[Idx];
	if (Other != EFlowColor::None && Other != ActiveColor)
	{
		FFlowPath& OtherPath = Paths.FindChecked(Other);
		const int32 Cut = OtherPath.Points.IndexOfByKey(Cell);
		if (Cut != INDEX_NONE)
		{
			OtherPath.Points.SetNum(Cut); // 保留这个格子之前的部分
		}
	}

	// 5. 延伸（如果 Cell 是同色的另一个端点，这一步之后就连通了）
	Path.Points.Add(Cell);
	RebuildOccupancy();
	return true;
}

void UFlowPuzzleState::EndPath()
{
	ActiveColor = EFlowColor::None;
}

bool UFlowPuzzleState::IsColorConnected(EFlowColor Color) const
{
	const FFlowPath* Path = Paths.Find(Color);
	const FFlowPair* Pair = PairsByColor.Find(Color);
	if (!Path || !Pair || Path->Points.Num() < 2) return false;

	const FIntPoint First = Path->Points[0];
	const FIntPoint Last = Path->Points.Last();
	return (First == Pair->PointA && Last == Pair->PointB)
		|| (First == Pair->PointB && Last == Pair->PointA);
}

bool UFlowPuzzleState::IsSolved() const
{
	// 条件 1：每种颜色都连通
	for (const TPair<EFlowColor, FFlowPair>& Entry : PairsByColor)
	{
		if (!IsColorConnected(Entry.Key)) return false;
	}
	// 条件 2：没有空格子
	for (const EFlowColor C : Occupancy)
	{
		if (C == EFlowColor::None) return false;
	}
	return true;
}

bool UFlowPuzzleState::IsInBounds(FIntPoint Cell) const
{
	return Cell.X >= 0 && Cell.Y >= 0 && Cell.X < GridSize && Cell.Y < GridSize;
}

EFlowColor UFlowPuzzleState::GetCellColor(FIntPoint Cell) const
{
	return IsInBounds(Cell) ? Occupancy[ToIndex(Cell)] : EFlowColor::None;
}

EFlowColor UFlowPuzzleState::GetEndpointColor(FIntPoint Cell) const
{
	return IsInBounds(Cell) ? Endpoints[ToIndex(Cell)] : EFlowColor::None;
}

bool UFlowPuzzleState::AreAdjacent(FIntPoint A, FIntPoint B)
{
	// 曼哈顿距离为 1：上下左右相邻，不含斜向
	return FMath::Abs(A.X - B.X) + FMath::Abs(A.Y - B.Y) == 1;
}

void UFlowPuzzleState::RebuildOccupancy()
{
	Occupancy = Endpoints; // 端点永远属于自己的颜色
	for (const TPair<EFlowColor, FFlowPath>& Entry : Paths)
	{
		for (const FIntPoint& P : Entry.Value.Points)
		{
			Occupancy[ToIndex(P)] = Entry.Key;
		}
	}
}

TCHAR UFlowPuzzleState::ColorToChar(EFlowColor Color)
{
	switch (Color)
	{
	case EFlowColor::Red:    return TEXT('R');
	case EFlowColor::Green:  return TEXT('G');
	case EFlowColor::Blue:   return TEXT('B');
	case EFlowColor::Yellow: return TEXT('Y');
	case EFlowColor::Orange: return TEXT('O');
	default:                 return TEXT('?');
	}
}

void UFlowPuzzleState::DebugPrint() const
{
	// 端点用大写字母，线用小写字母，空格用 .
	for (int32 Y = 0; Y < GridSize; ++Y)
	{
		FString Line;
		for (int32 X = 0; X < GridSize; ++X)
		{
			const int32 Idx = ToIndex(FIntPoint(X, Y));
			const EFlowColor C = Occupancy[Idx];
			TCHAR Ch = TEXT('.');
			if (C != EFlowColor::None)
			{
				Ch = ColorToChar(C);
				if (Endpoints[Idx] == EFlowColor::None) Ch = FChar::ToLower(Ch);
			}
			Line.AppendChar(Ch);
			Line.AppendChar(TEXT(' '));
		}
		UE_LOG(LogTemp, Log, TEXT("[FlowState]   %s"), *Line);
	}
}