#include "agentproc.h"
#include "agentnet.h"
#include "par.h"
#include "workingmemory.h"
#include "metaaction.h"
#include "actionnet.h"
#include "partable.h"
#include <list>
#include "actionary.h"

extern iPAR *getIPAR(PyObject *obj);
Actionary* actionary;
extern parTime* partime;
extern AgentTable agentTable;
ActionTable actionTable; 
// Used by the action table to figure out the time it takes to reach an object
// This function is used to anticipate the lead up to an action’s start time. If an action is scheduled for a particular time at a particular location, the agent will need to move toward that location before the scheduled start. This method is used to calculate how much of a head start is needed. Currently we’ve set it to 1 second, but in practice it involves an equation that includes the agent’s walk speed and the distance to the location. 
float getTimeToObject(MetaObject* obj, MetaObject* agent){
	return 1.0f;
}

AgentProc::AgentProc(const std::string& name){
  //initialized = 0;

  
  
  // Check if the agent exists in the working memory
	AgentProc *existance = agentTable.getAgent(name);
	MetaObject* agent = NULL;
  //If the agent is not already created, then it should be created
  if(existance ==NULL){
	 MetaObject *parent=actionary->searchByNameObj(actionary->extractParentName(name));//First we attempt to extract the parent from the name
	 if (parent == NULL){
		 parent=actionary->searchByIdObj(actionary->getAgent(0)); //A parent is needed for the Agent Object, we get the first one if we do not have one
	 }
	 assert(parent);
	 agent=actionary->create(name, parent, true);
  }else{
	  //If the agent already exists,then we should not be creating it
	  char failbuf[MAX_FAILBUF];
	  sprintf_s(failbuf,MAX_FAILBUF,"%s already exists, aborting\n",name.c_str());
	  throw iPARException(std::string(failbuf));
  }
  ipt = new IParTable;
  this->name = std::string(name.c_str()); //deep copy
  object = agent; // keep pointer to the corresponding object
  iparq = new std::list<iPAR*>; 
  
  // spawn a patnet for controlling the queue of actions
  LWNetList::addnet(new AgentNet(iparq,this));

  // Add agent to the agent table
  agentTable.addAgent(name,this);
  
  //initialized = 1;
  
}

void
AgentProc::addAction(iPAR *ipar){
	par_debug("AgentProc::addAction %s\n", ipar->par->getActionName().c_str());
	if(ipar == NULL)
		return;//No need to add an ipar that does not exist
	this->iparq->push_back((iPAR *)ipar);
	
	// add to the iparTable - this maintains the current status of
	// the ipar
	ipt->add(ipar,onqueue);
}

// suspend the current action
void 
AgentProc::suspendAction(){
	// if there is an action currently executing, get it and put it on the front of the queue
	iPAR* tempPAR = this->actionExecuting();
	if (tempPAR != NULL){
		// NOTE: the following should actually be a while loop that loops through all of the enabling actions
		// stopping them and then suspending the last not enabling action
		// might be a prep spec for another action
		int enableActID = tempPAR->getPurposeEnableAct();
		if (enableActID > 1) // 1 is definitely a parent
		{
			// stop this action
			tempPAR->setDuration(1.0); // preempts action
			ipt->setStatus(tempPAR, aborted);
			
			// get enabled action
			MetaAction* act = actionary->searchByIdAct(enableActID);
			if (act == NULL)
				return;
			// get the ipar and suspend enabled action
			tempPAR = ipt->getIPAR(act->getActionName().c_str());
			if (tempPAR == NULL)
				return;
		}
		// create new iPAR
		iPAR* newPAR = tempPAR->copyIPAR();
		if (newPAR != NULL)
		{
			// figure out what the new duration should be
			float duration = tempPAR->getDuration();
			if (duration != -999 && duration != 99999) // not valid durations
			{
				int startTime = (int)tempPAR->getStartTime();
				int currentTime = partime->getCurrentTime();
				int timeSpent = currentTime - startTime;
				if ((duration - timeSpent) < 0)
					timeSpent = 0;
				newPAR->setDuration(duration - timeSpent);
			}

			tempPAR->setDuration(1.0); // preempts action
			//actionary->setActionStatus(tempPAR->par, 2); // keep a record of suspension
			ipt->setStatus(tempPAR, aborted);

			newPAR->setStartTime(partime->getCurrentTime() + 20); // start the action later to allow other action to get popped
			this->iparq->push_front(newPAR);
			// update it's status
			ipt->add(newPAR, onqueue);
		}
	}
}
///////////////////////////////////////////////////////////////////////////////
//Adds multiple actions to the queue at once
///////////////////////////////////////////////////////////////////////////////
void
AgentProc::addAction(std::list<iPAR *> &iparlist){
   std::list<iPAR *>::iterator iter;
   for(iter=iparlist.begin(); iter!=iparlist.end(); iter++)
   {
      iPAR *nipar = *iter;
      addAction(nipar);
   }
}

