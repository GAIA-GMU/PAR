def applicability_condition(self,agent,Indicated_entity):
	if not isSet(Indicated_entity) or not checkObjectCapability(Indicated_entity,self.id,0):
		return FAILURE
	return SUCCESS
def preparatory_spec(self,agent,Indicated_entity):
	return SUCCESS

def execution_steps(self,agent,Indicated_entity):
	return {'PRIMITIVE':('Gesticulate',{'agents':agent,'objects':(Indicated_entity)})}

def culmination_condition(self,agent,Indicated_entity):
	if finishedAction(self.id):
		return SUCCESS
	return INCOMPLETE

