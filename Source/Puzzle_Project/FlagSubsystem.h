// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FlagSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnFlagChanged, FName /*FlagId*/, bool /*bValue*/);

/**
 * 
 */
UCLASS()
class PUZZLE_PROJECT_API UFlagSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//设置旗标；值没有变化时什么都不做
	void SetFlag(FName FlagId, bool bValue);

	//读取旗标；不存在时返回 bDefault
	bool GetFlag(FName FlagId, bool bDefault = false) const;

	//这个旗标是否被记录过
	bool HasFlag(FName FlagId) const;

	//清空所有旗标（开始新游戏时使用）
	void ClearAllFlags();

	//获取全部旗标，下次作业做存档时会用到 
	const TMap<FName, bool>& GetAllFlags() const { return Flags; }

	//把所有旗标打印到日志，用于调试 
	void DumpFlags() const;

	FOnFlagChanged OnFlagChanged;

private:
	UPROPERTY()
	TMap<FName, bool> Flags;
	
};
