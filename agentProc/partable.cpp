#include "par.h"
#include "partable.h"
#include "actionary.h"
#include "interpy.h"

extern Actionary *actionary;

extern ActionTable actionTable;


void
ActionTable::addFunctions(const char *name, int(*f1)(iPAR *ipar))
{
	 assert(name);
	  assert(f1);

	  //debug("In ActionTable::addFunctions adding %s\n", name);

	  actfunc *actf = new actfunc;
	  actf->func = f1;
	  actf->name = _strdup(name);
	  actions.push_back(actf);
}

actfunc *
ActionTable::getFunctions(const char *name){
  if(name == NULL)
	  return NULL;
  actfunc *actf=NULL;
  std::list<actfunc*>::iterator iter=actions.begin();
  while (strcmp((*iter)->name, name) != 0 && iter!= actions.end())iter++;

  if(iter != actions.end())
    return (*iter); 
  else
	  return NULL;
}


void
AgentTable::addAgent(const std::string& name, AgentProc *ap)
{
  assert(ap);
  this->agents[name] = ap;
}

//Removes the agent from the par table, if it exists in the table
void
AgentTable::removeAgent(const std::string& name) {
	if (this->agents.find(name) != this->agents.end()) {
		this->agents.erase(name);
	}
}

AgentProc *
AgentTable::getAgent(const std::string& name)
{
  std::list<AgentProc*>::iterator iter;
  if(!agents.empty()){
	  std::map<std::string, AgentProc*>::const_iterator it = this->agents.find(name);
	  if (it != this->agents.end())
		  return (*it).second;
  }
     return NULL;
}


void
IParTable::add(iPAR *ipar, const Status status)
{
  assert(ipar);
   iparst *iparST = new iparst;
   iparST->ipar = ipar;
   iparST->status = status;
   iparstlist.push_back(iparST);

}

void
IParTable::remove(const iPAR *ipar)
{
  assert(ipar);
  std::vector<iparst*>::iterator iter, iter2, iter3;
  
  iter = iparstlist.begin();
  while(iter != iparstlist.end())
   {
     iter2 = iter;
     iter++;
     
     iparst *iparST = *iter2;
     if(iparST->ipar == ipar)
       {
			iparstlist.erase(iter2);
			break;
       }
   }

  // also remove any ipars enabled by this action: happens during failures
	int enableID = ipar->enabled_act;
	if (enableID > 0)
	{
	   iter3 = iparstlist.begin();
	   while (iter3 != iparstlist.end())
	   {
			if ((*iter3)->ipar->par->getID() == enableID)
			{
				iparstlist.erase(iter3);
				break;
			}
			iter3++;
	   }	
   }

}

int 
IParTable::setStatus(const iPAR *ipar, const Status status)
{
  assert(ipar);
   for(unsigned int p=0; p<iparstlist.size(); p++)
   {
       if(iparstlist[p]->ipar == ipar)
       {
	  iparstlist[p]->status = status;
	  par_debug("ProcessMgr: IParTable:Changed status of %s to %d\n",ipar->par->getActionName().c_str(), status);
	  return 1;
       }
   }
   return 0;
    
}

Status
IParTable::getStatus(const iPAR *ipar)
{
  assert(ipar);
   for(unsigned int p=0; p<iparstlist.size(); p++)
   {
       if(iparstlist[p]->ipar == ipar)
	  return iparstlist[p]->status;
   }
    
   par_debug("ProcessMgr: IPARTable: no such instance of ipar found\n");
   return notfound;
}

bool
IParTable::activeAction(void)
  // This method checks to see if there is any action that is active.
  // An action is active/bring-processed if it is in any one of the 
  // following status: preproc, prepspec, exec
{
   for(unsigned int p=0; p<iparstlist.size(); p++)
   {
     if((iparstlist[p]->status == preproc) || \
	(iparstlist[p]->status == prepspec) || \
	(iparstlist[p]->status == exec))
       return 1;
     
   }
   return 0;
}

bool
IParTable::onQAction()
{
   for(unsigned int p=0; p<iparstlist.size(); p++)
   {
     if(iparstlist[p]->status == onqueue)
	    return 1;
     
   }
   return 0;
}

iPAR* 
IParTable::actionExecuting(void)
{
	for(unsigned int p=0; p<iparstlist.size(); p++)
	{
     if(iparstlist[p]->status == exec)
       return iparstlist[p]->ipar;
     
	}
	return NULL;
}

iPAR*
IParTable::actionPrepSpecs(bool first)
{
	//First tells us whether to search foward or 
	//backwards in the ipt.  Since we are backwards
	//chaining, the latest action to be put on the queue
	//is also the one most likely spawning new ipars
	if(first){
		for(unsigned int p=0; p<iparstlist.size(); p++)
		{	
		 if(iparstlist[p]->status == prepspec)
		   return iparstlist[p]->ipar;
     
		}
	}
	else{
		for(int p=(int)iparstlist.size()-1; p >=0; --p){
			if(iparstlist[p]->status == prepspec)
				return iparstlist[p]->ipar;
		}
	}
	return NULL;	
}

