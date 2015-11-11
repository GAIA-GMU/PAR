#Self_motion
#The Self_mover, a living being, moves under its own power i
#n a directed fashion, i.e. along what could be described as
# a Path, with no separate vehicle.

#Note:Area Source Goal is the comibination of Area, Source, and Goal
#It is called this because Area,Source,and Goal are all exclusionary actions
def applicability_condition(self,agent,AreaSourceGoal,Cotheme=-1,Place=-1):
        if not checkCapability(agent,self.id):
                return FAILURE
	if not isSet(AreaSourceGoal) or not checkObjectCapability(AreaSourceGoal,self.id,0):
		return FAILURE
	return SUCCESS

def preparatory_spec(self,agent,AreaSourceGoal,Cotheme=-1,Place=-1):
        #posture=getProperty(agent,"Posture")
        #if posture is not None and posture != "Stand":
	return SUCCESS

def execution_steps(self,agent,AreaSourceGoal,Cotheme=-1,Place=-1):
        setProperty(agent,"Status","OPERATING")
        return {'PRIMITIVE':(self.name,{'agents':agent,'objects':(AreaSourceGoal,Cotheme,Place)})}

def culmination_condition(self,agent,AreaSourceGoal,Cotheme=-1,Place=-1):
	radius = getBoundingRadius(AreaSourceGoal);
        distance = dist(agent, AreaSourceGoal);
        if distance < radius:
                setProperty(agent,"Status","IDLE")
		return SUCCESS
	return INCOMPLETE

