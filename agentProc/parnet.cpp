#ifndef HAVE_ROUND
#define HAVE_ROUND
#endif
#include <Python.h>
#include "parnet.h"
#include "workingmemory.h"
#include "interpy.h"
#include "actionnet.h"
#include "partable.h"
#include "par.h"
#include "actionary.h"


extern "C" bool fromPyObjectToBool(PyObject *);
extern AgentTable agentTable;
extern ActionTable actionTable;
extern Actionary* actionary;

///////////////////////////////////////////////////////////////////////////////
//This function creates an iPAR from a python object, which means it should only
//be called from within the parnet's themselves. This function is only for constructing
//new actions from a parent's action
///////////////////////////////////////////////////////////////////////////////
iPAR *
getIPAR(PyObject *obj){
  char* actionName;
  char* key;
  int   id;
  int   dict_len;
  MetaAction *act = NULL;
  MetaObject *agent=NULL;
  float		duration=0.0;
  bool		existComplexParent = false;
  PyObject	*token, *dict, *item,*keys;
  iPAR *ipar;
  //A tuple comes in as name,reference back to self, and annotations
  
  //Gets the action's name
  if(PyString_Check(token=PyTuple_GetItem(obj, 0))){
      PyArg_Parse(token,"s",&actionName);
      par_debug("ProcessMgr: creating iPAR from Python String: actionName: %s\n",actionName);

  }else{
    par_debug("ProcessMgr: creating iPAR from Python String: The first token is not a string\n");
	return NULL;
  }
  act = actionary->searchByNameAct(actionName);
  if (act == NULL){
	  par_debug("ProcessMgr: Action does not Exist\n");
	  return NULL;
  }
  ipar = new iPAR(act);
  //Grabs the items (agent and objects) needed for the action
  if (PyDict_Check(dict = PyTuple_GetItem(obj, 1))){
	  dict_len = PyDict_Size(dict);//We get the length of the dictionary
	  if (dict_len < 1){ //A zero length dictionary is an error
		  par_debug("ProcessMgr: The annotations for %s are empty\n", actionName);
		  return NULL;
	  }
	  else{
		  keys = PyDict_Keys(dict); //Here we get the keys
		  for (int i = 0; i < dict_len; i++){
			  if (PyString_Check(token = PyList_GetItem(keys, i))){
				  PyArg_Parse(token, "s", &key);
				  item = PyDict_GetItemString(dict, key);
				  //par_debug("key is %s\n", key);
				  if(!strcmp(key, "agents")){
					  if (PyInt_Check(item)){
						  PyArg_Parse(item, "i", &id);
						  agent = actionary->searchByIdObj(id);
						  if (agent != NULL && agent->isAgent()){ //We make sure the agent is actually an agent
							  par_debug("ProcessMgr: creating iPAR from Python String: agentName:%s\n", agent->getObjectName().c_str());
							  ipar->setAgent(agent);
							  //agentTable.getAgent(agent->getObjectName())->addAction(ipar);
						  }
						  else{
							  par_debug("ProcessMgr: The agent key is invalid, aborting\n");
							  return NULL;
						  }
					  }
				  }
				  else if (!strcmp(key, "objects")){
					  if (PyTuple_Check(item)){
						  int len = (int)PyTuple_Size(item);
						  //debug("ProcessMgr: creating iPAR from Python String: number of objects = %d\n",len);
						  for (int j = 0; j<len; j++){
							  PyArg_Parse(PyTuple_GetItem(item, j), "i", &id);
							  if (id == -1)
								  ipar->setObject(NULL, j);
							  else
								  ipar->setObject(actionary->searchByIdObj(id), j);
						  }
					  }
					  else if (PyInt_Check(item))
					  {
						  PyArg_Parse(item, "i", &id);
						  if (id == -1)
							  ipar->setObject(NULL, 0);
						  else
							  ipar->setObject(actionary->searchByIdObj(id), 0);
					  }
					  else
						  par_debug("ProcessMgr: creating iPAR from Python String: objects item is neither a tuple nor a string\n");
				  }
				  else if (!strcmp(key, "caller")){
					  if (PyInt_Check(item)){
						  PyArg_Parse(item, "i", &id);
						  iPAR *parent = actionary->searchByIdiPAR(id);
						  if (parent != NULL){
							  ipar->setEnabledAct(id);
							  ipar->setDuration(parent->getDuration());
							  ipar->setPriority(parent->getPriority());
							  //We also copy any properties that have already not been set by the system
							  int count = 0;
							  parProperty *prop = parent->getPropertyType(count);
							  while (prop != NULL){
								  try{
									  ipar->getProperty(prop);
								  }
								  catch (iPARException *msg){
									  //If we have an exception, then we don't have the property
									  ipar->setProperty(prop, parent->getProperty(prop));
								  }
								  count++;
								  prop = parent->getPropertyType(count);
							  }
						  }
						  else{
							  par_debug("ProcessMgr: caller did not have a valid action id\n");
						  }
					  }
					  else{
						  par_debug("ProcessMgr: caller was not set with an action id\n");
					  }
				  }else{
					  //Here, we assume to get NIFI
					  //Special case for duration
					  if (!strcmp(key, "duration")){
						  if (PyFloat_Check(item)){
							  float duration;
							  PyArg_Parse(item, "f", &duration);
							  ipar->setDuration(duration);
						  }
					  }
					  else{

						parProperty *prop = actionary->searchByNameProperty(key);
						if (prop != NULL){
							if (prop->isInt()){
								int val=0;
								PyArg_Parse(item, "i", val);
								ipar->setProperty(prop, (double)val);
							}
							else if (prop->isCont()){
								double val= 0.0;
								PyArg_Parse(item, "d", val);
								ipar->setProperty(prop, val);
							}
							else{
								char *prop_name=NULL;
								PyArg_Parse(item, "s", prop_name);
								ipar->setProperty(prop, (double)prop->getPropertyValueByName(prop_name));
							}
						}
					 }
				  }
				}
			  else{
				  par_debug("ProcessMgr: Had a non-string in the annotations, aborting");
				  return NULL;
			  }
		  }
	  }
  }
  if (ipar->getAgent() == NULL){
	  par_debug("ProcessMgr: Did not have the agent in the annotations, aborting\n");
	  return NULL;
  }
  agentTable.getAgent(ipar->getAgent()->getObjectName())->ipt->add(ipar, preproc);
  return ipar;
}

