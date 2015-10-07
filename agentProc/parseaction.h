#ifndef _PARSE_ACTION_H
#define _PARSE_ACTION_H

#include "parnet.h"

class ActionNet;


class Parse
{

  enum OPTYPE {SEQUENCE,SELECT,PARJOIN, PARINDY, WHILE};
  
  public:
  static int parseSeq(ComplexObj *cobj, ActionNet *actionNet, int parent, int failExitNode, int *startSet, int *endSet);
  static int parseSel(ComplexObj* cobj, ActionNet *actionNet, int parent, int failExitNode, int *startSet, int *endSet);
  static int parsePar(ComplexObj *cobj, ActionNet *actionNet,  int parent, int failExitNode, int *startSet, int *endSet, int type);
  static int parseComplex(ComplexObj *cobj, ActionNet *actionNet, int parent, int failExitNode, int *startSet, int *endSet,bool fail_parnet=true);
  static int parseDict(ComplexObj *cobj, ActionNet *actionNet, int parent, int failExitNode);

};

#endif
