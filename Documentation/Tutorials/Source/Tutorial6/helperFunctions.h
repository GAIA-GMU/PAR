#include "agentproc.h"
#include "lwnet.h"

extern Actionary *actionary;
extern ActionTable actionTable; // Holds the mapping of actions to real world code


bool step(MetaObject *agent, MetaObject *obj){
	Vector<3> *ag=agent->getPosition();
	Vector<3> *ob=obj->getPosition();

	bool ran=false;

	if(ag->v[0] != ob->v[0] && !ran){
		ag->v[0] = ob->v[0];
		ran = true;
	}
	if(ag->v[1] != ob->v[1] && !ran){
		ag->v[1] = ob->v[1];
		ran = true;
	}
	if(ag->v[2] != ob->v[2] && !ran){
		ag->v[2] = ob->v[2];
		ran = true;
	}
	agent->setPosition(ag);
	return ran;
}

int doWalk(iPAR *ipar){
	MetaObject *agent=ipar->getAgent();
	MetaObject *object=ipar->getObject(0);

	while(step(agent, object))
		printf("Stepping\n");

	return 1;
}

int doStand(iPAR *ipar){
	MetaObject * agent=ipar->getAgent();
	printf("Agent %s is standing up\n",agent->getObjectName().c_str());
	ipar->setFinished(true);
	return 1;
}
void setUpActionTable(){

	//Creates an actionary
	if (actionary == NULL){
		actionary=new Actionary();
		actionary->init("");
	}
	//Set up the action stuff
	actionTable.addFunctions("Walk",&doWalk);
	actionTable.addFunctions("Stand",&doStand);

}