bool
IParTable::failAction(void)
  // This method checks to see if there is any action that has failed.
{
   for(unsigned int p=0; p<iparstlist.size(); p++)
   {
     if(iparstlist[p]->status == failure)
       return 1;
     
   }
   return 0;
}

void
IParTable::resetFailure(void)
  // This method resets all failure flags to failed. 
{
   for(unsigned int p=0; p<iparstlist.size(); p++)
   {
     if(iparstlist[p]->status == failure)
       setStatus(iparstlist[p]->ipar,failed);
     
   }
}


int
IParTable::preempt(int priority)
  // This method compares the given priority with the priority of each
  // active action. If any of the active actions have a lower
  // priority, then they are exempted and a value of 1 is returned. If
  // no action is pre-empted, a value of 0 is returned.
{
  int preemptDone = 0;

  for(unsigned int p=0; p<iparstlist.size(); p++)
   {
     iPAR *ipar = iparstlist[p]->ipar;
	 Status status=iparstlist[p]->status;
     if(((status == preproc) || (status == prepspec) || 
	 (status == exec)) && (priority > ipar->getPriority())){
		// pre-empt
		par_debug("ProcessMgr: preempting action %s\n",ipar->par->getActionName().c_str());
			//iparstlist[p]->ipar->setDuration(1.0);
		this->setStatus(ipar, aborted);
		
		 
		//For pre-emption and suspension, we no longer wish to run the parnet associated with the
		//ipar, so that we have to remove the highest pat-net from the system (i believe)

		preemptDone = 1;
       }  
   }
   return preemptDone;
}

int
IParTable::same(const iPAR *ipar1, const iPAR *ipar2)
{
   int match = 0;
   // first check the agent's, which should have the same metaobject pointer
   if(ipar1->agent== ipar2->agent)
   {
      // second, compare the upar names
	   if(!strcmp(ipar1->par->getActionName().c_str(),ipar2->par->getActionName().c_str()))
      {
	  // third compare the number of objects
		  int numobj = actionary->getNumObjects(ipar1->par);   
		  if(numobj == actionary->getNumObjects(ipar2->par))	
	  {
		 int i;
	     for(i=0; i<numobj;i++)
	     {
			 if(ipar1->objects[i] == ipar2->objects[i])
		 {
		     match = 0;
		     break;
		 }
	     }
	     if(i == numobj) // all objects match
	     {
		match = 1;
		}
	  }
      }
   }
   return match;
}
int 
IParTable::iparExist(const iPAR *ipar)
  // This method checks if the given ipar already exists.
  // For existence, only the following status are considered:
  //  onqueue, preproc, prepspec, exec.
{
  if(ipar == NULL)
	  return 0;
  // The ipar we need to check against is actually being generated
  // by the NLP module. Here, we actually need to check is the
  // ipars are similar rather than if the pointers are exact. This is
  // because for any Python tuple ipar sent to me, a new ipar is
  // automaticaly generated. Hence just checking the ipar pointers
  // will not work. We actually need to check the exact contents of
  // the ipar.
   for(unsigned int p=0; p<iparstlist.size(); p++)
   {
       if(same(iparstlist[p]->ipar,ipar))
       {
	 if((iparstlist[p]->status == onqueue) || \
	    (iparstlist[p]->status == preproc) || \
	    (iparstlist[p]->status == prepspec) || \
	    (iparstlist[p]->status == exec))
	   return 1;	 
       }
   }
   return 0;
}


iPAR *
IParTable::getIPAR(const char *uparName)
  // given the uparName , find the corresponding ipar in the
  // ipartable. This method was specifically constructed for the case
  // of complex actions. for a subaction, given only the name of the
  // complex parent, we need to find the ipar of the complex parent
  // action. This will enable us to pass on some of the complex
  // parent's parameters to the subactions
{
   for(unsigned int p=0; p<iparstlist.size(); p++)
   {
     if(!strcmp(iparstlist[p]->ipar->par->getActionName().c_str(),uparName))
       return iparstlist[p]->ipar;
   }
   return 0;
}

iPAR *
IParTable::getRecentIPAR(const char *uparName)
	// return the most recently performed ipar of the specified type
{
	iPAR* result = NULL;
	for(unsigned int p=0; p<iparstlist.size(); p++)
	{
		 if(!strcmp(iparstlist[p]->ipar->par->getActionName().c_str(),uparName))
			 if (result != NULL)
			 {
				 if (result->getStartTime() < iparstlist[p]->ipar->getStartTime() && iparstlist[p]->ipar->getFinished())
					 result = iparstlist[p]->ipar; 
			 }
			 else
				 result = iparstlist[p]->ipar;
	}

	return result;
}