#flip.v.06
#Cause_motion
#An Agent causes a Theme to undergo directed motion.  The mo
#tion may be described with respect to a Source, Path and/or
# Goal. In contrast with Placing, the Goal of motion is not 
#emphasized.  For many of these words (cast.v, throw.v, chuc
#k.v, etc.), the Agent has control of the Theme only at the 
#Source of motion, and does not experience overall motion.  
# For others (e.g. drag.v, push.v, shove.v, etc.) the Agent 
#has control of the Theme throughout the motion; for these w
#ords, the Theme is resistant to motion due to some friction
# with the surface along which they move.  (They thus differ
# from the words of the Carrying frame in that they are supp
#orted by this surface, rather than a Carrier.)  This frame 
#contrasts with the following frames which talk about an Age
#nt changing a Theme's position with respect to a landmark (
#either Source or Goal):  ''In Placing, the figure (Theme) i
#s profiled as the object, and ends up on the ground (Goal).
#   'Joyce placed the flowers onto the bed.'  In Filling, th
#e ground (Goal) is profiled as the object, and the figure (
#Theme) ends up on the ground (Goal).   'John filled the box
# with old toys.'  In Removing, the figure (Theme) is profil
#ed, and is removed from the ground (Source).   'Jennifer re
#moved the flowers from the bed.'  In Emptying, the ground (
#Source) is profiled and the figure (Theme) is removed from 
#it.   'Jason emptied the box of the old toys .'

def applicability_condition(self,agent,AreaSourceGoal,Theme,Place=-1,Instrument=-1):
        if not isSet(AreaSourceGoal) or not checkObjectCapability(AreaSourceGoal,self.id,0):
		return FAILURE
	if not isSet(Theme) or not checkObjectCapability(Theme,self.id,1):
		return FAILURE
	return SUCCESS


def preparatory_spec(self,agent,AreaSourceGoal,Theme,Place=-1,Instrument=-1):
	#We need to be in the range of the theme to flip it
        prep_steps=[]
        #First, we need to get the instrument if one is set
        if isSet(Instrument):
                if not contain(agent,Instrument):
                       prep_steps.append(("Get",{'agents':agent,'objects':(Instrument)}))
        #Then, we should make sure that we are in range of the theme
        radius = getBoundingRadius(Theme);
        distance = dist(agent, Theme);
        if(distance > radius):
                prep_steps.append(("Walk",{'agents':agent,'objects':(Theme,-1,Place)}))

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


def execution_steps(self,agent,AreaSourceGoal,Theme,Place=-1,Instrument=-1):
	return {'PRIMITIVE':('Flip',{'agents':agent,'objects':(AreaSourceGoal,Theme,Place,Instrument)})}

def culmination_condition(self,agent,AreaSourceGoal,Theme,Place=-1,Instrument=-1):
	if finishedAction(self.id):
		return SUCCESS
	return INCOMPLETE

