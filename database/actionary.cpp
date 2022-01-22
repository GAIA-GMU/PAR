#include <fstream>
#include <time.h>
#include <sstream>
#include <memory>
#include "actionary.h"
#include "utility.h"
#include "par.h"
#include "interpy.h"
#include "agentproc.h"
#include "partable.h"
#include "sqlite3.h"

extern AgentTable agentTable;
extern parTime *partime;

sqlite3* db; //Move the connection to a global so we don't have the dependency in the .h file

//Our sqlite3 helper functions

//This function gives us back multiple rows. Let's hope to god this isn't ascyn
static int sql_callback(void *data, int argc, char **argv, char **col_name) { //Taken from the tutorial on sqlite. This is called for each row, and so adds that data
	std::map<int, std::map<std::string, std::string> > *res = reinterpret_cast<std::map<int, std::map<std::string, std::string> >  *>(data);
	std::map<std::string,std::string> row;
	for (int i = 0; i < argc; i++) {
		row[col_name[i]] = argv[i] ? argv[i] : "";
	}
	res->insert(std::make_pair(res->size(), row));
	return 0;
}
//This function gives us back multiple rows. Let's hope to god this isn't ascyn
static int sql_mrsi_callback(void *data, int argc, char **argv, char **col_name) { //Taken from the tutorial on sqlite. This is called for each row, and so adds that data
	std::map<int, std::string> *res = reinterpret_cast<std::map<int, std::string> *>(data);
	res->insert(std::make_pair(res->size(), std::string(argv[0] ? argv[0] : "")));
	return 0;
}
//This function gives us back multiple rows. Let's hope to god this isn't ascyn
static int sql_srmi_callback(void *data, int argc, char **argv, char **col_name) { //Taken from the tutorial on sqlite. This is called for each row, and so adds that data
	std::map<std::string, std::string> *res = reinterpret_cast<std::map<std::string, std::string> *>(data);
	for (int i = 0; i < argc; i++) {
		res->insert(std::make_pair(col_name[i], std::string(argv[i] ? argv[i] : "")));
	}
	return 0;
}
//This allows to get back a single piece of data
//still need to limit in queries, but it's good practice to do that anyways
static int sql_callback_single(void *data, int argc, char**argv, char **col_name) {
	std::string &res = *static_cast<std::string*>(data);
	res = std::string(argv[0] ? argv[0] : "");
	return 0;
}

void Actionary::init(const std::string & actionary_path)
{
	int rc = 0;
	maxObjID  = 0;
	maxActID  = 0;
	maxPARID  = 0;
	std::string full_path = std::string(actionary_path) + "par.db";
	rc = sqlite3_open(full_path.c_str(), &db);
	if (rc){
		par_debug("ERROR in Actionary::init, database connection not established:%s\n", sqlite3_errmsg(db));
		return;
	}
	par_debug("Connection established to Actionary\n");

	initprop();
	std::stringstream query;
	std::map<int, std::map< std::string, std::string> > res;
	std::map<int, std::map< std::string, std::string> >::iterator it;
	char* error_msg;
	query<<"select prop_id,prop_name,is_int,omega from property_type order by prop_id";
	rc = sqlite3_exec(db, query.str().c_str(), sql_callback, &res, &error_msg);
	if (rc != SQLITE_OK){
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
		throw iPARException("Error in Actionary");
	}
	for (it = res.begin(); it != res.end(); it++){
		int not_const = (*it).first;
		parProperty *prop = new parProperty(not_const, 
			(*it).second.at("prop_name"),atoi((*it).second.at("is_int").c_str()), 
			atoi((*it).second.at("omega").c_str()));
		properties[prop->getPropertyID()] = prop;
	}
	res.clear();
	query.clear();
	query.str("");
	query<<"select obj_id,is_agent,obj_name from object order by obj_id";
	rc = sqlite3_exec(db, query.str().c_str(), sql_callback, &res, &error_msg);
	if (rc != SQLITE_OK){
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
		throw iPARException("Error in Actionary");
	}
	for (it = res.begin(); it != res.end(); it++){
		//Not using objStruct because the old trees have been gone for quite some time
		bool agent = false;
		maxObjID = atoi((*it).second.at("obj_id").c_str());
		if (atoi((*it).second.at("is_agent").c_str()) == 1)
			agent = true;
		par_debug("object name is %s\n", (*it).second.at("obj_name").c_str());
		objMap[maxObjID] = new MetaObject((*it).second.at("obj_name").c_str(), agent, false);
	}
	//This back-propigates known semantics up the object hierarchy
	for (int j = (int)objMap.size() - 1; j>0; j--){
		if (this->objMap[j]->getParent() != NULL){
			this->objMap[j]->getParent()->setAllProperties(this->objMap[j]->getAllProperties());
		}
	}
	res.clear();
	query.clear();
	query.str("");
	std::map<int, std::string> res2;
	query<<"select act_id from action order by act_id";
	rc = sqlite3_exec(db, query.str().c_str(), sql_mrsi_callback, &res2, &error_msg);
	if (rc != SQLITE_OK){
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
		throw iPARException("Error in Actionary");
	}
	MetaAction *act;
	std::map<int, MetaAction*>::iterator finder;
	for (std::map<int,std::string>::const_iterator it = res2.begin(); it != res2.end(); it++){
		maxActID = atoi((*it).second.c_str());
		finder = actMap.find(maxActID);
		if (finder == actMap.end()){
			act = new MetaAction(maxActID);
			if (!strcmp("", act->getActionName().c_str()))
				par_debug("Error creating action with id %d\n", maxActID);
			else{
				par_debug("Action name is %s\n", act->getActionName().c_str());
				this->actMap[maxActID] = act;
				this->allActions.push_back(maxActID);
				createPyActions(act); // create the Python rep for each actions
				// load all of the python conditions and specs too
				//par_debug("Loading the action functions\n");
				loadApplicabilityCond(act);
				loadCulminationCond(act);
				loadPreparatorySpec(act);
				loadExecutionSteps(act);
			}
		}
		else{
			par_debug("ActionID %d is already in actionary\n",maxActID);
		}
	}
	std::map<int, MetaAction*>::const_reverse_iterator rit;
	for (rit = actMap.rbegin(); rit != actMap.rend(); ++rit){
		for (int j = 0; j < (*rit).second->getNumParents(); j++){
			(*rit).second->getParent(j)->setAllProperties((*rit).second->getAllProperties());
		}
	}
}

/*Clears out everything in the actionary*/
Actionary::~Actionary() {
	/*Cleans out the agents and their table*/
	agentTable.clearTable();
	//Clears out the vectors (trival since they are ints)
	this->allAgents.clear();
	this->allObjects.clear();
	this->allTypes.clear();
	this->allObjects.clear();

	//Calls the destructor on each object
	this->objMap.clear();
	this->actMap.clear();
	this->parMap.clear();
	this->properties.clear();
	sqlite3_close(db);
}

// convert from epoch to seconds from midnight of current day
int
Actionary::convertTime(int epochTime)
{
	time_t eTime = epochTime;
    struct tm timeinfo;
	localtime_s(&timeinfo,&eTime);

	return timeinfo.tm_hour*3600 + timeinfo.tm_min*60 + timeinfo.tm_sec;

}

bool
Actionary::isParent(std::string name)
{

	size_t found;

	found = name.find('_');
	if (found == std::string::npos)
		return true;

	return false;
}

///////////////////////////////////////////////////////////////////
// create a metaobject
MetaObject *
Actionary::create(const std::string& obname, MetaObject* obparent, bool agent)
{
	// check to see if it is already in the table first?
	MetaObject* obj = this->searchByNameObj(obname);
	if (obj != NULL)
	{
		par_debug("Found Object %s, just returning from Actionary::create\n",obname.c_str());
		return obj;
	}

	obj = new MetaObject(obname.c_str(), agent);  // adds it to the database too
	if (obj == NULL)
		par_debug("ERROR: object not created in Actionary::create\n");

	// set the parent
   if (obparent != NULL)
		obj->setParent(obparent);
   else
   		obj->setParent(this->searchByNameObj("Root"));

    return obj;
}

