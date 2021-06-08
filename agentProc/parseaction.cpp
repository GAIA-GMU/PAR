#include <Python.h>
#include "par.h"
#include "lwnet.h"
#include "actionnet.h"
#include "parseaction.h"
#include "assert.h"

extern Actionary *actionary;

int
Parse::parseSeq(ComplexObj *cobj, ActionNet *actionNet, int parent, int failExitNode, int *startSet, int *endSet)
{
  int 		i;
  PyObject 	*item;
  int 		currentNode, startSeqNode;

  PyObject *seqobj = cobj->pyobj;

  if(PyTuple_Check(seqobj))
    {
      currentNode = parent;
	  par_debug("parseSeq: Starting to Parse\n");
      for(i=0; i<PyTuple_Size(seqobj); i++)
	{
	  item = PyTuple_GetItem(seqobj,i);
	  assert(item);
	  if(PyTuple_Check(item))
	    {
	      ComplexObj *itemobj = new ComplexObj(item,cobj->complexParent);
	      currentNode = parseComplex(itemobj, actionNet,currentNode,failExitNode, startSet, endSet);
		  actionNet->deftrans(currentNode, (CONDFUNC)&(ActionNet::errorcond), failExitNode);
	      if(i==0) 
		startSeqNode = currentNode;

	    }
	}
	  par_debug("parseSeq: Ending Parse\n");
    }
  else
    onError("parseSeq: input obj is not a tuple");

  // return the beginning and the end of the sequences
  *startSet = startSeqNode;
  *endSet = currentNode;

   return currentNode;
}

int
Parse::parseSel(ComplexObj *cobj, ActionNet *actionNet, int parent, int failExitNode, int *startSet, int *endSet)
{
	int 		i;
	PyObject 	*item;
	int 		currentNode, startSeqNode;

	PyObject *seqobj = cobj->pyobj;

	if (PyTuple_Check(seqobj))
	{
		currentNode = parent;
		par_debug("parseSel: Starting Parse\n");
		for (i = 0; i<PyTuple_Size(seqobj); i++)
		{
			item = PyTuple_GetItem(seqobj, i);
			assert(item);
			if (PyTuple_Check(item))
			{
				ComplexObj *itemobj = new ComplexObj(item, cobj->complexParent);
				if (i == 0)
					startSeqNode = currentNode;

				if (i == PyTuple_Size(seqobj) - 1){
					currentNode = parseComplex(itemobj, actionNet, currentNode, failExitNode, startSet, endSet);
					actionNet->deftrans(currentNode, (CONDFUNC)&(ActionNet::errorcond), failExitNode);
					par_debug("ProcessMgr: Building PaTNet - CONNECTING %d to %d\n", currentNode, failExitNode);
				}
				else{
					currentNode = parseComplex(itemobj, actionNet, currentNode, failExitNode, startSet, endSet,false);
				}
			}
		}
		par_debug("parseSel: Ending parse\n");
	}
	else
		onError("parseSel: input obj is not a tuple");

	// return the beginning and the end of the sequences
	*startSet = startSeqNode;
	*endSet = currentNode;

	return currentNode;
}


