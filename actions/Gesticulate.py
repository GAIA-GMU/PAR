#jiggle.v.01
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
	prep_steps=[]
	if isSet(Place):
		radius = getBoundingRadius(Place);
		distance = dist(agent, Place);
		if(distance > radius):
			prep_steps.append(("Walk",{'agents':agent,'objects':(Place),'caller':self.id}))     #Otherwise, we should go to the object
			#If this occurs, then we need to send a primitive
			actions['PRIMITIVE']=prep_steps[0]
			return actions

	return SUCCESS

def execution_steps(self,agent,Addressee=-1,Place=-1):
	setProperty(agent,"obj_status","OPERATING"); 
	if isActionType(self.id,"Jiggle"):
		return {'PRIMITIVE':('Jiggle',{'agents':agent,'objects':(Addressee,Place)})}
	else:
		return {'PRIMITIVE':('Nod',{'agents':agent,'objects':(Addressee,Place)})}

def culmination_condition(self,agent,Addressee=-1,Place=-1):
	if self.duration != -1:
		if self.start_time+self.duration < getElapsedTime():
			setProperty(agent,"obj_status","IDLE");
			return SUCCESS

	else:
		if finishedAction(self.id):
			return SUCCESS
	return INCOMPLETE

