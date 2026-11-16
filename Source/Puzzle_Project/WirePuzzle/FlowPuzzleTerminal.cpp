// Fill out your copyright notice in the Description page of Project Settings.


#include "WirePuzzle/FlowPuzzleTerminal.h"
#include "Blueprint/UserWidget.h"
#include "Components/StaticMeshComponent.h"

#include "FlowPuzzleState.h"
#include "FlowBoardWidget.h"

// Sets default values
AFlowPuzzleTerminal::AFlowPuzzleTerminal()
{
	PrimaryActorTick.bCanEverTick = false;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

}

void AFlowPuzzleTerminal::Interact(AActor* Interactor)
{
	UE_LOG(LogTemp, Log, TEXT("[FlowTerminal] Interact called by %s"), *GetNameSafe(Interactor));

	//if (bSolved) { UE_LOG(LogTemp, Log, TEXT("[FlowTerminal] Already solved"));          return; }
	if (IsOn()) { UE_LOG(LogTemp, Log, TEXT("[FlowTerminal] Already solved")); return; }
	if (ActiveWidget) { UE_LOG(LogTemp, Log, TEXT("[FlowTerminal] Widget already open"));     return; }
	if (!BoardWidgetClass) { UE_LOG(LogTemp, Warning, TEXT("[FlowTerminal] BoardWidgetClass not set")); return; }

	const FPuzzleLevel* Level = PuzzleRow.GetRow<FPuzzleLevel>(TEXT("FlowTerminal"));
	if (!Level) { UE_LOG(LogTemp, Warning, TEXT("[FlowTerminal] Row not found")); return; }

	APawn* Pawn = Cast<APawn>(Interactor);
	APlayerController* PC = Pawn ? Pawn->GetController<APlayerController>() : nullptr;
	if (!PC) { UE_LOG(LogTemp, Warning, TEXT("[FlowTerminal] No PlayerController from Interactor")); return; }

	ActiveWidget = CreateWidget<UFlowBoardWidget>(PC, BoardWidgetClass);
	if (!ActiveWidget || !ActiveWidget->InitPuzzle(*Level, this))
	{
		UE_LOG(LogTemp, Error, TEXT("[FlowTerminal] Failed to create or init board"));
		ActiveWidget = nullptr;
		return;
	}
	ActiveWidget->AddToViewport();
	ActivePC = PC;

	FInputModeUIOnly Mode;
	Mode.SetWidgetToFocus(ActiveWidget->TakeWidget());
	PC->SetInputMode(Mode);
	PC->SetShowMouseCursor(true);
	PC->SetIgnoreMoveInput(true);
	PC->SetIgnoreLookInput(true);
}

FText AFlowPuzzleTerminal::GetInteractPrompt() const
{
	/*return bSolved ? FText::GetEmpty()
		: NSLOCTEXT("Puzzle", "FlowPrompt", "调查配电盘");
		*/

	return IsOn() ? FText::GetEmpty()
		: NSLOCTEXT("Puzzle", "FlowPrompt", "调查配电盘");
}

void AFlowPuzzleTerminal::NotifySolved()
{
	/*
	if (bSolved) return;
	bSolved = true;
	ClosePuzzle();
	OnPuzzleSolved.Broadcast();
	*/

	if (IsOn()) return;

	ClosePuzzle();     // 先关闭 UI、恢复玩家操作
	SetIsOn(true);     // 再写入旗标并广播给门
}

void AFlowPuzzleTerminal::ClosePuzzle()
{
	if (ActiveWidget)
	{
		ActiveWidget->RemoveFromParent();
		ActiveWidget = nullptr;
	}
	if (ActivePC)
	{
		ActivePC->SetInputMode(FInputModeGameOnly());
		ActivePC->SetShowMouseCursor(false);
		ActivePC->ResetIgnoreMoveInput();
		ActivePC->ResetIgnoreLookInput();
		ActivePC = nullptr;
	}
}