void
ParNet::actfunc_start(void)
{
 if (ap == NULL){
		par_debug("ERROR: not agent process in ParNet::actfunc_start\n");
		markerror();
		return;
	}
 if (ap->ipt->getStatus(ipar) == aborted){
	markerror();
	return;
 }
  ap->ipt->setStatus(ipar,preproc);
  this->condfunc_culminate(); 
}

void
ParNet::condfunc_culminate(void){
	if (ap->ipt->getStatus(ipar) == aborted){
		markerror();
		return;
	}


  PyObject* res = actionary->testCulminationCond(ipar); 
  if(res == NULL){
	  par_debug("Received NULL culmination condition, aborting\n");
	  ap->ipt->setStatus(ipar,failure);
	  markerror();
  }
    switch(fromPyObjectToInt(res)){
	case 0:
		return;
		break;
	case SUCCESS:{
	  markfinished();
	   ap->ipt->setStatus(ipar,completed);
	  break;
	}
	case FAILURE:{
	  markerror(); // flag this node as error
	  ap->ipt->setStatus(ipar,failure);//Have to keep things consistant
	  if(ipar->getFailData() == NULL){
		FailData * failData=new FailData;
		failData->ipar=ipar;
		ipar->setFailData(failData);
	  }
	  ap->inform("Failure",(void *)ipar->getFailData()); 
	  break;
	}
	default:
	  break;
	} 
}

void
ParNet::actfunc_applicability(void){
	if (ap->ipt->getStatus(ipar) == aborted){
		markerror();
		return;
	}
	app_result = actionary->testApplicabilityCond(ipar);
	if(app_result ==NULL){
		par_debug("Applicability condition was NULL for ipar %s%d, returning\n",ipar->par->getActionName().c_str(),ipar->getID());
		return;
	}
	if (fromPyObjectToInt(app_result)==SUCCESS){
      //debug("ProcessMgr: The applicability condition of %s is true.\n", str);
      app_status = 1;
    }
	else{
      app_status = 0;
      ap->ipt->setStatus(ipar,failure);
	  markerror(); // flag this node as error
    }
}

bool
ParNet::condfunc_applicable(void)
{
  return app_status;
}

bool
ParNet::condfunc_isapplicdictionary(void)
{
  return app_dict;
}

LWNet *
ParNet::newpar(void *args){
  ComplexObj **cobj = (ComplexObj **)args;
  //  PyObject **a = (PyObject **)args;
  
  assert(cobj);
  //(*cobj)->pyobj = (PyObject *)args;  // Jan added 3/08
  ActionNet *actionnet = new ActionNet(*cobj);
  assert(actionnet);
  return actionnet;
}


