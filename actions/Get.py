def applicability_condition(self,agent,Theme):
	if not isSet(Theme) or not checkObjectCapability(Theme,self.id,0):
		return FAILURE
	return SUCCESS
def preparatory_spec(self,agent,Theme):
	return SUCCESS

def execution_steps(self,agent,Theme):
	return {'PRIMITIVE':('Get',{'agents':agent,'objects':(Theme)})}

def culmination_condition(self,agent,Theme):
	if finishedAction(self.id):
		return SUCCESS
	return INCOMPLETE

