

#ifndef _META_ACTION_H
#define _META_ACTION_H

#include "utility.h"
#include "parProperty.h"


class MetaAction{  
private:
	int		    actID;	     // the id of this action
	int         site_type;	//The site type of this action (if it has one)
	std::string name;
	std::vector<MetaAction *>parents;	//The parent action
	int         num_objects; //The maximum number of objects that the meta-action can use 
	std::map<parProperty*, std::list<double>> properties; //Properties are sets, and so we need to know which properties are allowed
	void setupProperties(); //A helper function to get all properties for actions
	bool        once; //Describes if the action has applicability (run once) coniditions
	bool        every;//Describes if the action has culmination (run every time) conditions
	bool        exec; //Describes if the action has execution steps
	bool        prep; //Describes if the action has preparitory specifications;


 public:				// should be protected;
     MetaAction(const char* str = "");
	 MetaAction(const char* str, MetaAction* parent);
	 MetaAction(int);  // just store the id
	 //Gives all of the properties an object has
	 void setAllProperties(std::map<parProperty*, std::list<double>> &);
	 std::map<parProperty*, std::list<double>>  &getAllProperties() { return properties; } //Returns all properties


	int		 getID() {return actID;}
	std::string  const&   getActionName(){return name;}
	void     setActionName(std::string newName);
	//These are the FIDAG functions
	void	 setParent(MetaAction* parent,int which = 0);// place this action in the hierarchy 
	MetaAction  *getParent(int which = 0);
	int getNumParents(){ return parents.size();}

	void	 setApplicabilityCond(const std::string& appCond);
	std::string 	 getApplicabilityCond();
	void	 setTerminationCond(const std::string& termCond);
	std::string	 getTerminationCond();
	void	 setPreparatorySpec(const std::string& prepSpec);
	std::string 	 getPreparatorySpec();
	void	 setExecutionSteps(const std::string& execSteps);
	std::string 	 getExecutionSteps();

	void	 setPurposeAchieve(const std::string& achieve);
	std::string 	 getPurposeAchieve();

	int		 getNumObjects(){return num_objects;}

	float	 getDuration();
	void	 setDuration(float d);

	MetaObject  *searchAffordance(int position,int which=0);

	std::string	 getAdverb();	// later allow for more than one adverb
	std::string	 getModifier();	// later allow for more than one modifier
	void	 setAdverb(std::string & , std::string &) ;
	

	// Path related stuff
	int	 getSiteType();

	//Action property information
	void setProperty(parProperty*, double,bool pass_parent = false);
	double getProperty(parProperty*,int which = 0);
	bool hasProperty(parProperty*, double);
	parProperty* getPropertyType(int which);

	//Action execution stuff
	bool getOnce() { return this->once; }
	bool getEvery(){ return this->every; }
	bool getExec() { return this->exec; }
	bool getPrep() { return this->prep; }
	void setOnce(bool ans) { this->once=ans; }
	void setEvery(bool ans){ this->every=ans; }
	void setExec(bool ans) { this->exec=ans; }
	void setPrep(bool ans) { this->prep=ans; }
	

};
#endif

