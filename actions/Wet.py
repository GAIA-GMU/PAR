#wet.v.01
#Cause_to_be_wet
#An Agent causes an Undergoer to be wet with a Liquid.  In t
#his frame, the Liquid can be a liquid or gas.  Undergoers a
#re porous objects, but not hollow (like containers).  'Jo s
#oaked the dog with water.' 'Ellen wet  the sponge with wate
#r.'

def applicability_condition(self,agent,Patient,Instrument=-1,Place=-1):
	if not checkCapability(agent,self.id):
		return FAILURE
	if not isSet(Patient) or not checkObjectCapability(Patient,self.id,0):
		return FAILURE
	return SUCCESS

def preparatory_spec(self,agent,Patient,Instrument=-1,Place=-1):
	prep_steps=[]
	#First, we need to get the instrument if one is set
	if isSet(Instrument):
		if not contain(agent,Instrument):
		       prep_steps.append(("Get",{'agents':agent,'objects':(Instrument),'caller':self.id}))
	if isSet(Place):
		radius = getBoundingRadius(Place);
		distance = dist(agent, Place);
		if(distance > radius):
		       prep_steps.append(("Walk",{'agents':agent,'objects':(Place),'caller':self.id}))     #Otherwise, we should go to the object
	else:
		radius = getBoundingRadius(Patient);
		distance = dist(agent, Patient);
		if(distance > radius):
		       prep_steps.append(("Walk",{'agents':agent,'objects':(Patient,-1,getLocation(Patient)),'caller':self.id}))

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

def execution_steps(self,agent,Patient,Instrument=-1,Place=-1):
	return {'PRIMITIVE':('wet',{'agents':agent,'objects':(Patient,Instrument,Place)})}

def culmination_condition(self,agent,Patient,Instrument=-1,Place=-1):
	if finishedAction(self.id):
		return SUCCESS
	return INCOMPLETE

