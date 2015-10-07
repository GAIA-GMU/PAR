def applicability_condition(self,agent,Created_entity,Instrument=-1):
	if not isSet(Created_entity) or not checkObjectCapability(Created_entity,self.id,0):
		return FAILURE
	return SUCCESS
def preparatory_spec(self,agent,Created_entity,Instrument=-1):
	return SUCCESS

def execution_steps(self,agent,Created_entity,Instrument=-1):
	return {'PRIMITIVE':('Make',{'agents':agent,'objects':(Created_entity,Instrument)})}

def culmination_condition(self,agent,Created_entity,Instrument=-1):
	if finishedAction(self.id):
		return SUCCESS
	return INCOMPLETE

