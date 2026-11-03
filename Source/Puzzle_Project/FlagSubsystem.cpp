// Fill out your copyright notice in the Description page of Project Settings.


#include "FlagSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"

void UFlagSubsystem::SetFlag(FName FlagId, bool bValue)
{
	if (FlagId.IsNone())
	{
		return;
	}

	if (const bool* Existing = Flags.Find(FlagId); Existing && *Existing == bValue)
	{
		return;
	}

	Flags.Add(FlagId, bValue);   // 不存在就新增，存在就覆盖
	UE_LOG(LogTemp, Log, TEXT("[Flag] %s = %s"), *FlagId.ToString(), bValue ? TEXT("true") : TEXT("false"));

	OnFlagChanged.Broadcast(FlagId, bValue);
}

bool UFlagSubsystem::GetFlag(FName FlagId, bool bDefault) const
{
	const bool* Value = Flags.Find(FlagId);
	return Value ? *Value : bDefault;
}

bool UFlagSubsystem::HasFlag(FName FlagId) const
{
	return Flags.Contains(FlagId);
}

void UFlagSubsystem::ClearAllFlags()
{
	Flags.Reset();
}

void UFlagSubsystem::DumpFlags() const
{
	UE_LOG(LogTemp, Log, TEXT("===== Flags (%d) ====="), Flags.Num());
	for (const TPair<FName, bool>& Pair : Flags)
	{
		UE_LOG(LogTemp, Log, TEXT("  %s = %s"), *Pair.Key.ToString(), Pair.Value ? TEXT("true") : TEXT("false"));
	}
}

// 控制台命令：游戏运行时按 ~ 键打开控制台，输入 Flags.Dump
static FAutoConsoleCommandWithWorld GDumpFlagsCommand(
	TEXT("Flags.Dump"),
	TEXT("Print all game flags to the log."),
	FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
		{
			const UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
			if (const UFlagSubsystem* FlagSubsystem = GameInstance ? GameInstance->GetSubsystem<UFlagSubsystem>() : nullptr)
			{
				FlagSubsystem->DumpFlags();
			}
		}));