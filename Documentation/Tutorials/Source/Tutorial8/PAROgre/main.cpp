#include "GameApplication.h"
#include "interpy.h"
#include "PARactions.h"

GameApplication* app;
extern Actionary *actionary;	// global pointer to the actionary (found in agentproc.cpp)
parTime *partime;				// global pointer to PAR time class (So we can manipulate time)
char* actionLocation;			// Relative path to par (or action) folder


int main(int argc, char *argv[])
{

	std::string path = __FILE__;	//gets the current cpp file's path with the cpp file
	path = path.substr(0,1+path.find_last_of('\\')); //removes filename to leave path
	path+= "../../../../../PAR/actions/";	//if txt file is in the same directory as cpp file
	actionLocation = strdup(path.c_str());

	partime = new parTime();			// setup the timing info for the simulation
	partime->setTimeOffset(8,30,30);	// hours, minutes, seconds from midnight (e.g. 8:30:30am)
	partime->setTimeRate(1);			// how fast should time change

	// Creates an actionary
	actionary = new Actionary();
	actionary->init();
	initprop();				// Initialize the Python propositions
	setUpActionTable();		// Builds the action table

	// Create application object
    app = new GameApplication();
	app->go();


    return 0;
}

