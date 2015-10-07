def applicability_condition(self,agent,Theme,Area=-1):
	if not isSet(Theme) or not checkObjectCapability(Theme,self.id,0):
		return FAILURE
	return SUCCESS
def preparatory_spec(self,agent,Theme,Area=-1):
	return SUCCESS

def execution_steps(self,agent,Theme,Area=-1):
	return {'PRIMITIVE':('Put',{'agents':agent,'objects':(Theme,Area)})}

def culmination_condition(self,agent,Theme,Area=-1):
	if finishedAction(self.id):
		return SUCCESS
	return INCOMPLETE

