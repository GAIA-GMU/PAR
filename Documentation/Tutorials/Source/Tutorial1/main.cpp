#include "agentproc.h"
#include "actionary.h"

extern Actionary *actionary;  // global pointer to the actionary (found in agentproc.cpp)
AgentProc* agent; // global pointer to an agent
parTime *partime;	// global pointer to PAR time class (So we can manipulate time)
char* actionLocation=strdup("../../../../PAR/actions/"); // Relative path to par (or action) folder


int main(void){
	partime = new parTime();			// setup the timing info for the simulation
	partime->setTimeOffset(8,30,30);	// hours, minutes, seconds from midnight (e.g. 8:30:30am)
	partime->setTimeRate(1);			// how fast should time change

	// Creates an actionary
	actionary = new Actionary();
	actionary->init();

	//creates a new agent process
	agent = new AgentProc("Agent_0");
	agent->setCapability("HumanAction");

	system("PAUSE"); // just holds the output window
}