int
Parse::parsePar(ComplexObj *cobj,ActionNet *actionNet, int parent, int failExitNode, int *startSet, int *endSet, int type)
{
  int 		branch;
  PyObject 	*item;

  PyObject *PJobj = cobj->pyobj;

  if(PyTuple_Check(PJobj))
    {

      int parNode = actionNet->uniqueID();
      int joinNode = actionNet->uniqueID();
      int nextNode = actionNet->uniqueID();

      int parJoinStart[] = {-1, -1, -1, -1};
      int parJoinEnd[] = {-1, -1, -1, -1};
      for(branch=0; branch<PyTuple_Size(PJobj); branch++)
	{
	  item = PyTuple_GetItem(PJobj,branch);
	  ComplexObj *itemobj = new ComplexObj(item,cobj->complexParent);
	  if(PyTuple_Check(item))
	    parseComplex(itemobj, actionNet,-1, failExitNode, &parJoinStart[branch], &parJoinEnd[branch]);
	}
      
      // par node
      actionNet->defparnode(parNode, parJoinStart[0], parJoinStart[1], parJoinStart[2], parJoinStart[3]);
      //debug("ProcessMgr: Building PaTNet - CREATED a parnode with ID %d and branches %d,%d,%d,%d\n",
	  //   parNode, parJoinStart[0], parJoinStart[1], parJoinStart[2], parJoinStart[3]);

      if(parent != -1)
	{
	  actionNet->deftrans(parent,(CONDFUNC)&(ActionNet::finishcond), parNode);
	  //debug("ProcessMgr: Building PaTNet - CONNECTING %d to %d\n",parent,parNode);
	}
      actionNet->setTotalNodes(actionNet->getTotalNodes() + 1);

      switch(type)
	{
	case PARJOIN:
	  actionNet->defjoinnode(joinNode,parNode, nextNode);
	  //debug("ProcessMgr: Building PaTNet - CREATED joinnode and nextNode with ID %d & %d\n",
	  //	 joinNode,nextNode);
	  break;
	case PARINDY:
	  actionNet->defindynode(joinNode,parNode, nextNode);
	  //debug("ProcessMgr: Building PaTNet - CREATED indynode and nextNode with ID %d & %d\n",
	  //	 joinNode,nextNode);
	  break;
	case WHILE:
	  actionNet->defkldpnode(joinNode,nextNode);
	  //debug("ProcessMgr: Building PaTNet - CREATED kldpnode and nextNode with ID %d & %d\n",
	  //	 joinNode,nextNode);
	  break;	
	}  
      actionNet->setTotalNodes(actionNet->getTotalNodes() + 1);

      // transfer from branches
      branch = 0;
      while(parJoinStart[branch] != -1)
	{
	  actionNet->deftrans(parJoinEnd[branch++],(CONDFUNC)&(ActionNet::finishcond),joinNode);
	  //debug("ProcessMgr: Building PaTNet - CONNECTING  branch %d to joinnode %d\n",parJoinEnd[branch-1],joinNode);
	}

      actionNet->defnormalnode(nextNode,(ACTFUNC)&ActionNet::actfunc);
      //debug("ProcessMgr: Building PaTNet - CREATED Next node with ID %d\n",nextNode);
      actionNet->setTotalNodes(actionNet->getTotalNodes() + 1);

      *startSet = parNode;
      *endSet = nextNode;
      assert(startSet);
      assert(endSet);
      assert(actionNet);
      return nextNode;
      
    }
  else
    onError("parseParJoin: input obj is not a tuple");

  return -1;
}


