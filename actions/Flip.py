def applicability_condition(self,agent,Area,Theme,Instrument=-1):
	if not isSet(Area) or not checkObjectCapability(Area,self.id,0):
		return FAILURE
	if not isSet(Theme) or not checkObjectCapability(Theme,self.id,1):
		return FAILURE
	return SUCCESS
def preparatory_spec(self,agent,Area,Theme,Instrument=-1):
	return SUCCESS

def execution_steps(self,agent,Area,Theme,Instrument=-1):
	return {'PRIMITIVE':('Flip',{'agents':agent,'objects':(Area,Theme,Instrument)})}

def culmination_condition(self,agent,Area,Theme,Instrument=-1):
	if finishedAction(self.id):
		return SUCCESS
	return INCOMPLETE

