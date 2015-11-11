#put.v.01
#Placing
#Generally without overall (translational) motion,  an Agent
# places a Theme at a location, the Goal, which is profiled.
#  In this frame, the Theme is under the control of the Agen
#t/Cause at the time of its arrival at the Goal.  'David pla
#ced his briefcase on the floor.'  This frame differs from F
#illing in that it focuses on the Theme rather than the effe
#ct on the Goal entity.  It differs from Removing in focusin
#g on the Goal rather than the Source of motion for the Them
#e.

def applicability_condition(self,agent,Theme,Area=-1,Source=-1,Place=-1):
        if not checkCapability(agent,self.id):
                return FAILURE
	if not isSet(Theme) or not checkObjectCapability(Theme,self.id,0):
		return FAILURE
	return SUCCESS

def preparatory_spec(self,agent,Theme,Area=-1,Source=-1,Place=-1):
	return SUCCESS

def execution_steps(self,agent,Theme,Area=-1,Source=-1,Place=-1):
	return {'PRIMITIVE':('put',{'agents':agent,'objects':(Agent,Theme,Area,Source,Place)})}

def culmination_condition(self,agent,Theme,Area=-1,Source=-1,Place=-1):
	if finishedAction(self.id):
		return SUCCESS
	return INCOMPLETE