////////////////////////////////////////////
// remove an object from the database
void
Actionary::removeObject(MetaObject *obj)
{
	if (obj == NULL)
		return;
	int objID = getObjectID(obj->getObjectName());
	if (objID == -1)
	{
		par_debug("ERROR: not a valid object in removeObject\n");
		return;
	}
	
	objMap.erase(objID);
	//Removes from the table based on removeObject (objID)
	if(!obj->isInstance())
		removeObject(objID);

	// kill the pointer
	delete obj;
	obj = NULL;

}
///////////////////////////////////////////////////////////////////////////////
//Removes an object from the actionary. Probably not the best thing to do since
//objects should be persistant, but could be useful
void
Actionary::removeObject(int objID)
{
	if (objID < 0)
		return;
		// remove it from all of the tables
	std::stringstream query;
	//sqlite3* db = con.get(); //they say this is bad
	char* tables[]={"object","obj_act","obj_prop","site"};
	char* error_msg;
	for (int i = 0; i < 4; i++){
		query.str(std::string());
		query.clear();
		query << "Delete from " << tables[i] << " where obj_id =" << objID;
		int rc = sqlite3_exec(db, query.str().c_str(), 0, 0, &error_msg);
		if (rc != SQLITE_OK) {
			par_debug("%s\n", error_msg);
		}
		sqlite3_free(error_msg);
	}
}

//////////////////////////////////////////////////
// set the parent of the object
// i.e. place the object in the hierarchy
void
Actionary::setParent(MetaObject *obj, MetaObject *parent)
{
	if (parent == NULL)
		par_debug("ERROR: null parent pointer in Actionary::setParent\n");

	//sqlite3* db = con.get();
	std::stringstream query;
	if (!parent->isInstance() && !obj->isInstance()) {
		int parentID = getObjectID(parent->getObjectName());
		int objID = getObjectID(obj->getObjectName());
		char* error_msg;
		query << "Update object set parent_id =" << parentID << " where obj_id=" << objID;
		int rc = sqlite3_exec(db, query.str().c_str(), 0, 0, &error_msg);
		if (rc != SQLITE_OK) {
			par_debug("%s\n", error_msg);
		}
		sqlite3_free(error_msg);
	}
}

MetaObject*
Actionary::getParent(MetaObject *obj)
{
	if (obj == NULL)
		return NULL;
	
	std::stringstream query;
	MetaObject *pobj = NULL;
	
	query << "Select parent_id from object where obj_id =" << obj->getID() <<" LIMIT 1";
	//sqlite3* db = con.get();
	char* error_msg;
	std::string res;
	int rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
		throw iPARException("Error in Actionary");
	}
	pobj = this->searchByIdObj(atoi(res.c_str()));
	return pobj;

}
///////////////////////////////////////////////////////////////////////////////
//Gets the which'd child of a metaobject
//This function is going to assume that all children are loaded into memory
//and that without the inital load, we shouldn't be calling this
///////////////////////////////////////////////////////////////////////////////
MetaObject *
Actionary::getChild(MetaObject *obj,int which){
	if(obj == NULL)
		return NULL;
	int counter=0;
	if (!objMap.empty()) {
		for (std::map<int, MetaObject*>::iterator it = objMap.begin(); it != objMap.end(); it++) {
			if ((*it).second->parent == obj) {
				if (counter == which)
					return (*it).second;
				else
					counter++;
			}
		}
	}
	return NULL;
}

///////////////////////////////////////////////
// Is this object already in the Actionary?
bool
Actionary::isObject(const std::string &objName)
{
	// check to see if it already exists and if so print a message
	std::stringstream query;
	query<<"select obj_id from object where obj_name = '"<<objName<<"' LIMIT 1";
	bool val = false;
	//sqlite3* db = con.get();
	char* error_msg;
	std::string res;
	int rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
		throw iPARException("Error in Actionary");
	}
	if (res != "") {
		val = true;
	}

	return val;
}

//////////////////////////////////////////
// is this object also an agent?
bool
Actionary::isAgent(MetaObject* obj)
{
	//We store all the agents in the agent vector
	//so we can do fast pointer comparisions to determine
	//if the object we found is an agent.  If it is,
	//it must be in the agent vector (or we have a problem)
	if(obj == NULL)
		return false; //NULLs only an agent of chaos
	for(unsigned int i=0; i<allAgents.size(); i++)
		if(obj->getID() == allAgents.at(i))
			return true;
	return this->isAgent(obj->getParent());
}
int 
Actionary::getAgent(int which){
	if (which < 0 || which > this->allAgents.size())
		return -1;
	return this->allAgents.at(which);
}

/////////////////////////////////////////////////
//Is the object a room object
bool
Actionary::isRoom(MetaObject* obj){
	//The same logic will apply here as it does to the agent's above
	if(obj == NULL)
		return false; //NULLs only an agent of chaos
	for(unsigned int i=0; i<allRooms.size(); i++)
		if(obj->getID() == allRooms.at(i))
			return true;

	return this->isRoom(obj->getParent());
}
//////////////////////////////////////////////
// create a new object
int
Actionary::addObject(MetaObject *obj, const std::string& objName, bool agnt,bool instance)
{
		// need a new object id and need to store the id in this MetaObject
	int objID = obj->getID();
	if (obj->getID() < 0) {
		objID = getNextObjID();
	}
	if(!instance){
		int isAgent = 0;
		if (agnt)
			isAgent = 1;
		std::stringstream query;
		query<< "INSERT INTO object (obj_id,obj_name,is_agent) VALUES("<<objID<<",'"<<objName<<"',"<<isAgent<<")";
		//sqlite3* db = con.get();
		char* error_msg;
		std::string res;
		int rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
		if (rc != SQLITE_OK) {
			par_debug("%s\n", error_msg);
			sqlite3_free(error_msg);
			throw iPARException("Error in Actionary adding object");
		}	
	}
	if(this->searchByIdObj(objID) == NULL) //Add it to the map if we don't have it already there
		objMap[objID] = obj;
	return objID;
}

