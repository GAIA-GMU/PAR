#include <Python.h>
#include <import.h>
#include "par.h"
#include "actionnet.h"
#include "parnet.h"

void
ActionNet::actfunc(void)
{
  //debug("ProcessMgr: Building PaTNet - NEXTNODE - finished prev parallel set\n");
  markfinished();
}

void
ActionNet::primact(void)
{
  static int count = 0;
  count++;
  if(count >= 5)
    {
      count = 0;
      markfinished();
    }
}

LWNet *
ActionNet::SubNet(void *args)
{
 if(!errorcond()){
	 ComplexObj *testcomp=(ComplexObj *)args;
	 try{
		 if(testcomp->ipar != NULL)
			 return (new ParNet((PyObject *)args));  // OJO
		 else
			 return (new ParNet((ComplexObj *)args));
	 }
	 catch(iPARException &err){
		 par_debug("Error in subnet creation:%s\n",err.printMsg());
	 }
 }
 return (new ParNet());//If there is an error, we should kill off the system as fast as possible, which two nodes should do
}

void
ActionNet::UnMark(void){
	unmarkerror();

}


ActionNet::ActionNet(void *args, int numNodes)
  :LWNet(numNodes)
{
 // debug("ProcessMgr: Building PaTNet - constructor\n");
  globalNum = 0;
  totalNodes = 0;
 
  //PyObject *Obj = (PyObject *)args;	
  ComplexObj *cobj = (ComplexObj *)args;
  // start node
  int startNode = uniqueID();
  defnormalnode(startNode,(ACTFUNC)&ActionNet::actfunc);
  par_debug("ProcessMgr: Building PaTNet - CREATED start node %d\n",startNode);
  totalNodes++;

  int failExitNode = uniqueID();
  deffailexitnode(failExitNode);
  par_debug("ProcessMgr: Building PaTNet - CREATED fail exit node %d\n",failExitNode);
  totalNodes++;

  int currentNode = Parse::parseDict(cobj,this, startNode, failExitNode);
	//int currentNode = Parse::parseDict(Obj,this, startNode, failExitNode);
  // exit node
  int exitNode = uniqueID();
  //deftrans(currentNode,(CONDFUNC)&defaultcond, exitNode);
  defexitnode(exitNode);
  //debug("ProcessMgr: Building PaTNet - CREATED exit node %d\n",exitNode);
  totalNodes++;
  deftrans(currentNode, (CONDFUNC) &LWNet::defaultcond, exitNode);
  //debug("ProcessMgr: Building PaTNet - CONNECTED %d to %d\n",currentNode, exitNode);
  
  

  // update number of nodes in the patnet;
  LWNet::numnodes = totalNodes;
  //debug("ProcessMgr: Building PaTNet - num of nodes = %d\n", totalNodes);
  

}


#if 0
int
parseStep(char *pymodname)
{

  PyObject *pmod, *Obj, *pstr, *pdict, *pstr2;
  char *cstr, runcmd[128];
  
  if(!(pmod = Load_Module(pymodname)))
    onError("Unable to load python module");
  if(!(pstr = Load_Attribute(pymodname,"actions")))
    onError("Unable to get attibute from module\n"); 

  if(PyDict_Check(pstr))
    {
      PyArg_Parse(pstr,"O", &Obj);
      debug("ProcessMgr: Building PaTNet -  this is a dictionary\n");
      //parseDict(Obj);
      ActionNet *actionNet = new ActionNet(Obj);
      LWNetList::addnet(actionNet);
    }
  else
    {
      printf("not a python dictionary\n");
      return 0;
    }
  Py_DECREF(pstr);

  
  Py_DECREF(pmod);
  Py_DECREF(pstr);

  return 1;
}
#endif





