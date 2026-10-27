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
	virtual FText GetInteractPrompt() const override { return InteractPrompt; }

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

};
