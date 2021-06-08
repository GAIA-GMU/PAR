#include "agentproc.h"
#include "actionary.h"
#include <fstream>

extern Actionary *actionary;  // global pointer to the actionary (found in agentproc.cpp)
AgentProc* agent; // global pointer to an agent
parTime *partime;	// global pointer to PAR time class (So we can manipulate time)
char* actionLocation=strdup("../../../../actions/"); // Relative path to par (or action) folder
int PAR::dbg = 1; /*< Used to see all the debug information in the code */
FILE* PAR::file_name = stdout; /*! <The output stream of all debug information */
AgentTable agentTable; //This became a new dependency and the wiki should be updated to reflect that
int main(void){
	//FILE* PAR::file_name = fopen("test.txt", "w");
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


