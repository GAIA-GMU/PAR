def applicability_condition(self,agent,Area=-1):
	return SUCCESS
def preparatory_spec(self,agent,Area=-1):
	return SUCCESS

def execution_steps(self,agent,Area=-1):
	return {'PRIMITIVE':('Nod',{'agents':agent,'objects':(Area)})}

def culmination_condition(self,agent,Area=-1):
	if finishedAction(self.id):
		return SUCCESS
	return INCOMPLETE

