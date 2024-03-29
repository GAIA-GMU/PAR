
#include <iostream>
#include "interpy.h"
#include "workingmemory.h"
#include "metaobject.h"
#include "par.h"
#include "actionary.h"
#include "agentproc.h"
#include <sstream>

extern "C" {
#include "graminit.h"
}

extern Actionary *actionary;
extern parTime* partime;
extern AgentTable agentTable;
PythonTable pytable;

static const char* controlStr1 = "\
class %s:\n\
\tpass\n\n\
%s = %s()\n";

static const  char* controlStr2 = "\
class %s(%s):\n\
\tpass\n\n\
%s = %s()\n";

static const char* controlStr3 ="\
%s=Class%s()\n";

static const char* std_catcher = "\
class StdoutCatcher:\n\
\tdef __init__(self):\n\
\t\tself.data = ''\n\
\tdef write(self, stuff):\n\
\t\tpar_debug(stuff)\n\
import sys\n\
catcher=StdoutCatcher()\n\
sys.stdout=catcher\n\
sys.stderr=catcher\n";

extern "C" int
createPyActions(MetaAction* act) {
	std::stringstream buffer;

   //char className[SMALLBUF], className2[SMALLBUF], buf[MAXBUF], buf2[MAXBUF];

   //debug("In createPyActions with action %s %d\n",obj->getActionName(),obj->getID());
   if (act->getNumParents() > 0) {
     //sprintf_s(className2,SMALLBUF, "Class%s", obj->getParent()->getActionName().c_str());
	   buffer << "class Class" << act->getID()<<"(";
	   for (int i = 0; i < act->getNumParents()-1; i++)
		buffer << "Class" << act->getParent(i)->getID()<<",";
	   buffer << "Class" << act->getParent(act->getNumParents() - 1)->getID();
	   buffer << "):\n\tpass\n";
     //sprintf_s(buf,MAXBUF,controlStr2, className, className2, obj->getActionName().c_str(), className);
   } else {
	   buffer << "class Class" << act->getID() << " :\n\tpass\n";
   }

   //buffer << obj->getActionName() << ".name = '" << obj->getActionName() << "'\n" << obj->getActionName() << ".id=" << obj->getID() << "\n";
   //par_debug("%s",buffer.str().c_str());
   return PyRun_SimpleString(buffer.str().c_str());

}

extern "C" int
createPyCondAsserts(MetaAction* act) {
	std::stringstream buffer;
	std::stringstream objects;
	

	//debug("In createPyActions with action %s %d\n",obj->getActionName(),obj->getID());
	if (act->getNumParents() > 0) {
		bool found = false;
		for (int i = 0; i < act->getNumParents() - 1; i++){
			if (act->getParent(i)->getOnce()){
				if (!found){
					found = true;
				}
				else{
					buffer << "\tthis." << act->getParent(i)->getActionName() << ".applicability_condition("<<objects.str()<<")\n";
				}
			}
		}
		if (found){
			buffer << "\treturn INCOMPLETE\n";
		}
		//buffer << "class Class" << act->getID() << "(";
		//
		//	buffer << "Class" << act->getParent(i)->getID() << ",";
		//buffer << "Class" << act->getParent(act->getNumParents() - 1)->getID();
		//buffer << "):\n\tpass\n";
		return PyRun_SimpleString(buffer.str().c_str());
		//sprintf_s(buf,MAXBUF,controlStr2, className, className2, obj->getActionName().c_str(), className);
	}
	return -1;
}

extern "C" int
createPyiPARs(iPAR* ipar) {
	std::stringstream buffer;
	buffer << ipar->par->getActionName() << "_" << ipar->getID();
	buffer << "=" << "Class" << ipar->par->getID()<<"()\n";
	buffer << ipar->par->getActionName() << "_" << ipar->getID() << ".name = '" << ipar->par->getActionName() << "'\n";
	buffer << ipar->par->getActionName() << "_" << ipar->getID() << ".id=" << ipar->getID() << "\n";
   //char className[64], buf[MAXBUF], buf2[MAXBUF];
   //sprintf(className,"%s_%d",ipar->par->getActionName().c_str(),ipar->getID());
   //sprintf(buf,controlStr3,className,ipar->par->getActionName().c_str());
   //sprintf(buf2, "%s.name = '%s'\n%s.id=%d\n", className, ipar->par->getActionName().c_str(),className,ipar->getID());
   //strcat(buf, buf2);
   //par_debug("Buffer is %s\n",buffer.str().c_str());
   return PyRun_SimpleString(buffer.str().c_str());

}

