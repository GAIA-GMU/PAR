#sit_down.v.01
#Posture
#An Agent supports their body in a particular Location.  The
# LUs of the frame convey which body part is the Point_of_co
#ntact where the Agent is supported, what orientation the bo
#dy is in, and some overall arrangement of the limbs (especi
#ally the legs) and the torso.  'He knelt down Location , ha
#nd on heart .'

def applicability_condition(self,agent,Location):
    if not checkCapability(agent,self.id):
        return FAILURE
    if not isSet(Location) or not checkObjectCapability(Location,self.id,0):
	return FAILURE
    return SUCCESS

def preparatory_spec(self,agent,Location):
	return SUCCESS

def execution_steps(self,agent,Location):
	return {'PRIMITIVE':('sit_down',{'agents':agent,'objects':(Location)})}

def culmination_condition(self,agent,Location):
	if finishedAction(self.id):
		return SUCCESS
	return INCOMPLETE

