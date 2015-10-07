def applicability_condition(self,agent,Expected_entity=-1):
	return SUCCESS
def preparatory_spec(self,agent,Expected_entity=-1):
	return SUCCESS

def execution_steps(self,agent,Expected_entity=-1):
	return {'PRIMITIVE':('Gaze',{'agents':agent,'objects':(Expected_entity)})}

def culmination_condition(self,agent,Expected_entity=-1):
	if finishedAction(self.id):
		return SUCCESS
	return INCOMPLETE

