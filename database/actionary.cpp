
#include <cppconn/driver.h>
#include <cppconn/connection.h>
#include <cppconn/exception.h>
#include <cppconn/statement.h>
#include <cppconn/resultset.h>
#include <cppconn/prepared_statement.h>
#include <fstream>
#include <time.h>
#include <sstream>

#include "actionary.h"
#include "par.h"
#include "interpy.h"
#include "agentproc.h"
#include "utility.h"




extern AgentTable agentTable;
extern parTime *partime;

std::unique_ptr<sql::Connection> con; //Move the connection to a global so we don't have the dependency in the .h file
	
void
Actionary::init()
{
	maxObjID  = 0;
	maxActID  = 0;
	maxPARID  = 0;
	try{
		sql::Driver *driver = get_driver_instance();
		con.reset(driver->connect("localhost","root","root"));
		con->setClientOption("libmysql_debug", "d:t:O,client.trace");
		con->setSchema("openpardb");
		par_debug("Connection established\n");
	}
	catch (sql::SQLException &e) {
		par_debug("ERROR in Actionary::init, database connection not established:%d\n",e.getErrorCode());
		return;
	}
	Py_Initialize();
	initprop();
	sql::Statement *query=NULL;
	sql::ResultSet *res = NULL;
	sql::ResultSet *res2 = NULL;
	// populate the object vector with pointers and ids
	query = con->createStatement();
	try{
		res2 = query->executeQuery("select prop_id,prop_name,is_int,omega from property_type order by prop_id");
		while (res2->next()){
			parProperty *prop = new parProperty(res2->getInt("prop_id"), res2->getString("prop_name"), res2->getInt("is_int"), res2->getInt("omega"));
			properties[prop->getPropertyID()] = prop;
		}
		res = query->executeQuery("select obj_id,is_agent,obj_name from object order by obj_id");
		while (res->next()){
			//Not using objStruct because the old trees have been gone for quite some time
			bool agent = false;
			maxObjID = res->getInt("obj_id");
			if (res->getInt("is_agent") == 1)
				agent = true;
			par_debug("object name is %s\n", res->getString("obj_name").c_str());
			objMap[maxObjID] = new MetaObject(res->getString("obj_name").c_str(), agent, false);
			
		}
		//This back-propigates known semantics up the object hierarchy
		for (int j = (int)objMap.size() - 1; j>0; j--){
			if (this->objMap[j]->getParent() != NULL){
				this->objMap[j]->getParent()->setAllProperties(this->objMap[j]->getAllProperties());
			}
		}
		delete res;
	}
	catch (sql::SQLException &e) {
		std::cerr << "ERROR in Actionary::init, error in reading objects from table:" << e.getErrorCode() << std::endl;
		return;
	}
	try{
		res2 = query->executeQuery("select act_id from action order by act_id");
		MetaAction *act;
		std::map<int, MetaAction*>::iterator finder;
		while (res2->next()){
			maxActID = res2->getInt("act_id");
			finder = actMap.find(maxActID);
			if (finder == actMap.end()){
				act = new MetaAction(maxActID);
				if (!strcmp("", act->getActionName().c_str()))
					par_debug("Error creating action with id %d\n", maxActID);
				else{
					par_debug("Action name is %s\n", act->getActionName().c_str());
					actMap[maxActID] = act;
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
		delete res2;
	}
	catch (sql::SQLException &e) {
		std::cerr << "ERROR in Actionary::init, error in reading actions from table:" << e.getErrorCode() << std::endl;
		return;
	}
	delete query;
	

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
		std::cout << "Just returning from Actionary::create" << std::endl;
		return obj;
	}

	obj = new MetaObject(obname.c_str(), agent);  // adds it to the database too
	if (obj == NULL)
		std::cout << "ERROR: object not created in Actionary::create" << std::endl;

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
		std::cout << "ERROR: not a valid object in removeObject" << std::endl;
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
	char* tables[]={"object","obj_act","obj_prop","site"};
	sql::Statement *stmt=NULL;
	stmt=con->createStatement();
	try{
		for (int i = 0; i < 4; i++){
			query.str(std::string());
			query.clear();
			query << "Delete from " << tables[i] << " where obj_id =" << objID;
			stmt->executeUpdate(query.str());
		}
	}
	catch (sql::SQLException &e) {

		std::cerr << "MySQL error:" << e.getErrorCode() << std::endl;
		return;
	}
	delete stmt;
}

//////////////////////////////////////////////////
// set the parent of the object
// i.e. place the object in the hierarchy
void
Actionary::setParent(MetaObject *obj, MetaObject *parent)
{
	if (parent == NULL)
		std::cout << "ERROR: null parent pointer in Actionary::setParent" << std::endl;

	sql::PreparedStatement *query = NULL;
	try{
		if (!parent->isInstance() && !obj->isInstance()){
			int parentID = getObjectID(parent->getObjectName());
			int objID = getObjectID(obj->getObjectName());

			query = con->prepareStatement("Update object set parent_id =(?) where obj_id=(?)");
			query->setInt(1, parentID);
			query->setInt(2, objID);
			query->executeUpdate();
		}
	}
	catch (sql::SQLException &e) {
		std::cerr << "MySQL error:" << e.getErrorCode() << std::endl;
		return;
	}
	delete query;
}

MetaObject*
Actionary::getParent(MetaObject *obj)
{
	if (obj == NULL)
		return NULL;

	std::stringstream query;
	sql::Statement *stmt=NULL;
	sql::ResultSet *res = NULL;
	MetaObject *pobj = NULL;
	try{
		query << "Select parent_id from object where obj_id =" << obj->getID();
		stmt = con->createStatement();
		res = stmt->executeQuery(query.str());
		if (res->next())
			pobj = this->searchByIdObj(res->getInt("parent_id"));
	}
	catch (sql::SQLException &e) {
		std::cerr << "MySQL error:" << e.getErrorCode() << std::endl;
	}
	delete stmt;
	delete res;
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
	for(std::map<int,MetaObject*>::iterator it=objMap.begin(); it != objMap.end(); it++){
		if((*it).second->parent == obj){
			if(counter == which)
				return (*it).second;
			else
				counter++;
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
	query<<"select obj_id from object where obj_name = '"<<objName<<"'";
	bool val = false;
	sql::Statement *stmt=NULL;
	sql::ResultSet *res = NULL;
	//par_debug("In isObject, query is %s\n", query.str().c_str());
	try{
		stmt = con->createStatement();
		res = stmt->executeQuery(query.str());
		
		if (res->next())
			val = true;
	}
	catch (sql::SQLException &e){
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete stmt;
	delete res;
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
	return false;
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

	return false;
}
//////////////////////////////////////////////
// create a new object
int
Actionary::addObject(MetaObject *obj, const std::string& objName, bool agnt,bool instance)
{
		// need a new object id and need to store the id in this MetaObject
		int objID = getNextObjID();
		// add this object to the database
		if(!instance){
			int isAgent = 0;
			if (agnt)
				isAgent = 1;
			sql::PreparedStatement *query = NULL;
			try{
				query = con->prepareStatement("INSERT INTO object (id,obj_name,is_agent,parent_id) VALUES(?,?,?,1)");
				query->setInt(1, objID);
				query->setString(2, objName);
				query->setInt(3, isAgent);
				query->execute();
				
			}
			catch (sql::SQLException &e){
				par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
			}
			delete query;
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
		for(unsigned int j=0; j<objMap.size(); j++){
			if(objMap[j] != NULL)
					if(!(objMap[j]->getObjectName().compare(objName)))
						return objMap[j]->getID();
		}
	}else{
		for(int j=(int)objMap.size()-1; j>0; j--)//Never count down from an unsigned int
			if(objMap[j] != NULL)
					if(!(objMap[j]->getObjectName().compare(objName)))
						return objMap[j]->getID();
	}
	sql::Statement *query=NULL;
	sql::ResultSet *res = NULL;
	int objID=-1;
	try{
		std::string queryStr = "select obj_id from object where obj_name = '" +objName + "'";
		query=con->createStatement();
		res=query->executeQuery(queryStr);
		if(res->next()){
			objID=res->getInt("obj_id");
		}
	}	catch(sql::SQLException &e){
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete res;
	delete query;
	return objID;
}

///////////////////////////////////////////////////////
// get the object name
std::string
Actionary::getObjectName(int id)
{
	sql::ResultSet *res = NULL;
	sql::PreparedStatement *query = NULL;
	try{
		query = con->prepareStatement("SELECT obj_name FROM object where obj_id=(?)");
		query->setInt(1, id);
		res = query->executeQuery();
		if (res->next())
			return res->getString("obj_name");
	}
	catch (sql::SQLException &e){
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete query;
	delete res;
	return NULL;
}

/////////////////////////////////////////////////////
// set the object name
void
Actionary::setObjectName(MetaObject *obj, std::string newName)
{
	if (obj == NULL)
		return;
	sql::PreparedStatement *query = NULL;
	try{
		query = con->prepareStatement("update object set obj_name = '" + newName + "' where obj_id = (?)");
		query->setInt(1, obj->getID());
		query->executeUpdate();
	}
	catch (sql::SQLException &e){
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete query;
}



/////////////////////////////////////////////
// get a pointer to the MetaObject
MetaObject *
Actionary::searchByNameObj(const std::string& name,bool reverse)
{
	// get the id associated with this name
	int objID = getObjectID(name,reverse);

	// return the MetaObject pointer stored in objVec/Map
	return searchByIdObj(objID);
}
///////////////////////////////////////////
// get a pointer to the object from its id
MetaObject *
Actionary::searchByIdObj(int objID)
{
	if (objID < 0)
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
	sql::Statement *stmt = NULL;
	std::stringstream query;
	try{
		query << "INSERT INTO site VALUES(";
		query << obj->getID() << ",";
		query << siteType << ",";
		query << sitePosX << "," << sitePosY << "," << sitePosZ << ",";
		query << siteOrientX << "," << siteOrientY << "," << siteOrientZ << ",";
		query << "-1)";
		stmt = con->createStatement();
		stmt->executeUpdate(query.str());

		
	}
	catch (sql::SQLException &e){
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete stmt;
}

void
Actionary::updateGraspSite(MetaObject* obj,int siteType,   
		float sitePosX, float sitePosY, float sitePosZ,
		float siteOrientX, float siteOrientY, float siteOrientZ)
{
	if(obj == NULL)
		return;
	sql::PreparedStatement *stmt = NULL;
	try{
		std::stringstream query;
		query << "update site set (?)=(?) WHERE obj_id =" << obj->getID() << " AND site_type= " << siteType;
		stmt = con->prepareStatement(query.str());
		stmt->setString(1, "site_pos_x");
		stmt->setDouble(2, (double)sitePosX);
		stmt->executeUpdate();

		stmt->setString(1, "site_pos_y");
		stmt->setDouble(2, (double)sitePosY);
		stmt->executeUpdate();

		stmt->setString(1, "site_pos_z");
		stmt->setDouble(2, (double)sitePosZ);
		stmt->executeUpdate();

		stmt->setString(1, "site_orient_x");
		stmt->setDouble(2, (double)siteOrientX);
		stmt->executeUpdate();

		stmt->setString(1, "site_orient_y");
		stmt->setDouble(2, (double)siteOrientY);
		stmt->executeUpdate();

		stmt->setString(1, "site_orient_z");
		stmt->setDouble(2, (double)siteOrientZ);
		stmt->executeUpdate();


	}
	catch (sql::SQLException &e){
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete stmt;
}

bool
Actionary::removeGraspSite(MetaObject* obj, int siteType)
{
	bool finished = false;
	sql::Statement *stmt = NULL;
	std::stringstream query;
	try{
		query << "Delete from site where obj_id=" << obj->getID() << " and site_type=" << siteType;
		stmt = con->createStatement();
		finished = stmt->execute(query.str());
	}
	catch (sql::SQLException &e){
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete stmt;
	return finished;
}


// return the MetaObject or NULL if not found. This way, we have a handle on the MetaObject
MetaObject*
Actionary::searchGraspSites(MetaObject* obj, int site_type)
{

	int objID = -1;
	if (obj == NULL)
		return NULL;
	sql::PreparedStatement *query = NULL;
	sql::ResultSet *res = NULL;
	try{
		query = con->prepareStatement("select obj_id from site where obj_id=(?) and site_type_id =(?)");
		
		MetaObject *parent = obj;
		
		while (parent != NULL && objID < 0){
			query->setInt(1, parent->getID());
			query->setInt(2, site_type);
			res = query->executeQuery();
			if (res->next()){
				objID = res->getInt("obj_id");
			}
			parent = obj->getParent();
		}
	}
	catch (sql::SQLException &e){
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete query;
	delete res;
	if(objID > 0)
		return this->searchByIdObj(objID);
	return NULL;
}

//returns the site type from the site name (inspect, operate, etc)
int
Actionary::getSiteType(const std::string& site_name){
	std::stringstream query;
	int val = -1;
	sql::Statement *stmt=NULL;
	sql::ResultSet *res = NULL;
	try{
		query << "select site_type_id from site_type where site_name ='" << site_name << "' LIMIT 1";
		stmt = con->createStatement();
		res = stmt->executeQuery(query.str());
		
		if (res->next())
			val = res->getInt(1);
	}
	catch (sql::SQLException &e){
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete stmt;
	delete res;
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
	std::string name = "";
	if (siteType <0)
		return NULL;
	sql::Statement *stmt=NULL;
	sql::ResultSet *res = NULL;
	try{
		std::stringstream query;
		query << "select site_name from site_type where site_type_id =" << siteType;
		stmt = con->createStatement();

		res = stmt->executeQuery(query.str());
		
		if (res->next())
			name = res->getString(1);
	}
	catch (sql::SQLException &e){
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete res;
	delete stmt;
	return name;
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
	sql::PreparedStatement *query = NULL;
	sql::ResultSet *res = NULL;
	try{
		query = con->prepareStatement("select site_pos_x, site_pos_y, site_pos_z from site where obj_id=(?) AND site_type_id=(?)");
		query->setInt(1, obj->getID());
		query->setInt(2, siteType);
		res = query->executeQuery();
		if (res->next()){
			pos.v[0] = (float)res->getDouble(1);
			pos.v[1] = (float)res->getDouble(2);
			pos.v[2] = (float)res->getDouble(3);
		}
	}
	catch (sql::SQLException &e){
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete query;
	delete res;
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
	sql::ResultSet *res = NULL;
	sql::PreparedStatement *query = NULL;
	try{
		query = con->prepareStatement("select site_orient_x, site_orient_y, site_orient_z  from site where obj_id=(?) AND site_type_id=(?)");
		query->setInt(1, obj->getID());
		query->setInt(2, siteType);
		res = query->executeQuery();
		if (res->next()){
			pos.v[0] = (float)res->getDouble(1);
			pos.v[1] = (float)res->getDouble(2);
			pos.v[2] = (float)res->getDouble(3);
		}
	}
	catch (sql::SQLException &e){
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete query;
	delete res;
	return pos;
}


int
Actionary::getCapabilities(MetaObject* obj, int which) // returns an action id
{
	if (obj == NULL || which < 0)
		return -1;

	MetaObject *parent=obj;//TB: Should this be reworked or is it even needed?
	int act_id = -1;
	sql::ResultSet *res = NULL;
	sql::PreparedStatement *query = NULL;
	try{
		query = con->prepareStatement("select action_id from agent_capable where obj_id =(?)");
		while (parent != NULL && act_id < 0){
			query->setInt(1, obj->getID());
			res = query->executeQuery();
			if (res->next())
				act_id = res->getInt(1);
			parent = parent->getParent();
		}
	}
	catch (sql::SQLException &e){
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete res;
	delete query;
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
	sql::ResultSet *res = NULL;
	sql::PreparedStatement * query = NULL;
	try{
		query = con->prepareStatement("select action_id from agent_capable where obj_id =(?)  and action_id =(?)");
		while (parent != NULL && !found){
			query->setInt(1, parent->getID());
			query->setInt(2, action->getID());
			res = query->executeQuery();
			if (res->next())
				found = true;
			parent = parent->getParent();
		}
	}
	catch (sql::SQLException &e){
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete query;
	delete res;
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
	sql::PreparedStatement *query = NULL;
	try{
		query=con->prepareStatement("INSERT INTO agent_capable VALUES((?),(?))");
		query->setInt(1,obj->getID());
		query->setInt(2,act->getID());
		query->executeUpdate();
	}
	catch(sql::SQLException &e){
		par_debug("%d:%s\n",e.getErrorCode(),e.what());
	}
	delete query;
}

void
Actionary::removeCapability(MetaObject* obj, char* action)
{
	if (obj == NULL)
		return;
	MetaAction* act = searchByNameAct(action);
	if (act == NULL)
		return;
	sql::PreparedStatement *query = NULL;
	try{
		query = con->prepareStatement("delete * from agent_capable where obj_id =(?)  and action_id =(?)");
		query->setInt(1, obj->getID());
		query->setInt(2, act->getID());
		query->execute();
	}
	catch (sql::SQLException &e){
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete query;
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
	sql::Statement *stmt=NULL;
	sql::ResultSet *res=NULL;
	try{
		stmt = con->createStatement();
		query << "SELECT obj_id from obj_prop WHERE obj_id =" << obj->getID() << " AND table_id = " << prop->getPropertyID() << " AND prop_value = " << value;
		res = stmt->executeQuery(query.str());
		query.str(std::string());
		query.clear();
		if (res->next()){
			query << "UPDATE obj_prop SET prop_value = " << value << " WHERE obj_id= " << obj->getID() << " and table_id ='" << prop->getPropertyID() << "';";
			if (stmt->execute(query.str()))
				updated = 1;
		}
		else{
			query << "INSERT INTO obj_prop VALUES(" << obj->getID() << ",'" << prop->getPropertyID() << "," << value << ");";
			if (stmt->execute(query.str()))
				updated = 1;
		}
	}
	catch (sql::SQLException &e){
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete stmt;
	delete res;
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

	parProperty *prop = this->searchByNameProperty(tab_name);
	std::string prop_name;
	if (prop != NULL){
		prop_name=prop->getPropertyNameByValue(value);
		if(prop_name.compare(""))
			return prop_name;
	}
	
	sql::Statement *stmt=NULL;
	sql::ResultSet *res=NULL; 
	try{
		std::stringstream query;
		query<<"SELECT name_value from "<<tab_name<<" WHERE id_value = "<<value;
		stmt=con->createStatement();
		res=stmt->executeQuery(query.str());
		if(res->next())
			prop_name=res->getString(1);

	}
	catch(sql::SQLException &e){
		par_debug("getPropertyNameByValue error-%d:%s\n",e.getErrorCode(),e.what());
	}
	delete stmt;
	delete res;
	return prop_name;
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
	sql::Statement *stmt=NULL;
	sql::ResultSet *res=NULL;
	try{
		std::stringstream query;
		query<<"Select id_value from "<<tab_name<<" where name_value='"<<prop_name<<"' LIMIT 1";
		stmt=con->createStatement();
		res=stmt->executeQuery(query.str());
		if(res->next())
			prop_id=res->getInt(1);

	}
	catch(sql::SQLException &e){
		par_debug("getPropertyValueByName error-%d:%s\n",e.getErrorCode(),e.what());
	}
	delete stmt;
	delete res;
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
	sql::Statement *stmt=NULL;
	sql::ResultSet *res = NULL;
	bool found = false;
	query<<"SELECT TABLE_NAME as tab_name from information_schema.tables WHERE TABLE_NAME ='"<<tab_name<<"'";
	//par_debug("%s\n", query.str().c_str());
	try{
		stmt = con->createStatement();
		res = stmt->executeQuery(query.str());
		
		if (res->next())
			found = true;
	}
	catch (sql::SQLException &e) {
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete stmt;
	delete res;
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
	int found = 0;
	sql::Statement *stmt=NULL;
	sql::ResultSet *res = NULL;
	try {
		std::stringstream query;
		query << "SELECT is_int from property_type WHERE prop_name ='" << tab_name << "'";
		stmt = con->createStatement();
		res = stmt->executeQuery(query.str());

		if (res->next())
			found = res->getInt(1);
		delete stmt;
		delete res;
	}
	catch (sql::SQLException &e) {
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	return found;
}
///////////////////////////////////////////////////////////////////////
//Gets a property for an action
parProperty*
Actionary::getProperty(MetaObject* act, int which){
	parProperty * found = NULL;
	sql::Statement *stmt = NULL;
	sql::ResultSet *res = NULL;
	try {
		std::stringstream query;
		query << "SELECT table_id from obj_prop WHERE obj_id = " << act->getID() << " LIMIT " << which << ",1";
		stmt = con->createStatement();
		res = stmt->executeQuery(query.str());

		if (res->next()){
			found = this->properties[res->getInt(1)];
		}
		delete stmt;
		delete res;
	}
	catch (sql::SQLException &e) {
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	return found;
}
//////////////////////////////////////////////////////////////////////
//We can have multiple properties for an action, so we need to know how
//many of those properties we have
int
Actionary::getNumProperties(MetaObject* act){
	int found = -1;
	sql::Statement *stmt = NULL;
	sql::ResultSet *res = NULL;
	try {
		std::stringstream query;
		query << "SELECT COUNT(*) from (SELECT table_id FROM obj_prop WHERE obj_id = " << act->getID() << " GROUP BY table_id) as table_counter";
		//par_debug("%s\n", query.str().c_str());
		stmt = con->createStatement();
		res = stmt->executeQuery(query.str());

		if (res->next())
			found = res->getInt(1);
		delete stmt;
		delete res;
	}
	catch (sql::SQLException &e) {
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	return found;
}
///////////////////////////////////////////////////////////////////////
//Gets a property for an action
int
Actionary::getProperty(MetaObject* act, parProperty* prop, int which){
	int found = -1;
	sql::Statement *stmt = NULL;
	sql::ResultSet *res = NULL;
	try {
		std::stringstream query;
		query << "SELECT prop_value from obj_prop WHERE obj_id = " << act->getID() << " AND table_id = " << prop->getPropertyID() << " LIMIT " << which << ",1";
		stmt = con->createStatement();
		res = stmt->executeQuery(query.str());

		if (res->next())
			found = res->getInt(1);
		delete stmt;
		delete res;
	}
	catch (sql::SQLException &e) {
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	return found;
}
//////////////////////////////////////////////////////////////////////
//We can have multiple properties for an action, so we need to know how
//many of those properties we have
int
Actionary::getNumProperties(MetaObject* act, parProperty *prop){
	int found = -1;
	sql::Statement *stmt = NULL;
	sql::ResultSet *res = NULL;
	try {
		std::stringstream query;
		query << "SELECT COUNT(*) from obj_prop WHERE obj_id = " << act->getID() << " AND table_id = " << prop->getPropertyID();
		stmt = con->createStatement();
		res = stmt->executeQuery(query.str());

		if (res->next())
			found = res->getInt(1);
		delete stmt;
		delete res;
	}
	catch (sql::SQLException &e) {
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	return found;
}
///////////////////////////////////////////////////////////////////////
//Gets a property for an action
parProperty*
Actionary::getProperty(MetaAction* act,  int which){
	parProperty * found = NULL;
	sql::Statement *stmt = NULL;
	sql::ResultSet *res = NULL;
	try {
		std::stringstream query;
		query << "SELECT table_id from act_prop WHERE act_id = " << act->getID() << " LIMIT " << which << ",1";
		stmt = con->createStatement();
		res = stmt->executeQuery(query.str());

		if (res->next()){
			found = this->properties[res->getInt(1)];
		}
		delete stmt;
		delete res;
	}
	catch (sql::SQLException &e) {
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	return found;
}
//////////////////////////////////////////////////////////////////////
//We can have multiple properties for an action, so we need to know how
//many of those properties we have
int
Actionary::getNumProperties(MetaAction* act){
	int found = -1;
	sql::Statement *stmt = NULL;
	sql::ResultSet *res = NULL;
	try {
		std::stringstream query;
		query << "SELECT COUNT(*) from (SELECT table_id FROM act_prop WHERE act_id = " << act->getID() << " GROUP BY table_id) as table_counter";
		//par_debug("%s\n", query.str().c_str());
		stmt = con->createStatement();
		res = stmt->executeQuery(query.str());

		if (res->next())
			found = res->getInt(1);
		delete stmt;
		delete res;
	}
	catch (sql::SQLException &e) {
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	return found;
}
///////////////////////////////////////////////////////////////////////
//Gets a property for an action
int 
Actionary::getProperty(MetaAction* act, parProperty* prop, int which){
	int found = -1;
	sql::Statement *stmt = NULL;
	sql::ResultSet *res = NULL;
	try {
		std::stringstream query;
		query << "SELECT prop_value from act_prop WHERE act_id = " << act->getID() << " AND table_id = "<<prop->getPropertyID()<<" LIMIT "<<which<<",1";
		stmt = con->createStatement();
		res = stmt->executeQuery(query.str());

		if (res->next())
			found = res->getInt(1);
		delete stmt;
		delete res;
	}
	catch (sql::SQLException &e) {
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	return found;
}
//////////////////////////////////////////////////////////////////////
//We can have multiple properties for an action, so we need to know how
//many of those properties we have
int 
Actionary::getNumProperties(MetaAction* act, parProperty *prop){
	int found = -1;
	sql::Statement *stmt = NULL;
	sql::ResultSet *res = NULL;
	try {
		std::stringstream query;
		query << "SELECT COUNT(*) from act_prop WHERE act_id = " << act->getID() << " AND table_id = " << prop->getPropertyID();
		stmt = con->createStatement();
		res = stmt->executeQuery(query.str());

		if (res->next())
			found = res->getInt(1);
		delete stmt;
		delete res;
	}
	catch (sql::SQLException &e) {
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
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
	std::string queryStr = "select act_id from action where act_name = '" +actName +"'";
	sql::Statement *stmt=NULL;
	sql::ResultSet *res = NULL;
	bool found = false;
	//par_debug("In isAction constructor, query is %s\n", queryStr.c_str());
	try{
		stmt = con->createStatement();
		res = stmt->executeQuery(queryStr);
		
		if (res->next())
			found = true;
	}
	catch (sql::SQLException &e) {
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete stmt;
	delete res;
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
		sql::Statement *stmt = NULL;
		try{
			stmt = con->createStatement();
			std::stringstream query;
			query << "INSERT INTO action (`act_id`,`act_name`) VALUES (";
			query << actID << ",";
			query<<actName<<")";
			stmt->execute(query.str());
			actMap[actID] = act;
		}
		catch (sql::SQLException &e) {
			par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
		}
		delete stmt;
		return actID;
}
///////////////////////////////////////////////////////////////////////
// Adds an action and assigns it's parent into the database.
/////////////////////////////////////////////////////////////////////
int
Actionary::addAction(MetaAction* act,std::string name, std::vector<MetaAction*> parents){

		int actID = getNextActID();
		//int parentID=parent->getID();
		sql::Statement *stmt = NULL;
		sql::PreparedStatement *pstmt = NULL;
		try{
			std::stringstream query;
			stmt = con->createStatement();
			query << "INSERT INTO action (`act_id`,`act_name`,`parent_act`) VALUES (";
			query<< actID<<",";
			query<<name<<",";
			query << ");";
			stmt->execute(query.str());
			pstmt = con->prepareStatement("INSERT INTO action_parent VALUES((?),(?));");
			for (int i = 0; i < parents.size(); i++){
				pstmt->setInt(actID, 1);
				pstmt->setInt(parents[i]->getID(), 2);
				pstmt->executeQuery();
			}
		}
		catch (sql::SQLException &e) {
			par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
		}
		actMap[actID] = act;
		delete stmt;
		delete pstmt;
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
	sql::Statement *stmt=NULL;
	sql::ResultSet *res = NULL;
	std::string val = "";
	try{
		query << "select act_name from action where act_id =" << act->getID();
		stmt = con->createStatement();
		res = stmt->executeQuery(query.str());
		
		if (res->next())
			val = res->getString(1);
	}
	catch (sql::SQLException &e) {
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete stmt;
	delete res;
	return val;

}

// set the action name
void
Actionary::setActionName(MetaAction *act, std::string newName)
{
	if (act == NULL)
		return;
	sql::Statement *stmt=NULL;
	try{
		stmt = con->createStatement();
		std::stringstream query;
		query << "update action set act_name = '" << newName << "' where act_id = " << act->getID();
		stmt->executeUpdate(query.str());
	}
	catch (sql::SQLException &e) {
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete stmt;
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
	}

	int parentID = parent->getID();  
	int actID = act->getID();
	sql::Statement *stmt=NULL;
	try{
		stmt = con->createStatement();
		std::stringstream query;
		//query << "update action set parent_id = " << parentID << " where act_id = " << actID;
		query << "INSERT INTO action_parent VALUES(" << actID << "," << parentID << ")";
		stmt->executeQuery(query.str());
		//stmt->executeUpdate(query.str());
	}
	catch (sql::SQLException &e) {
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete stmt;
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
	sql::Statement *stmt=NULL;
	sql::ResultSet *res = NULL;
	MetaAction *pact = NULL;
	try{
		stmt = con->createStatement();
		std::stringstream query;
		query << "select parent_id from action_parent where act_id = " << act->getID() << " ORDER BY parent_id LIMIT " << which << ",1";;
		res = stmt->executeQuery(query.str());
		if (res->next()){
			std::map<int, MetaAction*>::iterator finder = this->actMap.find(res->getInt(1));
			if (finder != actMap.end()){
				pact = this->searchByIdAct(res->getInt(1));
			}
			else{
				pact = new MetaAction(res->getInt(1));
				if (!strcmp("", pact->getActionName().c_str()))
					par_debug("Error creating action with id %d\n", res->getInt(1));
				else{
					par_debug("Action name is %s\n", act->getActionName().c_str());
					actMap[res->getInt(1)] = pact;
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
	}
	catch (sql::SQLException &e) {
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete stmt;
	delete res;
	return pact;
}

int
Actionary::getNumParents(MetaAction* act)
{
	if (act == NULL)
		return 0;
	sql::Statement *stmt = NULL;
	sql::ResultSet *res = NULL;
	int pact = 0;
	try{
		stmt = con->createStatement();
		std::stringstream query;
		query << "SELECT COUNT(*) FROM action_parent where act_id = " << act->getID();
		res = stmt->executeQuery(query.str());
		if (res->next()){
			pact = res->getInt(1);
		}
	}
	catch (sql::SQLException &e) {
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete stmt;
	delete res;
	return pact;
}


void
Actionary::setApplicabilityCond(MetaAction* act, const std::string& appCond)
{

	if (act == NULL)
		return;
	int actID = act->getID();
	sql::Statement *stmt=NULL;
	try{
		stmt = con->createStatement();
		std::stringstream query;
		query << "update action set act_appl_cond = '" << appCond << "' where act_id = " << actID;
		stmt->executeUpdate(query.str());
	}
	catch (sql::SQLException &e) {
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete stmt;
	// load it in python too
	loadApplicabilityCond(act);
}

std::string
Actionary::getApplicabilityCond(MetaAction* act)
{
	if (act == NULL)
		return " ";
	std::stringstream query;
	sql::Statement *stmt = NULL;
	sql::ResultSet *res = NULL;
	std::string val = " ";
	try{
		query << "select act_appl_cond from action where act_id =" << act->getID();
		stmt = con->createStatement();
		res = stmt->executeQuery(query.str());

		if (res->next())
			val = res->getString(1);
	}
	catch (sql::SQLException &e) {
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete stmt;
	delete res;
	return val;
}

void
Actionary::setCulminationCond(MetaAction* act, const std::string& termCond)
{
	if (act == NULL)
		return;
	std::stringstream query;
	sql::Statement *stmt=NULL;
	try{
		query << "update action set act_term_cond = '" << termCond << "' where act_id = " << act->getID();
		stmt = con->createStatement();
		stmt->executeUpdate(query.str());
	}
	catch (sql::SQLException &e) {
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete stmt;
	loadCulminationCond(act);
}
std::string
Actionary::getCulminationCond(MetaAction* act){
	if (act == NULL)
		return " ";
	std::stringstream query;
	sql::Statement *stmt = NULL;
	sql::ResultSet *res = NULL;
	std::string val = " ";
	try{
		query << "select act_term_cond from action where act_id  =" << act->getID();
		stmt = con->createStatement();
		res = stmt->executeQuery(query.str());

		if (res->next())
			val = res->getString(1);
	}
	catch (sql::SQLException &e) {
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete stmt;
	delete res;
	return val;
}

void
Actionary::setPreparatorySpec(MetaAction* act, const std::string& prepSpec)
{
	if (act == NULL)
		return;
	int actID = act->getID();
	std::stringstream query;
	sql::Statement *stmt=NULL;
	query << "update action set act_prep_spec = '" << prepSpec << "' where act_id = " << actID;
	stmt=con->createStatement();
	stmt->executeUpdate(query.str());
	delete stmt;
	loadPreparatorySpec(act);
}

std::string
Actionary::getPreparatorySpec(MetaAction* act){
	if (act == NULL)
		return " ";
	std::stringstream query;
	sql::Statement *stmt = NULL;
	sql::ResultSet *res = NULL;
	std::string val = " ";
	try{
		query << "select act_prep_spec from action where act_id  =" << act->getID();
		stmt = con->createStatement();
		res = stmt->executeQuery(query.str());

		if (res->next())
			val = res->getString(1);
	}
	catch (sql::SQLException &e) {
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete stmt;
	delete res;
	return val;
}

void
Actionary::setExecutionSteps(MetaAction* act, const std::string & execSteps)
{
	if (act == NULL)
		return;
	int actID = act->getID();
	sql::Statement *stmt=NULL;
	std::stringstream query;
	try{
		query << "update action set act_exec_steps = '" << execSteps << "' where act_id = " << actID;
		stmt = con->createStatement();
		stmt->executeUpdate(query.str());
	}
	catch (sql::SQLException &e){
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete stmt;
	loadExecutionSteps(act);
}

std::string
Actionary::getExecutionSteps(MetaAction* act){
	if (act == NULL)
		return " ";
	std::stringstream query;
	sql::Statement *stmt = NULL;
	sql::ResultSet *res = NULL;
	std::string val = " ";
	try{
		query << "select act_exec_steps from action where act_id  =" << act->getID();
		stmt = con->createStatement();
		res = stmt->executeQuery(query.str());

		if (res->next())
			val = res->getString(1);
	}
	catch (sql::SQLException &e) {
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete stmt;
	delete res;
	return val;	
}

void
Actionary::setPurposeAchieve(MetaAction* act, const std::string& achieve)
{
	if (act == NULL)
		return;
	int actID = act->getID();
	std::stringstream query;
	sql::Statement *stmt=NULL;
	try{
		query << "update action set act_purpose_achieve = '" << achieve << "' where act_id = " << actID;
		stmt = con->createStatement();
		stmt->executeUpdate(query.str());
	}
	catch (sql::SQLException &e){
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete stmt;
}

std::string
Actionary::getPurposeAchieve(MetaAction* act){
	if (act == NULL)
		return "";
	bool found=false;
	sql::PreparedStatement *pstmt = NULL;
	sql::ResultSet *res = NULL;
	std::string cond;
	try{
		pstmt = con->prepareStatement("select act_purpose_achieve from action where act_id = (?)");
		while (act != NULL && !found){
			pstmt->setInt(1, act->getID());
			res = pstmt->executeQuery();
			if (res->next()){
				cond = res->getString(1);
				found = true;
			}
			act = act->getParent();
		}
		if (!found)
			cond = "";
	}
	catch (sql::SQLException &e){
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete pstmt;
	delete res;
	return cond;
}
///////////////////////////////////////////////////////////////////////////////
//Determines all objects with a given purpose, if any of them exist
///////////////////////////////////////////////////////////////////////////////
std::vector<MetaAction*>
Actionary::getAllPurposed(const std::string& purpose){
	std::stringstream query;
	std::vector<MetaAction*> vec;
	sql::Statement *stmt=NULL;
	sql::ResultSet *res = NULL;
	try{
		query << "SELECT act_id from action WHERE act_purpose_achieve ='" << purpose << "'";
		stmt = con->createStatement();
		res = stmt->executeQuery(query.str());
		while (res->next()){
			MetaAction *act = this->searchByIdAct(res->getInt(1));
			if (act != NULL)
				vec.push_back(act);
		}
	}
	catch (sql::SQLException &e){
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete stmt;
	delete res;
	return vec;
}
///////////////////////////////////////////////////////////////////////////////
//Returns the maximum number of objects that a metaobject can hold
///////////////////////////////////////////////////////////////////////////////
int
Actionary::getNumObjects(MetaAction* act){
	if (act == NULL)
		return -1;
	bool found=false;
	sql::PreparedStatement *pstmt = NULL;
	sql::ResultSet *res = NULL;
	int val = -1;
	try{
		pstmt = con->prepareStatement("select act_obj_num from action where act_id = (?)");
		while (act != NULL && !found){
			pstmt->setInt(1, act->getID());
			res = pstmt->executeQuery();
			if (res->next()){
				val = res->getInt(1);
				found = true;
			}
			act = act->getParent();
		}
	}
	catch (sql::SQLException &e){
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete pstmt;
	delete res;
	return val;
}
void
Actionary::setNumObjects(MetaAction* act, int num)
{
	if (act == NULL)
		return;
	std::stringstream query;
	sql::Statement *stmt=NULL;
	query << "update action set act_obj_num = " << num << " where act_id = " << act->getID();
	try{
		stmt = con->createStatement();
		stmt->executeUpdate(query.str());
	}
	catch (sql::SQLException &e){
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete stmt;
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
	sql::Statement *stmt=NULL;
	try{
		query << "INSERT INTO obj_act VALUES(" << obj->getID() << "," << act->getID() << "," << position << ");";
		stmt = con->createStatement();
		stmt->executeUpdate(query.str());
	}
	catch (sql::SQLException &e){
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete stmt;
}

void
Actionary::removeAffordance(MetaAction *act, MetaObject *obj, int obj_num){
	if(act == NULL || obj == NULL)
		return;
	//Which's number doesn't matter, as if which doesn't exist, the query just will delete zero rows
	std::stringstream query;
	sql::Statement *stmt=NULL;
	try{
		query << "DELETE from obj_act WHERE obj_id = " << obj->getID() << " and act_id = " << act->getID() << " and obj_num = " << obj_num;
		stmt = con->createStatement();
		stmt->executeUpdate(query.str());
	}
	catch (sql::SQLException &e){
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete stmt;
}
///////////////////////////////////////////////////////////////////
//Grabs the first position a given object is in.  -1 is a failure
/////////////////////////////////////////////////////////////////
int
Actionary::searchAffordance(MetaAction* act, MetaObject* obj){
	int result=-1;
	sql::Statement *stmt=NULL;
	sql::ResultSet *res=NULL;
	try{
		if (act != NULL && obj != NULL){
			std::stringstream query;
			query << "SELECT obj_num from obj_act WHERE obj_id = " << obj->getID() << " and act_id = " << act->getID() << " LIMIT 1";
			stmt = con->createStatement();
			res = stmt->executeQuery(query.str());
			if (res->next())
				result = res->getInt(1);

		}
	}
	catch (sql::SQLException &e){
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete stmt;
	delete res;
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
	sql::ResultSet *res=NULL;
	MetaAction *act = NULL;
	sql::PreparedStatement *pstmt=NULL;
	try{
		query << "SELECT act_id from obj_act WHERE obj_id=(?) and position=" << position << "LIMIT " << which << ",1";
		pstmt = con->prepareStatement(query.str());

		MetaObject *finder = obj;

		while (finder != NULL && act == NULL){
			pstmt->setInt(1, finder->getID());
			res = pstmt->executeQuery();
			if (res->next())
				act = this->searchByIdAct(res->getInt(1));
			finder = finder->getParent();
		}
	}
	catch (sql::SQLException &e){
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete pstmt;
	delete res;
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
	sql::PreparedStatement *pstmt=NULL;
	MetaObject *obj = NULL;
	sql::ResultSet *res=NULL;
	try{
		query << "SELECT obj_id from obj_act WHERE act_id=(?) and position=" << position << "LIMIT " << which << ",1";
		pstmt = con->prepareStatement(query.str());

		MetaAction *finder = act;

		while (finder != NULL && obj == NULL){
			pstmt->setInt(1, finder->getID());
			res = pstmt->executeQuery();
			if (res->next())
				obj = this->searchByIdObj(res->getInt(1));
			finder = finder->getParent();
		}
	}
	catch (sql::SQLException &e){
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete pstmt;
	delete res;
	return obj;
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
	sql::Statement *stmt=NULL;
	// remove it from all of the tables
	try{
		stmt = con->createStatement();
		query << "delete from action where act_id = " << actID << ";";
		query << "delete from adverb_exp where act_id = " << actID << ";";
		query << "delete from obj_act where act_id = " << actID << ";";
		stmt->executeQuery(query.str());
	}
	catch (sql::SQLException &e){
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete stmt;
}



float
Actionary::getDuration(MetaAction* act)
{
	if (act == NULL)
		return -999;
	float val = -999;
	std::stringstream query;
	sql::Statement *stmt=NULL;
	sql::ResultSet *res=NULL;
	try{
		query << "select act_dur_time_id from action where act_id = " << act->getID();
		stmt = con->createStatement();
		res = stmt->executeQuery(query.str());

		if (res->next())
			val = (float)res->getDouble(1);
	}
	catch (sql::SQLException &e){
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete stmt;
	delete res;
	return val;
}

void
Actionary::setDuration(MetaAction* act, float d)
{
	if (d < 0)
		std::cout << "ERROR: duration should not be negative" << std::endl;
	if (act == NULL)
		return;
	sql::Statement *stmt=NULL;
	try{
		std::stringstream query;
		query << "update action set act_dur_time_id = " << d << " where act_id = " << act->getID();
		stmt = con->createStatement();
		stmt->executeUpdate(query.str());
	}
	catch (sql::SQLException &e){
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete stmt;
}




std::string
Actionary::getAdverb(MetaAction* act)	// later allow for more than one adverb
{
	if (act == NULL)
		return " ";
	std::stringstream query;
	std::string val = "";
	sql::Statement *stmt=NULL;
	sql::ResultSet *res=NULL;
	try{
		query << "select adverb_name from adverb_exp where act_id = " << act->getID();
		stmt = con->createStatement();
		res = stmt->executeQuery(query.str());

		if (res->next())
			val = res->getString(1);
	}
	catch (sql::SQLException &e){
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete stmt;
	delete res;
	return val;
}

std::string
Actionary::getModifier(MetaAction* act)	// later allow for more than one modifier
{
	if (act == NULL)
		return " ";
	std::stringstream query;
	std::string val = "";
	sql::Statement *stmt=NULL;
	sql::ResultSet *res=NULL;
	try{
		query << "select adverb_mod_name from adverb_exp where act_id = " << act->getID();
		stmt = con->createStatement();
		res = stmt->executeQuery(query.str());
		if (res->next())
			val = res->getString(1);
	}
	catch (sql::SQLException &e){
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete stmt;
	delete res;
	return val;
}

void
Actionary::setAdverb(MetaAction* act, const std::string& adverb, const std::string& modifier)
{
	if (act == NULL)
		return;
	std::stringstream query;
	query<<"INSERT INTO adv_exp VALUES("<<act->getID()<<","<<adverb<<","<<modifier<<");";
	sql::Statement *stmt=NULL;
	try{
		stmt = con->createStatement();
		stmt->executeUpdate(query.str());
	}
	catch (sql::SQLException &e){
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete stmt;
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
	sql::PreparedStatement *pstmt=NULL;
	sql::ResultSet *res=NULL;
	int val = -1;
	try{
		pstmt = con->prepareStatement("select act_site_type_id from action where act_id = (?)");

		
		while (looper != NULL && val < 0){
			pstmt->setInt(1, looper->getID());
			res = pstmt->executeQuery();
			if (res->next()){
				val = res->getInt(1);
			}
			looper = looper->getParent();
		}
	}
	catch (sql::SQLException &e){
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete pstmt;
	delete res;
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
	std::stringstream query;
	sql::Statement *stmt=NULL;
	sql::ResultSet *res=NULL;
	try{
		stmt = con->createStatement();
		if (site_shape_id < 0){//Create a site

			query << "SELECT MAX(site_shape_id) from site_shape";
			res = stmt->executeQuery(query.str());
			if (res->next())
				site_shape_id = res->getInt(1);
			query.clear();
			query.str(std::string());
			query << "INSERT INTO site_shape_id VALUES (" << site_shape_id << "," << shape_type << ",";
			query << first << "," << second << "," << third << ");";
			
		}
		else{
			query << "Update site_shape SET shape_type='" << shape_type << "',first_coord=" << first << ", second_coord=" << second << ", third_coord=" << third << " WHERE site_shape_id =" << site_shape_id;
		}
		success = stmt->execute(query.str());
	}
	catch (sql::SQLException &e){
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete res;
	delete stmt;
	return success;
}
////////////////////////////////////////////////////////////////////////////////
//Returns the site shape id
///////////////////////////////////////////////////////////////////////////////
int
Actionary::getSiteShapeID(MetaObject* obj, int siteType){
	if(obj == NULL || siteType < 0)
		return -1;
	sql::Statement *stmt=NULL;
	sql::ResultSet * res=NULL;
	int val=-1;
	try{
		stmt = con->createStatement();
		std::stringstream query;
		query << "SELECT site_shape_id from site WHERE obj_id =" << obj->getID() << " and site_type_id =" << siteType;
		res = stmt->executeQuery(query.str());
		if (res->next())
			val = res->getInt(1);
		else
			val = -1;
	}
	catch (sql::SQLException &e){
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete stmt;
	delete res;
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

	sql::Statement *stmt=NULL;
	sql::ResultSet *res=NULL;
	std::string val;
	try{
		std::stringstream query;
		query << "SELECT shape_type from site_shape WHERE site_shape_id = " << site_shape_id;
		stmt = con->createStatement();

		res = stmt->executeQuery(query.str());
		
		if (res->next())
			val = res->getString(1);
		else
			val = "";
	}
	catch (sql::SQLException &e){
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete stmt;
	delete res;
	return val;
}
//first coord
float
Actionary::getSiteShapeCoordinate(int site_shape_id,int coordinate){
	if(site_shape_id <0)
		return 0.0f;

	std::stringstream query;
	switch(coordinate){
	case 1:
		query<<"SELECT first_coord from site_shape WHERE site_shape_id = "<<site_shape_id;
		break;
	case 2:
		query<<"SELECT second_coord from site_shape WHERE site_shape_id = "<<site_shape_id;
		break;
	case 3:
		query<<"SELECT third_coord from site_shape WHERE site_shape_id = "<<site_shape_id;
		break;
	default:
		return 0.0f;
	}
	float val = 0.0f;
	sql::Statement *stmt=NULL;
	sql::ResultSet *res=NULL;
	try{
		stmt = con->createStatement();
		res = stmt->executeQuery(query.str());
		
		if (res->next())
			val = (float)res->getDouble(1);
	}
	catch (sql::SQLException &e){
		par_debug("SQL Error:%d:%s\n", e.getErrorCode(), e.what());
	}
	delete stmt;
	delete res;
	return val;

}
////////////////////////////////////////////////////////////////////////////////