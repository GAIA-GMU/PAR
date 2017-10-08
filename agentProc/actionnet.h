#ifndef _ACTION_NET_
#define _ACTION_NET_

#ifndef Py_PYTHON_H
#include <Python.h>
#endif Py_PYTHON_H

#include "lwnet.h"
#include "parseaction.h"


// This is the patnet that is built dynamically at runtime
// from the dictionaries returned by the preparatory specification,
// or applicability conditions or the complex execution steps.

class ActionNet : public LWNet 
{
 
private:
  // unique node number
  int globalNum;

  // total number of nodes in the patnet
  int totalNodes;

  // generate unique number for the patnet node
  int uniqueID(void) {return globalNum++;};

  // patnet node-action functions
  void primact(void);
  void actfunc(void);

  LWNet *SubNet(void *);
  void  UnMark(void);

public:

  // constructor
  ActionNet(void *args, int numNodes = 100);

  // set the total number of nodes in the patnet
  void	setTotalNodes(int tnodes) {totalNodes = tnodes;};

  // get the total number of nodes in the patnet
  int   getTotalNodes(void) {return totalNodes;};

  // friends
  friend int Parse::parseSeq(ComplexObj *cobj, ActionNet *actionNet, int parent, int failExitNode, int *startSet, int *endSet); 
  friend int Parse::parseSel(ComplexObj* cobj, ActionNet *actionNet, int parent, int failExitNode, int *startSet, int *endSet);
  friend int Parse::parsePar(ComplexObj *cobj, ActionNet *actionNet, int parent, int failExitNode, int *startSet, int *endSet, int type);
  friend int Parse::parseComplex(ComplexObj *cobj, ActionNet *actionNet, int parent, int failExitNode,  int *startSet, int *endSet,bool fail_parnet);
  friend int Parse::parseDict(ComplexObj *cobj, ActionNet *actionNet, int parent, int failExitNode);
  friend int Parse::parseGather(ComplexObj *cobj, ActionNet *actionNet, int parent, int failExitNode, int *startSet, int *endSet);
  friend int Parse::parseGatherComplex(ComplexObj *cobj, ActionNet *actionNet, int parent, int failExitNode, int *startSet, int *endSet, bool fail_parnet);
};
#endif




