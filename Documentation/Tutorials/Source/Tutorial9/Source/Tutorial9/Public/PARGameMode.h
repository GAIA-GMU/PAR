// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameMode.h"
#include "PARGameMode.generated.h"

/**
 * 
 */
UCLASS()
class TUTORIAL9_API APARGameMode : public AGameMode
{
	GENERATED_BODY()
	
	APARGameMode();//Initalizes the game mode. Mostly used to grab references to animation montages

	// Called when the game starts or when spawned
	virtual void PreInitializeComponents() override;

	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

private:
	int error;
	
	
};