bool
AgentProc::addAction(PyObject *iparlist,std::list<iPAR*> *returnList)
  // Here, we will first extract each individual Python Dictionary
  // representing an ipar from the Python list. Then, we check if
  // another copy of the same ipar is already being processed or is
  // currently on the queue waiting to be processed. If not such
  // additional ipar is found, we add this new python dictionary as is
  // to the queue. The python dictionary is expanded and proocessed
  // later down the queue in parnet
{
  if(iparlist == NULL)
	  return false;

  //list<iPAR*> *returnList = new list<iPAR*>;
  if(PyList_Check(iparlist)){
      for(int i=0; i<PyList_Size(iparlist);i++){
		PyObject *py_ipar = PyList_GetItem(iparlist,i);
		iPAR *ipar = getIPAR(py_ipar); // get the correct pointer
	  
		// Check if this ipar already exists in queue or if it is
		// currently being executed
		if(!(ipt->iparExist(ipar))){
			addAction(ipar);
			if(returnList)
				returnList->push_back(ipar);
	    }
	}
  }
  else
    {
      par_debug("AgentProc: addAction: The input object is not a Python List\n");
      return false;
    }

    return true; 
}


void
AgentProc::inform(char *flag, void *args){
    par_debug("AgentProc::inform and the flag is %s",flag);
}

int 
AgentProc::getAvailableTime()
{
  // return the action on the queue that has the least startTime
  // check in the queue

  iPAR *cfipar = NULL; // current first ipar on the queue
  int tempStartTime;
  int tempAvailTime;
  int tempAvailTime2;
  double currentTime = partime->getCurrentTime();
  std::vector<iPAR*> sortedQueue;
  std::vector<iPAR*>::iterator iter2;

  // Find the ipar with the smallest startTime
  std::list<iPAR*>::iterator iter;
  for(iter=iparq->begin(); iter!=iparq->end(); iter++)
  {
	  if (sortedQueue.empty())
		  sortedQueue.push_back(*iter);
	  else
	  {
		  for (iter2 = sortedQueue.begin(); iter2 != sortedQueue.end(); iter2++)
		  {
			  if ((*iter2)->getStartTime() > (*iter)->getStartTime())
			  {
				  sortedQueue.insert(iter2, (*iter));
				  break;
			  }
		  }
		  if (iter2 == sortedQueue.end())
			  sortedQueue.push_back(*iter);
	  }
  }

  // queue is sorted according to startTimes
  // find at least a 5 minute gap in time
	tempAvailTime = 0;
	tempStartTime = partime->getCurrentTime();
  	  // first check gap from current time plus duration of current action to starting action
  cfipar = this->actionExecuting();
  if (cfipar != NULL)
  {
	  int endTime;
	  int duration = (int)cfipar->getDuration();
	  if (duration != -999 && duration != 99999)
		  endTime = (int)cfipar->getStartTime() + duration;
	  else
		  endTime = (int)cfipar->getStartTime() + 300;  // assume at least a 5 minute duration

	  tempAvailTime = endTime - (int)cfipar->getStartTime();
	  tempStartTime = endTime;
  }
  for (iter2 = sortedQueue.begin(); iter2 != sortedQueue.end(); iter2++)
  {
	  // get start time and add duration of the first action
	  int iterStartTime =(int) (*iter2)->getStartTime();
	  int iterDuration =(int) (*iter2)->getDuration();
	  int iterEndTime;
	  if (iterDuration != -999)
		  iterEndTime = iterStartTime + iterDuration;
	  else
		  iterEndTime = iterStartTime + 180; // assume at least a 3 minute duration

	  // subtract end time from start time of next action
	   if (iter2+1 != sortedQueue.end())
	   {
		   iPAR* ipar2 = *(iter2+1);
		   int nextStartTime =(int) ipar2->getStartTime();
		   tempAvailTime2 = iterEndTime - nextStartTime;
	   }
	   else
		   tempAvailTime2 = 1800;  // just set a time

	   if (tempAvailTime2 - tempAvailTime > 300)  // if the new time is 5 minutes longer
	   {
		   tempStartTime = iterStartTime;
		   tempAvailTime = tempAvailTime2;
	   }
  }

  this->startTimeAvailable = tempStartTime;
  return tempAvailTime;

}
  

