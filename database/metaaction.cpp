#include <cstdio>
#include <string>
#include "metaaction.h"
#include "actionary.h"
#include "par.h"
#include "interpy.h"

extern Actionary *actionary; // global actionary pointer

MetaAction::MetaAction(const char* aname)
	:name(aname),parent(NULL){
	parent=this->getParent();
	// add this object to the database
	actID = actionary->addAction(this, aname,parent);
	num_objects=actionary->getNumObjects(this);
	site_type=actionary->getSiteType(this);
	actionary->allActions.push_back(actID);
}
MetaAction
::MetaAction(const char* str,MetaAction* aparent)
	:name(str),parent(aparent){
	actID=actionary->addAction(this,str,aparent);
	num_objects=actionary->getNumObjects(this);
	site_type=actionary->getSiteType(this);
	actionary->allActions.push_back(actID);

}
// just store the id
MetaAction::MetaAction(int act_ID):
	actID(act_ID),parent(NULL){
	parent=this->getParent();
	name =actionary->getActionName(this).c_str();
	num_objects=actionary->getNumObjects(this);
	site_type=actionary->getSiteType(this);
	num_objects=actionary->getNumObjects(this);
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
