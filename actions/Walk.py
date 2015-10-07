def applicability_condition(self,agent,Area,Cotheme=-1):
	if not isSet(Area) or not checkObjectCapability(Area,self.id,0):
		return FAILURE
	return SUCCESS
def preparatory_spec(self,agent,Area,Cotheme=-1):
	return SUCCESS

def execution_steps(self,agent,Area,Cotheme=-1):
	return {'PRIMITIVE':('Walk',{'agents':agent,'objects':(Area,Cotheme)})}

def culmination_condition(self,agent,Area,Cotheme=-1):
	if finishedAction(self.id):
		return SUCCESS
	return INCOMPLETE

