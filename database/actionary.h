//////////////////////////////////////////////////////////////////
// Interface to the MySQL Actionary database
// Actionary includes the actions and object 

#ifndef ACTIONARY_H
#define ACTIONARY_H
//#include "Python.h"
#include <iostream>
#include <string>
#include "metaaction.h"
#include "metaobject.h"

#include "parProperty.h"

class iPAR;
typedef struct _object PyObject;


class Actionary {
private:
	friend class MetaObject;			// Allow MetaObject access to private variables
	friend class MetaAction;			// Allow MetaAction access to private variables
	std::map<int, MetaObject*> objMap;
	std::map<int, MetaAction*> actMap;
	std::map<int,iPAR*> parMap;
	std::map<int,parProperty*> properties;

	int maxObjID;				   // the current maximum object id in the actionary db
	int maxActID;				   // the current maximum action id in the actionary db
	int maxPARID;				   // the current maximum iPAR id in the actionary db
	std::vector<int> allAgents;    // all of the agents in the database
	std::vector<int> allRooms;		// all of the rooms in the database
	std::vector<int> allObjects;	// all of the objects in the database
	std::vector<int> allTypes;		// all of the types in the database
	std::vector<int> allActions;	// all of the actions in the database
public:
	//mysqlpp::Connection con;					// connection to the MySQL database
	void  init();
	int  convertTime(int epochTime);				// convert from epoch to seconds from midnight of current day
	bool  isParent(std::string name);
	
	MetaObject  *create(const std::string& obname, MetaObject* obparent, bool agent); // create a metaobject
	void  removeObject(MetaObject *obj);			// remove an object from the database and other structures
	void  removeObject(int objID);				// remove an object from the database only				
	std::string  getObjectName(int objID);					// get the object name
	void  setObjectName(MetaObject *obj, std::string newName);	// set the object name

	int  findNewPARID(){this->maxPARID++; return this->maxPARID;}

	MetaObject  *searchByNameObj(const std::string& name,bool reverse=false);// get a pointer to the object
	MetaObject  *searchByIdObj(int objID);					// get a pointer to the object from its id
	void  setParent(MetaObject *obj, MetaObject *parent);	// set the parent of the object
	MetaObject  *getParent(MetaObject *obj);
	MetaObject *getChild(MetaObject *obj,int which);

	int  getNextObjID() {this->maxObjID++; return this->maxObjID;}
	int  getNextActID() {this->maxActID++; return this->maxActID;}
	bool  isObject(const std::string &objName);						// is this object already in the Actionary
	bool  isAction(const std::string &actName);						// is this action already in the Actionary
	bool  isAgent(MetaObject* obj);									// is this object also an agent?
	int   getAgent(int which);								//Gets the which'd agent id that we have
	bool  isRoom(MetaObject* obj);									// is the object a room
	int  getObjectID(const std::string& objName,bool reverse=false);					// return this object's ID
	int  getActionID(const std::string& actName,bool reverse=false);					// return this action's ID
	int  addObject(MetaObject* obj, const std::string& objName, bool agnt,bool instance); // create a new object

	void  setStatuses(iPAR* act, std::string status_type);  // set the status of all objects associated with the action

	bool  isType(MetaObject *obj, const char* typeName);
	bool  isType(MetaObject *obj, MetaObject *type);//Faster type checking
	
	//Site Stuff
	void  addGraspSite(MetaObject* obj,int siteType,
		float sitePosX, float sitePosY, float sitePosZ,
		float siteOrientX, float siteOrientY, float siteOrientZ);
	void  updateGraspSite(MetaObject *obj, int siteType,    // use -999 where values should not be altered
		float sitePosX, float sitePosY, float sitePosZ,
		float siteOrientX, float siteOrientY, float siteOrientZ);

	bool  removeGraspSite(MetaObject *obj, int siteType);
	MetaObject  *searchGraspSites(MetaObject* obj, int site_type); // return the site id or -1 if not found
	int    getSiteType(const std::string& siteName);
	MetaObject  *searchGraspSites(MetaObject* obj, const std::string& siteName); // return the site id or -1 if not found
	std::string  getGraspSiteName(int siteType);
	float  getGraspSitePos(MetaObject* obj, int siteType, int which);
	Vector<3>  getGraspSitePos(MetaObject* obj, int siteType);
	float  getGraspSiteOrient(MetaObject* obj, int siteType, int which);
	Vector<3>  getGraspSiteOrient(MetaObject* obj, int siteType);



	//Agent Capability stuff
	int  getCapabilities(MetaObject* obj, int which);		// returns an action id
	bool  searchCapability(MetaObject* obj, char* action);
	bool  searchCapability(MetaObject* obj, MetaAction* action);		// It's much faster to search by ID
	void  setCapability(MetaObject* obj, char* action);
	void  removeCapability(MetaObject* obj, char* action);
	
	// Generalized property object code
	int  setProperty(MetaObject* obj,parProperty*, int); //Sets the property for a type object. This is both add and update
	//Generalized Property Information for actions
	parProperty* getProperty(MetaObject*, int which);//Gets the par-property that we are using
	int getNumProperties(MetaObject*); //Gets the number of unique properties
	int getProperty(MetaObject* act, parProperty* prop, int which); //Gets the property values since we know the property that we are looking for
	int getNumProperties(MetaObject* act, parProperty *prop); //We can have multiple properties, so we use this to get all the properties


	//This lets us search for properties

