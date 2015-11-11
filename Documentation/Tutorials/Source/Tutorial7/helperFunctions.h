#include "workingmemory.h"
#include "par.h"
#include "agentproc.h"
#include "actionary.h"
#include "lwnet.h"

extern Actionary *actionary;
extern ActionTable actionTable; //Holds the mapping of actions to real world code

//This is a simple motion planner that assumes a grid-like
//surface
bool step(MetaObject *agent, MetaObject *obj){
	Vector<3> *ag=agent->getPosition();
	Vector<3> *ob=obj->getPosition();

	bool ran=false;

	if(ag->v[0] != ob->v[0] && !ran){
		ag->v[0]=ob->v[0];
		ran=true;
	}
	if(ag->v[1] != ob->v[1] && !ran){
		ag->v[1]=ob->v[1];
		ran=true;
	}
	if(ag->v[2] != ob->v[2] && !ran){
		ag->v[2]=ob->v[2];
		ran=true;
	}
	agent->setPosition(ag);
	return ran;
}

int doWalk(iPAR *ipar){
	MetaObject *agent=ipar->getAgent();
	MetaObject *object=ipar->getObject(0);
	printf("walking:%s,%s\n",agent->getObjectName().c_str(),object->getObjectName().c_str());
	while(step(agent,object))
		par_debug("Stepping\n");

	return 1;
}

//Gives a PickUp Function
int doPickUp(iPAR *ipar){
	par_debug("Inside PickUp\n");
	ipar->setFinished(true);
	return 1;
}
int doCook(iPAR *ipar){
	par_debug("Inside Cook\n");
	ipar->setFinished(true);
	return 1;
}
int doCarry(iPAR *ipar){
	par_debug("Carrying\n");
	ipar->setFinished(true);
	return 1;

}
int doSpeak(iPAR *ipar){
	printf("Speaking from iPAR_%d\n",ipar->getID());
	ipar->setFinished(true);
	return 1;
}

void setUpActionTable(){
	if (actionary == NULL) {
		actionary=new Actionary();
		actionary->init();
	}
	actionTable.addFunctions("Walk",&doWalk);
	actionTable.addFunctions("PickUp",&doPickUp);
	actionTable.addFunctions("Cook",&doCook);
	actionTable.addFunctions("Carry",&doCarry);
	actionTable.addFunctions("Speak",&doSpeak);
}