/////////////////////////////////////////
// return this object's ID from the actionary
int
Actionary::getObjectID(const std::string& objName,bool reverse)
{
	if(!reverse){
		for (std::map<int, MetaObject*>::iterator it = objMap.begin(); it != objMap.end(); it++) {
				if ((*it).second != NULL && !((*it).second->getObjectName().compare(objName)))
						return (*it).second->getID();
		}
	}else{
		for (std::map<int, MetaObject*>::reverse_iterator it = objMap.rbegin(); it != objMap.rend(); it++) {
			if ((*it).second!= NULL && !((*it).second->getObjectName().compare(objName)))
				return (*it).second->getID();
		}
	}
	int objID=-1;
	std::string queryStr = "select obj_id from object where obj_name = '" +objName + "'";
	//sqlite3* db = con.get();
	char* error_msg;
	std::string res;
	int rc = sqlite3_exec(db, queryStr.c_str(), sql_callback_single, &res, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
	if (res != "") {
		objID = atoi(res.c_str());
	}
	return objID;
}

///////////////////////////////////////////////////////
// get the object name
std::string
Actionary::getObjectName(int id)
{
	std::stringstream query;
	query<<"SELECT obj_name FROM object where obj_id="<<id<<" LIMIT 1";
	//sqlite3* db = con.get();
	char* error_msg;
	std::string res;
	int rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
	return res;
}

/////////////////////////////////////////////////////
// set the object name
void
Actionary::setObjectName(MetaObject *obj, std::string newName)
{
	if (obj == NULL)
		return;
	std::stringstream query;
	query << "update object set obj_name = '" <<newName <<"' where obj_id = "<<obj->getID();
	//sqlite3* db = con.get();
	char* error_msg;
	int rc = sqlite3_exec(db, query.str().c_str(), 0,0, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
}



/////////////////////////////////////////////
// get a pointer to the MetaObject
MetaObject *
Actionary::searchByNameObj(const std::string& name,bool reverse)
{
	// get the id associated with this name
	int objID = getObjectID(name,reverse);

	// return the MetaObject pointer stored in objVec/Map
	return this->searchByIdObj(objID);
}
///////////////////////////////////////////
// get a pointer to the object from its id
MetaObject *
Actionary::searchByIdObj(int objID)
{
	if (objMap.find(objID) == objMap.end())
		return NULL;
	return objMap[objID];
}
////////////////////////////////////////////////////////////////////
// set the status of all objects associated with the action
//////////////////////////////////////////////////////////////////
void
Actionary::setStatuses(iPAR* act, std::string status_type)
{
	if (act == NULL)
		return;
	int i = 0;
	MetaObject* obj = act->getObject(i);
	parProperty* prop = this->searchByNameProperty("obj_status");
	int status_value = prop->getPropertyValueByName(status_type);
	while (obj != NULL)
	{
		this->setProperty(obj,prop,status_value);
		i++;
		obj = act->getObject(i);
	}
}


bool
Actionary::isType(MetaObject *obj,  const char* typeName)
{
	if (obj == NULL)
		return false;

	if (strcmp(typeName, obj->getObjectName().c_str()) == 0)
		return true;

	return isType(obj->getParent(), typeName);
}
///////////////////////////////////////////////////////////////////////////////
//This is only comparing pointers, so it will be much more economical to use.
///////////////////////////////////////////////////////////////////////////////
bool
Actionary::isType(MetaObject *obj, MetaObject *type){
	if(obj == NULL || type == NULL)
		return false;

	if(obj == type)
		return true;

	return isType(obj->getParent(),type);
}

void
Actionary::addGraspSite(MetaObject* obj,int siteType,
		float sitePosX, float sitePosY, float sitePosZ,
		float siteOrientX, float siteOrientY, float siteOrientZ )
{
	if (obj == NULL)
		return;
	std::stringstream query;
		query << "INSERT INTO site VALUES(";
		query << obj->getID() << ",";
		query << siteType << ",";
		query << sitePosX << "," << sitePosY << "," << sitePosZ << ",";
		query << siteOrientX << "," << siteOrientY << "," << siteOrientZ << ",";
		query << "-1)";
		//sqlite3* db = con.get();
		char* error_msg;
		std::string res;
		int rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
		if (rc != SQLITE_OK) {
			par_debug("%s\n", error_msg);
			sqlite3_free(error_msg);
		}
}

void
Actionary::updateGraspSite(MetaObject* obj,int siteType,   
		float sitePosX, float sitePosY, float sitePosZ,
		float siteOrientX, float siteOrientY, float siteOrientZ)
{
	if(obj == NULL)
		return;
		std::stringstream query;
		query.clear();
		query.str("");
		query << "update site set " << "site_pos_x" << "=" << sitePosX << ", ";
		query << "site_pos_y" << "=" << sitePosY << ", ";
		query << "site_pos_z" << "=" << sitePosZ << ", ";
		query << "site_orient_x" << "=" << siteOrientX << ", ";
		query << "site_orient_y" << "=" << siteOrientY << ", ";
		query << "site_orient_z" << "=" << siteOrientZ << " ";
		query<<"WHERE obj_id =" << obj->getID() << " AND site_type= " << siteType;
		//sqlite3* db = con.get();
		char* error_msg;
		int rc = sqlite3_exec(db, query.str().c_str(), 0, 0, &error_msg);
		if (rc != SQLITE_OK) {
			par_debug("%s\n", error_msg);
			sqlite3_free(error_msg);
		}
}

bool
Actionary::removeGraspSite(MetaObject* obj, int siteType)
{
	bool finished = false;
	std::stringstream query;
	query << "Delete from site where obj_id=" << obj->getID() << " and site_type=" << siteType;
	//sqlite3* db = con.get();
	char* error_msg;
	std::string res;
	int rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
		return finished;
	}
	else {
		finished = true;
	}

	return finished;
}


// return the MetaObject or NULL if not found. This way, we have a handle on the MetaObject
MetaObject*
Actionary::searchGraspSites(MetaObject* obj, int site_type)
{

	int objID = -1;
	if (obj == NULL)
		return NULL;
	MetaObject *parent = obj;
	//sqlite3* db = con.get();
	char* error_msg;
	std::string res;
	std::stringstream query;
	while (parent != NULL && objID < 0){
		query.clear();
		query.str("");
		query << "select obj_id from site where obj_id="<<parent->getID()<<" and site_type_id ="<<site_type<<" LIMIT 1";
		int rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
		if (rc != SQLITE_OK) {
			par_debug("%s\n", error_msg);
			sqlite3_free(error_msg);
		}
		if (res != "") {
			objID = atoi(res.c_str());
		}
		parent = obj->getParent();
	}
	if(objID > 0)
		return this->searchByIdObj(objID);
	return NULL;
}

//returns the site type from the site name (inspect, operate, etc)
int
Actionary::getSiteType(const std::string& site_name){
	std::stringstream query;
	//sqlite3* db = con.get();
	char* error_msg;
	std::string res;
	int val = -1;
	int rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
		return val;
	}
	if (res != "")
		val = atoi(res.c_str());
	return val;
}

// return the site_type_id or -1 if not found
MetaObject*
Actionary::searchGraspSites(MetaObject* obj, const std::string& siteName)
{
	if (obj == NULL)
		return NULL;

	int siteType=getSiteType(siteName);
	return searchGraspSites(obj,siteType);
}

std::string
Actionary::getGraspSiteName(int siteType)
{
	std::string res = "";
	if (siteType <0)
		return NULL;
	std::stringstream query;
	query << "select site_name from site_type where site_type_id =" << siteType;
	//sqlite3* db = con.get();
	char* error_msg;
	int rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
		return res;
	}
	return res;
}

float
Actionary::getGraspSitePos(MetaObject* obj, int siteType, int which)
{
	if (siteType <0 || which <0 || which > 2 )
		return -999;
	Vector<3> pos=this->getGraspSitePos(obj,siteType);
	return pos.v[which];
}

Vector<3>
Actionary::getGraspSitePos(MetaObject* obj, int siteType)
{
	Vector<3> pos;
	for(int i=0; i<2 ;i++)
		pos.v[i]=-999;
	if(obj == NULL ||siteType < 0)
		return pos;
	std::stringstream query;
	query << "select site_pos_x, site_pos_y, site_pos_z from site where obj_id = "<<obj->getID()<<" AND site_type_id ="<< siteType<<" LIMIT 1";
	std::map<std::string, std::string> res;
	//sqlite3* db = con.get();
	char* error_msg;
	int rc = sqlite3_exec(db, query.str().c_str(), sql_srmi_callback, &res, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
		return pos;
	}
	if (!res.empty()){
		pos.v[0] = (float)atof(res.at("site_pos_x").c_str());
		pos.v[1] = (float)atof(res.at("site_pos_y").c_str());
		pos.v[2] = (float)atof(res.at("site_pos_z").c_str());
	}
	return pos;
}

float
Actionary::getGraspSiteOrient(MetaObject* obj, int siteType, int which)
{
	if (siteType <0 || which <0 || which > 2 )
		return -999;
	Vector<3> pos=this->getGraspSiteOrient(obj,siteType);
	return pos.v[which];
}

Vector<3>
Actionary::getGraspSiteOrient(MetaObject* obj, int siteType)
{
	Vector<3> pos;
	for(int i=0; i<2 ;i++)
		pos.v[i]=-999;
	if(obj == NULL ||siteType < 0)
		return pos;
	std::stringstream query;
	query <<"select site_orient_x, site_orient_y, site_orient_z  from site where obj_id="<<obj->getID()<<" AND site_type_id="<<siteType;
	std::map<std::string, std::string> res;
	//sqlite3* db = con.get();
	char* error_msg;
	int rc = sqlite3_exec(db, query.str().c_str(), sql_srmi_callback, &res, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
		return pos;
	}
	if (!res.empty()) {
		pos.v[0] = (float)atof(res.at("site_orient_x").c_str());
		pos.v[1] = (float)atof(res.at("site_orient_y").c_str());
		pos.v[2] = (float)atof(res.at("site_orient_z").c_str());
	}
	return pos;
}


int
Actionary::getCapabilities(MetaObject* obj, int which) // returns an action id
{
	if (obj == NULL || which < 0)
		return -1;
	MetaObject *parent=obj;//TB: Should this be reworked or is it even needed?
	int act_id = -1;
	std::stringstream query;
	query << "select action_id from agent_capable where obj_id IN (";
	while (parent != NULL){
		query << obj->getID();
		parent = parent->getParent();
		if (parent != NULL) {
			query << ",";
		}
	}
	query<<") ORDER BY action_id DESC LIMIT "<<which<<", 1";
	//sqlite3* db = con.get();
	char* error_msg;
	std::string res;
	int rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
		return act_id;
	}
	if (res != "") {
		act_id = atoi(res.c_str());
	}
	return act_id;
}
///////////////////////////////////////////////////////////////////////////////
//Overloaded to allow the author to just use the action name to search for
//capabilities
///////////////////////////////////////////////////////////////////////////////
bool
Actionary::searchCapability(MetaObject* obj, char* action)
{
	if (obj == NULL)
		return false;
	MetaAction* act = searchByNameAct(action);
	if (act == NULL)
		return false;
	return searchCapability(obj,act);

}
///////////////////////////////////////////////////////////////////////////////
//Searches through the agent's capabilities to figure out if an action is
//possible
///////////////////////////////////////////////////////////////////////////////
bool
Actionary::searchCapability(MetaObject* obj, MetaAction* action){
	if(obj == NULL || action == NULL)
		return false;

	MetaObject *parent=obj;
	bool found=false;
	std::stringstream query;
	query << "select action_id from agent_capable where obj_id IN (";  
	while (parent != NULL && !found){
		query<< parent->getID();
		parent = parent->getParent();
		if(parent != NULL){
			query << ",";
		}
	}
	query << ") and action_id =" << action->getID() << " LIMIT 1";
	//sqlite3* db = con.get();
	char* error_msg;
	std::string res;
	int rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
		return found;
	}
	if (res != "") {
		found = true;
	}
	return found;
}

