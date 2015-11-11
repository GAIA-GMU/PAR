// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "GameFramework/Character.h"
#include "Humanoid.generated.h"

UCLASS()
class TUTORIAL9_API AHumanoid : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AHumanoid();
	~AHumanoid();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	//Used as a get function for the par agent
	AgentProc* GetPARAgent(){ return par_agent; }

	/*! Adds an action to the current actors animation */
	void AddAction(UAnimMontage*); 

private:
	AgentProc *par_agent; /*! <A pointer to the par agent class, and what all par information goes through */
	USkeletalMeshComponent *mesh; /*! <A pointer to the mesh. Useful as a shorthand for getting the animation*/
	int idle;/*! <The idle counter that we use to provide new animations*/
	
};