	std::string  getPropertyNameByValue(const std::string& tab_name,int value);     // gets the property name from any property table
	int  getPropertyValueByName(const std::string& tab_name,const std::string& prop_name); // gets the integer property value from any property table
	parProperty  *searchByNameProperty(const std::string& tab_name);						// Determines if a property table type is actually a table
	bool  hasTable(std::string tab_name); //Determines if a table exists
	int  getPropertyTypeByName(const char*);//Determines the type of property by it's name
	//void addPropertyType(const char*,int);//Adds a property type to the database
	//void removePropertyType(int);//Removes a property type from the database

	//Generalized Property Information for actions
	parProperty* getProperty(MetaAction*, int which);//Gets the par-property that we are using
	int getNumProperties(MetaAction*); //Gets the number of unique properties
	int getProperty(MetaAction* act, parProperty* prop, int which); //Gets the property values since we know the property that we are looking for
	int getNumProperties(MetaAction* act, parProperty *prop); //We can have multiple properties, so we use this to get all the properties



	MetaAction  *searchByNameAct(const std::string& name,bool reverse=false);					// get a pointer to the action
	MetaAction  *searchByIdAct(int objID);						// get a pointer to the action from its id
	MetaAction  *breadthFirstSearch(std::queue<MetaAction*>&);
	int		 addAction(MetaAction* act, std::string actName);	// create a new action
	int      addAction(MetaAction* act, std::string name, std::vector<MetaAction*> parents);//Creates a new action with the parent
	MetaAction  *create(char* aname, MetaAction* aparent);		// create a metaaction
	std::string  getActionName(MetaAction *act);					// get the action name
	void	 setActionName(MetaAction *act, std::string newName);	// set the action name
	void	 setParent(MetaAction* act, MetaAction* parent);		// place this object in the hierarchy 
	MetaAction  *getParent(MetaAction* act, int which=0);
	int getNumParents(MetaAction *act);
	void  removeAction(MetaAction *act);							// remove an action from the database and other structures
	void  removeAction(int actID);								// remove an action from the database only

	void	 setPurposeAchieve(MetaAction* act, const std::string& achieve);
	std::string	 getPurposeAchieve(MetaAction* act);
	std::vector<MetaAction*>  getAllPurposed(const std::string& achieve);


	int		 getNumObjects(MetaAction* act);
	// Affordance stuff
	void  addAffordance(MetaAction* act, MetaObject* obj, int which);
	void  removeAffordance(MetaAction* act, MetaObject* obj, int which);
	int  searchAffordance(MetaAction* act,MetaObject *obj);
	int  getNumAffordance(MetaAction* act, int position);
	MetaAction  *searchAffordance(MetaObject *obj, int position,int which);
	MetaObject  *searchAffordance(MetaAction *act, int position,int which);
	// Duration stuff
	float	 getDuration(MetaAction* act);
	void	 setDuration(MetaAction* act, float d);

	std::string	 getAdverb(MetaAction* act);		// later allow for more than one adverb
	std::string	 getModifier(MetaAction* act);	// later allow for more than one modifier
	void  setAdverb(MetaAction*, const std::string  &, const std::string &);

	//Helper functions
	std::string extractParentName(const std::string&);

	
	// set the database entries
	void	 setApplicabilityCond(MetaAction* act, const std::string& appCond);
	std::string	 getApplicabilityCond(MetaAction* act);
	void	 setCulminationCond(MetaAction* act, const std::string& termCond);
	std::string	 getCulminationCond(MetaAction* act);
	void	 setPreparatorySpec(MetaAction* act, const std::string& prepSpec);
	std::string	 getPreparatorySpec(MetaAction* act);
	void	 setExecutionSteps(MetaAction* act, const std::string& execSteps);
	std::string	 getExecutionSteps(MetaAction* act);
	
	// load the conditions in Python.
	int  setCondition(MetaAction* act, int which);
	int  loadApplicabilityCond(MetaAction* act);
	int  loadCulminationCond(MetaAction* act);
	int  loadPreparatorySpec(MetaAction* act);
	int  loadExecutionSteps(MetaAction* act);

	int loadExternPythonFunctionFile(const std::string& file_name);
	
	// perform the tests
	PyObject  *Actionary::testCondition(iPAR* ipar, int which);
	PyObject  *Actionary::testApplicabilityCond(iPAR* ipar);
	PyObject  *Actionary::testCulminationCond(iPAR* ipar);
	PyObject  *Actionary::testPreparatorySpec(iPAR* ipar);
	PyObject  *Actionary::testExecutionSteps(iPAR* ipar);
	PyObject  *Actionary::testDuringAssertions(iPAR* ipar);
	PyObject  *Actionary::testPostAssertions(iPAR* ipar);



	int  getSiteType(MetaAction *act);


	MetaObject  *getAllAgents(int which);
	MetaObject  *getAllRooms(int which);
	MetaObject  *getAllObjects(int which); // not rooms and only instances
	MetaObject  *getAllTypes(int which);  
	MetaAction  *getAllActions(int which);

	// iPAR handling
	void  addiPAR(int ipar_id,iPAR *ipar);
	iPAR  *searchByIdiPAR(int ipar_id);
	iPAR  *searchByNameAgentiPAR(const std::string&, const std::string &);

	//Site and Region Definition
	bool  createSiteShape(MetaObject* obj, int siteType,const char* shape_type,float first,float second=0.0, float third=0.0);
	int  getSiteShapeID(MetaObject* obj, int siteType);
	std::string  getSiteShapeType(int site_shape_id);
	float  getSiteShapeCoordinate(int site_shape_id,int coordinate);
	////////////////////////////////////////////////////////////////////////////////
};

#endif
