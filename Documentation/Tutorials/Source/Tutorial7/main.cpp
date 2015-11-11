#include "helperFunctions.h"


extern Actionary *actionary;	// global pointer to the actionary
parTime *partime;				// global pointer to PAR time class (So we can manipulate time)
char* actionLocation = strdup("../../../../PAR/actions/"); // Relative path to par (or action) folder
int PAR::dbg = 1; /*< Used to see all the debug information in the code */
FILE* PAR::file_name = fopen("openPAR.log", "w"); /*! <The output stream of all debug information */


int main(void){
	partime = new parTime();			// setup the timing info for the simulation
	partime->setTimeOffset(8,30,30);	// hours, minutes, seconds from midnight
	partime->setTimeRate(1);			// how fast should time change
	setUpActionTable();					// Builds the action table
	// Builds a new agent and gives him HumanAction capabilities
	AgentProc* agent=new AgentProc("Agent_0");
	agent->setCapability("HumanAction");
	Vector<3>* vec= new Vector<3>();
	vec->v[0]=0.0f;
	vec->v[1]=0.0f;
	vec->v[2]=0.0f;
	agent->getObject()->setPosition(vec);

	//Builds two new Objects (an oven and food)
	MetaObject* ob1=new MetaObject("Food_0",false);
	vec->v[0]=-3;
	ob1->setPosition(vec);
	MetaObject* ob2=new MetaObject("Oven_0",false);
	vec->v[0]=3;
	vec->v[1]=3;
	vec->v[2]=3;
	ob2->setPosition(vec);

	// Creates an IPAR
	char* objects[]= {strdup(ob1->getObjectName().c_str()),strdup(ob2->getObjectName().c_str())};
	//Cook is used here as it uses a complex action as a primitive
	//to show that a complex action can be used as a primitive
	try{
		iPAR* iparTest = new iPAR("Cook",agent->getName(),objects);
		iparTest->setFinished(false);
		iparTest->setDuration(10);// This should run for 10 seconds
		iparTest->setStartTime(partime->getCurrentTime());
		iparTest->setPriority(1);
		int error=0;
		agent->addAction(iparTest);
		for(int i=0; i<100000; i++){
			LWNetList::advance(&error);      
		}
	}
	catch(iPARException &e){
		par_debug("%s\n",e.printMsg());
	}

	system("PAUSE");
	return 0;
}


