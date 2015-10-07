

#ifndef _PAR_
#define _PAR_

#ifndef __STDIO_H__
#include <stdio.h>
#endif

#ifndef __STDLIB_H__
#include <stdlib.h>
#endif

#ifndef __STRING_H__
#include <string.h>
#endif

#include "workingmemory.h"
#include <exception>

#define INCOMPLETE 0
#define SUCCESS 1
#define FAILURE 2


class PAR
{
//#define debug
public:	
  static int dbg;  // debug

};


// Failure data
class FailData 
{
public:
  int failcode; // the failure code represented as a string.
                   // This should be one of the entries in the failure.flags 
                   // file

  iPAR *complexIPAR; // pointer to the complex ipar that failed
  iPAR *ipar; // pointer to the primitive ipar that failed
  void *data; // any data that can used in the failure recovery process
 

   FailData() :complexIPAR(0), ipar(0) {}
};

#define par_debug if(PAR::dbg) printf
#define onError(msg) {printf("%s\n",msg); return 0;}

extern "C" char *expandFileName(char *filename);

#define MAX_FAILBUF 128
class iPARException:public std::exception{
private:
	std::string msg;
public:
	 iPARException(std::string str){msg=str;}
	const  char* printMsg(){return msg.c_str();}
};

#endif
