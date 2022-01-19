#include <cstdio>
#include <string.h>
#include "utility.h"
#include "time.h"
#include "windows.h"

extern "C" void
err(const char* str) {
   printf("%s\n", str);
}

parTime::parTime()
{
	this->simStartTime = timeGetTime() / 1000;
}

//how long after midnight is the simulation starting?
void 
parTime::setTimeOffset(int seconds)
{
	this->simTimeOffset = seconds;
}

void 
parTime::setTimeOffset(int hours, int min, int sec)
{
	this->simTimeOffset = sec + min * 60 + hours * 3600;
}

//how much faster (or slower) than real-time the sim should be	
void 
parTime::setTimeRate(float rate)
{
	int oldTime = this->getCurrentTime();
	this->simTimeRate = rate;  
	int newTime = this->getCurrentTime();
	this->simTimeOffset = this->simTimeOffset - (newTime - oldTime);
}

int 
parTime::getTimeOffset()
{
	return this->simTimeOffset;
}

float 
parTime::getTimeRate()
{
	return this->simTimeRate;
}

//not system time, includes rate and offset
int 
parTime::getCurrentTime()
{
	int time = (int)(timeGetTime() / 1000.0);
	//printf("time = %d, start = %d, offset = %d, rate = %f\n", time, simStartTime, simTimeOffset, simTimeRate);
	return (int)((time - this->simStartTime) * this->simTimeRate + this->simTimeOffset);
}

// return the difference between the current time and the parameter
int 
parTime::getElapseTime(int startTime)
{
	return this->getCurrentTime() - startTime;
}

//TB ADDED 3-25
PythonAgentFunc*
PythonTable::getFunctions(const char *name) {
	if (name == NULL)
		return NULL;
	PythonAgentFunc *actf = NULL;
	std::list<PythonAgentFunc*>::iterator iter = functions.begin();
	while (strcmp((*iter)->name, name) != 0 && iter != functions.end())iter++;

	if (iter != functions.end())
		return (*iter);
	else
		return NULL;
}
void
PythonTable::addFunctions(char* name, int(*func)(int, void*)) {

	//debug("In ActionTable::addFunctions adding %s\n", name);

	PythonAgentFunc *actf = new PythonAgentFunc;
	actf->func = func;
	actf->name = _strdup(name);
	functions.push_back(actf);
}