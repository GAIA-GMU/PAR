


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
//This holds the possible properties, not the actual values
//////////////////////////////////////////////////////////////////////////////

class parProperty {
private:
	int prop_id; //the semantics machine friendly calling card
	std::string prop_name; //the semantics human readable calling card
	bool is_int; //this keeps track of the kind of semantic we're representing
	bool is_cont; //This keeps track of the kind of semantic we're representing
	int property_type; //This describes if the property is a object, action, or both property
	//is_int puts a bunch of integer representations.
	std::vector<std::string> named_properties;//This holds our semantics if the semantic information is a set of strings 

public:
	 parProperty(int id,std::string name, bool is_semantic_int=false,int p_type = 0);
	
	std::string  getPropertyName() const{return prop_name;}//Gives back the semantic's name
	int  getPropertyID() const{return prop_id;}//Gives back the semantic's id
	bool  isInt()   const {return is_int;}//Gives back the semantics type
	bool  isCont()  const { return is_cont;}//We use this to determine if we use the continous value
	int   getType() const {	return property_type;}//Determines if the property is an object, action, or can be used for both
	//These two are strickly for string value types
	std::string  getPropertyNameByValue(int id);
	int  getPropertyValueByName(std::string name);

};
#endif