def applicability_condition(self,agent,Container=-1,Heating_Instrument=-1):
	return SUCCESS
def preparatory_spec(self,agent,Container=-1,Heating_Instrument=-1):
	return SUCCESS

def execution_steps(self,agent,Container=-1,Heating_Instrument=-1):
	return {'PRIMITIVE':('Cook',{'agents':agent,'objects':(Container,Heating_Instrument)})}

def culmination_condition(self,agent,Container=-1,Heating_Instrument=-1):
	if finishedAction(self.id):
		return SUCCESS
	return INCOMPLETE