void
ParNet::actfunc_preparatory(void){
	if (ap->ipt->getStatus(ipar) == aborted){
		prep_status=1;//So we can move onto something that can be marked as a failure
		markerror();
		return;
	}
  ap->ipt->setStatus(ipar,prepspec);
 
  prep_result = actionary->testPreparatorySpec(ipar);
  if(prep_result == NULL){
	  par_debug("ERROR:No prepratory specifications\n");
	  return;
  }

  if(PyDict_Check(prep_result)){
      //debug("ProcessMgr: The preparatory spec of %s returns a dictionary\n",str);
	  if (PyDict_GetItemString(prep_result, "COMPLEX")) {
		  // debug("ProcessMgr: the execution steps of %s describes a complex action\n",str);
		  prep_complex = 1;
	  }
	  else {
		  prep_dict = 1;
	  }
    }
  else if (fromPyObjectToBool(prep_result)){
      //debug("ProcessMgr: The preparatory spec of %s is true.\n", str);
      prep_status = 1;
    }
  else{
      //debug("ProcessMgr: The preparatory spec of %s is false.\n", str);
      prep_status = 0;
    }
}

bool
ParNet::condfunc_ready(void)
{
  return prep_status;
}

bool
ParNet::condfunc_isprepdictionary(void)
{
  return prep_dict;
}

bool
ParNet::condfunc_isprepcomplex(void) {

	return prep_complex;
}


void
ParNet::actfunc_coreexecution(void){
	if (ap->ipt->getStatus(ipar) == aborted){
		markerror();
		return;
	}
  // update iparstatus
  ap->ipt->setStatus(ipar,exec);
  PyObject *complexpar  = actionary->testExecutionSteps(ipar); //actionary->getExecutionSteps(ipar);
  if(complexpar == NULL){
	  //Going to say that we cannot have NULL execution steps
	  ap->ipt->setStatus(ipar,failure);
	  markerror();
	  return;
  }
  complexobj->pyobj = complexpar;
  complexobj->complexParent = ipar;
  //const std::string str= ipar->par->getActionName();
  if(PyDict_Check(complexpar)){
      //debug("ProcessMgr: The execution steps of %s returns a dictionary\n",str);
      prep_dict = 1;
    }

  if(PyDict_GetItemString(complexpar,"COMPLEX")) {
     // debug("ProcessMgr: the execution steps of %s describes a complex action\n",str);
      complex = 1;
    }
  else if (PyDict_GetItemString(complexpar,"PRIMITIVE")){
      //debug("ProcessMgr: the execution steps of %s describes a primitive action\n",str);
      complex = 0;
    }
}

bool
ParNet::condfunc_iscomplex(void){
  return complex;
}

void
ParNet::preactfunc_primitive(void){
  if(!errorcond()){
	callback = actionTable.getFunctions(ipar->par->getActionName().c_str());
	if(callback != NULL)
		callback->func(ipar);
	else	
		par_debug("ERROR: No execution function found for action %s\n",ipar->par->getActionName().c_str());
  }
  
}

void
ParNet::postactfunc(void){
	//We need to clean up the action in the post act function. For now this is setting the ipar to finished 
	//which helps us with complex actions
	ipar->setFinished(true);
}

