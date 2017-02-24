#include <cstdio>
#include <string>
#include "metaaction.h"
#include "actionary.h"
#include "par.h"
#include "interpy.h"

extern Actionary *actionary; // global actionary pointer

MetaAction::MetaAction(const char* aname)
	:name(aname),parent(NULL),
	properties(std::map<parProperty*, std::list<double>>()){
	parent=this->getParent();
	// add this object to the database
	actID = actionary->addAction(this, aname,parent);
	num_objects=actionary->getNumObjects(this);
	site_type=actionary->getSiteType(this);
	this->setupProperties();
	actionary->allActions.push_back(actID);
}
MetaAction
::MetaAction(const char* str,MetaAction* aparent)
	:name(str),parent(aparent),
	properties(std::map<parProperty*, std::list<double>>()){
	actID=actionary->addAction(this,str,aparent);
	num_objects=actionary->getNumObjects(this);
	site_type=actionary->getSiteType(this);
	this->setupProperties();
	actionary->allActions.push_back(actID);

}
// just store the id
MetaAction::MetaAction(int act_ID):
	actID(act_ID),parent(NULL),
	properties(std::map<parProperty*, std::list<double>>()){
	parent=this->getParent();
	name =actionary->getActionName(this).c_str();
	num_objects=actionary->getNumObjects(this);
	site_type=actionary->getSiteType(this);
	num_objects=actionary->getNumObjects(this);
	this->setupProperties();
}

void    
MetaAction::setActionName(std::string newName)
{
	name = newName;
	actionary->setActionName(this, newName);
}

void 
MetaAction::setParent(MetaAction* aparent)
{
	parent=aparent;
	actionary->setParent(this, aparent);
}

MetaAction* 
MetaAction::getParent()
{
	if(parent != NULL)
		return parent;
	else
		return actionary->getParent(this);
}


void	
MetaAction::setApplicabilityCond(const std::string& appCond)
{
	actionary->setApplicabilityCond(this, appCond);
}

std::string	
MetaAction::getApplicabilityCond()
{
	return actionary->getApplicabilityCond(this);
}
	
void	
MetaAction::setTerminationCond(const std::string& termCond)
{
	actionary->setCulminationCond(this, termCond);
}
	
std::string	
MetaAction::getTerminationCond()
{
	return actionary->getCulminationCond(this);
}
		
void	
MetaAction::setPreparatorySpec(const std::string& prepSpec)
{
	actionary->setPreparatorySpec(this, prepSpec);
}
	
std::string	
MetaAction::getPreparatorySpec()
{
	return actionary->getPreparatorySpec(this);
}

void	
MetaAction::setPurposeAchieve(const std::string& achieve)
{
	actionary->setPurposeAchieve(this, achieve);
}

void	
MetaAction::setExecutionSteps(const std::string& execSteps)
{
	actionary->setExecutionSteps(this, execSteps);
}

std::string	
MetaAction::getExecutionSteps()
{
	return actionary->getExecutionSteps(this);
}
	
std::string 	
MetaAction::getPurposeAchieve()
{
	return actionary->getPurposeAchieve(this);
}
	
	
void	
MetaAction::setNumObjects(int num)
{
	num_objects=num;
	actionary->setNumObjects(this, num);
}


	
float	
MetaAction::getDuration()
{
	return actionary->getDuration(this);
}
	
void	
MetaAction::setDuration(float d)
{
	actionary->setDuration(this, d);
}


MetaObject *
MetaAction::searchAffordance(int position,int which){
	return actionary->searchAffordance(this,position,which);
}

std::string	
MetaAction::getAdverb()	// later allow for more than one adverb
{
	return actionary->getAdverb(this);
}

std::string	
MetaAction::getModifier()	// later allow for more than one modifier
{
	return actionary->getModifier(this);
}

void	
MetaAction::setAdverb(std::string& adverb, std::string& modifier)
{
	actionary->setAdverb(this, adverb, modifier);
}


int	
MetaAction::getSiteType()
{
	return site_type;
}

///////////////////////////////////////////////////////////////////////////////
//These two functions allow us to get and set action specific propertys. These
//should change from iPAR to iPAR
///////////////////////////////////////////////////////////////////////////////
void
MetaAction::setProperty(parProperty* prop, double value){
	if (prop != NULL && prop->getType() != 0 && value > -1){
		this->properties[prop].push_back(value);
		if (this->parent != NULL){
			if (!this->parent->hasProperty(prop, value)) //If the parent doesn't have it, then it really should
				this->parent->setProperty(prop, value);
		}
	}
}
double
MetaAction::getProperty(parProperty* prop, int which){
	if (prop == NULL)
		throw iPARException("Property was passed as NULL");
	std::map<parProperty*, std::list<double>>::const_iterator it = this->properties.find(prop);
	if (it != this->properties.end()){
		std::list<double>::const_iterator it2 = (*it).second.begin();
		advance(it2, which);
		if (it2 != (*it).second.end()){
			return (*it2);
		}
	}
	throw iPARException("The property does not exist in the action");
}
///////////////////////////////////////////////////////////////////////////////
//This lets us know if the property "Exists" in par
bool 
MetaAction::hasProperty(parProperty* prop, double value){
	bool found = false;
	if (prop == NULL)
		return found;
	std::map<parProperty*, std::list<double>>::const_iterator it = this->properties.find(prop);
	if (it != this->properties.end()){
		if (!(*it).first->isInt()){
			for (std::list<double>::const_iterator it2 = (*it).second.begin(); it2 != (*it).second.end(); it2++){
				if ((*it2) == value)
					found = true;
			}
		}
		else{ //Otherwise, we just have to make sure it is between the values
			if ((*it).second.size() > 2){
				double min = (*it).second.front();
				double max = (*it).second.back();
				if (min > value && max < value)
					found = true;
			}
			else if ((*it).second.size() == 1){
				if ((*it).second.front() == value)
					found = true;
			}
		}
	}
	return found;
}
///////////////////////////////////////////////////////////////////////////////
//Since we have a map, this allows us to iterate through all the set property
//types that we have assigned to this iPAR
///////////////////////////////////////////////////////////////////////////////
parProperty*
MetaAction::getPropertyType(int which){
	if (which < 0)
		return NULL;
	std::map<parProperty*, std::list<double>>::const_iterator it = this->properties.begin();
	advance(it, which);
	if (it == this->properties.end())
		return NULL;
	return (*it).first;
}
///////////////////////////////////////////////////////////////////////////////
//Gets the current properties hooked up from the database
//////////////////////////////////////////////////////////////////////////////
void
MetaAction::setupProperties(){
	int num_props = actionary->getNumProperties(this);
	if (num_props == 0){
		if (parent != NULL){
			this->properties = parent->getAllProperties();
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
		for (std::map<parProperty*, std::list<double>>::const_iterator it = parent->getAllProperties().begin(); it != parent->getAllProperties().end(); it++){
			std::map<parProperty*, std::list<double>>::const_iterator it2 = this->properties.find((*it).first);
			if (it2 == this->properties.end()){
				this->properties[(*it).first] = (*it).second;
			}
		}
	}
}
