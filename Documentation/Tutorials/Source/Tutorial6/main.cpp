#include "helperFunctions.h"

extern Actionary *actionary;  // global pointer to the actionary
parTime *partime;	// global pointer to PAR time class (So we can manipulate time)
char* actionLocation=strdup("../../../../PAR/actions/");//Path to par(or action)folder


int main(void){
	partime = new parTime();	// setup the timing info for the simulation
	partime->setTimeOffset(8,30,30); // hours, minutes, seconds from midnight
	partime->setTimeRate(1);		 // how fast should time change
	setUpActionTable();//Builds the action table

	// Builds a new agent and gives him HumanAction capabilities
	AgentProc* agent=new AgentProc("Agent_0");
	agent->setCapability("HumanAction");
	//agent->getObject()->updateProperty("obj_posture", "SIT");

	// Give the agent a position
	Vector<3> *vec=new Vector<3>();
	vec->v[0]=-1.0f;
	vec->v[1]=-1.0f;
	vec->v[2]=0.0f;
	agent->getObject()->setPosition(vec);

	// Create an object and give it a position
	MetaObject *object=new MetaObject("Sink_0",false);
	vec->v[0]=1.0f;
	vec->v[1]=1.0f;
	object->setPosition(vec);

	//This gives the ipar constuctor with objects
	char* objs[]={strdup(object->getObjectName().c_str())};
	iPAR* iparTest = new iPAR("Walk", agent->getName(),objs);
	iparTest->setDuration(1);
	iparTest->setStartTime(partime->getCurrentTime());
	iparTest->setPriority(1);

	agent->addAction(iparTest);

	int error=0;

	for(int i=0; i<500; i++){
			LWNetList::advance(&error);
		}

	system("PAUSE");

}


