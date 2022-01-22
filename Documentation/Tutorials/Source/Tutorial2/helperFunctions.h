#include "agentproc.h"
#include "lwnet.h"
#include "interpy.h"

extern Actionary *actionary;
extern ActionTable actionTable; //Holds the mapping of actions to performance code

int doSpeak(iPAR *ipar){
	std::string subject = ipar->getAgent()->getObjectName();
	std::cout << "Hello, my name is " << subject << "." << std::endl;
return 1;
}

void setUpActionTable(){
	// Make sure that the Actionary was created
	if (actionary == NULL) {
		actionary=new Actionary();
		actionary->init("");
	}
	actionTable.addFunctions("Speak", &doSpeak); // Link the Speak action with the code to perform it.
}
