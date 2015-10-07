def applicability_condition(self,agent):
	return SUCCESS

def preparatory_spec(self,agent):
	return SUCCESS

def execution_steps(self,agent):
	return {'PRIMITIVE':('Inform',{'agents':agent,'objects':None})}

def culmination_condition(self,agent):
	if finishedAction(self.id):
		return SUCCESS
	return INCOMPLETE

