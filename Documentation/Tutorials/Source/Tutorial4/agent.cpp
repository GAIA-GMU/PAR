#include "agent.h"

extern parTime *partime;	// global pointer to PAR time class 
extern Actionary *actionary;
std::list<MetaObject*> all_objs;

Agent::Agent(char *name):AgentProc(name){
	idle = true;	// I'm not doing anything right now, so of course the agent is idle
	idleCount = 0;
}

// Called at every time step 
int Agent::update(void* val){
	if(!activeAction())		// Are there any PAR actions being processed or performed?
		idleCount++;		// Wait a few frames before starting an idle behavior
	
	if(idleCount > 5)			
		idle = true;
	if(idle)				// Start the agent's idle behavior
		{
			std::cout << "Starting idle behavior" << std::endl;
			addIdleAction();
			idleCount = 0;
		}
	return 0;
}

// Start processing an action and also set the idle flag
void Agent::addNewAction(iPAR* ipar){
	addAction(ipar);
	idle = false;
}

// When this agent has nothing else to do, it speaks.
void Agent::addIdleAction(){
	iPAR* iparTest = new iPAR("SpeakOne", this->getName());
	iparTest->setPriority(3);
	iparTest->setStartTime(partime->getCurrentTime());
	iparTest->setDuration(2);
	this->addNewAction(iparTest);
}

//This provides an action (if possible) to an agent
bool Agent::createIntentFromDesire(const char* desire){
	if (desire == NULL)
			return false;
	//First, query the desire
	std::vector<MetaAction*> possible_acts=actionary->getAllPurposed(desire);
	if(possible_acts.empty())
		return false;

	//Second, query the Belief
	MetaAction *act=NULL;
	int counter=0;
	for(std::vector<MetaAction*>::const_iterator it=possible_acts.begin(); it != possible_acts.end(); it++){
		int num_objs=(*it)->getNumObjects();
		if(num_objs < 0){
			if(act == NULL)
				act=(*it);
		}
		else{
			bool satisfied=true;
			for(int i=0; i<num_objs; i++){
				bool found=false;
				for(std::list<MetaObject*>::const_iterator o_it=all_objs.begin(); o_it != all_objs.end(); o_it++){
					if(actionary->searchAffordance((*it),(*o_it))==i)
						found=true;
				}
				if(!found)
					satisfied=false;
			}
			if(satisfied){
				act=(*it);
			}
		}
	}
	if(act == NULL)
		return false;
	//Finally, start the intent
	iPAR *ipar=new iPAR(act->getActionName(),this->getObject()->getObjectName());
	for(int i=0; i<act->getNumObjects(); i++){
		//We can always cache the objects that we have found if desired. However, there really is no need
		for(std::list<MetaObject*>::const_iterator o_it=all_objs.begin(); o_it != all_objs.end(); o_it++){
			if(actionary->searchAffordance(act,(*o_it))==i){
				ipar->setObject((*o_it),i);
				break;
			}
		}
	}
	ipar->setPriority(5);
	ipar->setStartTime(partime->getCurrentTime());
	ipar->setDuration(5);
	this->addAction(ipar);
	return true;
}
