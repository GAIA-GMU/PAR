// Fill out your copyright notice in the Description page of Project Settings.

#include "Tutorial9.h"
#include "PARGameMode.h"
#include "Humanoid.h"
#include "lwnet.h"

extern parTime *partime;
extern Actionary *actionary;
extern TArray<AHumanoid*> all_agents;
extern TArray<UAnimMontage*> all_montages;
extern ActionTable actionTable; //Holds the mapping of actions to real world code


int doNod(iPAR *ipar){
	MetaObject * agent=ipar->getAgent();
	AHumanoid * hum = NULL;
	for (auto& ag : all_agents){
		if (agent == ag->GetPARAgent()->getObject()){
			hum = ag;
		}
	}
	if (hum != NULL){
		hum->AddAction(all_montages[0]);
	}
	return 1;
}

APARGameMode::APARGameMode(){
	error = 0;
	static ConstructorHelpers::FObjectFinder<UAnimMontage> TestMontage(TEXT("AnimMontage'/Game/Actions/TestMontage'"));
	all_montages.Add(TestMontage.Object);
}
// Called when the game starts or when spawned
void
APARGameMode::PreInitializeComponents(){
	Super::PreInitializeComponents();
	partime = new parTime();
	partime->setTimeOffset(8,30,0);
	partime->setTimeRate(1);			// how fast should time change
	actionary = new Actionary();
	actionary->init();
	//Finally, we add all of our actions to the action table
	actionTable.addFunctions("Nod", &doNod);
	
}

// Called every frame
void
APARGameMode::Tick(float DeltaSeconds){
	Super::Tick(DeltaSeconds);
	//partime->setTimeOffset(partime->getCurrentTime() + DeltaSeconds);
	int error = 0;
	LWNetList::advance(&error);
}


