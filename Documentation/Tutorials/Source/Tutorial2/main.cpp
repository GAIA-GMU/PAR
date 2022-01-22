#include "helperFunctions.h"

extern Actionary *actionary;  // global pointer to the actionary
AgentProc* agent; // global pointer to the agent
parTime *partime;	// global pointer to PAR time class (So we can manipulate time)
char* actionLocation = strdup("../../../../../PAR/actions/"); // Relative path to par (or action) folder
int PAR::dbg = 1; /*< Used to see all the debug information in the code */
FILE* PAR::file_name = stdout; /*! <The output stream of all debug information */

int main(void){
	partime = new parTime();			// setup the timing info for the simulation
	partime->setTimeOffset(0);	// hours, minutes, seconds from midnight
	partime->setTimeRate(1);			// how fast should time change

	setUpActionTable();		// Builds the action table

	//Builds a new agent and gives him HumanAction capabilities
	agent = new AgentProc("Agent_0");
	agent->setCapability("Speak");

	MetaObject * obj = new MetaObject("Cup_1");
	Vector<3> * vec = new Vector<3>;
	vec->v[0] = 0;
	vec->v[1] = 0;
	vec->v[2] = 0;
	obj->setPosition(vec);

	// Creates an iPAR
	iPAR* iparTest = new iPAR("Speak", agent->getName());
	iparTest->setObject(obj, 0);
	if (iparTest == NULL)
	{
		printf("ERROR: iPAR could not be created.");
		return 0;
	}

	iparTest->setDuration(2);	// This should run for 10 seconds
	iparTest->setStartTime(partime->getCurrentTime()); // Set the start time for now
	iparTest->setPriority(1);

	// Add this iPAR to the agent's action queue for processing and execution
	agent->addAction(iparTest);

	// Advance the PaTNets so that the action gets popped from the queue, processes, and executed.
	int error = 0;
	for(int i=0; i<100000; i++)
		LWNetList::advance(&error);

	system("PAUSE");

}


