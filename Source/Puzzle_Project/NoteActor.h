// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "NoteActor.generated.h"

class UStaticMeshComponent;

UCLASS()
class PUZZLE_PROJECT_API ANoteActor : public AActor, public IInteractable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANoteActor();

	virtual void Interact(AActor* Interactor) override;
	//virtual FText GetInteractPrompt() const override { return InteractPrompt; }
	virtual FText GetInteractPrompt() const override;   // 实现移到cpp了


protected:
	UPROPERTY(VisibleAnywhere, Category = "Note")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditAnywhere, Category = "Note")
	FText Title;

	//正文；MultiLine 让编辑器中的输入框支持换行Shift+Enter
	UPROPERTY(EditAnywhere, Category = "Note", meta = (MultiLine = true))
	FText Body;

	UPROPERTY(EditAnywhere, Category = "Note")
	FText InteractPrompt;

	//新增，旗标系统所需
	//读过之后显示的提示
	UPROPERTY(EditAnywhere, Category = "Note")
	FText ReadAgainPrompt;

	//"已读"旗标名，例如 "Stage1.Note.C"；留空则不记录 */
	UPROPERTY(EditInstanceOnly, Category = "Note|Flag")
	FName ReadFlagId;

private:
	bool HasBeenRead()const;

};
