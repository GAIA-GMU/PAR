#wash.v.02
#Grooming
#In this frame, an Agent engages in  personal body care.  An
# Instrument can be used in this process as well as a Medium
#. 

def applicability_condition(self,agent,Instrument=-1,Place=-1):
	if not checkCapability(agent,self.id):
		return FAILURE
	return SUCCESS
def preparatory_spec(self,agent,Instrument=-1,Place=-1):
	prep_steps=[]
	#First, we need to get the instrument if one is set
	if isSet(Instrument):
		if not contain(agent,Instrument):
		       prep_steps.append(("Get",{'agents':agent,'objects':(Instrument),'caller':self.id}))
	#Then, we should make sure that we are in the correct place
	if isSet(Place):
		radius = getBoundingRadius(Place);
		distance = dist(agent, Place);
	if(distance > radius):
		prep_steps.append(("Walk",{'agents':agent,'objects':(Place),'caller':self.id}))

	if prep_steps > 0:
		actions={}
		if prep_steps == 1:
			#If this occurs, then we need to send a primitive
			actions['PRIMITIVE']=prep_steps[0]
		else:
			#Complex action
			actions['COMPLEX']=tuple([SEQUENCE]+prep_steps)
		return actions
	return SUCCESS

def execution_steps(self,agent,Instrument=-1,Place=-1):
	return {'PRIMITIVE':('wash',{'agents':agent,'objects':(Agent,Instrument,Place)})}

def culmination_condition(self,agent,Instrument=-1,Place=-1):
	if finishedAction(self.id):
		return SUCCESS
	return INCOMPLETE

