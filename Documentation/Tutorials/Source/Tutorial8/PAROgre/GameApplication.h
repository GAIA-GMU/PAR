#ifndef __GameApplication_h_
#define __GameApplication_h_

#include "BaseApplication.h"
#include "Agent.h"

class Agent;

class GameApplication : public BaseApplication
{
private:
	std::list<Agent*> agents; // store a pointer to the character

public:
    GameApplication(void);
    virtual ~GameApplication(void);

	void loadEnv();			// Load the buildings or ground plane, etc.
	void setupEnv();		// Set up the lights, shadows, etc
	void loadObjects();		// Load other props or objects (e.g. furniture)
	void loadCharacters();	// Load actors, agents, characters
	Agent* findAgent(std::string name);		// return a pointer to the agent

	void addTime(Ogre::Real deltaTime);		// update the game state

protected:
    virtual void createScene(void);
};

#endif // #ifndef __TutorialApplication_h_