int
//Parse::parseComplex(PyObject *obj, ActionNet *actionNet, int parent, int failExitNode, int *startSet, int *endSet)
Parse::parseComplex(ComplexObj *cobj, ActionNet *actionNet, int parent, int failExitNode, int *startSet, int *endSet,bool fail_parnet)
{     
  PyObject 	*token;
  char 		*tokenName;
  int 		opType;
  int 		currentNode;

  PyObject *obj = cobj->pyobj;

  if(!PyTuple_Check(obj))
    onError("parseComplex: This is not a tuple");

  /* Get the first token */
  token = PyTuple_GetItem(obj, 0);
  if(!token)
    onError("parseComplex:Error in getting first token\n"); 
  Py_INCREF(token);


  if(PyLong_Check(token))    
     {
       ComplexObj *slicecobj = \
	 new ComplexObj(PyTuple_GetSlice(obj,1,PyTuple_Size(obj)),\
			   cobj->complexParent);

       PyArg_Parse(token,"i",&opType);
       switch(opType)
	 {
	 //Start and end set could be useful in saying how the last action should behave
	 case SEQUENCE:
	   currentNode = parseSeq(slicecobj,\
				  actionNet,parent, failExitNode, startSet, endSet);
	   break;

	 case SELECT:
		 currentNode = parseSel(slicecobj, \
			 actionNet, parent, failExitNode, startSet, endSet);
	 break;
	 case PARJOIN:
	   currentNode = parsePar(slicecobj,\
				  actionNet, parent, failExitNode, \
				  startSet, endSet, \
				  PARJOIN);
	   break;	   
	 case PARINDY:
	   currentNode = parsePar(slicecobj,\
				  actionNet,parent, failExitNode, \
				  startSet, endSet, \
				  PARINDY);
	   break;
	 case WHILE:
	   currentNode = parsePar(slicecobj,\
				  actionNet,parent, failExitNode, \
				  startSet, endSet,WHILE);
	   break;
	 case GATHER:
		 currentNode = parseGather(slicecobj, \
			 actionNet, parent, failExitNode, \
			 startSet, endSet);
	   break;
	 default:
	   onError("not the correct operator type");
	 }
     }
  else if(PyUnicode_Check(token))
    {
      PyArg_Parse(token,"s",&tokenName);
	  if (fail_parnet){
		  actionNet->defcallnode(currentNode = actionNet->uniqueID(), (LWNEW)&ActionNet::SubNet, NULL, NULL, (void *)cobj);
	  }
	  else{
		  actionNet->defcallnode(currentNode = actionNet->uniqueID(), (LWNEW)&ActionNet::SubNet, NULL,(ACTFUNC)&ActionNet::UnMark, (void *)cobj);
	  }
		  
	 // }
	 // else {
	//	  actionNet->deftrans(currentNode, (CONDFUNC)&(ActionNet::errorcond), currentNode);
	 // }
      par_debug("ProcessMgr: Building PaTNet - CREATED node %d for action %s\n",currentNode,tokenName);
      

      if(parent != -1)
	{
	  actionNet->deftrans(parent,(CONDFUNC)&(ActionNet::finishcond),currentNode);
	  par_debug("ProcessMgr: Building PaTNet - CONNECTING %d to %d\n",parent, currentNode);
	}
      *startSet = *endSet = currentNode;
      actionNet->setTotalNodes(actionNet->getTotalNodes() + 1);
      
    }
  else
    onError("token is neither a operator or action name");

  return currentNode;

}


int 
//Parse::parseDict(PyObject *dict, ActionNet *actionNet, int parent, int failExitNode)
Parse::parseDict(ComplexObj *cobj, ActionNet *actionNet, int parent, int failExitNode)
{
  PyObject *complex, *primitive;
  int currentNode;
  int startSet, endSet;

  if (cobj == NULL)
  {
	  std::cout << "ERROR: not a complex object in PARSE::parsedict" << std::endl;
	  return -1;
  }

  PyObject *dict;
  if(cobj->ipar !=NULL) {
  if (PyDict_Check(cobj))  
  {
	  //std::cout << "There is a dictionary in parseDict" << std::endl;
	  dict = (PyObject *)cobj;
	 
  }
  else
  {
	  dict = cobj->pyobj;
	  
	
  }
  }
  else{
	  dict=cobj->pyobj;
  }

  if (dict == NULL)
	{
	  std::cout << "ERROR: dict is NULL in PARSE::parsedict" << std::endl;
	  return -1;
	}

  if(complex = PyDict_GetItemString(dict,"COMPLEX"))
    {
      ComplexObj *cobj2 = new ComplexObj(complex,cobj->complexParent);
      if(!(currentNode =  parseComplex(cobj2, actionNet, parent, failExitNode, &startSet, &endSet)))
	return 0;
      else
	return currentNode;
    }
  else if (primitive = PyDict_GetItemString(dict,"PRIMITIVE"))
    {
      actionNet->defcallnode(currentNode = actionNet->uniqueID(),(LWNEW)&ActionNet::SubNet,NULL,NULL,(void *)primitive);
      // if there is a failure in the called patnet, abort this patnet too
      //actionNet->deftrans(currentNode,(CONDFUNC)&(ActionNet::errorcond),failExitNode);
      
      //debug("ProcessMgr: Building PaTNet - CREATED node %d for primitive action\n",currentNode);
      //debug("ProcessMgr: Building PaTNet - CONNECTING %d to %d\n",currentNode,failExitNode);
      if(parent != -1)
	{
	  actionNet->deftrans(parent,(CONDFUNC)&(ActionNet::finishcond),currentNode);
	  //debug("ProcessMgr: Building PaTNet - CONNECTING %d to %d\n",parent, currentNode);
	}
      actionNet->setTotalNodes(actionNet->getTotalNodes() + 1);
      return currentNode;
    }
  else
    printf("parseDict: Wrong keys in the dictionary\n");

  return 0;
  
}

