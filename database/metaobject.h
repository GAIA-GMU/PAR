

#ifndef _META_OBJECT_H
#define _META_OBJECT_H


#include "utility.h"
#include "actionary.h"
#include "parProperty.h"

class MetaObject {  
private:
	friend class Actionary;			// Allow the Actionary access to private variables
	int				objID;			// The id of this object in the Actionary
	std::string		name;			// storing the name to save some database look ups
	MetaObject*     parent;			// Likewise, storing the parent in the database saves look ups as well
	std::list<MetaObject*> contents; //Holds the physcial contents of the metaobjects
	MetaObject*     location;		   //Likewise, this holds this metaobject's location (what has it in the contents)
	std::list<MetaObject*> possessions;//Holds the items possesions, or items the object owns
	MetaObject*     possessedBy;         //For symetry and computational effiecientcy
	std::map<parProperty*, std::list<double>> properties; //Properties are sets, and so we need to know which properties are allowed
	bool            instance;
	Vector<3>		position; 
	Vector<3>		velocity;
	Vector<3>		acceleration;
	Vector<3>		orientation; 
	Vector<3>		coordinateSystem;  // essentially a site
	Vector<3>       boundingVol[8];
	void setupProperties(); //A helper function to get all properties for actions


	
public:
	int  getID() {return objID;};
	const  std::string &getObjectName(){return name;}//Set as const ref because the name should never be changed
	bool  isAgent();
	bool  isRoom();
	bool  isInstance(){return instance;}

	 MetaObject(const char* obName, bool agent = false,bool obj_inst=true); // constructor
	 ~MetaObject();//Destructor
	void  setParent(MetaObject*);// place this object in the hierarchy 
	MetaObject  *getParent(){return parent;}

	float	    getBoundingRadius();					// get the object's bounding radius
	void        setBoundingRadius(float radius);		// set the object's bounding radius
	Vector<3>   *getBoundingPoint(int which);			// Grabs a point in the bounding box
	void        setBoundingPoint(Vector<3> *point, int which);//Sets a point in the bounding radius

	void     setObjectName(std::string newName);

	Vector<3>  *getPosition();
	void        setPosition(Vector<3>* vector);
	Vector<3>  *getVelocity();
	void        setVelocity(Vector<3>* vector);
	Vector<3>  *getAcceleration();
	void        setAcceleration(Vector<3>* vector);
	Vector<3>  *getOrientation();
	void        setOrientation(Vector<3>* vector);  
	Vector<3>  *getCoordinateSystem();
	void        setCoordinateSystem(Vector<3>* vector);

	/*Location and content*/
	void         setLocation(MetaObject* obj);
	void		 setLocation(std::string objName);
	MetaObject  *getLocation();
	MetaObject  *getRoomLocation();//Room location keeps searching until it finds the room the object is in
	void         addContents(MetaObject* obj);
	void         removeFromContents(MetaObject* obj);
	void         deleteContents();
	int          numContents(){ return contents.size(); }
	bool         searchContents(MetaObject* obj);
	bool         searchContents(std::string obj_name);
	MetaObject  *searchContents(int which);//Gets contents based on the number
	MetaObject  *searchContentsForType(std::string type_name,bool not_agent=false);
	MetaObject  *searchContentsForType(MetaObject *type,bool not_agent=false);
	void  updateContentsPosition(Vector<3>* new_position);

    /*Possessions*/
	void         setPossessedBy(MetaObject* obj);
	MetaObject  *getPossessedBy();
	void         addPossession(MetaObject* obj);
	void		 removeFromPossessions(MetaObject* obj);
	void         deletePossessions();
	bool         searchPossession(MetaObject *obj);
	bool         searchPossession(std::string objName);
	MetaObject  *getPossessionOfType(MetaObject* obj);
	MetaObject  *getPossessionOfType(std::string typeName);
	int          numPossessions(){ return possessions.size(); }

	void  addSite(int siteType,
		float sitePosX, float sitePosY, float sitePosZ,
		float siteOrientX, float siteOrientY, float siteOrientZ);
	void  updateSite(int siteType,     // use -999 where values should not be altered
		float sitePosX, float sitePosY, float sitePosZ,
		float siteOrientX, float siteOrientY, float siteOrientZ);
	void  removeSite(int siteType);
	MetaObject  *searchSites(char* siteName); // return the obj holding the site name or NULL
	std::string  getSiteName(int siteType);
	MetaObject  *searchSites(int site_type);//return the obj holding the site name or NULL
	float  getSitePos(int siteType, int which);
	Vector<3>  getSitePos(int siteType);//Returns the whole thing
	float  getSiteOrient(int siteType, int which);
	Vector<3>  getSiteOrient(int siteType);//Returns the whole thing

	//These set our properties
	int  setProperty(parProperty*,double,bool send_up=false,bool write_to_db=false);
	int  setProperty(parProperty*, std::string, bool send_up = false, bool write_to_db = false);
	void  removeProperty(parProperty*, int which=0);

	//Gives all of the properties an object has
	std::map<parProperty*, std::list<double>>  &getAllProperties() { return properties; } //Returns all properties
	void setAllProperties(std::map<parProperty*, std::list<double>>&); //Sets all the properties based on a list

	//These two functions return the name and value that a given object has for a given property
	std::string  getPropertyName(parProperty*,int which=0);
	bool hasProperty(parProperty*, double);
	double    getPropertyValue(parProperty*,int which = 0);

	//The affordances replace object capabilities (which state what an object's capable of performing)
	bool  searchAffordance(MetaAction *act, int which);
	MetaAction  *searchAffordance(int position, int which=0);

	//Getters and setters for forcing something to be an agent or a room. This is only for types
	bool setAgent();
	bool setRoom();

	


};		

#endif