void
Actionary::setCapability(MetaObject* obj, char* action)
{
	if (obj == NULL)
		return;
	MetaAction* act = searchByNameAct(action);
	if (act == NULL)
		return;
	std::stringstream query;
	query<<"INSERT INTO agent_capable VALUES("<<obj->getID()<<","<<act->getID()<<")";
	//sqlite3* db = con.get();
	char* error_msg;
	int rc = sqlite3_exec(db, query.str().c_str(), 0, 0, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
}

void
Actionary::removeCapability(MetaObject* obj, char* action)
{
	if (obj == NULL)
		return;
	MetaAction* act = searchByNameAct(action);
	if (act == NULL)
		return;
	std::stringstream query;
	query << "delete * from agent_capable where obj_id =" << obj->getID() << "  and action_id =" << act->getID();
	//sqlite3* db = con.get();
	char* error_msg;
	int rc = sqlite3_exec(db, query.str().c_str(), 0, 0, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
}
/*
///////////////////////////////////////////////////////////////////////////////////
//This applies a generalized property to a metaobject.  Idealy, the object
//can have many different properties (color,status,posture) and they all be found
//within the property table.
/////////////////////////////////////////////////////////////////////////////////
int
Actionary::setProperty(MetaObject* obj,std::string tab_name, std::string value) {
	if (obj == NULL)
		return -1;

	int prop_value=this->getPropertyValueByName(tab_name,value);
	return this->setProperty(obj,tab_name,prop_value);


}*/
///////////////////////////////////////////////////////////////////////////////////
//We're making the big assumption that this will never be called with a property that
//doesn't exist
///////////////////////////////////////////////////////////////////////////////
int
Actionary::setProperty(MetaObject* obj,parProperty* prop, int value){
	if(obj == NULL)
		return -1;

	//First, we see if the property exists.  If it does, then we're just
	//updating
	int updated=-1;
	std::stringstream query;
	//sqlite3* db = con.get();
	char* error_msg;
	int rc = 0;
	query << "SELECT obj_id from obj_prop WHERE obj_id =" << obj->getID() << " AND table_id = " << prop->getPropertyID() << " AND prop_value = " << value <<" LIMIT 1";
	std::string res;
	rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
		return updated;
	}
	query.str(std::string());
	query.clear();
	if (res != ""){
		query << "UPDATE obj_prop SET prop_value = " << value << " WHERE obj_id= " << obj->getID() << " and table_id ='" << prop->getPropertyID() << "';";
		rc = sqlite3_exec(db, query.str().c_str(), 0, 0, &error_msg);
		if (rc != SQLITE_OK) {
			par_debug("%s\n", error_msg);
			sqlite3_free(error_msg);
			return updated;
		}
		else {
			updated = 1;
		}
	}
	else {
		query << "INSERT INTO obj_prop VALUES(" << obj->getID() << ",'" << prop->getPropertyID() << "," << value << ");";
		rc = sqlite3_exec(db, query.str().c_str(), 0, 0, &error_msg);
		if (rc != SQLITE_OK) {
			par_debug("%s\n", error_msg);
			sqlite3_free(error_msg);
			return updated;
		}
		else {
			updated = 1;
		}
	}
	return updated;
}/*
//////////////////////////////////////////////////////////////////////////////////
//Retrieves the natural name of the property in the database that the current object
//has.
/////////////////////////////////////////////////////////////////////////////////
std::string
Actionary::getPropertyName(MetaObject* obj, std::string tab_name){

	//Made this a recursive call so we can search parents properties
	bool found=false;
	std::string prop_name;
	std::stringstream query;
	sql::PreparedStatement *pstmt=NULL;
	sql::ResultSet *res=NULL;
	try{
		query << "select name_value from obj_prop," << tab_name << " where prop_value=id_value  AND obj_id = (?) and table_name=(?) LIMIT 1";
		pstmt = con->prepareStatement(query.str());

		while (!found && obj != NULL){
			pstmt->setInt(1, obj->getID());
			pstmt->setString(2, tab_name);
			res = pstmt->executeQuery();
			if (!res->next())
				obj = obj->getParent();
			else{
				prop_name = res->getString(1);
				found = true;
			}
		}
	}
	catch (sql::SQLException &e){
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete pstmt;
	delete res;
	return prop_name;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Retrieves the numerical property of the value at a given location
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int
Actionary::getPropertyValue(MetaObject* obj, std::string tab_name){
	//This is now also a recursive call
	if(obj == NULL)
		return -1;
	std::stringstream query;
	query << "select prop_value from obj_prop where obj_id =(?) and table_name='"<<tab_name<<"'";
	sql::PreparedStatement *pstmt=NULL;
	sql::ResultSet *res=NULL;
	int val=-1;
	try{
		pstmt = con->prepareStatement(query.str());
		while (obj != NULL && val < 0){
			pstmt->setInt(1, obj->getID());
			res = pstmt->executeQuery();
			if (res->next())
				val = res->getInt(1);
			else{
				obj = obj->getParent();
			}
		}
	}
	catch (sql::SQLException &e){
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete pstmt;
	delete res;
	return val;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Removes a property from a given object type, if it has one. We will assume that this is being called from
//meta-object and not from another part of the program (as this just removes it from the database).
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
Actionary::removeProperty(MetaObject*obj,std::string tab_name){
	if(obj == NULL)
		return;
	std::stringstream query;
	query <<"Delete from obj_prop WHERE obj_id = "<<obj->getID()<<" and table_name ='"<<tab_name<<"'";
	sql::Statement *stmt=NULL;
	try{
		stmt = con->createStatement();
		stmt->execute(query.str());
	}
	catch (sql::SQLException &e){
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete stmt;
}
*/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Retrieves the name of a property from a table.  This looks at the id-name tables instead of the property
//table, and so is object independent.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::string
Actionary::getPropertyNameByValue(const std::string& tab_name,int value){
	std::string res;
	parProperty *prop = this->searchByNameProperty(tab_name);
	if (prop != NULL){
		res=prop->getPropertyNameByValue(value);
		if(res.compare(""))
			return res;
	}
	std::stringstream query;
	query<<"SELECT name_value from "<<tab_name<<" WHERE id_value = "<<value <<" LIMIT 1";
	//sqlite3* db = con.get();
	char* error_msg;
	int rc = 0;
	rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
		return "";
	}
	return res;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Retrieves the name of a property from a table.  This looks at the id-name tables instead of the property
//table, and so is object independent.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int
Actionary::getPropertyValueByName(const std::string& tab_name,const std::string& prop_name){
	int prop_id = -1;
	parProperty *prop = this->searchByNameProperty(tab_name);
	if(prop != NULL){
		prop_id=prop->getPropertyValueByName(prop_name);
		if(prop_id > -1)
			return prop_id;
	}
	std::stringstream query;
	query<<"Select id_value from "<<tab_name<<" where name_value='"<<prop_name<<"' LIMIT 1";
	//sqlite3* db = con.get();
	char* error_msg;
	int rc = 0;
	std::string res;
	rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
		return prop_id;
	}
	if (res != "") {
		prop_id = atoi(res.c_str());
	}
	return prop_id;
}
///////////////////////////////////////////////////////////////////////////////
//This allows the user to check if the given property table is actually a table
//We will assume that no tables will be added to the db from outside the sytem
//during simulation, so that we only have to check the property table once
///////////////////////////////////////////////////////////////////////////////
parProperty*
Actionary::searchByNameProperty(const std::string& tab_name){
	parProperty *prop = NULL;
	for (std::map<int, parProperty*>::const_iterator it = properties.begin(); it != properties.end() && prop ==NULL; it++){
		if (!strcmp((*it).second->getPropertyName().c_str(), tab_name.c_str()))
			prop = (*it).second;
	}
	return prop;
}
/*
///////////////////////////////////////////////////////////////////////////////
//Finds all the properties an object has within the database, and returns them
//as a map
///////////////////////////////////////////////////////////////////////////////
std::map<parProperty*,int>
Actionary::getAllProperties(MetaObject *obj){
	std::map<parProperty*,int> properties;
	sql::Statement *stmt=NULL;
	sql::ResultSet* res = NULL;
	if (obj != NULL){
		try{
			std::stringstream query;
			query << "SELECT table_name,prop_value from obj_prop WHERE obj_id =" << obj->getID();
			stmt = con->createStatement();
			res = stmt->executeQuery(query.str());
			while (res->next()){
				parProperty *prop = this->searchByNameProperty(res->getString(1));
				if (prop != NULL)
					properties[prop] = res->getInt(2);
			}
		}
		catch (sql::SQLException &e) {
			par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
		}
		delete stmt;
		delete res;
	}
	return properties;
}
*/
///////////////////////////////////////////////////////////////////////////////
//Determines if a table exists in the database
//////////////////////////////////////////////////////////////////////////////
bool 
Actionary::hasTable(std::string tab_name){
	std::stringstream query;
	bool found = false;
	//query<<"SELECT TABLE_NAME as tab_name from information_schema.tables WHERE TABLE_NAME ='"<<tab_name<<"'";
	query << "SELECT name FROM sqlite_master WHERE type='table' AND  name ='" << tab_name << "'";
	//sqlite3* db = con.get();
	char* error_msg;
	std::string res;
	int rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
		return found;
	}
	if (res != "")
		found = true;


	return found;
}
////////////////////////////////////////////////////////////////////////////////
//Determines if the property is integer or string based on it's attributes in the
//cached list or table
int
Actionary::getPropertyTypeByName(const char* tab_name){
	parProperty *prop = this->searchByNameProperty(tab_name);
	if (prop != NULL)
		return prop->isInt();
	int found = -1;//changed this to -1 because many other failures rely on -1
	std::stringstream query;
	query << "SELECT is_int from property_type WHERE prop_name ='" << tab_name << "'";

	//sqlite3* db = con.get();
	char* error_msg;
	std::string res;
	int rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
	else {
		if (res != "")
			found = atoi(res.c_str());
	}
	return found;
}
///////////////////////////////////////////////////////////////////////
//Gets a property for an action
parProperty*
Actionary::getProperty(MetaObject* act, int which){
	parProperty * found = NULL;
	std::stringstream query;
	query << "SELECT table_id from obj_prop WHERE obj_id = " << act->getID() << " LIMIT " << which << ",1";
	//sqlite3* db = con.get();
	char* error_msg;
	std::string res;
	int rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
	else {
		if (res != "") {
			int prop_id = atoi(res.c_str());
			if (this->properties.find(prop_id) != this->properties.end()) {
				found = this->properties[prop_id];
			}
		}
	}

	return found;
}
//////////////////////////////////////////////////////////////////////
//We can have multiple properties for an action, so we need to know how
//many of those properties we have
int
Actionary::getNumProperties(MetaObject* act){
	int found = -1;
	std::stringstream query;
	query << "SELECT COUNT(*) from (SELECT table_id FROM obj_prop WHERE obj_id = " << act->getID() << " GROUP BY table_id) as table_counter";
	//sqlite3* db = con.get();
	char* error_msg;
	std::string res;
	int rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
	else {
		if (res != "")
			found = atoi(res.c_str());
	}
	return found;
}
///////////////////////////////////////////////////////////////////////
//Gets a property for an action
int
Actionary::getProperty(MetaObject* act, parProperty* prop, int which){
	int found = -1;
	std::stringstream query;
	query << "SELECT prop_value from obj_prop WHERE obj_id = " << act->getID() << " AND table_id = " << prop->getPropertyID() << " LIMIT " << which << ",1";
	//sqlite3* db = con.get();
	char* error_msg;
	std::string res;
	int rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
	else {
		if (res != "")
			found = atoi(res.c_str());
	}
	return found;
}
//////////////////////////////////////////////////////////////////////
//We can have multiple properties for an action, so we need to know how
//many of those properties we have
int
Actionary::getNumProperties(MetaObject* act, parProperty *prop){
	int found = -1;
	std::stringstream query;
	query << "SELECT COUNT(*) from obj_prop WHERE obj_id = " << act->getID() << " AND table_id = " << prop->getPropertyID();
	//sqlite3* db = con.get();
	char* error_msg;
	std::string res;
	int rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
	else {
		if (res != "")
			found = atoi(res.c_str());
	}
	return found;
}
///////////////////////////////////////////////////////////////////////
//Gets a property for an action
parProperty*
Actionary::getProperty(MetaAction* act,  int which){
	parProperty * found = NULL;
	std::stringstream query;
	query << "SELECT table_id from act_prop WHERE act_id = " << act->getID() << " LIMIT " << which << ",1";
	//sqlite3* db = con.get();
	char* error_msg;
	std::string res;
	int rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
	else {
		if (res != "" && this->properties.find(atoi(res.c_str())) != this->properties.end()) {
			found = this->properties[atoi(res.c_str())];
		}
	}
	return found;
}
//////////////////////////////////////////////////////////////////////
//We can have multiple properties for an action, so we need to know how
//many of those properties we have
int
Actionary::getNumProperties(MetaAction* act){
	int found = -1;
	std::stringstream query;
	query << "SELECT COUNT(*) from (SELECT table_id FROM act_prop WHERE act_id = " << act->getID() << " GROUP BY table_id) as table_counter";
	//sqlite3* db = con.get();
	char* error_msg;
	std::string res;
	int rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
	else {
		if (res != "")
			found = atoi(res.c_str());
	}
	return found;
}
///////////////////////////////////////////////////////////////////////
//Gets a property for an action
int 
Actionary::getProperty(MetaAction* act, parProperty* prop, int which){
	int found = -1;
	std::stringstream query;
	query << "SELECT prop_value from act_prop WHERE act_id = " << act->getID() << " AND table_id = "<<prop->getPropertyID()<<" LIMIT "<<which<<",1";
	//sqlite3* db = con.get();
	char* error_msg;
	std::string res;
	int rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
	else {
		if (res != "")
			found = atoi(res.c_str());
	}
	return found;
}
//////////////////////////////////////////////////////////////////////
//We can have multiple properties for an action, so we need to know how
//many of those properties we have
int 
Actionary::getNumProperties(MetaAction* act, parProperty *prop){
	int found = -1;
	std::stringstream query;
	query << "SELECT COUNT(*) from act_prop WHERE act_id = " << act->getID() << " AND table_id = " << prop->getPropertyID();
	//sqlite3* db = con.get();
	char* error_msg;
	std::string res;
	int rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
	else {
		if (res != "")
			found = atoi(res.c_str());
	}
	return found;
}
///////////////////////////////////////////////////////////////////////////////
// return this action's ID from the list of actions
int
Actionary::getActionID(const std::string& actName,bool reverse)
{
	if(!reverse){
		for(std::map<int,MetaAction*>::const_iterator it=actMap.cbegin(); it != actMap.cend(); it++)
			if(!(*it).second->getActionName().compare(actName))
				return (*it).first;
	}else{
		for(std::map<int,MetaAction*>::reverse_iterator it=actMap.rbegin(); it != actMap.rend(); it++)
			if(!(*it).second->getActionName().compare(actName))
				return (*it).first;
	}
	return -1;
}

