#shake.v.01
#Cause_to_move_in_place
#An Agent causes a Theme to move with respect to a certain F
#ixed_location, generally with a certain Periodicity, withou
#t undergoing unbounded translational motion or significant 
#alteration of configuration/shape. 'Paul shook the remote c
#ontrol frantically.'

def applicability_condition(self,agent,Theme,Instrument=-1,Place=-1):
        if not checkCapability(agent,self.id):
                return FAILURE
	if not isSet(Theme) or not checkObjectCapability(Theme,self.id,0):
		return FAILURE
	return SUCCESS

def preparatory_spec(self,agent,Theme,Instrument=-1,Place=-1):
	return SUCCESS

def execution_steps(self,agent,Theme,Instrument=-1,Place=-1):
	return {'PRIMITIVE':('shake',{'agents':agent,'objects':(Theme,Instrument,Place)})}

def culmination_condition(self,agent,Theme,Instrument=-1,Place=-1):
	if finishedAction(self.id):
		return SUCCESS
	return INCOMPLETE

