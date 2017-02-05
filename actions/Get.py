#get.v.01
#Getting
#A Recipient starts off without the Theme in their possessio
#n, and then comes to possess it.  Although the Source from 
#which the Theme came is logically necessary, the Recipient 
#and its changing relationship to the Theme is profiled.  'I
# got two whistles from John.'

def applicability_condition(self,agent,Theme,Source=-1,Place=-1):
        if not checkCapability(agent,self.id):
                return FAILURE
	if not isSet(Theme) or not checkObjectCapability(Theme,self.id,0):
		return FAILURE
	return SUCCESS

def preparatory_spec(self,agent,Theme,Source=-1,Place=-1):
        #We need to be in the range of the theme to get it
        prep_steps=[]
        #First, we need to go to the place of the object
        if isSet(Place):
                radius = getBoundingRadius(Place);
                distance = dist(agent, Place);
                if(distance > radius):
                       prep_steps.append(("Walk",{'agents':agent,'objects':(Place),'caller':self.id}))
        #If we have a source, we should go to that for the object
        if isSet(Source):
                radius = getBoundingRadius(Source);
                distance = dist(agent, Source);
                if(distance > radius):
                       prep_steps.append(("Walk",{'agents':agent,'objects':(Source,-1,getLocation(Source)),'caller':self.id}))
        #Otherwise, we should go to the object
        else:
                radius = getBoundingRadius(Theme);
                distance = dist(agent, Theme);
                if(distance > radius):
                       prep_steps.append(("Walk",{'agents':agent,'objects':(Theme,-1,getLocation(Theme)),'caller':self.id}))

        if prep_steps > 0:
                actions={}
                if prep_steps == 1:
                        #If this occurs, then we need to send a primitive
                        actions['PRIMITIVE']=prep_steps[0]
                else:
                        #Complex action
                        actions['COMPLEX']=tuple(SEQUENCE,tuple(prep_steps))
                return actions
	return SUCCESS

def execution_steps(self,agent,Theme,Source=-1,Place=-1):
	return {'PRIMITIVE':('get',{'agents':agent,'objects':(Theme,Source,Place)})}

def culmination_condition(self,agent,Theme,Source=-1,Place=-1):
        if contain(agent,Theme):
                return SUCCESS #If the character already has it, then it shouldn't be getting it
	if finishedAction(self.id):
                changeContents(agent,Theme)#If the action finished, then the character should contain the new object (Theme)
		return SUCCESS
	return INCOMPLETE

