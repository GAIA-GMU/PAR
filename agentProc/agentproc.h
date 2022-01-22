#ifndef _AGENT_PROC_
#define _AGENT_PROC_

class AgentProc;
#include <list>

#ifndef _META_ACTION_H
#include "metaaction.h"
#endif

#ifndef _WORKING_MEMORY_H
#include "workingmemory.h"
#endif

#ifndef _PAR_TABLE_H
#include "partable.h"
#endif

class AgentNet;

// this is the base AgentProc class created for each agent
class AgentProc
{
  // name of the agent 
  std::string name;  

  // action queue that will be managed by the queue manager
  std::list<iPAR*> *iparq;

  //boolean to indicate if the agent process was correctly constructed.
  //bool initialized;

  //Vector of MetaActions that the agent can perform
  std::list<MetaAction*> capabilities;
  // keep a pointer to the MetaObject so that we don't have to keep searching for it
  MetaObject *object;

  AgentNet *agent_network;
  
public:
  // sort the queue and determine available time in schedule
  int getAvailableTime();
  int startTimeAvailable;  // the start time of the available time

  // The ipar table or the Experienced Situations table
  IParTable *ipt;

  iPAR* actionExecuting(); //return a pointer to the par that is running
  // Is there an ipar on the queue?
  bool emptyQueue(){return iparq->empty();};
  
  // ipt status
  bool onQAction(void);

  // Is there an ipar being processed?
  bool activeAction();

  // constructor
  AgentProc(const std::string& name);
  ~AgentProc(); //Destructor that cleans things up when the agent is no longer there

  // boolean function to check if the agent process was correctly
  // initialized
  //bool  error(void) {return !initialized;};

  // add an action to the agent's queue
  void addAction(iPAR *ipar);

  // suspend the current action
  void suspendAction();
  
  // add a list of actions to the agent's queue 
  void addAction(std::list<iPAR *> &iparlist);

  // add a list of actions to the agent's queue - each action is a
  // python object from which an ipar can be created internally
  bool addAction(PyObject *iparlist, std::list<iPAR*> *returnList=0);

  // get the name of the agent associated with the process
  const std::string & getName() {return name;}

  // get the MetaObject pointer
  MetaObject* getObject() {return object;};

 // virtual function for message passing between agents 
  virtual void inform(char *flag, void *args); 

  // flush the action queue of the agent. This is done during 
  // the failure recovery process.
  void flushQ(void);

  // restart the queue. Whenever a failure is detected by the
//  motion generator and hence by the process manager, the queue is
//  iternally frozen such that no other action canbe popped from the
//  action  queue. After the agent has taken care of the failure
//  recovery process, this method can be called to restart the queue
//  in the as-is condition
  void restartQ(void);

  //This function searches through the executing action and iPARtable looking for 
  //an iPAR with the given name, returning the first one that it finds
  iPAR *searchiPAR(const std::string& ipar_name);

  //This virtual function is meant to figure out how the agent (not the agent's action)
  //should change, usually with each step
  virtual int update(void *){return 0;}

  //These three functions define the capabilities of an agent
  //Since agents are the only objects that should be able to
  //perform actions, by including capabilities in the agentProc
  //(instead of the actionary), we reinforce this requirement
  void setCapability(MetaAction *act);
  void setCapability(const char *actname);
  void removeCapability(MetaAction *act);
  bool searchCapability(MetaAction *act);

};


  

#endif
