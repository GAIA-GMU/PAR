#include "agentproc.h"
#include "lwnet.h"

extern Actionary *actionary;
extern ActionTable actionTable; // Holds the mapping of PAR actions to code

int doSpeak(iPAR *ipar){
	std::string subject=ipar->getAgent()->getObjectName();
	printf("Hello World. My name is %s\n", subject.c_str());
	//ipar->setFinished(true);
return 1;
}

void setUpActionTable(){
	if (actionary == NULL) {
		actionary = new Actionary();
		actionary->init();
	}
	actionTable.addFunctions("Speak", &doSpeak);
}
