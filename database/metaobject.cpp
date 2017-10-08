#include <cstdio>
#include <string>
#include "metaobject.h"
#include "interpy.h"
#include "workingmemory.h"
#include "par.h"


extern Actionary *actionary; // global actionary pointer



MetaObject::MetaObject(const char* obName, bool agent,bool obj_inst):
	instance(obj_inst),location(NULL),possessedBy(NULL),
	properties(std::map<parProperty*, std::list<double>>())
{
	// check to see if it already exists and if so print a message
	if (actionary->isObject(std::string(obName)))
	{
		par_debug("Object already exists in MetaObject constructor %s\n",obName);
		// get the id number
		objID = actionary->getObjectID(obName);
		name = std::string(obName);
		MetaObject* rentObj=actionary->getParent(this);
		parent=rentObj;
		instance=false;
	}
	else
	{
		//If it doesn't exist in the database, then we need to add it to the actionary,
		//and if it isn't an instance, add it to the database
		objID = actionary->addObject(this, obName, agent,obj_inst);
		name = std::string(obName);
		std::string rentName = actionary->extractParentName(name);
		MetaObject* rentObj = actionary->searchByNameObj(rentName);
		if(rentObj != NULL){
			actionary->setParent(this,rentObj);
			parent=rentObj;
		}
		else
			parent=actionary->searchByIdObj(1);//This is the root, so if the instanced object doesn't have a parent,
											   //this will keep it from having funny business
	}
	//sets the default value if the object is an instance
	if(instance)
		boundingVol[0].v[0]=-1;
	setupProperties();
	// store the id in the appropriate list
	bool roomQ=actionary->isType(this,"Room");
	
	// check all agents
	if (agent && instance)
	{
		actionary->allAgents.push_back(objID);
		return;
	}
	// check type
	if(!roomQ && !instance)
	{
		actionary->allTypes.push_back(objID);
		return;
	}
	// check object instances
	if(!roomQ && instance)
	{
		actionary->allObjects.push_back(objID);
		return;
	}
	// check all rooms
	if(roomQ && instance)
	{
		actionary->allRooms.push_back(objID);
		return;
	}

}

///////////////////////////////////////////////////////////////////////////////
//Destroys the meta-object
///////////////////////////////////////////////////////////////////////////////
MetaObject::~MetaObject(){
	//We attach all children of the objects to the parent, all contents its container, and all possessions to its possessor
	int child_counter = 0;
	MetaObject *child=actionary->getChild(this,child_counter);
	while(child != NULL){
		child->setParent(this->parent);
		child_counter++;
		child = actionary->getChild(this, child_counter);
	}
	//Clean up the locations that are set to this
	for(std::list<MetaObject*>::iterator it =contents.begin(); it != contents.end(); it++){
		(*it)->setLocation(this->location);
	}
	//Clean up the possessions that are set to this
	for(std::list<MetaObject*>::iterator it=possessions.begin(); it != possessions.end(); it ++){
		(*it)->setPossessedBy(this->possessedBy);
	}

}
/////////////////////////////////////////
// place this object in the hierarchy
void 
MetaObject::setParent(MetaObject* aparent)
{
	parent=aparent;
	if(!instance)
		actionary->setParent(this, aparent);
}


/////////////////////////////////////////
// get the object's bounding radius
float 
MetaObject::getBoundingRadius()
{
	if(instance)
		return boundingVol[0].v[0];
	else
		return -1;
}

// set the object's bounding radius
void  
MetaObject::setBoundingRadius(float radius)
{
	if(instance)
		boundingVol[0].v[0]=radius;
}

////////////////////////////////////////////////////
//Gets a point on the bounding radius of the eight
//possible points
Vector<3>*
MetaObject::getBoundingPoint(int which){
	if(which <0 || which >8)
		return NULL;//A box has eight points

	return &boundingVol[which];
}
//////////////////////////////////////////////////
//This sets a point on the bounding box.  As long
//as the user keeps these two consistant, there 
//shouldn't be any problems
//////////////////////////////////////////////////
void
MetaObject::setBoundingPoint(Vector<3>* point, int which){
	if(which <0 || which >8 || point == NULL)
		return;//A box has eight points
	boundingVol[which].v[0]=point->v[0];
	boundingVol[which].v[1]=point->v[1];
	boundingVol[which].v[2]=point->v[2];

}

