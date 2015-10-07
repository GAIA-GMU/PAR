#include "helperFunctions.h"
#include "agent.h"
#include <list>

parTime *partime;	// global pointer to PAR time class (So we can manipulate time)
char* actionLocation=strdup("../../../../PAR/actions/");//Path to par(or action)folder


int main(void){

	partime = new parTime();			// setup the timing info for the simulation
	partime->setTimeOffset(8,30,30);	// hours, minutes, seconds from midnight
	partime->setTimeRate(1);			// how fast should time change
	setUpActionTable();					// Builds the action table

	// Builds a new agent and gives him HumanAction capabilities
	char agent_name[10];
	std::list<Agent*> agents;
	for(int k=0; k<1; k++){
		sprintf(agent_name,"Agent_%d",k);
		Agent* agent = new Agent(agent_name);
		agent->setCapability("Speak");
		agents.push_back(agent);
	}
	int error=0;
	//system("PAUSE");
	for(int i=0; i<100000; i++){ // Simulating time steps
		for(std::list<Agent*>::iterator it=agents.begin(); it!=agents.end(); it++)
			(*it)->update(0);
		LWNetList::advance(&error);
	}

	system("PAUSE");
}


