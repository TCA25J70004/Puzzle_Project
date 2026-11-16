#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "FlowTypes.h"
#include "FlowPuzzleState.generated.h"

/*
连线谜题的纯逻辑。不知道 UI 的存在，只处理格子和路径。
坐标约定：X = 列（向右），Y = 行（向下）
 */
UCLASS()
class PUZZLE_PROJECT_API UFlowPuzzleState : public UObject
{
	GENERATED_BODY()

public:
	// 用关卡数据初始化；数据有问题时返回 false 并打印原因
	bool Init(const FPuzzleLevel& Level);

	// 输入操作：按下 / 拖动 / 松开
	bool BeginPath(FIntPoint Cell);
	bool ExtendPath(FIntPoint Cell);
	void EndPath();

	// 判定
	bool IsColorConnected(EFlowColor Color) const;
	bool IsSolved() const;

	// 查询（给 Widget 绘制用）
	int32 GetGridSize() const { return GridSize; }
	bool IsInBounds(FIntPoint Cell) const;
	EFlowColor GetCellColor(FIntPoint Cell) const;
	EFlowColor GetEndpointColor(FIntPoint Cell) const;
	const FFlowPath* FindPath(EFlowColor Color) const { return Paths.Find(Color); }
	const TMap<EFlowColor, FFlowPath>& GetPaths() const { return Paths; }   // ← 新增
	EFlowColor GetActiveColor() const { return ActiveColor; }



	// 调试：把棋盘画到 Output Log
	void DebugPrint() const;

private:
	int32 ToIndex(FIntPoint Cell) const { return Cell.Y * GridSize + Cell.X; }
	static bool AreAdjacent(FIntPoint A, FIntPoint B);
	static TCHAR ColorToChar(EFlowColor Color);
	void RebuildOccupancy();

	UPROPERTY()
	int32 GridSize = 0;

	// 静态数据：每格的端点颜色（不是端点则为 None），Init 后不再改变
	UPROPERTY()
	TArray<EFlowColor> Endpoints;

	// 唯一的真实状态：每种颜色当前画出的路径
	UPROPERTY()
	TMap<EFlowColor, FFlowPath> Paths;

	// 派生数据：每格当前被哪种颜色占用，由 RebuildOccupancy 从 Paths 算出
	UPROPERTY()
	TArray<EFlowColor> Occupancy;

	// 每种颜色的端点对，用于判定是否连通
	UPROPERTY()
	TMap<EFlowColor, FFlowPair> PairsByColor;

	// 正在拖动的颜色；没有拖动时为 None
	EFlowColor ActiveColor = EFlowColor::None;
};