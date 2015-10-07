#include "Python.h"
#include <ctime>
#include <fstream>
#include "workingmemory.h"
#include "interpy.h"
#include "par.h"
#include <string>
#include "Python.h"
#include <iostream>
#include "windows.h"


extern parTime* partime;
extern Actionary *actionary; // global actionary pointer
extern char* actionLocation;

iPAR::iPAR(std::string actname,const std::string& agname, std::vector<char*> *objname):
	fail_data(NULL),start_time(partime->getCurrentTime()),duration(-1),priority(1),enabled_act(-1),enable_act(-1),finished(false),manner(""){
	//First, we check to make sure we're actually sending an agent
	agent=actionary->searchByNameObj(agname);
	//If there is not an agent, then the action cannot be performed
	if(agent == NULL || !agent->isAgent()){
		char failbuf[MAX_FAILBUF];
		sprintf_s(failbuf,MAX_FAILBUF,"%s is not an agent in the system, aborting",agname.c_str());
		throw iPARException(std::string(failbuf));
	}
	
	MetaAction* action = actionary->searchByNameAct(actname);  // creates a new entry in the Actionary with a new id.
	//Likewise, if the action is undefined, an instance of it cannot be created
	if(action == NULL){
		char failbuf[MAX_FAILBUF];
		sprintf_s(failbuf,MAX_FAILBUF,"%s has no parent action, aborting\n",actname.c_str());
		throw iPARException(std::string(failbuf));
	}
	par=action;
	
	//Gives the ipar an id, and adds it to the map
	iparID=actionary->findNewPARID();
	actionary->addiPAR(iparID,this);
	createPyiPARs(this);

	int num = (int)objname->size(); 
	if (num <= 0) return;
	
	MetaObject* obj;
	for (int i = 0; i < num; i++) 
	{
			obj = actionary->searchByNameObj((*objname)[i]);
			if (obj != NULL)
				objects.push_back(obj);
	}
	fail_parent = true; //The parent always fails unless we give it permission not to
}

iPAR::iPAR(std::string actname, const std::string& agname, char* objname[]):
	fail_data(NULL),start_time(partime->getCurrentTime()),duration(-1),priority(1),enabled_act(-1),enable_act(-1),finished(false),manner(""){
	//First, we check to make sure we're actually sending an agent
	agent=actionary->searchByNameObj(agname);
	//If there is not an agent, then the action cannot be performed
	if(agent == NULL ||!agent->isAgent()){
		char failbuf[MAX_FAILBUF];
		sprintf_s(failbuf,MAX_FAILBUF,"%s is not an agent in the system, aborting",agname.c_str());
		throw iPARException(std::string(failbuf));
	}
	MetaAction* action = actionary->searchByNameAct(actname);  // creates a new entry in the Actionary with a new id.
		//Likewise, if the action is undefined, an instance of it cannot be created
	if(action == NULL){
		char failbuf[MAX_FAILBUF];
		sprintf_s(failbuf,MAX_FAILBUF,"%s has no parent action, aborting\n",actname.c_str());
		throw iPARException(std::string(failbuf));
	}
	par=action;
	
	//Gives the ipar an id, and adds it to the map
	iparID=actionary->findNewPARID();
	actionary->addiPAR(iparID,this);
	createPyiPARs(this);
	int num = actionary->getNumObjects(par); 	 // THIS IS A PROBLEM WHEN OPTIONAL OBJECTS ARE NOT GIVEN

	if (num <= 0) return;
	
	MetaObject* obj;
	for (int i = 0; i < num; i++) 
	{
			obj = actionary->searchByNameObj(objname[i]);
			if (obj != NULL)
				objects.push_back(obj);
	}
	fail_parent = true;
}

