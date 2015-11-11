#include "agentproc.h"
#include <iostream>

extern Actionary *actionary;	// global pointer to the actionary
parTime *partime;				// global pointer to PAR time class (So we can manipulate time)
char* actionLocation = strdup("../../../../PAR/actions/"); // Relative path to par (or action) folder
int PAR::dbg = 1; /*< Used to see all the debug information in the code */
FILE* PAR::file_name = fopen("openPAR.log", "w"); /*! <The output stream of all debug information */


int main(void){
	partime = new parTime();	// setup the timing info for the simulation
	partime->setTimeOffset(8,30,30); // hours, minutes, seconds from midnight
	partime->setTimeRate(1);		 // how fast should time change

	// Create an Actionary
	actionary=new Actionary();
	actionary->init();

	MetaObject* object = new MetaObject("Sink_0"); // Creates a single Sink object

	// Give the object a position of 8,1,2
	Vector<3> *pos=new Vector<3>();
	pos->v[0]=8.0f;
	pos->v[1]=1.0f;
	pos->v[2]=2.0f;
	object->setPosition(pos);

	// Check to see what actions are associated with a sink
	std::cout << "The sink can be cleaned (true or false)? " << object->searchAffordance(actionary->searchByNameAct("Clean"),0) << std::endl;
	std::cout << "The sink can be eaten (true or false)? " << object->searchAffordance(actionary->searchByNameAct("Eat"),0) << std::endl;

	//Allows the object to clean
	std::cout << "The sink can be cleaned (true or false)? " << object->searchAffordance(actionary->searchByNameAct("Clean"),1) << std::endl;
	actionary->addAffordance(actionary->searchByNameAct("Clean"),object,1);
	std::cout << "The sink can be cleaned (true or false)? " << object->searchAffordance(actionary->searchByNameAct("Clean"),1) << std::endl;

	//Creates a new object, and adds the sink to it's contents
	MetaObject* container= new MetaObject("Bookshelf_large");
	container->addContents(object);

	//Checks to see if the object is in the contents of the container (physcially) and the possession (mentally)
	std::cout<<"The sink is physically in the bookshelf(true or false)? "<<container->searchContents("Sink_0")<<std::endl;
	std::cout<<"The sink is owned by the bookshelf(true or false)? "<<container->searchPossession("Sink_0")<<std::endl;

	system("PAUSE");
}


