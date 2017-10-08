

#ifndef _UTILITY_H
#define _UTILITY_H

#include <list>
#include <vector>
#include <queue>
#include <algorithm>
#include <iterator>
#define SMALLBUF    64
#define MAXBUF		512
#define MAX_MAXBUF	6144

class MetaObject;
class MetaAction;


typedef char* Script;


template<int sz> struct Vector {
   float	v[sz];
};


struct Site {
   Vector<3>	postition;
   Vector<3>	orientation;
};

// standard relative directions
enum RELATIVE_DIR {
	           FRONT,
		       BACK,
		       LEFT,
		       RIGHT,
		       TOP,
		       BOTTOM,
		       NUM_RELATIVE_DIR
	       };


extern "C"
void  err(const char*);


class parTime {
public:
	int simTimeOffset;		//in second 0 = midnight, 9am = 9 * 3600 for when the simulation starts
	int simStartTime;		//what is the system time of when the simulation started.  (seconds)
	float simTimeRate;		//how much faster or slower should the simulation be from real-time

	 parTime();
	void  setTimeOffset(int seconds);	//how long after midnight is the simulation starting?
	void  setTimeOffset(int hours, int min, int sec); 
	void  setTimeRate(float);			//how much faster (or slower) than real-time the sim should be
	int  getTimeOffset();			
	float  getTimeRate();
	int  getCurrentTime();	//not system time, includes rate and offset
	int  getElapseTime(int startTime);	// return the difference between the current time and the parameter
};

#endif
