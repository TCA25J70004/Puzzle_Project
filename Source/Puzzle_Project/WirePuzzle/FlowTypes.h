// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "FlowTypes.generated.h"


UENUM(BlueprintType)
enum  class EFlowColor:uint8
{
	None UMETA(DisplayName = "None"), // 空格子
	Red    UMETA(DisplayName = "Red"),
	Green  UMETA(DisplayName = "Green"),
	Blue   UMETA(DisplayName = "Blue"),
	Yellow UMETA(DisplayName = "Yellow"),
	Orange UMETA(DisplayName = "Orange"),
	//改成和旧 E_FlowColor 一样的颜色列表

};

USTRUCT(BlueprintType)
struct FFlowPair
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EFlowColor Color = EFlowColor::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FIntPoint PointA = FIntPoint::ZeroValue;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FIntPoint PointB = FIntPoint::ZeroValue;
};

// 运行时状态：一种颜色当前画出的路径（按顺序）
USTRUCT(BlueprintType)
struct FFlowPath
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FIntPoint> Points;
};

// DataTable 的行结构：一行就是一关
USTRUCT(BlueprintType)
struct FPuzzleLevel : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "2"))
	int32 GridSize = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FFlowPair> Pairs;
};