void
AgentProc::flushQ(void)
{
  // erase each action from the actionQ and from the ipar table
  std::list<iPAR*>::iterator iter, iter2;
  std::cout << "flushQ: there are " << iparq->size() \
       <<  "items in the iparq" << std::endl;
  
  iter = iparq->begin();
  while(iter != iparq->end())
    {
      iter2 = iter;
      iter++;
      // Delete from ipartable
      iPAR *nipar = *iter2;
	  std::cout << "removing action " << nipar->par->getActionName() \
	   << " from ipartable " << std::endl;
      ipt->remove(nipar);

      // remove from actionQ
      iparq->erase(iter2);
    } 

  // restart the queue
  restartQ();
       
}

void
AgentProc::restartQ(void)
{
  ipt->resetFailure();
}


iPAR* //return a pointer to the par that is running
AgentProc::actionExecuting()
{
	return ipt->actionExecuting();
}

bool
AgentProc::activeAction()
{
	return ipt->activeAction();
}

bool
AgentProc::onQAction()
{
	return ipt->onQAction();
}

iPAR*
AgentProc::searchiPAR(const std::string& ipar_name){
	
	iPAR *found=NULL;
	
	iPAR *searcher=actionExecuting();
	//If we didn't find it executing (a shortcut really), then we should check the table.
	if(searcher == NULL)
		found=ipt->getIPAR(ipar_name.c_str());
	else
		searcher=found;
	
	return found;
}

///////////////////////////////////////////////////////////////////////////////
//Agent Capabilities
///////////////////////////////////////////////////////////////////////////////
void
AgentProc::setCapability(MetaAction *act){
	if(act == NULL)
		return;
	//To avoid duplication, we first make sure
	//the agent isn't already capable of the 
	//given action
	if(!this->searchCapability(act))
		capabilities.push_back(act);
}
void
AgentProc::setCapability(const char *actname){
	MetaAction *act=actionary->searchByNameAct(actname);
	this->setCapability(act);
}
void
AgentProc::removeCapability(MetaAction *act){
	if(act==NULL)
		return;
	capabilities.remove(act);
}

bool
AgentProc::searchCapability(MetaAction *act){
	if(act==NULL)
		return false; //Default action
	
	std::list<MetaAction*>::iterator traveler;
	for(traveler=capabilities.begin(); traveler != capabilities.end(); traveler++){
		if(*traveler == act)
			return true;
	}
	return false;
}