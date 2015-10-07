def applicability_condition(self,agent,Instrument=-1):
	return SUCCESS
def preparatory_spec(self,agent,Instrument=-1):
	return SUCCESS

def execution_steps(self,agent,Instrument=-1):
	return {'PRIMITIVE':('Wash',{'agents':agent,'objects':(Instrument)})}

def culmination_condition(self,agent,Instrument=-1):
	if finishedAction(self.id):
		return SUCCESS
	return INCOMPLETE

