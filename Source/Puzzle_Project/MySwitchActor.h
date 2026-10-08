// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Interactable.h"

#include "MySwitchActor.generated.h" // 必须是最后一个 include

class AMySwitchActor;
class UStaticMeshComponent;
class UMaterialInterface;

// 声明一种委托类型：广播时带上"哪个开关"和"当前是否打开"两个参数
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSwitchToggled, AMySwitchActor* /*Switch*/, bool /*bIsOn*/);

UCLASS()
class PUZZLE_PROJECT_API AMySwitchActor : public AActor , public IInteractable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMySwitchActor();

	// 实现 IInteractable 接口
	virtual void Interact(AActor* Interactor)override;

	bool IsOn() const { return bIsOn; }

	// 其他对象订阅这个委托来接收开关状态变化
	FOnSwitchToggled OnSwitchToggled;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Switch")
	TObjectPtr<UStaticMeshComponent> SwitchMesh;

	// 初始状态，可以在每个关卡实例上单独设置
	UPROPERTY(EditAnywhere, Category = "Switch")
	bool bIsOn = false;

	// 打开/关闭时使用的材质
	UPROPERTY(EditAnywhere, Category = "Switch|Visual")
	TObjectPtr<UMaterialInterface> OnMaterial;

	UPROPERTY(EditAnywhere, Category = "Switch|Visual")
	TObjectPtr<UMaterialInterface> OffMaterial;

private:
	void UpdateVisual();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
