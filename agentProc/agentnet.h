#ifndef _AGENT_NET_H
#define _AGENT_NET_H

#ifndef _LWNET_H
#include "lwnet.h"
#endif

#include <list>


#ifndef Py_PYTHON_H
#include "Python.h"
#endif

#ifndef _AGENT_PROC_
#include "agentproc.h"
#endif

// This is the QueueManager of the Agent Process.
// This patnet is created internally by the AgentProc base class
// and is not accessible to other classes

class AgentNet : public LWNet
{
private:
  // The top-level queue of ipars to be executed by the agent
  std::list<iPAR*> 		*iparQ;

  // current ipar under execution
  iPAR			*ipar; 

  // boolean to check if an action was popped correctly from the queue
  bool			popError; 

  // The corresponding AgentProc
  AgentProc		*agentproc;

  enum NODES {
 	 START, 
	 CURRACTION, 
	 PREEMPT, 
	 POPACTION, 
	 PARACTION, 
	 EXITNODE, 
	 NUMNODES
   };

  // Patnet node functions
  void 	 		actfunc_start(void);
  void			actfunc_curraction(void);
  void			actfunc_preempt(void);
  void  		actfunc_popaction(void);
  void			actfunc_paraction(void);

  // is the ipar queue non-empty?
  bool			nonemptyQ(void);

  // is the ipar queue empty?
  bool			emptyQ(void);

  // patnet boolean function to check if an action was popped correctly 
  bool			condfunc_poperror(void);

  // is there any action being actively processed by the process
  //manager?
  bool			activeAction(void);

  // is there any action that is in status "failure"
  bool			noFailAction(void);

  // return the action on the queue that has the least startTime
  iPAR*			getFirstAction(void);

  // is this the timing such that this ipar is ready to be executed?
  bool			readyToGo(iPAR* cfipar);

public:

  // constructor
  AgentNet(std::list<iPAR*> *IParQ, AgentProc *ap);

};
#endif