// set the objects separately
iPAR::iPAR(std::string actname, const std::string& agname):
	fail_data(NULL),start_time(partime->getCurrentTime()),duration(-1),priority(1),enabled_act(-1),enable_act(-1),finished(false),manner(""){
	//First, we check to make sure we're actually sending an agent
	agent=actionary->searchByNameObj(agname);
	//If there is not an agent, then the action cannot be performed
	if(agent == NULL || !agent->isAgent()){
		char failbuf[MAX_FAILBUF];
		sprintf_s(failbuf,MAX_FAILBUF,"%s is not an agent in the system, aborting",agname.c_str());
		throw iPARException(std::string(failbuf));
	}
	
	MetaAction* parParent = actionary->searchByNameAct(actname);
	//Likewise, if the action is undefined, an instance of it cannot be created
	if(parParent == NULL){
		char failbuf[MAX_FAILBUF];
		sprintf_s(failbuf,MAX_FAILBUF,"%s has no parent action, aborting\n",actname.c_str());
		throw iPARException(std::string(failbuf));
	}
	//MetaAction* action = actionary->create(actname, parParent);  // creates a new entry in the Actionary with a new id.
	par = parParent;
	//Gives the ipar an id, and adds it to the map
	iparID=actionary->findNewPARID();
	actionary->addiPAR(iparID,this);
	createPyiPARs(this);
	fail_parent = true;
}

// these iPARs come straight from the Actionary
iPAR::iPAR(int actID):
	fail_data(NULL),start_time(partime->getCurrentTime()),duration(-1),priority(1),enabled_act(-1),enable_act(-1),finished(false),manner(""){
	MetaAction* action = actionary->searchByIdAct(actID);
	if(action == NULL){
		char failbuf[MAX_FAILBUF];
		sprintf_s(failbuf,MAX_FAILBUF,"%d has no parent action, aborting\n",actID);
		throw iPARException(std::string(failbuf));
	}
	par = action;
	fail_parent = true;
}

// return a copy of this iPAR
iPAR* 
iPAR::copyIPAR()
{
	iPAR* newPAR = new iPAR(this->par->getActionName(), this->getAgent()->getObjectName());
	if (newPAR == NULL)
	{
		std::cout << "ERROR: couldn't create a new PAR" << std::endl;
		return NULL;
	}

	int num = this->par->getNumObjects();
	for (int i = 0; i < num; i++)
	{
		newPAR->setObject(this->getObject(i), i);	
	}
	
	// purpose
	newPAR->par->setPurposeAchieve(this->par->getPurposeAchieve());
	// purpose
	newPAR->setPurposeEnableAct(this->getPurposeEnableAct());
	// parent
	newPAR->par->setParent(this->par->getParent());
	// previous action
	//newPAR->par->setPrevAction(this->par->getPrevAction());
	// start_time
	newPAR->setStartTime(this->getStartTime());
	// duration
	newPAR->setDuration(this->getDuration());
	// priority
	newPAR->setPriority(this->getPriority());
	// path_id

	// next action
	//Gives the ipar an id, and adds it to the map
	iparID=actionary->findNewPARID();
	actionary->addiPAR(iparID,this);
	createPyiPARs(this);
	/*Since we haven't started the action, we certainly haven't finished it*/
	finished=false;
	newPAR->setFailParent(this->getFailParent());
	// execution steps
	// act_group



	return newPAR;

}
///////////////////////////////////////////////////////////////////////////////
//This simple function allows for more python-esque scripting. It is only called
//when the iPAR is about to run
///////////////////////////////////////////////////////////////////////////////
void 
iPAR::copyValuesToPython(){addPyiPARValues(this);}
///////////////////////////////////////////////////////////////////////////////
//This is the get and set features for the agent.  I'm not sure if set stuff
//is nessicary after building the iPAR, but I guess you could pass an iPAR off
//to another agent
///////////////////////////////////////////////////////////////////////////////
MetaObject*
iPAR::getAgent(){

	return agent;
}
void
iPAR::setAgent(MetaObject* agent_obj){
	agent=agent_obj;
}
void
iPAR::setAgent(const std::string&  agname){
	MetaObject *agent_obj=actionary->searchByNameObj(agname);
	setAgent(agent_obj);
  }
