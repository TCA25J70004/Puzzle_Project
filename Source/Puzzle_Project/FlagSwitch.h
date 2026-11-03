// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SwitchBase.h"
#include "FlagSwitch.generated.h"


UCLASS()
class PUZZLE_PROJECT_API AFlagSwitch : public ASwitchBase
{
	GENERATED_BODY()

public:
	AFlagSwitch();

	virtual void PostInitializeComponents() override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 状态完全由被监视的旗标决定，不需要从自己的旗标恢复
	virtual bool ShouldRestoreFromFlag() const override { return false; }

	/** 要监视的旗标名，例如 "Stage1.Note.C" */
	UPROPERTY(EditInstanceOnly, Category = "Flag Switch")
	FName WatchedFlagId;

private:
	void HandleFlagChanged(FName ChangedFlagId, bool bValue);



	
};