///////////////////////////////////////////////////////////////////////////////
// Is this action already in the Actionary?. This is the way to check and see
//if the action exist in the database
bool
Actionary::isAction(const std::string& actName)
{
	// check to see if it already exists and if so print a message
	std::string query = "select act_id from action where act_name = '" +actName +"'";
	bool found = false;
	//par_debug("In isAction constructor, query is %s\n", queryStr.c_str());
	//sqlite3* db = con.get();
	char* error_msg;
	std::string res;
	int rc = sqlite3_exec(db, query.c_str(), sql_callback_single, &res, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
	else {
		if (res != "")
			found = true;
	}
	return found;
}

/////////////////////////////////////////////
// get a pointer to the MetaAction
MetaAction *
Actionary::searchByNameAct(const std::string& name,bool reverse)
{
	// get the id associated with this name
	int actID = getActionID(name,reverse);

	// return the MetaObject pointer stored in objVec/objMap
	return searchByIdAct(actID);
}
///////////////////////////////////////////
// get a pointer to the object from its id
MetaAction *
Actionary::searchByIdAct(int actID)
{
	if (actID < 0)
		return NULL;

	return actMap[actID];
}
///////////////////////////////////////////////////////////////////////////////
//Performs one step of a breadth first search on the actions
//Remember that a path exists from a child to parent, so the BFS actually goes
//up the FIDAG, not down it. Also, actions should be set with some initial root
//actions.push(root) before attempting the BFS
MetaAction*
Actionary::breadthFirstSearch(std::queue<MetaAction*>& actions){
	if (actions.empty()){
		return NULL;
	}
	MetaAction *result = actions.front();
	actions.pop();//Remove the action from the list
	for (int i = 0; i < result->getNumParents(); result++){
		actions.push(result->getParent(i));
	}
	return result;
}


//////////////////////////////////////////////
// create a new action
/////////////////////////////////////////////
int
Actionary::addAction(MetaAction *act, std::string actName)
{
		// add this action to the database
		// need a new action id and need to store the id in this MetaAction
		int actID = getNextActID();
		std::stringstream query;
		query << "INSERT INTO action (`act_id`,`act_name`) VALUES (";
		query << actID << ",";
		query<<actName<<")";
		//sqlite3* db = con.get();
		char* error_msg;
		std::string res;
		int rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
		if (rc != SQLITE_OK) {
			par_debug("%s\n", error_msg);
			sqlite3_free(error_msg);
			throw iPARException("Error in Actionary adding an Action");
		}
		actMap[actID] = act;
		return actID;
}
///////////////////////////////////////////////////////////////////////
// Adds an action and assigns it's parent into the database.
/////////////////////////////////////////////////////////////////////
int
Actionary::addAction(MetaAction* act,std::string name, std::vector<MetaAction*> parents){

		int actID = getNextActID();
		std::stringstream query;

		query << "INSERT INTO action (`act_id`,`act_name`,`parent_act`) VALUES ("
			  << actID<<","<<name<<","<< ");";//Insert the action
		for (int i = 0; i < parents.size(); i++) {
			query << "INSERT INTO action_parent VALUES(" << actID << "," << parents[i]->getID() << ");";
		}
		//sqlite3* db = con.get();
		char* error_msg;
		int rc = sqlite3_exec(db, query.str().c_str(), 0, 0, &error_msg);
		if (rc != SQLITE_OK) {
			par_debug("%s\n", error_msg);
			sqlite3_free(error_msg);
		}
		actMap[actID] = act;
		return actID;
}
///////////////////////////////////////////////////////////////////////
// create a metaaction
// 
/////////////////////////////////////////////////////////////////////
MetaAction *
Actionary::create(char* aname, MetaAction* aparent)
{
	MetaAction* act=NULL;
	if(aparent == NULL)
	{
		MetaAction* parent=this->searchByNameAct("Root");
		if(parent == NULL)
			if(strcmp(aname,"Root") !=0)
				parent=create("Root",NULL);
			else
				 act=new MetaAction(aname);
		else
			parent=create(aname,parent);
	}
	else
		 act = new MetaAction(aname,aparent);  // adds it to the database too

	if (act == NULL)
		std::cout << "ERROR: action not created in Actionary::create" << std::endl;

   createPyActions(act); // create the Python rep for each actions
    return act;
}

// get the action name
std::string
Actionary::getActionName(MetaAction *act)
{
	if (act == NULL)
		return "";
	std::stringstream query;
	query << "select act_name from action where act_id =" << act->getID();
	//sqlite3* db = con.get();
	char* error_msg;
	std::string res;
	int rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
	return res;

}

// set the action name
void
Actionary::setActionName(MetaAction *act, std::string newName)
{
	if (act == NULL)
		return;
	std::stringstream query;
	query << "update action set act_name = '" << newName << "' where act_id = " << act->getID();
	//sqlite3* db = con.get();
	char* error_msg;
	int rc = sqlite3_exec(db, query.str().c_str(), 0, 0, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
}

//////////////////////////////////////////////////
// set the parent of the action
// i.e. place the action in the hierarchy
void
Actionary::setParent(MetaAction *act, MetaAction *parent)
{
	if (parent == NULL){
		par_debug("ERROR: null parent pointer in Actionary::setParent\n");
		return;
	}
	if(act == NULL){
		par_debug("Error: null object in Actionary::setParent\n");
		return;
	}

	int parentID = parent->getID();  
	int actID = act->getID();
	std::stringstream query;
	query << "INSERT INTO action_parent VALUES(" << actID << "," << parentID << ")";
	//sqlite3* db = con.get();
	char* error_msg;
	int rc = sqlite3_exec(db, query.str().c_str(), 0, 0, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
	act->setParent(parent);//This keeps things consistant
}

///////////////////////////////////////////////////////////////////////////////
//The actionary's getParent function should only be called when we are building
//the object tree, since these variables are cached
MetaAction*
Actionary::getParent(MetaAction* act,int which)
{
	if (act == NULL)
		return NULL;
	MetaAction *pact = NULL;
	std::stringstream query;
	query << "select parent_id from action_parent where act_id = " << act->getID() << " ORDER BY parent_id LIMIT " << which << ",1";;
	//sqlite3* db = con.get();
	char* error_msg;
	std::string res;
	int rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
	if (res != ""){
		int val = atoi(res.c_str());
		std::map<int, MetaAction*>::iterator finder = this->actMap.find(val);
		if (finder != actMap.end()){
			pact = this->searchByIdAct(val);
		}
		else{
			pact = new MetaAction(val);
			if (!strcmp("", pact->getActionName().c_str()))
				par_debug("Error creating action with id %d\n", val);
			else{
				par_debug("Action name is %s\n", act->getActionName().c_str());
				actMap[val] = pact;
				createPyActions(pact); // create the Python rep for each actions
				// load all of the python conditions and specs too
				//par_debug("Loading the action functions\n");
				loadApplicabilityCond(pact);
				loadCulminationCond(pact);
				loadPreparatorySpec(pact);
				loadExecutionSteps(pact);
			}//If we don't find the parent, then it may be that we haven't created it yet, which is a problem
		}
	}
	return pact;
}

int
Actionary::getNumParents(MetaAction* act)
{
	if (act == NULL)
		return 0;
	int pact = 0;
	std::stringstream query;
	query << "SELECT COUNT(*) FROM action_parent where act_id = " << act->getID();
	//sqlite3* db = con.get();
	char* error_msg;
	std::string res;
	int rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
	else {
		if (res != "") {
			pact = atoi(res.c_str());
		}
	}
	return pact;
}


void
Actionary::setApplicabilityCond(MetaAction* act, const std::string& appCond)
{

	if (act == NULL)
		return;
	int actID = act->getID();
	std::stringstream query;
	query << "update action set act_appl_cond = '" << appCond << "' where act_id = " << actID;
	//sqlite3* db = con.get();
	char* error_msg;
	int rc = sqlite3_exec(db, query.str().c_str(), 0, 0, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
	// load it in python too
	loadApplicabilityCond(act);
}

std::string
Actionary::getApplicabilityCond(MetaAction* act)
{
	if (act == NULL)
		return " ";
	std::stringstream query;
	std::string res = " ";
	query << "select act_appl_cond from action where act_id =" << act->getID();
	//sqlite3* db = con.get();
	char* error_msg;
	int rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
	return res;
}

void
Actionary::setCulminationCond(MetaAction* act, const std::string& termCond)
{
	if (act == NULL)
		return;
	std::stringstream query;
	query << "update action set act_term_cond = '" << termCond << "' where act_id = " << act->getID();
	//sqlite3* db = con.get();
	char* error_msg;
	int rc = sqlite3_exec(db, query.str().c_str(), 0, 0, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
	loadCulminationCond(act);
}
std::string
Actionary::getCulminationCond(MetaAction* act){
	if (act == NULL)
		return " ";
	std::stringstream query;
	std::string res;
	query << "select act_term_cond from action where act_id  =" << act->getID();
	//sqlite3* db = con.get();
	char* error_msg;
	int rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
	return res;
}

void
Actionary::setPreparatorySpec(MetaAction* act, const std::string& prepSpec)
{
	if (act == NULL)
		return;
	int actID = act->getID();
	std::stringstream query;
	query << "update action set act_prep_spec = '" << prepSpec << "' where act_id = " << actID;
	//sqlite3* db = con.get();
	char* error_msg;
	int rc = sqlite3_exec(db, query.str().c_str(), 0, 0, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
	loadPreparatorySpec(act);
}

std::string
Actionary::getPreparatorySpec(MetaAction* act){
	if (act == NULL)
		return " ";
	std::stringstream query;
	std::string res;
	query << "select act_prep_spec from action where act_id  =" << act->getID();
	//sqlite3* db = con.get();
	char* error_msg;
	int rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
	return res;
}

void
Actionary::setExecutionSteps(MetaAction* act, const std::string & execSteps)
{
	if (act == NULL)
		return;
	int actID = act->getID();
	std::stringstream query;
	query << "update action set act_exec_steps = '" << execSteps << "' where act_id = " << actID;
	//sqlite3* db = con.get();
	char* error_msg;
	int rc = sqlite3_exec(db, query.str().c_str(), 0, 0, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
	loadExecutionSteps(act);
}

std::string
Actionary::getExecutionSteps(MetaAction* act){
	if (act == NULL)
		return " ";
	std::stringstream query;
	std::string res;
	query << "select act_exec_steps from action where act_id  =" << act->getID();
	//sqlite3* db = con.get();
	char* error_msg;
	int rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
	return res;
}

void
Actionary::setPurposeAchieve(MetaAction* act, const std::string& achieve)
{
	if (act == NULL)
		return;
	int actID = act->getID();
	std::stringstream query;
	query << "update action set act_purpose_achieve = '" << achieve << "' where act_id = " << actID;
	//sqlite3* db = con.get();
	char* error_msg;
	int rc = sqlite3_exec(db, query.str().c_str(), 0, 0, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
}

std::string
Actionary::getPurposeAchieve(MetaAction* act){
	if (act == NULL)
		return "";
	std::stringstream query;
	query << "select act_purpose_achieve from action where act_id IN (";  
	while (act != NULL){
		query << act->getID();
		act = act->getParent();
		if (act != NULL) {
			query << ",";
		}
	}
	query << ") LIMIT 1 ORDER BY act_id DESC";
	//sqlite3* db = con.get();
	char* error_msg;
	std::string res;
	int rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
	return res;
}
///////////////////////////////////////////////////////////////////////////////
//Determines all objects with a given purpose, if any of them exist
///////////////////////////////////////////////////////////////////////////////
std::vector<MetaAction*>
Actionary::getAllPurposed(const std::string& purpose){
	std::stringstream query;
	std::vector<MetaAction*> vec;
	std::map<int, std::string > data;
	query << "SELECT act_id from action WHERE act_purpose_achieve ='" << purpose << "'";
	//sqlite3* db = con.get();
	char* error_msg;
	int rc = sqlite3_exec(db, query.str().c_str(), sql_mrsi_callback, &data, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
	for(std::map<int,std::string>::const_iterator it = data.begin(); it != data.end(); it++){
		MetaAction *act = this->searchByIdAct(atoi((*it).second.c_str()));
		if (act != NULL)
			vec.push_back(act);
	}
	return vec;
}
///////////////////////////////////////////////////////////////////////////////
//Returns the maximum number of objects that a metaobject can hold
///////////////////////////////////////////////////////////////////////////////
int
Actionary::getNumObjects(MetaAction* act){
	std::stringstream query;
	int result = -1;
		
	query << "SELECT MAX(obj_num)+1 as num_obj from obj_act WHERE act_id IN ("; 
	MetaAction *finder = act;
	while (finder != NULL){
		query<<finder->getID();
		finder = finder->getParent();
		if (finder != NULL) {
			query << ",";
		}
	}
	query << ") ORDER BY act_id DESC LIMIT 1"; //This is grabbing the max from all of them and therefore does not need a group by. However, 
	//sqlite3* db = con.get();
	char* error_msg;
	std::string res;
	int rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
	else {
		if (res != "") {
			result = atoi(res.c_str());
		}
	}
	return result;
}

void
Actionary::addAffordance(MetaAction* act, MetaObject* obj, int position){

	if(act == NULL || obj == NULL)
		return;
	if(position < 0 || position > act->getNumObjects())
		return;
	if(obj->isInstance())
		addAffordance(act,obj->getParent(),position);//This keeps us from setting affordances on object instances
	int exist=searchAffordance(act,obj);
	if(exist == position)//In this case, it's already in there
		return;
	std::stringstream query;
	query << "INSERT INTO obj_act VALUES(" << obj->getID() << "," << act->getID() << "," << position << ");";
	//sqlite3* db = con.get();
	char* error_msg;
	int rc = sqlite3_exec(db, query.str().c_str(), 0, 0, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
}

void
Actionary::removeAffordance(MetaAction *act, MetaObject *obj, int obj_num){
	if(act == NULL || obj == NULL)
		return;
	//Which's number doesn't matter, as if which doesn't exist, the query just will delete zero rows
	std::stringstream query;
	query << "DELETE from obj_act WHERE obj_id = " << obj->getID() << " and act_id = " << act->getID() << " and obj_num = " << obj_num;
	//sqlite3* db = con.get();
	char* error_msg;
	int rc = sqlite3_exec(db, query.str().c_str(), 0, 0, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
}
///////////////////////////////////////////////////////////////////
//Grabs the first position a given object is in.  -1 is a failure
/////////////////////////////////////////////////////////////////
int
Actionary::searchAffordance(MetaAction* act, MetaObject* obj){
	int result=-1;
	if (act != NULL && obj != NULL) {
		std::stringstream query;
		query << "SELECT obj_num from obj_act WHERE obj_id = " << obj->getID() << " and act_id = " << act->getID() << " LIMIT 1";
		//sqlite3* db = con.get();
		char* error_msg;
		std::string res;
		int rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
		if (rc != SQLITE_OK) {
			par_debug("%s\n", error_msg);
			sqlite3_free(error_msg);
		}
		else {
			if (res != "") {
				result = atoi(res.c_str());
			}
		}
	}
	return result;
}
////////////////////////////////////////////////////////////////////////////////
//Gets the number of affordances for a given predicate of a given action
///////////////////////////////////////////////////////////////////////////////
int  Actionary::getNumAffordance(MetaAction* act, int position) {
	if (act == NULL || position < 0)
		return -1;
	std::stringstream query;
	int result = 0;

	query << "SELECT COUNT(obj_id) as counter from obj_act WHERE act_id IN ("; 
	MetaAction *finder = act;
	while (finder != NULL) { //Since we have multiple parents, this should be a breadth first search
			query<< finder->getID();
			finder = finder->getParent();
			if (finder != NULL) {
				query << ",";
			}
		}
	query << ") and obj_num=" << position << " GROUP BY act_id ORDER BY act_id DESC LIMIT 1";
	char* error_msg;
	std::string res;
	int rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
	else {
		if (res != "") {
			result = atoi(res.c_str());
		}
	}
	return result;
}
//////////////////////////////////////////////////////////////
//Grabs the which'th action that can be used with a given object
//in a given position (NULL for failure)
//////////////////////////////////////////////////////////////
MetaAction*
Actionary::searchAffordance(MetaObject* obj, int position, int which){
	if(obj == NULL || position < 0 || which < 0)
		return NULL;
	std::stringstream query;
	MetaAction *act = NULL;

	query << "SELECT act_id from obj_act WHERE obj_id IN (";
	MetaObject *finder = obj;
	while (finder != NULL ){
		query << finder->getID();
		finder = finder->getParent();
		if (finder != NULL) {
			query << ",";
		}
	}
	query << ") and obj_num=" << position << " ORDER BY act_id DESC " << " LIMIT " << which << ",1"; //I should probably put an order by here
	//sqlite3* db = con.get();
	char* error_msg;
	std::string res;
	int rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
	else {
		if (res != "") {
			int act_id = atoi(res.c_str());
			act = this->searchByIdAct(act_id);
		}
	}
	return act;
}
//////////////////////////////////////////////////////////////
//Grabs the which'th object that can be used with a given object
//in a given position (NULL for failure)
//////////////////////////////////////////////////////////////
MetaObject*
Actionary::searchAffordance(MetaAction* act, int position, int which){
	if(act == NULL || position < 0 || which < 0)
		return NULL;
	std::stringstream query;
	MetaObject *obj = NULL;
	query << "SELECT obj_id from obj_act WHERE act_id IN ("; 
	MetaAction *finder = act;
	while (finder != NULL) {
		query << finder->getID();
		finder = finder->getParent();
		if (finder != NULL) {
			query << ",";
		}
	}
	query << ") and obj_num=" << position << " LIMIT " << which << ",1";
	//sqlite3* db = con.get();
	char* error_msg;
	std::string res;
	int rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
	else {
		if (res != "") {
			int obj_id = atoi(res.c_str());
				obj = this->searchByIdObj(obj_id);
			}
		}
	return obj;
}


std::string
Actionary::getAffordancePositionName(MetaAction*act, int pos) {
	if (act == NULL) {
		return "";
	}
	std::stringstream query;
	query << "SELECT position_name from affordance_description WHERE act_id = " << act->getID() << " AND pos = " << pos << " LIMIT 1";
	char* error_msg;
	std::string res;
	int rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
	else {
		return res;
		}
}


////////////////////////////////////////////////////////////
// remove an object from the database and the actionary
void
Actionary::removeAction(MetaAction *act)
{
	if (act == NULL)
		return;
	int actID = act->getID(); //getActionID(act->getActionName());
	if (actID < 0)
	{
		std::cout << "ERROR: not a valid action in removeAction" << std::endl;
		return;
	}
	// remove it from the actVec
	actMap.erase(actID);

	// kill the pointer
	delete act;
	act = NULL;

	// remove it from all of the tables
	removeAction(actID);
}


// remove an action from the database only
void
Actionary::removeAction(int actID)
{
	if (actID < 0)
		return;
	std::stringstream query;
	query << "delete from action where act_id = " << actID << ";";
	query << "delete from adverb_exp where act_id = " << actID << ";";
	query << "delete from obj_act where act_id = " << actID << ";";
	//sqlite3* db = con.get();
	char* error_msg;
	int rc = sqlite3_exec(db, query.str().c_str(), 0, 0, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
}



float
Actionary::getDuration(MetaAction* act)
{
	if (act == NULL)
		return -999;
	float val = -999;
	std::stringstream query;
	query << "select act_dur_time_id from action where act_id = " << act->getID();
	//sqlite3* db = con.get();
	char* error_msg;
	std::string res;
	int rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
	else {
		if (res != "") {
			val = (float)atof(res.c_str());
		}
	}
	return val;
}

void
Actionary::setDuration(MetaAction* act, float d)
{
	if (d < 0)
		std::cout << "ERROR: duration should not be negative" << std::endl;
	if (act == NULL)
		return;
	std::stringstream query;
	query << "update action set act_dur_time_id = " << d << " where act_id = " << act->getID();
	//sqlite3* db = con.get();
	char* error_msg;
	int rc = sqlite3_exec(db, query.str().c_str(), 0, 0, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
}




std::string
Actionary::getAdverb(MetaAction* act)	// later allow for more than one adverb
{
	if (act == NULL)
		return " ";
	std::stringstream query;
	std::string val = "";
	query << "select adverb_name from adverb_exp where act_id = " << act->getID();
	//sqlite3* db = con.get();
	char* error_msg;
	int rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &val, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
	return val;
}

std::string
Actionary::getModifier(MetaAction* act)	// later allow for more than one modifier
{
	if (act == NULL)
		return " ";
	std::stringstream query;
	std::string val = "";
	query << "select adverb_mod_name from adverb_exp where act_id = " << act->getID();
	//sqlite3* db = con.get();
	char* error_msg;
	int rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &val, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
	return val;
}

void
Actionary::setAdverb(MetaAction* act, const std::string& adverb, const std::string& modifier)
{
	if (act == NULL)
		return;
	std::stringstream query;
	query<<"INSERT INTO adv_exp VALUES("<<act->getID()<<","<<adverb<<","<<modifier<<");";
	//sqlite3* db = con.get();
	char* error_msg;
	int rc = sqlite3_exec(db, query.str().c_str(), 0, 0, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
}



std::string 
Actionary::extractParentName(const std::string& name){
	// this is not an instance
	size_t found=name.find('_');
	if (name.find('_') == std::string::npos)
		return name;
	return name.substr(0,found);
}

int
Actionary::getSiteType(MetaAction *act){
	if (act == NULL)
		return -1;
	MetaAction* looper=act;
	std::stringstream query;
	int val = -1;
	query << "select act_site_type_id from action where act_id IN ("; 
	while (looper != NULL){
		query << looper->getID();
		looper = looper->getParent();
		if (looper != NULL) {
			query << ",";
		}
	}
	query << ") ORDER BY act_id DESC LIMIT 1";
	//sqlite3* db = con.get();
	char* error_msg;
	std::string res;
	int rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
	else {
		if (res != "") {
			val = atoi(res.c_str());
		}
	}
	return val;
}





MetaObject*
Actionary::getAllAgents(int which){
	if (which < (int)this->allAgents.size() && which > -1)
		return this->searchByIdObj(this->allAgents[which]);
	return NULL;
}

MetaObject*
Actionary::getAllRooms(int which){
	if (which < (int)this->allRooms.size() && which > -1)
		return this->searchByIdObj(this->allRooms[which]);

	return NULL;
}

// not rooms and only instances
MetaObject*
Actionary::getAllObjects(int which){
	if (which < (int)this->allObjects.size() && which > -1)
		return this->searchByIdObj(this->allObjects[which]);

	return NULL;
}

MetaAction*
Actionary::getAllActions(int which){
	//par_debug("Which is %d\n", which);
	if (which < (int) this->allActions.size() && which > -1)
		return this->searchByIdAct(this->allActions[which]);

	return NULL;
}


MetaObject*
Actionary::getAllTypes(int which){
	//Added in the check that which cannot be less then zero
	if (which < (int)this->allTypes.size() && which > -1)
		return this->searchByIdObj(this->allTypes[which]);

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////
//This adds an iPAR to the iparMap
///////////////////////////////////////////////////////////////////////////////
void
Actionary::addiPAR(int ipar_id,iPAR *ipar){

	if(ipar_id < 0)
		return;
	else
		parMap[ipar_id]=ipar;

}
///////////////////////////////////////////////////////////////////////////////
//Searches the iPAR map by the id, so it's just a simple lookup
///////////////////////////////////////////////////////////////////////////////
iPAR*
Actionary::searchByIdiPAR(int ipar_id){
	if(ipar_id < 0)
		return NULL;
	return parMap[ipar_id];
}
///////////////////////////////////////////////////////////////////////////////
//This grabs the agent from the agent table, and searches through the agent's
//iPAR list to find the first instance of an iPAR with the same name
///////////////////////////////////////////////////////////////////////////////
iPAR*
Actionary::searchByNameAgentiPAR(const std::string& ag_name, const std::string& ipar_name){
	AgentProc *agent =agentTable.getAgent(ag_name);
	if(agent == NULL)
		return NULL;

	return agent->searchiPAR(ipar_name);//If we can remove the dependency on agentProc, we really should
}

///////////////////////////////////////////////////////////////////////////////
//Adds a new shape site to the database, and set's a site to that shape-site
///////////////////////////////////////////////////////////////////////////////
bool
Actionary::createSiteShape(MetaObject *obj, int siteType, const char* shape_type, float first, float second, float third){
	if(shape_type == NULL || obj == NULL)
		return false;

	MetaObject* site_obj=searchGraspSites(obj,siteType);
	if(site_obj == NULL)
		return false;//Without a site, no need for a site/region/volume
	bool success = false;
	int site_shape_id=getSiteShapeID(obj,siteType);
	//sqlite3* db = con.get();
	char* error_msg;
	std::string res;
	std::stringstream query;
	if (site_shape_id < 0){//Create a site
		query << "SELECT MAX(site_shape_id)+1 from site_shape";
		int rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
		if (res!= "") {
			site_shape_id = atoi(res.c_str()); //We don't want to overwrite the data
			query.clear();
			query.str(std::string());
			query << "INSERT INTO site_shape_id VALUES (" << site_shape_id << "," << shape_type << ",";
			query << first << "," << second << "," << third << ");";
			sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
			if (rc != SQLITE_OK) {
				par_debug("%s\n", error_msg);
				sqlite3_free(error_msg);
			}
			else {
				success = true;
			}
		}	
	}
	else{
		query << "Update site_shape SET shape_type='" << shape_type << "',first_coord=" << first << ", second_coord=" << second << ", third_coord=" << third << " WHERE site_shape_id =" << site_shape_id;
		int rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
		if (rc != SQLITE_OK) {
			par_debug("%s\n", error_msg);
			sqlite3_free(error_msg);
		}
		else {
			success = true;
		}
	}
	return success;
}
////////////////////////////////////////////////////////////////////////////////
//Returns the site shape id
///////////////////////////////////////////////////////////////////////////////
int
Actionary::getSiteShapeID(MetaObject* obj, int siteType){
	if(obj == NULL || siteType < 0)
		return -1;
	int val=-1;
	std::stringstream query;
	query << "SELECT site_shape_id from site WHERE obj_id =" << obj->getID() << " and site_type_id =" << siteType;
	//sqlite3* db = con.get();
	char* error_msg;
	std::string res;
	int rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
	else {
		if (res != "") {
			val = atoi(res.c_str());
		}
	}


	return val;
}
///////////////////////////////////////////////////////////////////////////////
//This returns the type the site is based upon it's id (the next few are pretty
//simple)
///////////////////////////////////////////////////////////////////////////////
std::string
Actionary::getSiteShapeType(int site_shape_id){
	if(site_shape_id <0)
		return "";
	std::string res;
	std::stringstream query;
	query << "SELECT shape_type from site_shape WHERE site_shape_id = " << site_shape_id;
	//sqlite3* db = con.get();
	char* error_msg;
	int rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
	return res;
}
//first coord
float
Actionary::getSiteShapeCoordinate(int site_shape_id,int coordinate){
	if(site_shape_id <0)
		return 0.0f;

	std::stringstream query;
	switch(coordinate){
	case 1:
		query<<"SELECT first_coord from site_shape WHERE site_shape_id = "<<site_shape_id<<" LIMIT 1";
		break;
	case 2:
		query<<"SELECT second_coord from site_shape WHERE site_shape_id = "<<site_shape_id << " LIMIT 1";
		break;
	case 3:
		query<<"SELECT third_coord from site_shape WHERE site_shape_id = "<<site_shape_id << " LIMIT 1";
		break;
	default:
		return 0.0f;
	}
	float val = 0.0f;
	//sqlite3* db = con.get();
	char* error_msg;
	std::string res;
	int rc = sqlite3_exec(db, query.str().c_str(), sql_callback_single, &res, &error_msg);
	if (rc != SQLITE_OK) {
		par_debug("%s\n", error_msg);
		sqlite3_free(error_msg);
	}
	else {
		if (res != "") {
			val = (float)atof(res.c_str());
		}
	}
	return val;

}
////////////////////////////////////////////////////////////////////////////////