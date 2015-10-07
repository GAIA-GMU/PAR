#ifndef _PAR_NET_H
#define _PAR_NET_H

#include "workingmemory.h"
#include "lwnet.h"
#include "partable.h"
#include "agentproc.h"

class ComplexObj
{
  // This is used to pass information about the identity of the top
  // level complex action to all the lower level primitive actions -
  // This is especially useful in the failure inform process
public:
  // ipar of the top level complex action
  iPAR *complexParent;

  // current primitive ipar
  iPAR *ipar;

  // current primitive ipar as a python object
  PyObject *pyobj;

  ComplexObj(void) 
   {complexParent=NULL;
   ipar=NULL;
   pyobj=NULL;};
  
  ComplexObj(iPAR *cpar, iPAR *ppar) 
  {ipar=cpar;
   pyobj=NULL;
   complexParent=ppar;};

  ComplexObj(PyObject *cparobj, iPAR *ppar) 
  {pyobj=cparobj;
   ipar=NULL;
   complexParent=ppar;}
};


class ParNet : public LWNet 
{
  // This is the Process Manager
private:
  enum nodes {\
	      START,\
	      APPLICABILITY,\
	      PREPARATORY,\
	      NEWPARNODE,\
	      NEWSUBPARNODE,\
		  NEWSUBPARCOMPLEX,\
	      COREEXEC,\
	      COMPLEX,\
	      PRIMITIVE,\
	      NEWACTNODE,\
	      EXITNODE, \
	      FAILEXITNODE,\
	      NUMNODES};

  PyObject *app_result; // holds the applicability condition's result
  bool 	   app_status; // was the applicability condition satisfied?
  bool      app_dict;  // did the applicability condition return a dictionary?

  PyObject *prep_result; // holds the preparatory specification's result
  bool 	   prep_status; // was the preparatory specification satisfied?
  bool      prep_dict;  //did the preparatory specification return a dictionary?
  bool     prep_complex; //did the preparatory specification return a complex tuple?

  ComplexObj *complexobj;
  bool      complex;
  actfunc  *callback; // callback functions to actually execute the core function
  iPAR		*ipar;
  AgentProc *ap; // agent process of the agent executing the action


  // patnet node functions
  void 		actfunc_start(void);

  void		actfunc_applicability(void);
  bool		condfunc_applicable(void);
  bool		condfunc_isapplicdictionary(void);

  LWNet *	newpar(void *args);

  void		actfunc_preparatory(void);
  bool		condfunc_ready(void);
  bool      condfunc_isprepdictionary(void);
  bool      condfunc_isprepcomplex(void);

  void		actfunc_coreexecution(void);
  bool		condfunc_iscomplex(void);

  void 		preactfunc_primitive(void);
  void		condfunc_culminate(void);

  void		postactfunc(void);

public:

  ParNet(ComplexObj *cobj);
  ParNet(PyObject *cobj); 
  ParNet(void);
							


};




#endif





