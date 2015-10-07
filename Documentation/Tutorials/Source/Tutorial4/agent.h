#include "agentproc.h"

// Creating a subclass of AgentProc to customize behaviors
class Agent : public AgentProc {

	public:
		Agent(char* agentName);	// Customized constructor
		int update(void *val);	// Basic update method
		void addNewAction(iPAR *action);	// We extend the addAction method to update agent state
	private:
		void addIdleAction();	// A method for initiating an idle behavior
		bool createIntentFromDesire(const char* desire);//If possible,creates an action from a given desire
		bool idle;				// Is the agent idle?
		int idleCount;			// Start an idle action only after a period of time
};
