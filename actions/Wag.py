#wag.v.01
#Body_movement
#This frame contains words for motions or actions an Agent p
#erforms using some part of his/her body.   A number of word
#s in this frame occur as blends with Communication, in whic
#h the action has an Addressee.  For example,   'Pat nodded 
#at Kim.'  These examples differ from Communication.Gesture 
#in that no specific message need be expressed, as in  'She 
#nodded to him to sit down.'  Since this frame involves a pa
#rticular type of motion, it contains the frame elements Sou
#rce, Path, Goal and Area, which originate in the motion fra
#me.  All of these frame elements are generally expressed in
# PP Complements.  'The boy swung his legs from under the ta
#ble.'

def applicability_condition(self,agent,Addressee=-1,Place=-1):
	if not checkCapability(agent,self.id):
		return FAILURE
	return SUCCESS

def preparatory_spec(self,agent,Addressee=-1,Place=-1):
	return SUCCESS

def execution_steps(self,agent,Addressee=-1,Place=-1):
	return {'PRIMITIVE':('jiggle',{'agents':agent,'objects':(Addressee,Place)})}

def culmination_condition(self,agent,Addressee=-1,Place=-1):
	if finishedAction(self.id):
		return SUCCESS
	return INCOMPLETE

