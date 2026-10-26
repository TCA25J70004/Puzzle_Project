// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
//#include "GameFramework/Actor.h"
//SwitchBase.h 里已经包含了它。

#include "SwitchBase.h" //新增
#include "Interactable.h"

#include "MySwitchActor.generated.h" // 必须是最后一个 include

//class AMySwitchActor;
class UStaticMeshComponent;
class UMaterialInterface;


// 声明一种委托类型：广播时带上"哪个开关"和"当前是否打开"两个参数
//DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSwitchToggled, AMySwitchActor* /*Switch*/, bool /*bIsOn*/);
// 删除,已移到 SwitchBase.h

UCLASS()
class PUZZLE_PROJECT_API AMySwitchActor : public ASwitchBase, public IInteractable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMySwitchActor();

	// 实现 IInteractable 接口
	virtual void Interact(AActor* Interactor)override;

	/*删除IsOn()、OnSwitchToggled,已移到基类
	bool IsOn() const { return bIsOn; }

	// 其他对象订阅这个委托来接收开关状态变化
	FOnSwitchToggled OnSwitchToggled;
	*/

protected:
	virtual void BeginPlay() override;
	virtual void OnStateChanged() override;

	UPROPERTY(VisibleAnywhere, Category = "Switch")
	TObjectPtr<UStaticMeshComponent> SwitchMesh;

	/* 删除 bIsOn，已移到基类
	// 初始状态，可以在每个关卡实例上单独设置
	UPROPERTY(EditAnywhere, Category = "Switch")
	bool bIsOn = false;
	*/

	// 打开/关闭时使用的材质
	UPROPERTY(EditAnywhere, Category = "Switch|Visual")
	TObjectPtr<UMaterialInterface> OnMaterial;

	UPROPERTY(EditAnywhere, Category = "Switch|Visual")
	TObjectPtr<UMaterialInterface> OffMaterial;

private:
	void UpdateVisual();

public:	
	// Called every frame
	//virtual void Tick(float DeltaTime) override;

};
