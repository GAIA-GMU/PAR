#include <time.h>
#include <windows.h>
#include "par.h"
#include "workingmemory.h"
#include "agentnet.h"
#include "parnet.h"
#include "actionary.h"

extern Actionary* actionary;
extern parTime* partime;
float getTimeToObject(MetaObject* agentName, MetaObject* obj); // defined in user's file

void
AgentNet::actfunc_start(void)
{
}

void
AgentNet::actfunc_curraction(void)
{
}

void
AgentNet::actfunc_preempt(void)
{
  // Get the priority of the first action on the queue.
	iPAR* cfipar = this->getFirstAction();
  
	if (ipar != NULL && this->readyToGo(cfipar))
		agentproc->ipt->preempt(cfipar->getPriority()); // still doesn't take time into account time
			//If we have an action that is pre-empted, we need to check in all the parNets and not run their command if so
		
}

iPAR* 
AgentNet::getFirstAction(void)
{
  // return the action on the queue that has the least startTime
  int i;
  // check in the queue
  iPAR *cfipar = NULL; // current first ipar on the queue
  iPAR *pfipar = NULL; // ipar that's start time has passed with the highest priority
  double tempTime;
  double currentTime = partime->getCurrentTime();
  int priority = 0;

  par_debug("QueueMgr: there are %d items on the  iparQ\n",iparQ->size());
  // Find the ipar with the smallest startTime
  std::list<iPAR*>::iterator iter;
  for(iter=iparQ->begin(),i=0; iter!=iparQ->end(); iter++,i++)
  {
	  if (i == 0)
	  {
			cfipar = *iter;
			tempTime = (*iter)->getStartTime();
	  }
	  else
	  {
		  if ((*iter)->getStartTime() < tempTime)
		  {
			  cfipar = *iter;
			  tempTime = (*iter)->getStartTime();
		  }
	  }

	  if ((*iter)->getStartTime() <= currentTime && 
		  (*iter)->getPriority() > priority)
	  {
		  priority = (*iter)->getPriority();
		  pfipar = *iter;
	  }
		

  }

  // decide between cfipar and pfipar
  if (pfipar == NULL)
	return cfipar;

  if (priority > cfipar->getPriority())
	  return pfipar;

  return cfipar;
}


bool	
AgentNet::readyToGo(iPAR* cfipar)
{
	// is this the timing such that this ipar is ready to be executed?
	double tempTime = cfipar->getStartTime(); 

	if (tempTime <= partime->getCurrentTime()){
		return true;
	}

	// if there is an object parameter, get the distance from the current agent location to this 
	// object.  If the action isn't locomoting, then the agent will likely have to locomote to the object
	// We will pop the action sooner to try to allow for the time it will take.  
	// We will always assume that the first object is the salient object.
	if (cfipar->par->getNumObjects() > 0)
	{
		MetaObject* blah = cfipar->getObject(0);
		if (blah != NULL)
		{
			MetaObject *obj = cfipar->getObject(0);
			assert(obj);
			par_debug("In AgentNet::readyToGo, object is %s\n",obj->getObjectName().c_str());
			
			
			//Vector<3> *objPos = obj->getPosition();
			std::string agentName = cfipar->getAgent()->getObjectName();
			//MetaObject *agent = actionary->searchByNameObj(agentName);
			//assert(agent);
			/*Vector<3> *agtPos = agent->getPosition();
			float dist;
				
			dist = (objPos->v[0] - agtPos->v[0]) * (objPos->v[0] - agtPos->v[0]);
			dist = dist + (objPos->v[1] - agtPos->v[1]) * (objPos->v[1] - agtPos->v[1]);
			dist = dist + (objPos->v[2] - agtPos->v[2]) * (objPos->v[2] - agtPos->v[2]);
			dist = sqrt(dist);*/

			double adjustedTime = tempTime - getTimeToObject(cfipar->getAgent(), obj);
				//(dist * 0.6897 * 10);  // assuming walking speed of 1.45m/s
			par_debug("Adjusted time is %f\n", adjustedTime);
			if (adjustedTime <= partime->getCurrentTime())
				return true;
		}
	}
	
	return false;
}

void
AgentNet::actfunc_popaction(void)
{
  iPAR *cfipar = NULL; // current first ipar on the queue
 
  cfipar = this->getFirstAction();
  if (cfipar == NULL)
	  return;

  if (this->readyToGo(cfipar)) // is the timing right?
  {
	ipar = cfipar;
	ipar->copyValuesToPython();//At this point, the action is a go, and so we assign values from it into python
	iparQ->remove(cfipar);
	par_debug("QueueMgr: now popped action %s from the queue\n",ipar->par->getActionName().c_str());
	markfinished();
  }
}

void
AgentNet::actfunc_paraction(void)
{
  // This is the action that has just popped out from the queue and
  // so is at the highlest level in the queue. Hence its complex Parent is 0
  //ipar->complexParent = 0;
  ComplexObj *complexobj = new ComplexObj(ipar,0);
  LWNetList::addnet(new ParNet(complexobj));
}

bool
AgentNet::nonemptyQ(void)
{
  return (!(iparQ->empty()));
}

bool
AgentNet::emptyQ(void)
{
  return iparQ->empty();
}

bool
AgentNet::condfunc_poperror(void)
{
  return popError;
}

bool
AgentNet::activeAction(void)
{
  return agentproc->ipt->activeAction();
}


bool
AgentNet::noFailAction(void)
{
  return !(agentproc->ipt->failAction());
}

AgentNet::AgentNet(std::list<iPAR*> *IParQ, AgentProc *ap)
  :LWNet(NUMNODES)
{

  iparQ = IParQ;
  agentproc = ap;

  ipar = 0;
  popError = 0;

  defnormalnode(START, (ACTFUNC)&AgentNet::actfunc_start);
  deftrans(START, (CONDFUNC) &AgentNet::nonemptyQ, CURRACTION);

  defnormalnode(CURRACTION, (ACTFUNC)&AgentNet::actfunc_curraction);
  deftrans(CURRACTION,(CONDFUNC)&AgentNet::emptyQ, START);
  deftrans(CURRACTION,(CONDFUNC)&AgentNet::activeAction, PREEMPT);
  deftrans(CURRACTION,(CONDFUNC)&AgentNet::noFailAction, POPACTION);

  defnormalnode(PREEMPT, (ACTFUNC)&AgentNet::actfunc_preempt);
  deftrans(PREEMPT,(CONDFUNC)&LWNet::defaultcond, START);

  defnormalnode(POPACTION, (ACTFUNC)&AgentNet::actfunc_popaction);
  deftrans(POPACTION,(CONDFUNC)&AgentNet::condfunc_poperror, EXITNODE);
  deftrans(POPACTION, (CONDFUNC)&LWNet::finishcond, PARACTION);

  defnormalnode(PARACTION, (ACTFUNC)&AgentNet::actfunc_paraction);
  deftrans(PARACTION,(CONDFUNC)&LWNet::defaultcond, START);

  defexitnode(EXITNODE);
}