//GATHER NODE CODE
int
Parse::parseGather(ComplexObj *cobj, ActionNet *actionNet, int parent, int failExitNode, int *startSet, int *endSet)
{
	int 		i;
	PyObject 	*item;
	int 		currentNode, startSeqNode;

	PyObject *seqobj = cobj->pyobj;

	if (PyTuple_Check(seqobj))
	{
		currentNode = parent;
		par_debug("parseGather: Starting to Parse\n");
		//The issue is that we do not know what the parent type is
		for (i = 0; i<PyTuple_Size(seqobj); i++)
		{
			item = PyTuple_GetItem(seqobj, i);
			assert(item);
			if (PyTuple_Check(item))
			{
				ComplexObj *itemobj = new ComplexObj(item, cobj->complexParent);//At this stage, we have a complex action, we have to parse the cobj though
				//When we parse the complex action, we need to know that the parent is a GATHER node
				//So, we use a special complex node since gather is only a single PAR action
				currentNode = parseGatherComplex(itemobj, actionNet, currentNode, failExitNode, startSet, endSet);
				actionNet->deftrans(currentNode, (CONDFUNC)&(ActionNet::errorcond), failExitNode);
				if (i == 0)
					startSeqNode = currentNode;

			}
		}
		par_debug("parseSeq: Ending Parse\n");
	}
	else
		onError("parseSeq: input obj is not a tuple");

	// return the beginning and the end of the sequences
	*startSet = startSeqNode;
	*endSet = currentNode;

	return currentNode;
}

int
//Parse::parseComplex(PyObject *obj, ActionNet *actionNet, int parent, int failExitNode, int *startSet, int *endSet)
Parse::parseGatherComplex(ComplexObj *cobj, ActionNet *actionNet, int parent, int failExitNode, int *startSet, int *endSet, bool fail_parnet)
{
	PyObject 	*token;
	char 		*tokenName;
	int 		opType;
	int 		currentNode;

	PyObject *obj = cobj->pyobj; //Here, we have a parse action

	if (!PyTuple_Check(obj))
		onError("parseComplexGather: This is not a tuple");

	/* Get the first token */
	token = PyTuple_GetItem(obj, 0); //We only have one item in our gather tuple for now
	PyObject *dict = PyTuple_GetItem(obj, 1);
	if (!token)
		onError("parseComplex:Error in getting first token\n");
	Py_INCREF(token);
	if (PyUnicode_Check(token)){
		PyObject* objects = PyDict_GetItemString(dict, "objects");
		int id;
		PyArg_Parse(PyTuple_GetItem(objects, 0), "i", &id); //Get the first object
		MetaObject *obj = actionary->searchByIdObj(id);
		if (fail_parnet || obj == NULL){//Sometimes we just fail, so this stays in
			actionNet->defcallnode(currentNode = actionNet->uniqueID(), (LWNEW)&ActionNet::SubNet, NULL, NULL, (void *)cobj);
		}
		else{//Here is where the magic needs to happen
			//I need to parse the dictionary here, and create a call node to the parent for each 
			//Subsequent parse. It'll loop over the first object for now. We create a new python object for
			//each item in the contents. Which means a new complex object
			for (int i = 0; i < obj->numContents(); i++){
				actionNet->defcallnode(currentNode = actionNet->uniqueID(), (LWNEW)&ActionNet::SubNet, NULL, (ACTFUNC)&ActionNet::UnMark, (void *)cobj);
			}
		}

		// }
		// else {
		//	  actionNet->deftrans(currentNode, (CONDFUNC)&(ActionNet::errorcond), currentNode);
		// }
		par_debug("ProcessMgr: Building PaTNet - CREATED node %d for action %s\n", currentNode, tokenName);


		if (parent != -1)
		{
			actionNet->deftrans(parent, (CONDFUNC)&(ActionNet::finishcond), currentNode);
			par_debug("ProcessMgr: Building PaTNet - CONNECTING %d to %d\n", parent, currentNode);
		}
		*startSet = *endSet = currentNode;
		actionNet->setTotalNodes(actionNet->getTotalNodes() + 1);

	}
	else
		onError("token is neither a operator or action name");

	return currentNode;

}





