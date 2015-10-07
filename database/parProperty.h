


#ifndef _PAR_SEMANTICS_H
#define _PAR_SEMANTICS_H

#include <string>
#include <vector>
#include <map>
///////////////////////////////////////////////////////////////////////////////
//TB:This gives us a full class representation of property information, that 
//we can then use to combine different ontological information for objects,
//actions, and abilities. All acess of it should be confined to this library
//and so not used in the main program. The semantic class is really a way of 
//speeding up calculations that would need to repeatedly call the database.
//////////////////////////////////////////////////////////////////////////////

class parProperty {
private:
	int prop_id; //the semantics machine friendly calling card
	std::string prop_name; //the semantics human readable calling card
	bool is_int; //this keeps track of the kind of semantic we're representing
	std::vector<std::string> named_properties;//This holds our semantics if the semantic information is a set of strings 

public:
	 parProperty(std::string name, bool is_semantic_int=false);
	
	std::string  getPropertyName() const{return prop_name;}//Gives back the semantic's name
	int  getPropertyID() const{return prop_id;}//Gives back the semantic's id
	bool  isInt() const{return is_int;}//Gives back the semantics type

	//These two are strickly for string value types
	std::string  getPropertyNameByValue(int id);
	int  getPropertyValueByName(std::string name);

};
#endif