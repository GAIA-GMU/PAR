#include "agent.h"

extern Actionary *actionary;
extern ActionTable actionTable; //Holds the mapping of actions to performance code
extern GameApplication* app;

int doSpeak(iPAR *ipar){
	std::string subject = ipar->getAgent()->getObjectName();
	std::cout << "Hello, my name is " << subject << "." << std::endl;
	
	Agent* agent = app->findAgent(subject);
	if (agent == NULL) 
		return 0;

	agent->setBaseAnimation(agent->ANIM_DANCE);	// Start playing an OGRE animation
	agent->setTopAnimation(agent->ANIM_DANCE);
return 1;
}

void setUpActionTable(){
	// Make sure that the Actionary was created
	if (actionary == NULL) {
		actionary=new Actionary();
		actionary->init();
	}
	
	actionTable.addFunctions("Speak", &doSpeak); // Link the Speak action with the code to perform it.
}