///////////////////////////////////////////////////////////////////////////////
//Sets all values of the ipar that should remain static throughout the couse
//of the action, from the time it is popped from the iPAR queue to it's end
///////////////////////////////////////////////////////////////////////////////
extern "C" int
addPyiPARValues(iPAR *ipar){
	if(ipar == NULL){
		return 0;
	}
	std::stringstream prop_string;	
	//We add in the start time
	prop_string<<ipar->par->getActionName()<<"_"<<ipar->getID()<<".start_time="<<ipar->getStartTime()<<'\n';
	//duration
	prop_string<<ipar->par->getActionName()<<"_"<<ipar->getID()<<".duration="<<ipar->getDuration()<<'\n';
	//manner
	prop_string<<ipar->par->getActionName()<<"_"<<ipar->getID()<<".manner='"<<ipar->getManner()<<"'"<<'\n';
	//Each specific iPAR property that is set
	int counter=0;
	parProperty* prop=ipar->getPropertyType(counter);
	while(prop != NULL){
		if (prop->isInt()){ //If it is an integer, python should represent it in that manner
			prop_string << ipar->par->getActionName() << "_" << ipar->getID() << ".properties['" << prop->getPropertyName() << "']=" <<(int)ipar->getProperty(prop) << '\n';
		}
		else if (prop->isCont()){ //If it is continous, python should represent it in that manner
			prop_string << ipar->par->getActionName() << "_" << ipar->getID() << ".properties['" << prop->getPropertyName() << "']=" <<ipar->getProperty(prop) << '\n';
		}
		else{ //It should be represented as a string
			
			prop_string << ipar->par->getActionName() << "_" << ipar->getID() << ".properties['" << prop->getPropertyName() << "']=" << prop->getPropertyNameByValue(ipar->getProperty(prop)) << '\n';
		}
		counter++;
		ipar->getPropertyType(counter);
	}
	par_debug("%s\n", prop_string.str().c_str());
	return PyRun_SimpleString(prop_string.str().c_str());
}

extern "C" int
runPySimple(char* str, bool file) {
   //char buf[MAXBUF];
   int res;

   if (file) {
	//The new way of doing this is to open the file as a file pointer in C, and 
	//then run the file 
	   FILE* file_name = _Py_fopen(str, "rb"); 
	   if (file_name == NULL) {
		   par_debug("Failed to open %s\n", str);
		   return -1;
	   }
	   //We need to ensure the file exists before we do anything else
	   res = PyRun_SimpleFile(file_name, str);
	   if (res == -1) {
		   PyErr_Print();
		   PyErr_Clear();
	   }
	   //fclose(file_name);
	//PyObject* PyFileObject = PyFile_FromString(str, "r");
	//Py_XINCREF(PyFileObject);
	//if (PyFileObject == NULL) {
	//	PyErr_Print();
	//	PyErr_Clear();
	//	return -1; // Let the user know the error.
	//}
	// Function Declration is: int PyRun_SimpleFile(FILE *fp, char *filename);
	// So where the hack should we get it a FILE* ? Therefore we have "PyFile_AsFile".
	//res = PyObject_AsFileDescriptor(PyFileObject);
	//res = PyRun_SimpleFile(PyFile_AsFile(PyFileObject), str);
	//Py_XDECREF(PyFileObject);//Should be of no use, but just in case
   } else {
      res = PyRun_SimpleString(str);
   }
   return res;
}


extern "C" PyObject*
runPy(const char* str) {
   PyObject* pmod = PyImport_ImportModule("__main__");
   PyObject* pdict = PyModule_GetDict(pmod);

   PyObject* res = PyRun_String(str, eval_input, pdict, pdict);

   if (PyErr_Occurred()) {
      PyErr_Print();
      PyErr_Clear();
   }

   return res;
}



extern "C" bool
fromPyObjectToBool(PyObject* pobj) {
  if (!pobj) {				// NULL
    					//err("error occured.");
    return false;
  } else if (pobj == Py_None) {		// None
    					// err("Py_None is returned.");
    return false;
  } else if (PyLong_Check(pobj)) {	// integer
    					// err("Integer is returned.");
    int i;
    if (!PyArg_Parse(pobj, "i", &i)) return false;
    if (i) return true;
    else return false;
  } else if (PyDict_Check(pobj)) {	// dictionary
    					// err("Dictionary is returned.");
    if (PyDict_Size(pobj) > 0)
      return true;
    else
      return false;
  } else {				// other types
    					// err("Return value is illegal.");
    return false;
  }
}

