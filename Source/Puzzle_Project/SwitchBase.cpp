// Fill out your copyright notice in the Description page of Project Settings.


#include "SwitchBase.h"

/*
// Sets default values
ASwitchBase::ASwitchBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASwitchBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASwitchBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
*/

void ASwitchBase::SetIsOn(bool bNewIsOn)
{
	if (bIsOn == bNewIsOn)
	{
		return;
	}

	bIsOn = bNewIsOn;
	OnStateChanged();
	OnSwitchToggled.Broadcast(this, bIsOn);
}