///////////////////////////////////////////////////////////////////////////////
//Manages all the object stuff within the iPAR
///////////////////////////////////////////////////////////////////////////////
void	
iPAR::setObject(MetaObject* obj, int which){
	
	if(objects.size() <= (unsigned int)which){
		if(objects.size() <(unsigned int)which){ 
			do{
				objects.push_back(NULL);
			}while(objects.size() < (unsigned int)which);

		}
		objects.push_back(obj);
	}

	//Added this to make sure objects are being placed in the proper place
	else{
		objects.at(which)=obj;
	}

}

MetaObject* 
iPAR::getObject(int which)
{
	if(objects.size() <= (unsigned int) which || which < 0 )
		return NULL;
	return objects.at(which);
}
///////////////////////////////////////////////////////////////////////////////
//This gives time management to the iPAR
///////////////////////////////////////////////////////////////////////////////
double
iPAR::getStartTime()
{
	return start_time;
}

void
iPAR::setStartTime(double t)
{
	start_time=t;
}

void
iPAR::setStartTime(int hours, int minutes, int seconds)
{
	int startTime = hours*3600+minutes*60+seconds;  //convert to all seconds
	start_time=startTime;
}

float
iPAR::getDuration()
{
	if(duration == -1)
		duration=actionary->getDuration(par);
	return duration;
}

void
iPAR::setDuration(float t)
{
	duration=t;
}
///////////////////////////////////////////////////////////////////////////////
//Allows the iPAR to set and have priorities in order to sort them
///////////////////////////////////////////////////////////////////////////////
int			
iPAR::getPriority()
{
	return priority;
}
 
void
iPAR::setPriority(int prior)
{
	priority=prior;
}
///////////////////////////////////////////////////////////////////////////////
//Allows the iPARS to track which iPAR created them
///////////////////////////////////////////////////////////////////////////////
void
iPAR::setPurposeEnableAct(int par_id){
	enable_act=par_id;
}
int
iPAR::getPurposeEnableAct(){
	return enable_act;	
};
///////////////////////////////////////////////////////////////////////////////
//Allows the ipars to track it's immediate child.  So, if an ipar spawns another
//ipar in prep-spec, it would go here.  Also, if the ipar is a complex ipar,
//the last spawned ipar is here, which also generally is the ipar that is 
//running at the moment.
///////////////////////////////////////////////////////////////////////////////
void
iPAR::setEnabledAct(int par_id){
	enabled_act=par_id;
}
int
iPAR::getEnabledAct(){
	return enabled_act;	
};
///////////////////////////////////////////////////////////////////////////////
//These two functions allow us to get and set action specific propertys. These
//should change from iPAR to iPAR
///////////////////////////////////////////////////////////////////////////////
void 
iPAR::setProperty(parProperty* prop,int value){
	if(prop != NULL && value > -1)
		this->properties[prop]=value;
}
int 
iPAR::getProperty(parProperty* prop){
	if(prop == NULL)
		return -1;
	std::map<parProperty*,int>::const_iterator it=this->properties.find(prop);
	if(it != this->properties.end())
		return (*it).second;
	return -1;
}
///////////////////////////////////////////////////////////////////////////////
//Since we have a map, this allows us to iterate through all the set property
//types that we have assigned to this iPAR
///////////////////////////////////////////////////////////////////////////////
parProperty*
iPAR::getPropertyType(int which){
	if(which < 0)
		return NULL;
	std::map<parProperty*,int>::const_iterator it=this->properties.begin();
	advance(it,which);
	if(it == this->properties.end())
		return NULL;
	return (*it).first;
}
///////////////////////////////////////////////////////////////////////////////
//The manner of an action designates how it is to be performed. This can be
//considered an adverb modifing clause to the iPAR
///////////////////////////////////////////////////////////////////////////////
void
iPAR::setManner(const std::string & mann){
	this->manner=mann;
}
std::string
iPAR::getManner(){
	return manner;
}
///////////////////////////////////////////////////////////////////////////////
//Python specific iPAR properties
///////////////////////////////////////////////////////////////////////////////
/*PyObject *
iPAR::testCulminationCond()
{
	return actionary->testCulminationCond(this);
}
*/

