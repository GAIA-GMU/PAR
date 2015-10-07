

#ifndef _META_ACTION_H
#define _META_ACTION_H

#include "utility.h"


class MetaAction{  
private:
	int		    actID;	     // the id of this action
	int         site_type;	//The site type of this action (if it has one)
	std::string name;
	MetaAction *parent;	//The parent action
	int         num_objects; //The maximum number of objects that the meta-action can use 
	
 public:				// should be protected;
     MetaAction(const char* str = "");
	 MetaAction(const char* str, MetaAction* parent);
	 MetaAction(int);  // just store the id

	int		 getID() {return actID;};
	std::string  const&   getActionName(){return name;}
	void     setActionName(std::string newName);
	void	 setParent(MetaAction* parent);			// place this action in the hierarchy 
	MetaAction  *getParent();

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
	void	 setNumObjects(int num);

	float	 getDuration();
	void	 setDuration(float d);

	MetaObject  *searchAffordance(int position,int which=0);

	std::string	 getAdverb();	// later allow for more than one adverb
	std::string	 getModifier();	// later allow for more than one modifier
	void	 setAdverb(std::string & , std::string &) ;
	

	// Path related stuff
	int	 getSiteType();

};
#endif

