#ifndef _PAR_TABLE_H
#define _PAR_TABLE_H


//class AgentTable;
//class ActionTable;
//class IParTable;

#include "workingmemory.h"
#include <map>
//#include "agentproc.h"
#include <vector>
#include "par.h"
#include <iostream>

class AgentProc;

struct actfunc
{
	char *name;  
    int (*func)(iPAR* ipar);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////
//The AgentTable is required for the PARs to function properly.  When iPARs are popped from a queue,
//the AgentTable is searched for the cooresponding agent process that uses this.
////////////////////////////////////////////////////////////////////////////////////////////////////
class AgentTable
{
  
	std::map<std::string,AgentProc*> agents; 
public:
	void addAgent(const std::string&, AgentProc *);
	void removeAgent(const std::string& name);
	AgentProc *getAgent(const std::string& name); 

	void clearTable() { agents.clear(); }//Clears the table so we don't have to
	
};


class ActionTable
// map UPARS to callback functions
{
  
	std::list<actfunc*> actions; 

public:
  void addFunctions(const char *name, int (*f1)(iPAR *ipar));

  actfunc *getFunctions(const char *name);

	
  
};
enum Status {\
	     notfound, \
	     onqueue, \
	     preproc, \
	     prepspec, \
	     exec, \
	     completed, \
	     aborted, \
	     failure, // set as soon as failure has been detected 
	     failed // set after failure recovery and this is what is 
	            // seen by the experienced situations
};

struct iparst
{
    iPAR *ipar;
    Status status;
};


// ipar table or the experienced situations table
class IParTable
{
   std::vector<iparst*> iparstlist;
   int same(const iPAR *ipar1, const iPAR *ipar2);

public:
  IParTable(void){};
  ~IParTable(){};

  // add an ipar to the table with its status
  void add(iPAR *ipar, const Status status);

  // remove an ipar from the table
  void remove(const iPAR *ipar);

  // set the status of an ipar existing in the table
   int  setStatus(const iPAR *ipar, const Status status);

  // get the current status of the ipar
  Status getStatus(const iPAR *ipar);

  // is there any active action?
  bool activeAction(void); 

  // is there an action on the Q
  bool onQAction(void);

  // return a pointer to the action that is currently executing
  iPAR* actionExecuting(void);

   // return a pointer to the action that is currently in prep specs
  iPAR* actionPrepSpecs(bool first=true);

  // has any action failed
  bool failAction(void); 

  // preempt any active actions with lower priorities
  int preempt(int priority); 

  // does the ipar already exist in the table
  int iparExist(const iPAR *ipar);

  // get the ipar from the table
  iPAR *getIPAR(const char *uparName);

  // get the most recent ipar from the table
  iPAR *getRecentIPAR(const char *uparName);

  // chnage the status of all ipars which have status "failure" to failed
  void resetFailure(void); 
};


#endif