ParNet::ParNet(ComplexObj *cobj)
  :LWNet(NUMNODES){
  if(cobj->pyobj){
      if(PyTuple_Check((PyObject *)cobj->pyobj)){
	  // convert to ipar instance
		PyObject *obj = (PyObject *)cobj->pyobj;
		if(obj == NULL)
		  throw iPARException(std::string("Failure in PARNet Creation, python object is NULL"));
		ipar = getIPAR(obj);
		if(ipar == NULL)
		  throw iPARException(std::string("Failure in PARNet Creation, ipar is NULL"));
		if(cobj->complexParent != NULL){
		  ipar->setPurposeEnableAct(cobj->complexParent->getID());
		  cobj->complexParent->setEnabledAct(ipar->getID());
		}
	   }
      else if(PyDict_Check((PyObject *)cobj->pyobj)){
		par_debug("ProcessMgr: The input to PARnet is a dictionary \n");
		defcallnode(NEWACTNODE, (LWNEW)&ParNet::newpar, NULL, NULL, (void *)cobj);
		deftrans(NEWACTNODE, (CONDFUNC)&LWNet::finishcond, EXITNODE);
		return;
	  }	
  }	
  else{
      ipar = (iPAR *)cobj->ipar; 
      if(ipar == NULL)
		  throw iPARException("Failure in PARNet Creation, iPar is NULL\n");
  } 


  if(ipar){
	  ap = agentTable.getAgent(ipar->getAgent()->getObjectName());
	  if(ap == NULL)
		  throw iPARException(std::string("Failure in PARNet Creation, agent is NULL"));
  }
  // initialization
  app_result = 0;
  app_status = app_dict = 0;
  prep_result = 0;
  prep_status = prep_dict = prep_complex = 0;
  complexobj = new ComplexObj;
  complex = 0;
  callback = 0;
  
  defnormalnode(START, (ACTFUNC)&ParNet::actfunc_start);
  deftrans(START, (CONDFUNC)&ParNet::finishcond, EXITNODE);
  deftrans(START, (CONDFUNC)&LWNet::defaultcond, APPLICABILITY);

  defnormalnode(APPLICABILITY, (ACTFUNC)&ParNet::actfunc_applicability);
  deftrans(APPLICABILITY, (CONDFUNC)&ParNet::condfunc_applicable, PREPARATORY);
  deftrans(APPLICABILITY, (CONDFUNC)&LWNet::defaultcond, EXITNODE);

  defnormalnode(PREPARATORY, (ACTFUNC)&ParNet::actfunc_preparatory);
  deftrans(PREPARATORY, (CONDFUNC)&ParNet::condfunc_isprepcomplex, NEWSUBPARCOMPLEX);
  deftrans(PREPARATORY, (CONDFUNC)&ParNet::condfunc_isprepdictionary, NEWSUBPARNODE);
  deftrans(PREPARATORY, (CONDFUNC)&ParNet::condfunc_ready, COREEXEC);

  defcallnode(NEWSUBPARCOMPLEX, (LWNEW)&ParNet::newpar,NULL,NULL,(ComplexObj**)&prep_result);
  deftrans(NEWSUBPARCOMPLEX, (CONDFUNC)&LWNet::errorcond, FAILEXITNODE);
  deftrans(NEWSUBPARCOMPLEX, (CONDFUNC)&LWNet::finishcond, COREEXEC);

  defcallnode(NEWSUBPARNODE, (LWNEW)&ParNet::newpar,NULL,NULL,&prep_result);
  deftrans(NEWSUBPARNODE,(CONDFUNC)&LWNet::errorcond,FAILEXITNODE);
  deftrans(NEWSUBPARNODE, (CONDFUNC)&LWNet::finishcond, COREEXEC);

  
  defnormalnode(COREEXEC, (ACTFUNC)&ParNet::actfunc_coreexecution);
  deftrans(COREEXEC, (CONDFUNC)&ParNet::condfunc_iscomplex, COMPLEX);
  deftrans(COREEXEC, (CONDFUNC)&LWNet::defaultcond, PRIMITIVE);
  
  defcallnode(COMPLEX,(LWNEW)&ParNet::newpar, NULL, (ACTFUNC)&ParNet::postactfunc, &complexobj);
  deftrans(COMPLEX,(CONDFUNC)&LWNet::errorcond,FAILEXITNODE);
  deftrans(COMPLEX, (CONDFUNC)&LWNet::finishcond, EXITNODE);

  defnormalnode(PRIMITIVE, (ACTFUNC)&ParNet::condfunc_culminate, (ACTFUNC)&ParNet::preactfunc_primitive,(ACTFUNC)&ParNet::postactfunc);
  deftrans(PRIMITIVE,(CONDFUNC)&LWNet::errorcond,FAILEXITNODE);
  deftrans(PRIMITIVE, (CONDFUNC)&LWNet::finishcond, EXITNODE);

  defexitnode(EXITNODE);  
  deffailexitnode(FAILEXITNODE);
}

