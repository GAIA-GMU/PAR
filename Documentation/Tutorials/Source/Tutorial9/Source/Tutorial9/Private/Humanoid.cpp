// Fill out your copyright notice in the Description page of Project Settings.

#include "Tutorial9.h"
#include "Humanoid.h"

extern TArray<AHumanoid*> all_agents;
extern parTime *partime;

// Sets default values
AHumanoid::AHumanoid()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	par_agent = NULL;
	mesh = NULL;
	idle = 0;
}

AHumanoid::~AHumanoid(){
	delete par_agent;
}

// Called when the game starts or when spawned
void AHumanoid::BeginPlay()
{
	Super::BeginPlay();
	par_agent = new AgentProc(std::string(TCHAR_TO_ANSI(*this->GetName())).c_str());
	par_agent->setCapability("Nod");
	this->mesh = this->GetMesh();
	all_agents.Emplace(this);
}

// Called every frame
void AHumanoid::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
	//For now, we only care about the position and orientation of the character
	FVector trans = this->GetActorLocation();
	Vector<3> *vec = new Vector < 3 >();
	vec->v[0] = trans.X;
	vec->v[1] = trans.Y;
	vec->v[2] = trans.Z;
	this->par_agent->getObject()->setPosition(vec);
	trans = this->GetActorQuat().Euler();
	vec->v[0] = trans.X;
	vec->v[1] = trans.Y;
	vec->v[2] = trans.Z;
	this->par_agent->getObject()->setOrientation(vec);

	//This is our idle checker, also seen in Tutorial 4
	if (this->par_agent->emptyQueue() && !this->par_agent->activeAction()){
		idle++;
	}
	if (idle == 10){
		iPAR *par = new iPAR("Nod",this->par_agent->getName());
		par->setStartTime(partime->getCurrentTime());
		par->setDuration(2);
		par->setFinished(false);
		this->par_agent->addAction(par);
		idle = 0;
	}
}

//!This function starts playing an animation montage on our character, and is useful for displaying animation
/*!
	\param montage The montage we wish to play on the agent.
*/
void AHumanoid::AddAction(UAnimMontage *montage){
	if (mesh != NULL && montage != NULL){
		mesh->AnimScriptInstance->Montage_Play(montage, 1.0f);
	}
}

// Called to bind functionality to input
void AHumanoid::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

}