static char* conditionNames[] = {"applicability_condition", "culmination_condition", 
		"preparatory_spec", "execution_steps","during_assertion","post_assertion"};


PyObject*
Actionary::testCondition(iPAR* ipar, int which) {
   char buf[MAXBUF];
   char *argstr = new char[MAXBUF];
   MetaObject *obj;
   sprintf_s(argstr,MAXBUF,"%d",ipar->getAgent()->getID());
   // strcpy_s(argstr, MAXBUF, singleQuote(ipar->getAgent()->getObjectName().c_str()));//Gets the agent from the instanced par
   int num = ipar->par->getNumObjects();
   //Finds all the objects the par requires to test it's condition
   for (int i = 0; i < num; i++) {
      strcat_s(argstr, MAXBUF, ", ");
	  obj=ipar->getObject(i);
	  if (obj != NULL)
		sprintf_s(argstr,MAXBUF,"%s%d",argstr,obj->getID());
		//strcat_s(argstr, MAXBUF, singleQuote(obj->getObjectName().c_str()));
	  else
		sprintf_s(argstr,MAXBUF,"%s%d",argstr,-1);
		//strcat_s(argstr, MAXBUF, singleQuote(" "));
   }
   //Actually tests the condition
   sprintf_s(buf, MAXBUF,"%s_%d.%s(%s)\n", ipar->par->getActionName().c_str(),ipar->getID(),conditionNames[which], argstr);
   //debug("%s\n",buf);
   delete [] argstr;

   return runPy(buf);
}

PyObject*
Actionary::testApplicabilityCond(iPAR* ipar) {
   return testCondition(ipar, 0);
}

PyObject*
Actionary::testCulminationCond(iPAR* ipar) {
  return testCondition(ipar, 1);
}

PyObject*
Actionary::testPreparatorySpec(iPAR* ipar) {
  return testCondition(ipar, 2);
}

PyObject*
Actionary::testExecutionSteps(iPAR* ipar) {
  return testCondition(ipar, 3);
}

PyObject*
Actionary::testDuringAssertions(iPAR* ipar) {
	return testCondition(ipar,4);
}

PyObject*
Actionary::testPostAssertions(iPAR* ipar) {
	return testCondition(ipar,5);
}

int 
Actionary::setCondition(MetaAction* act, int which)
{
	char buf[MAXBUF], className[64];
	char locbuf[MAXBUF];
	int res;

	if (act == NULL) 
		return -1;

	std::string str;
	switch(which) {
		case 0:
			str = this->getApplicabilityCond(act);
			break;
		case 1:
			str = this->getCulminationCond(act);
			break;
		case 2:
			str = this->getPreparatorySpec(act);
			break;
		case 3:
			str = this->getExecutionSteps(act);
			break;
		default:
			std::cerr << "Error in Actionary::setCondition" << std::endl;
	}

	
	if (!strcmp(str.c_str(),"no") || !strcmp(str.c_str()," ") ||!strcmp(str.c_str(),""))
		return -1;
	strcpy_s(locbuf,512,actionLocation);
	strcat_s(locbuf,512,str.c_str());
	par_debug("In Actionary::setCondition, str = %s\n", str.c_str());
	res = runPySimple(locbuf, true); // str is the filename
	   if (res < 0) 
	   {
			//debug("Returning early from setCondition\n");
			return res;
	   }

	//debug("In setCondition for action %s with id %d\n",act->getActionName(),act->getID());
	sprintf_s(className, 64, "Class%s", act->getActionName().c_str());
	sprintf_s(buf, 128, "%s.%s = %s\n", className,
		   conditionNames[which], conditionNames[which]);
	int result = PyRun_SimpleString(buf);
	return result;
}

int 
Actionary::loadApplicabilityCond(MetaAction* act)
{
	  return setCondition(act, 0);
}

int 
Actionary::loadCulminationCond(MetaAction* act)
{
	return setCondition(act, 1);
}

int 
Actionary::loadPreparatorySpec(MetaAction* act)
{
	return setCondition(act, 2);
}

int 
Actionary::loadExecutionSteps(MetaAction* act)
{
	return setCondition(act, 3);
}

