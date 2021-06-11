#inform.v.01
#Telling
#A Speaker addresses an Addressee with a Message, which may 
#be indirectly referred to as a Topic.

def applicability_condition(self,agent,Addressee,Place=-1):
    if not checkCapability(agent,self.id):
        return FAILURE
    if not isSet(Addressee) or not checkObjectCapability(Addressee,self.id,0):
        return FAILURE
    return SUCCESS

def preparatory_spec(self,agent,Addressee,Place=-1):
    prep_steps=[]
    if isSet(Place):
        radius = getBoundingRadius(Place);
        distance = dist(agent, Place);
        if(distance > radius):
            prep_steps.append(("Walk",{'agents':agent,'objects':(Place),'caller':self.id}))
    actions={}
    if len(prep_steps) == 1:
        #If this occurs, then we need to send a primitive
        actions['PRIMITIVE']=prep_steps[0]
        return actions
    return SUCCESS

def execution_steps(self,agent,Addressee,Place=-1):
    return {'PRIMITIVE':('inform',{'agents':agent,'objects':(Addressee,Place)})}

def culmination_condition(self,agent,Addressee,Place=-1):
    if finishedAction(self.id):
        return SUCCESS
    return INCOMPLETE

