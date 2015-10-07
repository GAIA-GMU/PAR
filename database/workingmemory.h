#ifndef _WORKING_MEMORY_H
#define _WORKING_MEMORY_H


#include "metaobject.h"
#include "metaaction.h"
//#include "Python.h"
#include "utility.h"

class FailData;

class iPAR {		
private:
	int    iparID;
	double start_time; 
	float  duration;  
	bool finished;
	int priority;
	std::map<parProperty*,int> properties;
	FailData *fail_data;
	bool fail_parent;//This is set when the parent action should fail (for certain types of complex actions)
	std::string manner;
	
public:
  std::vector<MetaObject*> objects; 
  MetaObject *agent; 
  int enable_act; 
  int enabled_act; 
  MetaAction*		par;
 
  iPAR(std::string, const std::string&, char* objnames[]);
  iPAR(std::string, const std::string&, std::vector<char*> *objnames); 
  iPAR(std::string, const std::string&); // set the objects separately
  iPAR(int actID);  // iPARs for iPAR actions already in the Actionary 

  iPAR* copyIPAR();  // return a copy of this iPAR
  void copyValuesToPython();//Copies all python specific values to python for easier scripting

  int           getID(){return iparID;}
  MetaObject*   getAgent();
  void          setAgent(MetaObject* agent);
  void          setAgent(const std::string&  agname);
  void	        setObject(MetaObject* obj, int which);
  MetaObject*   getObject(int which);
  double		getStartTime();
  float			getDuration();
  void			setDuration(float d);
  void			setStartTime(double t);
  void			setStartTime(int hours, int minutes, int seconds);
  int			getPriority();
  void			setPriority(int prior);
  void          setPurposeEnableAct(int par_id);
  int           getPurposeEnableAct();
  void			setEnabledAct(int par_id);
  int			getEnabledAct();
  void setProperty(parProperty*,int);
  int getProperty(parProperty*);
  parProperty* getPropertyType(int which);
  void setManner(const std::string&);
  std::string getManner();

  void setFailParent(bool val) { fail_parent = val; }
  bool getFailParent() { return fail_parent; }

  void setFinished(bool fin){finished=fin;}
  bool getFinished(){return finished;}

  void setFailData(FailData* dat){fail_data=dat;}
  FailData* getFailData(){return fail_data;}

  //PyObject 		*testCulminationCond();

};

#endif
