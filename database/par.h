

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
  static FILE *file_name;
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
#ifndef FILE_NAME
	#define FILE_NAME PAR::file_name
#endif

#define par_debug(fmt,...)  do{if(PAR::dbg){ if(FILE_NAME != NULL) fprintf(FILE_NAME,fmt, __VA_ARGS__);}} while(0) 

#define onError(msg) {printf("%s",msg); return 0;}

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
