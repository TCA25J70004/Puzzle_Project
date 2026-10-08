// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameMessageSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnGameMessage, const FText& /*Message*/);

/**
 * 
 */
UCLASS()
class PUZZLE_PROJECT_API UGameMessageSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	//任何对象都可以调用这个函数来发布一条屏幕提示
	void PostMessage(const FText& Message)
	{
		OnGameMessage.Broadcast(Message);
	}

	//负责显示的一方（HUD）订阅这个委托
	FOnGameMessage OnGameMessage;
	
};
