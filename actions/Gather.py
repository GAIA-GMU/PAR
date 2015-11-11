#gather.v.01
#Gathering_up
#This frame describes an Agent's gathering of Individuals (p
#eople or entities) into a group, the  Aggregate, defined by
# relative proximity.  'The police gathered the suspects int
#o a line-up.' Compare this frame with Congregating, and con
#trast it with Amalgamating.

def applicability_condition(self,agent,Individuals,Source=-1,Instrument=-1,Place=-1):
        if not checkCapability(agent,self.id):
                return FAILURE
        if not isSet(Individuals) or not checkObjectCapability(Individuals,self.id,0):
		return FAILURE
	return SUCCESS

def preparatory_spec(self,agent,Individuals,Source=-1,Instrument=-1,Place=-1):
	return SUCCESS

def execution_steps(self,agent,Individuals,Source=-1,Instrument=-1,Place=-1):
	return {'PRIMITIVE':('gather',{'agents':agent,'objects':(Individuals,Source,Instrument,Place)})}

def culmination_condition(self,agent,Individuals,Source=-1,Instrument=-1,Place=-1):
	if finishedAction(self.id):
		return SUCCESS
	return INCOMPLETE

