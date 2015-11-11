// Fill out your copyright notice in the Description page of Project Settings.

#include "Tutorial9.h"
#include "Humanoid.h"

parTime *partime;
char *actionLocation = "D:/openpar-repo/PAR/actions/";
TArray<AHumanoid*> all_agents; /*! <This is the array that holds all of our PAR agents*/
TArray<UAnimMontage*> all_montages; /*! <This array holds all of the used montages connected to PAR*/
int PAR::dbg = 1;
FILE* PAR::file_name = fopen("D:/par.log","w");


IMPLEMENT_PRIMARY_GAME_MODULE( FDefaultGameModuleImpl, Tutorial9, "Tutorial9" );
