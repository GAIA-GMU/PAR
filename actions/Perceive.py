#gaze.v.01
#Perception_active
#This frame contains perception words whose perceivers inten
#tionally direct their attention to some entity or phenomeno
#n in order to have a perceptual experience.  For this reaso
#n we call the perceiver role in this frame Perceiver_agenti
#ve.    'She gazed upon him fondly.'  Comparing the Percepti
#on_active frame to the Perception_experience frame, we note
# that for some modalities there are different lexical items
# in each frame.  For instance, whereas Perception_active co
#ntains the verb phrase look at, Perception_experience conta
#ins see.   For other sense modalities, we find the same lex
#ical item in both frames.  To illustrate, consider the verb
# smell.  This first sentence exemplifies the Perception_act
#ive use of the verb smell:'Smell this to see if it's fresh.
#CNI' This second sentence exemplifies its Perception_experi
#ence sense: 'I smell something rotten.'  

def applicability_condition(self,agent,Phenomenon,Place=-1):
        if not checkCapability(agent,self.id):
                return FAILURE
	if not isSet(Phenomenon) or not checkObjectCapability(Phenomenon,self.id,0):
		return FAILURE
	return SUCCESS

def preparatory_spec(self,agent,Phenomenon,Place=-1):
	return SUCCESS

def execution_steps(self,agent,Phenomenon,Place=-1):
	return {'PRIMITIVE':(self.name,{'agents':agent,'objects':(Phenomenon,Place)})}

def culmination_condition(self,agent,Phenomenon,Place=-1):
	if finishedAction(self.id):
		return SUCCESS
	return INCOMPLETE