void 
MetaObject::setObjectName(std::string newName)
{
	name = newName;
	if(!instance)
		actionary->setObjectName(this, newName);
}
///////////////////////////////////////////////////////////////////////////////
//This uses the all agents vector to determine if the object in question
//is an agent
///////////////////////////////////////////////////////////////////////////////
bool 
MetaObject::isAgent()
{
	return actionary->isAgent(this);
}
///////////////////////////////////////////////////////////////////////////////
//This uses the all rooms vector to determine if the object in question
//is an agent
///////////////////////////////////////////////////////////////////////////////
bool 
MetaObject::isRoom()
{
	return actionary->isRoom(this);
}


Vector<3>*
MetaObject::getPosition()
{	
	if(instance)
		return &position;
	else
		return NULL;
}

void
MetaObject::setPosition(Vector<3>* vector)
{
	//Note: position should only be for instanced object
	position.v[0]=vector->v[0];
	position.v[1]=vector->v[1];
	position.v[2]=vector->v[2];
}

Vector<3>*
MetaObject::getVelocity()
{
	if(instance)
		return &velocity;
	else
		return NULL;
}

void
MetaObject::setVelocity(Vector<3>* vector)
{
   	velocity.v[0]=vector->v[0];
	velocity.v[1]=vector->v[1];
	velocity.v[2]=vector->v[2];
}

Vector<3>*
MetaObject::getAcceleration()
{
   	if(instance)
		return &acceleration;
	else
		return NULL;
}

void
MetaObject::setAcceleration(Vector<3>* vector)
{
	acceleration.v[0]=vector->v[0];
	acceleration.v[1]=vector->v[1];
	acceleration.v[2]=vector->v[2];
}

Vector<3>*
MetaObject::getOrientation()
{
	if(instance)
		return &orientation;
	else
		return NULL;
}

void
MetaObject::setOrientation(Vector<3>* vector)
{
	
	orientation.v[0]=vector->v[0];
	orientation.v[1]=vector->v[1];
	orientation.v[2]=vector->v[2];
}

Vector<3>*
MetaObject::getCoordinateSystem()
{
   	if(instance)
		return &coordinateSystem;
	else
		return NULL;
}

void
MetaObject::setCoordinateSystem(Vector<3>* vector){
	coordinateSystem.v[0]=vector->v[0];
	coordinateSystem.v[1]=vector->v[1];
	coordinateSystem.v[2]=vector->v[2];
}

void
MetaObject::addSite( int siteType,
		float sitePosX, float sitePosY, float sitePosZ,
		float siteOrientX, float siteOrientY, float siteOrientZ)
{
	actionary->addGraspSite(this,  siteType, sitePosX, sitePosY, sitePosZ,
		siteOrientX, siteOrientY, siteOrientZ);
}

void
MetaObject::updateSite(int siteType,     // use -999 where values should not be altered
		float sitePosX, float sitePosY, float sitePosZ,
		float siteOrientX, float siteOrientY, float siteOrientZ)
{
	actionary->updateGraspSite(this,siteType, sitePosX, sitePosY, sitePosZ,
		siteOrientX, siteOrientY, siteOrientZ);
}

void
MetaObject::removeSite(int siteType){
	actionary->removeGraspSite(this,siteType);
}

MetaObject*
MetaObject::searchSites(char* siteName){ // return the site id or -1 if not found
	int site_type=actionary->getSiteType(siteName);
	return actionary->searchGraspSites(this, site_type);
}

MetaObject*
MetaObject::searchSites(int site_type){

	return actionary->searchGraspSites(this,site_type);
}

std::string
MetaObject::getSiteName(int siteType){
	return actionary->getGraspSiteName(siteType);
}

float
MetaObject::getSitePos(int siteType, int which){
	return actionary->getGraspSitePos(this, siteType, which);
}
Vector<3>
MetaObject::getSitePos(int siteType){
	return actionary->getGraspSitePos(this,siteType);
}
float
MetaObject::getSiteOrient(int siteType, int which){
	return actionary->getGraspSiteOrient(this, siteType, which);
}
Vector<3>
MetaObject::getSiteOrient(int siteType){
	return actionary->getGraspSiteOrient(this, siteType);
}


