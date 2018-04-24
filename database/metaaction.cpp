#include <cstdio>
#include <string>
#include "metaaction.h"
#include "actionary.h"
#include "par.h"
#include "interpy.h"

extern Actionary *actionary; // global actionary pointer

MetaAction::MetaAction(const char* aname)
	:name(aname),parents(std::vector<MetaAction*>()),
	properties(std::map<parProperty*, std::list<double>>()),
	once(false),
	every(false),
	exec(false),
	prep(false){
	int num_parents = actionary->getNumParents(this);
	if ( num_parents > 0){
		this->parents.resize(num_parents, NULL);
		for (int i = 0; i < num_parents; i++){
			MetaAction *p = actionary->getParent(this, i);
			if (p != NULL){
				parents[i] = p;
			}
		}
	}
	// add this object to the database
	actID = actionary->addAction(this, aname,parents);
	num_objects=actionary->getNumObjects(this);
	site_type=actionary->getSiteType(this);
	this->setupProperties();
	actionary->allActions.push_back(actID);
}
MetaAction
::MetaAction(const char* str,MetaAction* aparent)
	:name(str),
	parents(std::vector<MetaAction*>()),
	properties(std::map<parProperty*, std::list<double>>()),
	once(false),
	every(false),
	exec(false),
	prep(false){
	parents.push_back(aparent);
	actID=actionary->addAction(this,str,parents);
	num_objects=actionary->getNumObjects(this);
	site_type=actionary->getSiteType(this);
	this->setupProperties();
	actionary->allActions.push_back(actID);

}
// just store the id
MetaAction::MetaAction(int act_ID):
actID(act_ID),
parents(std::vector<MetaAction*>()),
properties(std::map<parProperty*, std::list<double>>()),
once(false),
every(false),
exec(false),
prep(false){
	int num_parents = actionary->getNumParents(this);
	if (num_parents > 0){
		this->parents.resize(num_parents, NULL);
		for (int i = 0; i < num_parents; i++){
			MetaAction *p = actionary->getParent(this, i);
			if (p != NULL){
				parents[i] = p;
			}
		}
	}
	name =actionary->getActionName(this).c_str();
	num_objects=actionary->getNumObjects(this);
	site_type=actionary->getSiteType(this);
	//num_objects=actionary->getNumObjects(this);
	this->setupProperties();
}

void    
MetaAction::setActionName(std::string newName)
{
	name = newName;
	actionary->setActionName(this, newName);
}

void 
MetaAction::setParent(MetaAction* aparent,int which)
{
	if (which > this->parents.size()){
		this->parents.resize(which, NULL);
	}
	parents[which] = aparent;
	actionary->setParent(this, aparent);
}

MetaAction* 
MetaAction::getParent(int which)
{
	if(!this->parents.empty() && which > -1 && which < this->parents.size() && this->parents[which] != NULL)
		return this->parents[which];
	else
		return actionary->getParent(this,which);
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
MetaAction::setProperty(parProperty* prop, double value, bool pass_parent){
	if (prop != NULL && prop->getType() != 0 && value > -1){
		if (!this->hasProperty(prop,value)){ //Two cases, the property doesn't exist or the value doesn't
			//Case 1, property doesn't exist
			std::map<parProperty*, std::list<double>>::const_iterator it = this->properties.find(prop);
			if (it == this->properties.end()){
				this->properties[prop] = std::list<double>();
				
			}
			//Case 1 and 2, the property exists but the value t
			this->properties[prop].push_back(value);
			if (!this->parents.empty() && pass_parent){
				for (int i = 0; i < this->parents.size(); i++){
					if (!this->parents[i]->hasProperty(prop, value)) //If the parent doesn't have it, then it really should
						this->parents[i]->setProperty(prop, value);
				}
			}
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
		if (!this->parents.empty()){
			for (int i = 0; i < this->getNumParents(); i++){
				std::map<parProperty*, std::list<double> > props = this->getParent(i)->getAllProperties();
				for (std::map<parProperty*, std::list<double> >::const_iterator it = props.begin(); it != props.end(); it++){
					std::map<parProperty*, std::list<double> >::iterator it2 = this->properties.find((*it).first);
					if (it2 != this->properties.end()){
						//Perform intersection
						std::list<double> intersection;
						std::set_intersection((*it).second.begin(), 
											  (*it).second.end(), 
											  (*it2).second.begin(), 
											  (*it2).second.end(),
											  std::back_inserter(intersection));
						this->properties[(*it).first] = intersection;
					}
					else{
						//KNOWN BUG: if no other parent has this property, then the property shouldn't be added
						properties[(*it).first] = std::list<double>();
						for (std::list<double>::const_iterator it3 = (*it).second.begin(); it3 != (*it).second.end(); it3++)
							properties[(*it).first].push_back((*it3));
					}
				}
			}
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
				this->properties[prop].sort(); //Makes it useful if it is 
			}
		}
		for (int i = 0; i < this->getNumParents(); i++){
			for (std::map<parProperty*, std::list<double>>::const_iterator it = parents[i]->getAllProperties().begin(); it != parents[i]->getAllProperties().end(); it++){
				std::map<parProperty*, std::list<double>>::const_iterator it2 = this->properties.find((*it).first);
				if (it2 == this->properties.end()){
					this->properties[(*it).first] = (*it).second;
				}
			}
		}
	}
}

void
MetaAction::setAllProperties(std::map<parProperty*, std::list<double>>& properties){
	std::map<parProperty*, std::list<double>>::const_iterator it;
	for (it = properties.begin(); it != properties.end(); it++){
		std::list<double>::const_iterator it2;
		for (it2 = (*it).second.begin(); it2 != (*it).second.end(); it2++){
			this->setProperty((*it).first, (*it2)); //We do not update the parents because this method is used in both top-down
			//and bottom-up processing
		}
	}
}
