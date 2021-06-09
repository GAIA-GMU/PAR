

#ifndef _INTERPY_H
#define _INTERPY_H
#define PY_SSIZE_T_CLEAN
#include "Python.h"

class MetaObject;
class MetaAction;
class iPAR;

extern "C" {
extern int createPyActions(MetaAction* obj);
extern int createPyiPARs(iPAR *ipar);/*Creates a full condition-assertion concatination from all the parents if we do not have one attached to the action*/
extern int createPyCondAsserts(MetaAction* act);
/* run Python script from string or file without a return value */
extern int runPySimple(char* str, bool file = false);
/* run Python script in the __main__ module */
extern PyObject* runPy(const char* str);
extern bool fromPyObjectToBool(PyObject* pobj);
extern int fromPyObjectToInt(PyObject* pobj);
extern void  initprop();
extern int addPyiPARValues(iPAR *ipar);
}
#endif