///////////////////////////////////////////////////////////////////////////
//Gives back the integer returned from a python script.  Useful for error
//codes within the python script.
//////////////////////////////////////////////////////////////////////////
extern "C" int
fromPyObjectToInt(PyObject* pobj){
	if(!pobj || pobj == Py_None)
		return 0;

	if (PyLong_Check(pobj)){
		int i;
		if(!(PyArg_Parse(pobj,"i",&i))) return 0;
        else return i;
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////
//ability to use generalized object properties in our checks
//////////////////////////////////////////////////////////////////////////
extern "C" PyObject*
prop_getProperty(PyObject* ,PyObject* args){
	   int oName;
	   char* tab_name;

   if (!PyArg_ParseTuple(args, "i|s", &oName,&tab_name))
     return Py_BuildValue("s", "");

   MetaObject* obj=actionary->searchByIdObj(oName);
   if(obj){
	   parProperty *prop=actionary->searchByNameProperty(tab_name);
	   if(prop != NULL)
		if(prop->isInt())
			return PyLong_FromLong(obj->getPropertyValue(prop));
	  
	   return Py_BuildValue("s",obj->getPropertyName(prop).c_str());
   }
   return Py_BuildValue("s", "");
}
/////////////////////////////////////////////////////////////////////////////////////
// This python code allows post assertions to 
//change object properties
///////////////////////////////////////////////////////////////////////////////////
extern "C" PyObject*
prop_setProperty(PyObject*,PyObject* args){
	int oName;
	char* tab_name;
	char* prop_name;
	int prop_value=-1;//Defualt value for there not being a property. Removes warning 4701
	if(!PyArg_ParseTuple(args,"i|s|s",&oName,&tab_name,&prop_name))
		if(!PyArg_ParseTuple(args,"i|s|i",&oName,&tab_name,&prop_value))
			return PyLong_FromLong(0);

	MetaObject* obj=(MetaObject*) actionary->searchByIdObj(oName);
	parProperty *prop = actionary->searchByNameProperty(tab_name);
	if(obj == NULL || prop == NULL )
		return PyLong_FromLong(0);
	if(prop->isInt())
		obj->setProperty(prop,prop_value);
	else
		obj->setProperty(prop,prop_name);

	return PyLong_FromLong(1);
}
//////////////////////////////////////////////////////////////////////////////////
//This places the time_duration check within a python function.  By doing this
//we move all of our culmination checks outside the c-code (hardcoded), and so
//we can perform checks as logic dictates
/////////////////////////////////////////////////////////////////////////////////
extern "C" PyObject*
prop_getElapsedTime(PyObject*,PyObject*){
	//par_debug("Elapsed is %d\n", partime->getCurrentTime());
	return PyLong_FromLong(partime->getCurrentTime());
}
//////////////////////////////////////////////////////////////////////////////////////
//This is a simple action Completed function.  Some actions should be completed when
//the agent has finished the action (for example, picking up an object or washing an
//agents hands).  In this case, the core-execution function, usually specified as 
//doAction, should end the action. 
///////////////////////////////////////////////////////////////////////////////////
extern "C" PyObject*
action_isCompleted(PyObject*,PyObject* args){
	int act_id;
	//We can't tell if the action has finished 
	//unless we have an id
	if(!PyArg_ParseTuple(args,"i",&act_id))
		return PyLong_FromLong(0);
	iPAR* par=actionary->searchByIdiPAR(act_id);
	if(par !=NULL){
		if(par->getFinished()){
			return PyLong_FromLong(1);
		}
	}
	return PyLong_FromLong(0);
}
///////////////////////////////////////////////////////////////////////////////
//This allows us to set a failure code 
/////////////////////////////////////////////////////////////////////////////
extern "C" PyObject*
action_setFailure(PyObject*,PyObject* args){
	int act_id,fail_id;
	if(!PyArg_ParseTuple(args,"ii",&act_id,&fail_id))
		return NULL;
	iPAR *par=actionary->searchByIdiPAR(act_id);
	if(par == NULL)
		return NULL;
	//If we do not have failure data, then we need to
	//create the data structure
	if(par->getFailData() == NULL){
		par->setFailData(new FailData());
		par->getFailData()->ipar=par;
		par->getFailData()->failcode=fail_id;
	}
	par->getFailData()->failcode=fail_id;
	return PyLong_FromLong(1);
}

extern "C" PyObject*
prop_getLocation(PyObject*, PyObject* args) {
   int oName;

   if (!PyArg_ParseTuple(args, "i", &oName))
     return Py_BuildValue("i",-1);

   MetaObject* obj = (MetaObject*) actionary->searchByIdObj(oName);
   if (obj) {
     MetaObject* locObj = obj->getLocation();
	 if (locObj != NULL)
		 return Py_BuildValue("i", locObj->getID());
	 else
		 return Py_BuildValue("i", -1);
   } else {
    // Py_INCREF(Py_None);
     return Py_BuildValue("i",-1);
   }
}

extern "C" PyObject*
prop_dist(PyObject*, PyObject* args) {
   int oName1;
   int oName2;

   //std::cout << "In prop_dist" << std::endl;

   if (!PyArg_ParseTuple(args, "ii", &oName1, &oName2))
     return Py_None;  //Py_BuildValue("s", "");

   MetaObject* obj1 = actionary->searchByIdObj(oName1);
   MetaObject* obj2 = actionary->searchByIdObj(oName2);
   if (obj1 && obj2) {
	   Vector<3> *vec1 = obj1->getPosition();
	   Vector<3> *vec2 = obj2->getPosition();
	   double dist = (vec2->v[0] - vec1->v[0]) * (vec2->v[0] - vec1->v[0]) +
					(vec2->v[1] - vec1->v[1]) * (vec2->v[1] - vec1->v[1]) +
					(vec2->v[2] - vec1->v[2]) * (vec2->v[2] - vec1->v[2]);
	   dist = sqrt(dist);
	//debug("In prop_dist, the distance is %f\n", dist);
	 
	 return Py_BuildValue("f", dist);
   } else {
     Py_INCREF(Py_None);
     return Py_None; //Py_BuildValue("s", "");
   }
}

extern "C" PyObject*
prop_getBoundingRadius(PyObject*, PyObject* args) {
   int oName1;
   

   //std::cout << "In prop_getBoundingRadius" << std::endl;

   if (!PyArg_ParseTuple(args, "i", &oName1))
     return Py_None;  //Py_BuildValue("s", "");

   MetaObject* obj1 = actionary->searchByIdObj(oName1);
   if (obj1) {
		double value = obj1->getBoundingRadius();
		return Py_BuildValue("f",value);
   } else {
     Py_INCREF(Py_None);
     return Py_None; //Py_BuildValue("s", "");
   }
}

extern "C" PyObject*
prop_inFront(PyObject*, PyObject* args) {
	int oName1;
	int oName2;
	if (!PyArg_ParseTuple(args, "ii", &oName1, &oName2))
		return Py_None;  //Py_BuildValue("s", "");
	MetaObject* obj1 = actionary->searchByIdObj(oName1);
	MetaObject* obj2 = actionary->searchByIdObj(oName2);
	if (obj1 && obj2) {
		//getGlobalFromNuria
		par_debug("INFRONT IS NOT IMPLEMENTED YET\n");
		return Py_BuildValue("i", 1);		// false
	}

	 return Py_BuildValue("i", 0);		// false

}

extern "C" PyObject*
agent_checkCapability(PyObject*, PyObject* args) {
   int agname;
   int act_id; //JTB March 2015

   if (!PyArg_ParseTuple(args, "ii", &agname, &act_id))
     return Py_None;				// error

   AgentProc* ag = agentTable.getAgent(actionary->searchByIdObj(agname)->getObjectName());
  if(ag == NULL)
	  return Py_None;
   //weizi, March 2010
  MetaAction* act;
  iPAR* par = actionary->searchByIdiPAR(act_id);
  if(par != NULL)
	  act=par->par;
  else
	  act=actionary->searchByIdAct(act_id);
  if(act == NULL)
	  return Py_None;

  bool actCapable =ag->searchCapability(act);
  
  while(!actCapable && act != NULL)
  {
	  act=act->getParent();
	  if(act != NULL)
	  {
          actCapable = ag->searchCapability(act); 
	  }
  } 
  if(!actCapable){
	  return Py_BuildValue("i",0);
  }
   return Py_BuildValue("i", 1);

}
//////////////////////////////////////////////////////////////////////////
//Searches for capabilities through the object tree.  This also searches
//by obj_affordances instead of obj_capable in order to perserve poisiton
////////////////////////////////////////////////////////////////////////
extern "C" PyObject*
object_checkCapability(PyObject*, PyObject *args){
	int agname;
     int act_id;
     int position;

   if (!PyArg_ParseTuple(args, "iii", &agname, &act_id, &position))
     return Py_None;				// error
  MetaObject* ag = actionary->searchByIdObj(agname);   
  MetaAction* act;
  iPAR* par = actionary->searchByIdiPAR(act_id);
  if(par != NULL)
	  act=par->par;
  else
	  act=actionary->searchByIdAct(act_id);
  if(act == NULL || ag == NULL)
	  return Py_None;
  //Here, we check both the actions and object hierarchy to figure out if 
  //there is a connection
  int actCapable = actionary->searchAffordance(act,ag);
  MetaObject *on_action = actionary->searchAffordance(act, position, 0);//Probably can optimize this
  if (on_action){
	  MetaObject *travel_obj = ag;
	  while (position != actCapable && travel_obj != NULL){
		  actCapable = actionary->searchAffordance(act, travel_obj);
		  travel_obj = travel_obj->getParent();
	  }
  }
  else{
	  //If the role does not exist, then we want to ask the parents. We don't want to ask them if it does
	  std::queue<MetaAction*> parents;
	  parents.push(act);
	  MetaAction* travel_act = act;
	  while (position != actCapable && travel_act != NULL){
		  parents.pop(); //We first remove the one we are looking at from the queue
		  MetaObject *travel_obj = ag;
		  while (position != actCapable && travel_obj != NULL){
			  actCapable = actionary->searchAffordance(travel_act, travel_obj);
			  travel_obj = travel_obj->getParent();
		  }
		  if (position != actCapable){ //If we did not find it, lets add stuff to the queue
			  for (int i = 0; i < travel_act->getNumParents(); i++){
				  parents.push(travel_act->getParent(i));
			  }
		  }
		  if (parents.empty()){
			  travel_act = NULL;
		  }
		  else{
			  travel_act = parents.front();
		  }
	  }
	  /*while(position !=actCapable && ag != NULL){
		  MetaAction* travel_act = act;
		  parents.pop();
		  if (ag != NULL){
		  while (position != actCapable && travel_act != NULL){
		  if (travel_act != NULL){
		  actCapable = actionary->searchAffordance(travel_act, ag);
		  travel_act = travel_act->getParent();
		  }
		  }
		  ag = ag->getParent();
		  }
		  }*/
  }
  if(position != actCapable)
	  return Py_BuildValue("i",0);

	return Py_BuildValue("i",1);
}

extern "C" PyObject*
object_contain(PyObject*, PyObject* args) {
  int obj1name;
  int obj2name;

  if (!PyArg_ParseTuple(args, "ii", &obj1name, &obj2name))
    return NULL;

  MetaObject* obj1 = actionary->searchByIdObj(obj1name);
  MetaObject* obj2 = actionary->searchByIdObj(obj2name);
  if (obj1 != NULL) {
      if (obj1->searchContents(obj2))
		  return Py_BuildValue("i", 1);
	  else if (obj1->searchContentsForType(obj2))
		  return Py_BuildValue("i", 1);
      else
		  return Py_BuildValue("i", 0);
  }

  Py_INCREF(Py_None);
  return Py_None;
}

///////////////////////////////////////////////////////////////////
//This gets the id of the contents at the which'd position.
//This function is useful for getting all the contents of the object
//////////////////////////////////////////////////////////////////
extern "C" PyObject*
object_getContents(PyObject*, PyObject* args) {
	int obj1name;
	int which;

	if (!PyArg_ParseTuple(args, "ii", &obj1name, &which))
		return NULL;

	MetaObject* obj1 = actionary->searchByIdObj(obj1name);
	if (obj1 != NULL) {
		MetaObject *obj2 = obj1->searchContents(which);
		if (obj2 != NULL)
			return Py_BuildValue("i", obj2->getID());
		else {
			Py_INCREF(Py_None);
			return Py_None;
		}
	}

	Py_INCREF(Py_None);
	return Py_None;
}

extern "C" PyObject*
object_getNumContents(PyObject*, PyObject* args) {
	int obj1name;

	if (!PyArg_ParseTuple(args, "i", &obj1name))
		return NULL;
	MetaObject* obj1 = actionary->searchByIdObj(obj1name);
	if (obj1 != NULL) {
		return Py_BuildValue("i", obj1->numContents());
	}
	return Py_BuildValue("i", 0);
}
/////////////////////////////////////////////////////////////////
//Allows us to update the contents of an object.  If the object
//has the item as it's contents, we remove it.  If it doesn't,
//we add it.
///////////////////////////////////////////////////////////////
extern "C" PyObject*
object_changeContents(PyObject*, PyObject* args){
	int obj1name;
	int obj2name;

	if(!PyArg_ParseTuple(args,"ii",&obj1name,&obj2name))
		return NULL;

	MetaObject* ob1=actionary->searchByIdObj(obj1name);
	MetaObject* ob2=actionary->searchByIdObj(obj2name);
	if(ob1 != NULL && ob2 != NULL){
		if(ob1->searchContents(ob2))
			ob1->removeFromContents(ob2);
		else
			ob1->addContents(ob2);
		return Py_BuildValue("i",1);
	}
	else {
		//If object 1 is NULL, then we want to remove the location of object2
		if (ob2 != NULL) {
			ob2->setLocation(NULL);
		}
	}
	return Py_None;
}

//////////////////////////////////////////////////////////////////////////////////
//Since the most used vector is position, getVector is going to be called 
//getPosition in python
////////////////////////////////////////////////////////////////////////////////
extern "C" PyObject*
prop_getVector(PyObject*, PyObject* args) {
  int oName;

  if (!PyArg_ParseTuple(args, "i", &oName))
    return Py_None;

 Vector<3> *vec = actionary->searchByIdObj(oName)->getPosition();
 
  if (vec) {
    PyObject* l = PyTuple_New(3);
    for (int i = 0; i < 3; i++)
      PyTuple_SetItem(l, i, Py_BuildValue("f", vec->v[i]));
    return l;
 } else 
	  return Py_None;
}

/////////////////////////////////////////////////////////////////////////////////////////
//Like getVector above, setVector is just going to be for position for now
///////////////////////////////////////////////////////////////////////////////////////
extern "C" PyObject*
prop_setVector(PyObject* , PyObject* args) {
  int oName;

  float	v0, v1, v2;

  if (!PyArg_ParseTuple(args, "i(fff)", &oName, &v0, &v1, &v2))
    return NULL;
  MetaObject* obj = (MetaObject*) actionary->searchByIdObj(oName);
  if (obj) {
    Vector<3>* vec = new Vector<3>;
    vec->v[0] = v0;
    vec->v[1] = v1;
    vec->v[2] = v2;
	obj->setPosition(vec); 
  }

  Py_INCREF(Py_None);
  return Py_None;
}
extern "C" PyObject*
prop_testAppCond(PyObject*, PyObject* args) 
{
    char *actName;
    char *agentName;
    char *objs;			// a sequence of object strings
    const char *separator = "\t ,;";

    if (!PyArg_ParseTuple(args, "sss", &actName, &agentName, &objs))
	return NULL;

    MetaAction *act = actionary->searchByNameAct(actName);
    if (!act) return Py_None;

    MetaObject* obj = actionary->searchByNameObj(agentName);
    if (!obj) return Py_None;

    int num = actionary->getNumObjects(act);
    if (num < 0) return Py_None;

    char **objlist = (char **)malloc(sizeof(char *) * num);
    int i = 0;
	char* obj_token;
    char *ptr = strtok_s(objs, separator,&obj_token);
    while ((ptr != NULL) && (i < num)) {
	objlist[i] = new char[strlen(ptr)+1];
	strcpy_s(objlist[i], strlen(ptr) + 1, ptr);
	ptr = strtok_s(NULL, separator,&obj_token);	
        //printf("%s\t", objlist[i]);
	i++;
    }     

    iPAR *ipar = new iPAR(actName, agentName, objlist);

    PyObject *res = actionary->testApplicabilityCond(ipar);
    if (fromPyObjectToBool(res)) {
	return Py_BuildValue("i", 1);
    } else {
	return Py_BuildValue("i", 0);
    }

}

extern "C" PyObject*
prop_testCulCond(PyObject*, PyObject* args) 
{
    char *actName;
    char *agentName;
    char *objs;			// a sequence of object strings
    const char *separator = "\t ,;";

    if (!PyArg_ParseTuple(args, "sss", &actName, &agentName, &objs))
	return  Py_BuildValue("i", 0);

    MetaAction *act = actionary->searchByNameAct(actName);
    if (!act) return  Py_BuildValue("i", 0);

	MetaObject* obj = actionary->searchByNameObj(agentName);
    if (!obj) return  Py_BuildValue("i", 0);

	int num = actionary->getNumObjects(act);
    if (num < 0) return Py_BuildValue("i", 0);

    char **objlist = (char **)malloc(sizeof(char *) * num);
    int i = 0;
	char* obj_token;
    char *ptr = strtok_s(objs, separator,&obj_token);
    // if num of object given over required, then truncate off
    while ((ptr != NULL) && (i < num)) {
	objlist[i] = new char[(strlen(ptr)+1)];
	strcpy_s(objlist[i], strlen(ptr) + 1, ptr);
	ptr = strtok_s(NULL, separator,&obj_token);	
        //fprintf(stdout, "%s\t", objlist[i]);
	i++;
    }        
    //fprintf(stdout, "\n");

    // if num of objects given less than required, then return false
    if (i < num)  return Py_BuildValue("i", 0);

    iPAR *ipar = new iPAR(actName, agentName, objlist);

    PyObject *res = actionary->testCulminationCond(ipar);
    if (fromPyObjectToBool(res)) {
	return Py_BuildValue("i", 1);
    } else {
	return Py_BuildValue("i", 0);
    }
}

extern "C" PyObject*
prop_testPreSpec(PyObject*, PyObject* args) 
{
    char *actName;
    char *agentName;
    char *objs;			// a sequence of object strings
    const char *separator = "\t ,;";

    if (!PyArg_ParseTuple(args, "sss", &actName, &agentName, &objs))
	return NULL;

    MetaAction *act = actionary->searchByNameAct(actName);
    if (!act) return Py_None;

    MetaObject* obj = actionary->searchByNameObj(agentName);
    if (!obj) return Py_None;

    int num = actionary->getNumObjects(act);
    if (num < 0) return Py_None;

    char **objlist = (char **)malloc(sizeof(char *) * num);
    int i = 0;
	char* obj_token;
    char *ptr = strtok_s(objs, separator,&obj_token);
    while ((ptr != NULL) && (i < num)) {
	objlist[i] = new char[strlen(ptr)+1];
	strcpy_s(objlist[i], strlen(ptr) + 1, ptr);
	ptr = strtok_s(NULL, separator,&obj_token);	
        //printf("%s\t", objlist[i]);
	i++;
    }        

    iPAR *ipar = new iPAR(actName, agentName, objlist);

    PyObject *res = actionary->testPreparatorySpec(ipar);
    if (fromPyObjectToBool(res)) {
	return Py_BuildValue("i", 1);
    } else {
	return Py_BuildValue("i", 0);
    }
}
////////////////////////////////////////////////////////////////////////
//Checks to see if the object is set.  This might be overcomplicating
//the matter
//////////////////////////////////////////////////////////////////////
extern "C" PyObject*
object_isSet(PyObject*,PyObject* args){

	int obj;
	if(!PyArg_ParseTuple(args,"i",&obj))
		return Py_BuildValue("i",0);//If they didn't include an argument, then it should fail

	if(obj == -1) //An obvious false
		return Py_BuildValue("i",0);
	MetaObject *o = actionary->searchByIdObj(obj);
	if(o != NULL)
		return Py_BuildValue("i",1);
	return Py_BuildValue("i",0);

}
////////////////////////////////////////////////////////////////////////
//Determines if the object is of a given type of object. Returns True 
//if it is, and False otherwise
//////////////////////////////////////////////////////////////////////
extern "C" PyObject*
object_isType(PyObject*,PyObject* args){

	int obj;
	char *type;
	if(!PyArg_ParseTuple(args,"is",&obj,&type))
		return Py_BuildValue("i",0);//If they didn't include an argument, then it should fail

	if(actionary->isType(actionary->searchByIdObj(obj),type))
		return Py_BuildValue("i",1);
	return Py_BuildValue("i",0);
}
////////////////////////////////////////////////////////////////////////
//Determines if the ation is of a given type of action. Returns True 
//if it is, and False otherwise
//////////////////////////////////////////////////////////////////////
extern "C" PyObject*
action_isType(PyObject*, PyObject* args){

	int act;
	char *type;
	if (!PyArg_ParseTuple(args, "is", &act, &type))
		return Py_BuildValue("i", 0);//If they didn't include an argument, then it should fail

	iPAR *search_ipar = actionary->searchByIdiPAR(act);
	if (search_ipar == NULL)
		return Py_BuildValue("i", 0);
	MetaAction* search_act = search_ipar->par;
	MetaAction *type_act = actionary->searchByNameAct(type);
	if (search_act == NULL || type_act == NULL){
		return Py_BuildValue("i", 0);
	}
	bool found = false;
	while (search_act != NULL && found == false){
		if (search_act == type_act){
			found = true;
		}
		else{
			search_act = search_act->getParent();
		}
	}
	if (found == true){
		return Py_BuildValue("i", 1);
	}
	return Py_BuildValue("i", 0);
}

///////////////////////////////////////////////////////////////////////////////
//Allows us to capture debug messages from python
///////////////////////////////////////////////////////////////////////////////
static PyObject*
debug_parDebug(PyObject*, PyObject* args){
	const char *string;
	if (!PyArg_ParseTuple(args, "s", &string))
		return NULL;
	par_debug("%s\n", string);
	Py_INCREF(Py_None);
	return Py_None;
}
///////////////////////////////////////////////////////////////////////////////
//Returns an objects name from the given object ID, or the empty string
//if there isn't one
///////////////////////////////////////////////////////////////////////////////
extern "C" PyObject*
object_getName(PyObject*,PyObject* args){

	int objId;
	if(!PyArg_ParseTuple(args,"i",&objId))
		return Py_None;

	MetaObject* obj=actionary->searchByIdObj(objId);

	char obj_name[50];
	if(obj != NULL)
		sprintf_s(obj_name,"'%s'",obj->getObjectName().c_str());
	else
		return Py_None;

	return Py_BuildValue("s",obj_name);

}
////////////////////////////////////////////////////////////////////////////////
//

extern "C" PyObject*
object_getType(PyObject*, PyObject* args) {
	int objId;
	if (!PyArg_ParseTuple(args, "i", &objId))
		return Py_None;

	MetaObject* obj = actionary->searchByIdObj(objId);
	if (obj == NULL)
		return Py_None;
	char obj_name[50];
	if (obj->isAgent())
		sprintf_s(obj_name, "Agent");
	else if (obj->isRoom())
		sprintf_s(obj_name, "Room");
	else if (obj->isInstance())
		sprintf_s(obj_name, "Instance");
	else
		sprintf_s(obj_name, "Type");

	return Py_BuildValue("s", obj_name);

}
///////////////////////////////////////////////////////////////////////////////
//Returns an action name from the given action ID, or the empty string
//if there isn't one
/// \pararm args The arguments passed from the python function
/// \returns The action's name, or None otherwise
///////////////////////////////////////////////////////////////////////////////
extern "C" PyObject*
action_getName(PyObject*, PyObject* args){

	int actId;
	if (!PyArg_ParseTuple(args, "i", &actId))
		return Py_None;

	MetaAction* act = actionary->searchByIdAct(actId);

	if (act == NULL)
		return Py_None;

	return Py_BuildValue("s",act->getActionName().c_str());

}



//////////////////////////////////////////////////////////////////////////////
//Runs a python function from a given name. The function accepts an interger value
//and a void pointer argument that can be converted into other arguments
//This will not be pretty
extern "C" PyObject*
func_runPythonFunction(PyObject*, PyObject* args) {
	char* function_name;
	int agent_id;
	int arg_value;

	if (!PyArg_ParseTuple(args, "sii", &function_name, &agent_id, &arg_value)) {
		return Py_None;
	}
	PythonAgentFunc *callback = pytable.getFunctions(function_name);

	if (callback != NULL) {
		int ret_val = callback->func(agent_id, (void*)&arg_value);
		return PyLong_FromLong(ret_val);
	}
	return Py_None;
}

///////////////////////////////////////////////////////////////////////////////
///Gets the name of either the parent object or action.
/// \pararm args The arguments passed from the python function
/// \returns The parent's id, or None if there is an error
/////////////////////////////////////////////////////////////////////////////
extern "C" PyObject*
db_getParent(PyObject*, PyObject* args){

	int objId; /*! < The id of either the object or action*/
	char* type; /*! <The type. This should be either action or object*/
	if (!PyArg_ParseTuple(args, "is", &objId, &type)){
		return Py_None;
	}

	if (!strcmp(type, "object")){
		MetaObject* obj = actionary->searchByIdObj(objId);
		if (obj == NULL || obj->getParent() == NULL)
			return Py_None;
		return PyLong_FromLong(obj->getParent()->getID());
	}
	else if (!strcmp(type, "action")){
		MetaAction* act = actionary->searchByIdAct(objId);
		if (act == NULL || act->getParent() == NULL)
			return Py_None;
		return PyLong_FromLong(act->getParent()->getID());
	}
	return Py_None;
}

/*The methods must be defined prior to the start of the python interpreter
 and the module must be init and added to the built in modules before 
 spinning up the interpreter in order for the interpreter to find the
 module*/
static PyMethodDef prop_methods[] = {
	{"changeContents",object_changeContents,METH_VARARGS,NULL},
	{"checkCapability",agent_checkCapability, METH_VARARGS,NULL},
	{"checkObjectCapability",object_checkCapability,METH_VARARGS,NULL},
	{"contain", object_contain, METH_VARARGS,NULL},
	{"dist", prop_dist, METH_VARARGS,NULL},  //distance between two objects
	{"finishedAction",action_isCompleted,METH_VARARGS,NULL},
	{"getBoundingRadius", prop_getBoundingRadius, METH_VARARGS,NULL},
	{"getElapsedTime",prop_getElapsedTime,METH_VARARGS,NULL},
	{"getLocation", prop_getLocation, METH_VARARGS,NULL},//What's the location of obj1
	{"getObjectName",object_getName,METH_VARARGS,NULL},
	{"getObjectType",object_getType,METH_VARARGS,NULL},
	{"getContent", object_getContents, METH_VARARGS,NULL},
	{"getActionName",action_getName,METH_VARARGS,NULL},
	{"getParent", db_getParent, METH_VARARGS,NULL},
	{"getPosition", prop_getVector, METH_VARARGS,NULL},
	{"getProperty",prop_getProperty,METH_VARARGS,NULL},
	{"inFront", prop_inFront, METH_VARARGS,NULL},  //is obj1 in front of obj2
	{"isSet",object_isSet,METH_VARARGS,NULL},
	{"isType",object_isType,METH_VARARGS,NULL},
	{"isActionType",action_isType,METH_VARARGS,NULL},
	{ "numContent", object_getNumContents, METH_VARARGS,NULL},
	{"par_debug",debug_parDebug,  METH_VARARGS,NULL},
	{"setFailure",action_setFailure,METH_VARARGS,NULL},
	{"setPosition", prop_setVector, METH_VARARGS,NULL},
	{"setProperty",prop_setProperty,METH_VARARGS,NULL},
	{"testAppCond", prop_testAppCond, METH_VARARGS,NULL},
	{"testCulCond", prop_testCulCond, METH_VARARGS,NULL},
	{"testPreSpec", prop_testPreSpec, METH_VARARGS,NULL},
	{"runFunction", func_runPythonFunction,METH_VARARGS,NULL},
	{NULL, NULL}
};

//This is from http://python3porting.com/cextensions.html
//to replace Py_InitModule
#if PY_MAJOR_VERSION >= 3
static struct PyModuleDef moduledef = {
	PyModuleDef_HEAD_INIT,
	"PAR",     // m_name 
	"Defines function interop between python and c for par",  // m_doc 
	-1,                  // m_size 
	prop_methods,    //m_methods 
	NULL,                // m_reload 
	NULL,                // m_traverse 
	NULL,                // m_clear 
	NULL,                // m_free 
};
#endif
PyMODINIT_FUNC PyInit_PAR(void){
	PyObject* module;
	#if PY_MAJOR_VERSION >= 3
		 module = PyModule_Create(&moduledef);
		if (module == NULL) {
			PyErr_Print();
			PyErr_Clear();
		}
	#else
	Py_InitModule3("PAR",
		module = prop_methods, "Defines function interop between python and c for par");
	#endif
	return module;
}

extern "C" void
initprop() {
	if (PyImport_AppendInittab("PAR", PyInit_PAR) == -1) {
		par_debug("Error: could not extend in-built modules table\n");
		return;
	}

   Py_Initialize();
   PyRun_SimpleString("from PAR import *\n");
   //PyRun_SimpleString(std_catcher);
   PyRun_SimpleString("SEQUENCE = 0\n");
   PyRun_SimpleString("SELECTOR = 1\n");
   PyRun_SimpleString("PARJOIN = 2\n");
   PyRun_SimpleString("PARINDY = 3\n");
   PyRun_SimpleString("WHILE = 4\n");
   PyRun_SimpleString("GATHER = 5\n");
   PyRun_SimpleString("INCOMPLETE = 0\n");
   PyRun_SimpleString("SUCCESS = 1\n");
   PyRun_SimpleString("FAILURE = 2\n");
   PyRun_SimpleString("agents = 10\n");
   PyRun_SimpleString("objects = 10\n");
   PyRun_SimpleString("par_debug('Finished Initalizing Python')\n");
   
}