void 
MetaObject::setPossessedBy(MetaObject* obj)
{
	//We want to get out of the loop if we have already set the location
	if (obj == NULL || possessedBy == obj)
		return;
  if(possessedBy != obj){
	  //This will automatically remove this from the other's possession
	if(possessedBy != NULL)
		obj->removeFromPossessions(this);

	possessedBy=obj;
	possessedBy->addPossession(this);//We need to make sure these two are the same
	}
}
MetaObject* 
MetaObject::getPossessedBy()
{
	return possessedBy;
}

void		  
MetaObject::removeFromPossessions(MetaObject* obj)
{
	if(obj == NULL)
		return;
	possessions.remove(obj);
}

void 
MetaObject::addPossession(MetaObject* obj)
{
	if(!this->searchPossession(obj))
		possessions.push_back(obj);
}

void 
MetaObject::deletePossessions()
{
	possessions.clear();
}
bool
MetaObject::searchPossession(MetaObject *obj){
	if(obj == NULL)
		return false;
	for(std::list<MetaObject*>::iterator it=possessions.begin(); it != possessions.end(); it++){
		if(obj == (*it))
			return true;
	}
	return false;
}
//Quick overload for easy authoring
bool 
MetaObject::searchPossession(std::string objName)
{
	MetaObject *obj=actionary->searchByNameObj(objName);
	return this->searchPossession(obj);
}
MetaObject*
MetaObject::getPossessionOfType(MetaObject *obj){
	if(obj == NULL)
		return NULL;

	for(std::list<MetaObject*>::iterator it=possessions.begin(); it != possessions.end(); it++){
		if(actionary->isType((*it),obj))
			return (*it);
	}
	return NULL;
}
MetaObject* 
MetaObject::getPossessionOfType(std::string typeName)
{
	MetaObject* obj=actionary->searchByNameObj(typeName);
	return this->getPossessionOfType(obj);
}


/*Location functions*/
MetaObject*
MetaObject::getLocation()
{
   return location;
}
///////////////////////////////////////////////////////////////////////////////
//Keeps searching to location until it finds a room.
///////////////////////////////////////////////////////////////////////////////
MetaObject*
MetaObject::getRoomLocation(){
	if(location->isRoom())
		return location;
	//Before recursing, we need to make sure we have something to recurse to
	if(location->getLocation() == NULL)
		return NULL;

	return location->getRoomLocation();
	
}
///////////////////////////////////////////////////////////////////////////////
//Set's the location, which can be understood as the contents of the object
///////////////////////////////////////////////////////////////////////////////
void
MetaObject::setLocation(MetaObject* loc)
{
	//We want to get out of the loop if we have already set the location
	if (loc == NULL || location == loc)
		return;
  if(location != loc){
	  //This will automatically remove this from the other contents
	if(location != NULL)
		location->removeFromContents(this);

	location=loc;
	location->addContents(this);//We need to make sure these two are the same
  }
}
///////////////////////////////////////////////////////////////////////////////
//Overloaded to allow easier authoring
///////////////////////////////////////////////////////////////////////////////
void		
MetaObject::setLocation(std::string objName)
{
	MetaObject *loc=actionary->searchByNameObj(objName);
	setLocation(loc);
}


void
MetaObject::addContents(MetaObject* obj)
{	
	if(obj == NULL)
		return;
	//First, we should make sure the object isn't in the contents
	//This will probably slow it down a bit
	if(searchContents(obj))
		return;
	contents.push_back(obj);
	//We should also update the location of the object to be 
	//within the contents of the object
	obj->setLocation(this);
}
void 
MetaObject::removeFromContents(MetaObject* obj)
{
	if(obj == NULL)
		return;
	contents.remove(obj);
}


void
MetaObject::deleteContents()
{
	contents.clear();  
}

