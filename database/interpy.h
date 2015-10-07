

#ifndef _INTERPY_H
#define _INTERPY_H
#include "Python.h"

class MetaObject;
class MetaAction;
class iPAR;

extern "C" {
extern int createPyActions(MetaAction* obj);
extern int createPyiPARs(iPAR *ipar);
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

