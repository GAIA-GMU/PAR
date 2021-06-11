#cook.v.01
#Cooking_creation
#This frame describes food and meal preparation.  A Cook cre
#ates a Produced_food from (raw) Ingredients.  The Heating_I
#nstrument and/or the Container may also be specified.  Cait
#lin baked some cookies from the pre-packaged dough. 

def applicability_condition(self,agent,Produced_food,Container=-1,Heating_Instrument=-1,Ingredients=-1,Place=-1):
	if not checkCapability(agent,self.id):
		return FAILURE
	if not isSet(Produced_food) or not checkObjectCapability(Produced_food,self.id,0):
		return FAILURE
	return SUCCESS

def preparatory_spec(self,agent,Produced_food,Container=-1,Heating_Instrument=-1,Ingredients=-1,Place=-1):
	return SUCCESS

def execution_steps(self,agent,Produced_food,Container=-1,Heating_Instrument=-1,Ingredients=-1,Place=-1):
	return {'PRIMITIVE':('cook',{'agents':agent,'objects':(Produced_food,Container,Heating_Instrument,Indredients,Place)})}

def culmination_condition(self,agent,Produced_food,Container=-1,Heating_Instrument=-1,Ingredients=-1,Place=-1):
	if finishedAction(self.id):
		return SUCCESS
	return INCOMPLETE