bool
MetaObject::searchContents(MetaObject* obj)
{
	if(obj == NULL)
		return false;
	for(std::list<MetaObject*>::iterator it=contents.begin(); it != contents.end(); it++)
		if((*it)==obj)
			return true;
		
	/*We also wish to recursively find objects that could be within
	other objects*/
	for(std::list<MetaObject*>::iterator it=contents.begin(); it != contents.end(); it++)
		if((*it)->searchContents(obj))
			return true;
	
	return false;

}
///////////////////////////////////////////////////////////////////////////////
//Overloaded operator for ease of authoring
///////////////////////////////////////////////////////////////////////////////
bool
MetaObject::searchContents(std::string obj_name){
	MetaObject* obj=actionary->searchByNameObj(obj_name);
	return searchContents(obj);
}
///////////////////////////////////////////////////////////////////////////////
//Search contents for types attempts to find an object within another object's
//contents.  the not_agents flag filter's out agents, so one agent won't search
//another agent's contents for an item.  This prevents stealing.
///////////////////////////////////////////////////////////////////////////////
MetaObject* 
MetaObject::searchContentsForType(MetaObject *type, bool not_agents)
{
	if(type == NULL)
		return NULL;

	MetaObject *found=NULL;
	/*We're going to recurse like we did last time*/
	for(std::list<MetaObject*>::iterator it=contents.begin(); it != contents.end() && (found == NULL); it++)
		if(actionary->isType((*it),type))
			found=(*it);

	for(std::list<MetaObject*>::iterator it=contents.begin(); it != contents.end() && (found == NULL); it++)
		if(not_agents){
			if(!(*it)->isAgent())
				found=(*it)->searchContentsForType(type,not_agents);
		}
		else
			found=(*it)->searchContentsForType(type,not_agents);
			
	return found; 
}
///////////////////////////////////////////////////////////////////////////////
//This is the overloaded search that can act using a string
//////////////////////////////////////////////////////////////////////////////
MetaObject*
MetaObject::searchContentsForType(std::string type_name,bool not_agents){
	MetaObject *type=actionary->searchByNameObj(type_name);
	return this->searchContentsForType(type,not_agents);
}
///////////////////////////////////////////////////////////////////////////////
//Changes the contents position to match a given position.  This will almost
//always be the object's position
///////////////////////////////////////////////////////////////////////////////
void
MetaObject::updateContentsPosition(Vector<3>* new_position){
	if(new_position == NULL)
		return;  //Why wouldn't they send a position

	if(contents.size() == 0)
		return; //Need some way of stopping the recursion

	for(std::list<MetaObject*>::iterator it=contents.begin(); it != contents.end(); it++){
		//Here, we should check for the object's contents, and then essentially the contents
		//of the objects within the contents 
		(*it)->setPosition(new_position);
		(*it)->updateContentsPosition(new_position);
	}
}
// property
int
MetaObject::setProperty(parProperty* prop, std::string prop_name, bool send_up, bool write_to_db){
	if(prop != NULL)
		return this->setProperty(prop, prop->getPropertyValueByName(prop_name), send_up, write_to_db);

		return -1;
}

int
MetaObject::setProperty(parProperty* prop, double prop_value, bool send_up,bool write_to_db){
	if(prop != NULL && prop_value >-1 && prop->getType() != 1){ //A type of one is an action
		std::map<parProperty*, std::list<double>>::const_iterator it = this->properties.find(prop);
		if (it == this->properties.end()){ //Does not exist, needs to be added
			std::list<double> vals;
			vals.push_back(prop_value);
			this->properties[prop] = vals;
		}
		else{
			if (this->instance){
				properties[prop].front() = prop_value;
			}
			else{
				//See if the property is already in there!
				if (!this->hasProperty(prop, prop_value)){
					this->properties[prop].push_back(prop_value);
				}
			}
		}
		if (write_to_db){
			actionary->setProperty(this, prop, prop_value);
		}
		if (parent != NULL && send_up){
			this->parent->setProperty(prop, prop_value, false); //Writing to the database only occurs on the object, so that we can maintain subsets without having too much in the database 
		}
		return 1;
	}
	else
		return -1;
}

