def applicability_condition(self,agent,Sought_entity=-1):
	return SUCCESS
def preparatory_spec(self,agent,Sought_entity=-1):
	return SUCCESS

def execution_steps(self,agent,Sought_entity=-1):
	return {'PRIMITIVE':('Perceive',{'agents':agent,'objects':(Sought_entity)})}

def culmination_condition(self,agent,Sought_entity=-1):
	if finishedAction(self.id):
		return SUCCESS
	return INCOMPLETE