ParNet::ParNet(PyObject *cobj)
  :LWNet(NUMNODES){

  if(cobj){
      if(PyTuple_Check((PyObject *)cobj)){
	  // convert to ipar instance
	  PyObject *obj = (PyObject *)cobj;
	  if(obj == NULL)
		  throw iPARException(std::string("Failure in PatNet Creation, python object is NULL"));
	  ipar = getIPAR(obj);
	  if(ipar == NULL)
		  throw iPARException(std::string("Failure in PatNet Creation, ipar is NULL"));
	}
    else if(PyDict_Check((PyObject *)cobj)){
	  par_debug("ProcessMgr: The input to parnet is a dictionary \n");
	  defcallnode(NEWACTNODE, (LWNEW)&ParNet::newpar, NULL, NULL, (void *)cobj);
	  deftrans(NEWACTNODE, (CONDFUNC)&LWNet::finishcond, EXITNODE);
	  return;
	}	
  }	
  else 
    {
	  std::cout << "ERROR: shouldn't be a complex action here?" << std::endl;
    } 


  if(ipar){
	  ap = agentTable.getAgent(ipar->getAgent()->getObjectName());
	  if(ap == NULL)
		  throw iPARException(std::string("Failure in PatNet Creation, agent is NULL"));
  }
  // initialization
  app_result = 0;
  app_status = app_dict = 0;
  prep_result = 0;
  prep_status = prep_dict = prep_complex = 0;
  complexobj = new ComplexObj;
  complex = 0;
  callback = 0;
  
  defnormalnode(START, (ACTFUNC)&ParNet::actfunc_start);
  deftrans(START, (CONDFUNC)&ParNet::finishcond, EXITNODE);
  deftrans(START, (CONDFUNC)&LWNet::defaultcond, APPLICABILITY);

  defnormalnode(APPLICABILITY, (ACTFUNC)&ParNet::actfunc_applicability);
  deftrans(APPLICABILITY, (CONDFUNC)&ParNet::condfunc_applicable, PREPARATORY);
  deftrans(APPLICABILITY,(CONDFUNC)&LWNet::errorcond,FAILEXITNODE);
  deftrans(APPLICABILITY, (CONDFUNC)&LWNet::defaultcond, EXITNODE);

  defnormalnode(PREPARATORY, (ACTFUNC)&ParNet::actfunc_preparatory);
  deftrans(PREPARATORY, (CONDFUNC)&ParNet::condfunc_isprepcomplex, NEWSUBPARCOMPLEX);
  deftrans(PREPARATORY, (CONDFUNC)&ParNet::condfunc_isprepdictionary, NEWSUBPARNODE);
  deftrans(PREPARATORY, (CONDFUNC)&ParNet::condfunc_ready, COREEXEC);

  defcallnode(NEWSUBPARCOMPLEX, (LWNEW)&ParNet::newpar, NULL, NULL, &prep_result);
  deftrans(NEWSUBPARCOMPLEX, (CONDFUNC)&LWNet::errorcond, FAILEXITNODE);
  deftrans(NEWSUBPARCOMPLEX, (CONDFUNC)&LWNet::finishcond, COREEXEC);

  defcallnode(NEWSUBPARNODE, (LWNEW)&ParNet::newpar, NULL, NULL, &prep_result);
  deftrans(NEWSUBPARNODE, (CONDFUNC)&LWNet::errorcond, FAILEXITNODE);
  deftrans(NEWSUBPARNODE, (CONDFUNC)&LWNet::finishcond, COREEXEC);

  
  defnormalnode(COREEXEC, (ACTFUNC)&ParNet::actfunc_coreexecution);
  deftrans(COREEXEC, (CONDFUNC)&ParNet::condfunc_iscomplex, COMPLEX);
  deftrans(COREEXEC, (CONDFUNC)&LWNet::defaultcond, PRIMITIVE);
  
  defcallnode(COMPLEX,(LWNEW)&ParNet::newpar, NULL, (ACTFUNC)&ParNet::postactfunc, &complexobj);
  deftrans(COMPLEX,(CONDFUNC)&LWNet::errorcond,FAILEXITNODE);
  deftrans(COMPLEX, (CONDFUNC)&LWNet::finishcond, EXITNODE);

  defnormalnode(PRIMITIVE, (ACTFUNC)&ParNet::condfunc_culminate, (ACTFUNC)&ParNet::preactfunc_primitive,(ACTFUNC)&ParNet::postactfunc);
  deftrans(PRIMITIVE,(CONDFUNC)&LWNet::errorcond,FAILEXITNODE);
  deftrans(PRIMITIVE, (CONDFUNC)&LWNet::finishcond, EXITNODE);

  defexitnode(EXITNODE);  
  deffailexitnode(FAILEXITNODE);
}
///////////////////////////////////////////////////////////////////////////////
//A voided call to PARNET basically means we have a DFA that only enters a failure
//state. If something went wrong in the creation of a PAT-net, this is what
//needs to be called so that the whole system can be marked as a failure
///////////////////////////////////////////////////////////////////////////////
ParNet::ParNet(void)
  :LWNet(NUMNODES){//A start and a failure is all that is needed for this subnet
	  //At each stage of execution, a function is called that is meant to change a variable
	  //name. This in turn is checked in each transitive state. So, each node needs to have 
	  //a funcion.
	  defnormalnode(START, (ACTFUNC)&ParNet::actfunc_start);
	  deftrans(START, (CONDFUNC)&LWNet::defaultcond, EXITNODE);
	  defexitnode(EXITNODE);
	  app_result = 0;
	  app_status = app_dict = 0;
	  prep_result = 0;
      prep_status = prep_dict = 0;
      complex = 0;
      callback = 0;
	  ap=NULL;
}
