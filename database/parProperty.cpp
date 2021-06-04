#include "parProperty.h"
#include "actionary.h"
#include "par.h"

extern Actionary* actionary;

///////////////////////////////////////////////////////////////////////////////
//This constucts our semantic information type
///////////////////////////////////////////////////////////////////////////////
parProperty::parProperty(int id,std::string name, bool is_semantic_int, int p_type) :
	prop_id(id),prop_name(name),is_int(is_semantic_int),property_type(p_type){
	//If we have the semantic property within the database, we can
	//use all that information to populate our understanding
	//Since we're in here, we might as well populate the set vector with 
	//our actual semantic information
	if(!is_int){
		if(actionary->hasTable(prop_name)){
			int string_id=0;
			std::string prop;
			bool finished=false;
			while(!finished){
				prop=actionary->getPropertyNameByValue(prop_name,string_id);
				if(prop_name.compare(""))
					finished=true;
				else{
					while((int)named_properties.size() <string_id)
						named_properties.push_back("");
					named_properties.push_back(prop_name);
				}
			string_id++;
			}
		}
		else{
			par_debug("Table doesn't exist for property %s\n",name.c_str());
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//This parallels the actionary's getPropertyNameByValue, and in reality, the 
//actionary just calls this
///////////////////////////////////////////////////////////////////////////////
std::string 
parProperty::getPropertyNameByValue(int id){
	if(id <0 || id > (int)named_properties.size()) 
		return "";
	std::string prop=named_properties[id];

	return prop;
}
///////////////////////////////////////////////////////////////////////////////
//This parallels the actionary's getPropertyValueByName.  It too is called by
//the actionary, which is much faster then looking up the data in a table
///////////////////////////////////////////////////////////////////////////////
int
parProperty::getPropertyValueByName(std::string prop){
	if (!strcmp(prop.c_str(),""))
		return -1;
	for(unsigned int i=0; i<named_properties.size(); i++)
		if(!strcmp(prop.c_str(),named_properties[i].c_str()))
			return (int)i;

	return -1;
}