void
MetaObject::removeProperty(parProperty* prop,int which){
	if(prop != NULL){
		std::map<parProperty*,std::list<double>>::iterator it=properties.find(prop);
		if (it != properties.end()){
			if (this->instance)
				properties.erase(it);
			else{
				std::list<double>::const_iterator it2 = (*it).second.begin();
				advance(it2, which);
				(*it).second.erase(it2);
			}
		}
		/*if(!instance)
			actionary->removeProperty(this,prop_type);*/
	}
}
///////////////////////////////////////////////////////////////////////////////
//Determines if the property is actually in the set value itself
///////////////////////////////////////////////////////////////////////////////
bool
MetaObject::hasProperty(parProperty* prop, double value){
	if (prop == NULL)
		return false;
	std::map<parProperty*, std::list<double>>::const_iterator it = this->properties.find(prop);
	if (it == this->properties.end()){ //Does not exist, needs to be added
		return false;
	}
	std::list<double>::const_iterator it2;
	for (it2 = this->properties[prop].begin(); it2 != this->properties[prop].end(); it2++){
		if ((*it2) == value)
			return true;
	}
	return false;
}

std::string
MetaObject::getPropertyName(parProperty* prop, int which){
	std::string result = "";
	if (prop != NULL){
		double ans = this->getPropertyValue(prop, which);
		result = prop->getPropertyNameByValue(ans);
	}
	return result;
}

double
MetaObject::getPropertyValue(parProperty* prop, int which){
	if(prop != NULL){
		std::map<parProperty*,std::list<double>>::const_iterator it=properties.find(prop);
		if (it != properties.end()){
			if (this->instance){
				return (*it).second.front();
			}
			else{
				std::list<double>::const_iterator it2 = (*it).second.begin();
				advance(it2, which);
				return (*it2);
			}
		}
	}
	return -1;
}

///////////////////////////////////////////////////////////////////////////////
//Gets the current properties hooked up from the database. This is a top-down
//procedure, where all 
//////////////////////////////////////////////////////////////////////////////
void
MetaObject::setupProperties(){
	int num_props = actionary->getNumProperties(this);
	if (num_props == 0){
		if (parent != NULL){
			this->setAllProperties(parent->getAllProperties());
		}
	}
	else{
		for (int i = 0; i < num_props; i++){ //In the database, we start our limit at 1
			parProperty* prop = actionary->getProperty(this, i);
			if (prop != NULL){ //Safety sanity check
				int num_vals = actionary->getNumProperties(this, prop);
				for (int j = 0; j < num_vals; j++){
					double val = (double)actionary->getProperty(this, prop, j);
					this->setProperty(prop, val);
				}
			}
		}
		/*This section gets all the properties from the parent that do not explicitly exist in the child*/
		for (std::map<parProperty*, std::list<double>>::const_iterator it = parent->getAllProperties().begin(); it != parent->getAllProperties().end(); it++){
			std::map<parProperty*, std::list<double>>::const_iterator it2 = this->properties.find((*it).first);
			if (it2 == this->properties.end()){
				this->properties[(*it).first] = (*it).second;
			}
		}
	}
}

void
MetaObject::setAllProperties(std::map<parProperty*, std::list<double>>& properties){
	std::map<parProperty*, std::list<double>>::const_iterator it;
	for (it = properties.begin(); it != properties.end(); it++){
		std::list<double>::const_iterator it2;
		for (it2 = (*it).second.begin(); it2 != (*it).second.end(); it2++){
			this->setProperty((*it).first, (*it2)); //We do not update the parents because this method is used in both top-down
			//and bottom-up processing
		}
	}
}
///////////////////////////////////////////////////////////////////////////////
//This basically calls the actionary's check for affordance
//and returns true if the object can be used in that position
///////////////////////////////////////////////////////////////////////////////
bool
MetaObject::searchAffordance(MetaAction *act, int which){
	if(act == NULL || which <0 || which > act->getNumObjects())
		return false;

	int all_pos=actionary->searchAffordance(act,this);
		if(all_pos==which)
			return true;

	//Breaks the recursion
	if(this->parent == NULL)
		return false;

	return this->parent->searchAffordance(act,which);
}

MetaAction*
MetaObject::searchAffordance(int position, int which){
	return actionary->searchAffordance(this,position,which);
}