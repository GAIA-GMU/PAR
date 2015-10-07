#include "GameApplication.h"
#include "lwnet.h"

extern Actionary *actionary;

//-------------------------------------------------------------------------------------
GameApplication::GameApplication(void)
{
}
//-------------------------------------------------------------------------------------
GameApplication::~GameApplication(void)
{
}

//-------------------------------------------------------------------------------------
void GameApplication::createScene(void)
{
    loadEnv();
	setupEnv();
	loadObjects();
	loadCharacters();
}

void // Load the buildings or ground plane, etc
GameApplication::loadEnv()
{
	using namespace Ogre;

	//create a floor mesh resource
	MeshManager::getSingleton().createPlane("floor", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
		Plane(Vector3::UNIT_Y, 0), 100, 100, 10, 10, true, 1, 10, 10, Vector3::UNIT_Z);

	//create a floor entity, give it material, and place it at the origin
	Entity* floor = mSceneMgr->createEntity("Floor", "floor");
	floor->setMaterialName("Examples/Rockwall");
	floor->setCastShadows(false);
	mSceneMgr->getRootSceneNode()->attachObject(floor);
}

void // Set up lights, shadows, etc
GameApplication::setupEnv()
{
	using namespace Ogre;

	// set shadow properties
	mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_MODULATIVE);
	mSceneMgr->setShadowColour(ColourValue(0.5, 0.5, 0.5));
	mSceneMgr->setShadowTextureSize(1024);
	mSceneMgr->setShadowTextureCount(1);

	// disable default camera control so the character can do its own 
	mCameraMan->setStyle(OgreBites::CS_FREELOOK); // CS_FREELOOK, CS_ORBIT, CS_MANUAL

	// use small amount of ambient lighting
	mSceneMgr->setAmbientLight(ColourValue(0.3f, 0.3f, 0.3f));

	// add a bright light above the scene
	Light* light = mSceneMgr->createLight();
	light->setType(Light::LT_POINT);
	light->setPosition(-10, 40, 20);
	light->setSpecularColour(ColourValue::White);
}

void // Load other props or objects
GameApplication::loadObjects()
{
	using namespace Ogre;

	SceneNode* mNode = app->mSceneMgr->getRootSceneNode()->createChildSceneNode();
	Entity* mEntity = app->mSceneMgr->createEntity("Sink_0", "knot.mesh");
	mNode->attachObject(mEntity);

	mNode->scale(0.02, 0.02, 0.02);
	mNode->setPosition(8, 5, 8);

	// set up PAR representation of the object
	MetaObject* obj = new MetaObject("Sink_0");
	Vector<3> *pos=new Vector<3>();
	pos->v[0]=8.0f;
	pos->v[1]=5.0f;
	pos->v[2]=8.0f;
	obj->setPosition(pos);
}

void // Load actors, agents, characters
GameApplication::loadCharacters()
{
	Agent* agent = new Agent("Sinbad_0", "Sinbad.mesh");
	agents.push_back(agent);
	
}

Agent* 
GameApplication::findAgent(std::string name)	// return a pointer to the agent
{
	std::list<Agent*>::iterator iter;
	for (iter = agents.begin(); iter!=agents.end();iter++)
	{
		if ((*iter)->getName() == name)
			return (*iter);
	}
	return NULL;
}

void
GameApplication::addTime(Ogre::Real deltaTime)	// Called at every time step
{
	int error = 0;
	LWNetList::advance(&error);			// advance the PaTNets

	std::list<Agent*>::iterator iter;
	for (iter = agents.begin(); iter!=agents.end();iter++)	// Update all of the agents
	{
		if ((*iter) != NULL)
			(*iter)->update(deltaTime);
	}
}