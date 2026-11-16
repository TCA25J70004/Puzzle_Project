
#pragma once

#include "FlowTypes.h"
#include "CoreMinimal.h"
//#include "GameFramework/Actor.h"
#include "SwitchBase.h"          // 替换 "GameFramework/Actor.h"
#include "Interactable.h"
#include "FlowPuzzleTerminal.generated.h"

//class UUserWidget;
class UFlowBoardWidget;   // 替换原来的 class UUserWidget;

//DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPuzzleSolved);

UCLASS()
class PUZZLE_PROJECT_API AFlowPuzzleTerminal : public ASwitchBase, public IInteractable   // 父类改为 ASwitchBase
{
	GENERATED_BODY()
	
public:	
	AFlowPuzzleTerminal();

	// IInteractable
	virtual void Interact(AActor* Interactor) override;
	virtual FText GetInteractPrompt() const override;

	// 由谜题 UI 调用：解开 / 中途退出
	void NotifySolved();
	void ClosePuzzle();

	// 删除：父类已经有 OnSwitchToggled
	/*
	UPROPERTY(BlueprintAssignable)
	FOnPuzzleSolved OnPuzzleSolved;
	*/

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent>Mesh;

	// 在编辑器里指定用哪个 Widget、哪一关
	UPROPERTY(EditAnywhere, Category = "Puzzle")
	TSubclassOf<UFlowBoardWidget> BoardWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Puzzle", meta = (RowType = "/Script/Puzzle_Project.PuzzleLevel"))
	FDataTableRowHandle PuzzleRow;

private:	
	UPROPERTY()
	TObjectPtr<UFlowBoardWidget> ActiveWidget;

	UPROPERTY()
	TObjectPtr<APlayerController> ActivePC;

	//删除：父类的 bIsOn 就是"已解开"
	//bool bSolved = false;

	

	

};
