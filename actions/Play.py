#play.v.01
#Competition
#This frame is concerned with the idea that people (Particip
#ant_1, Participant_2, or Participants) participate in an or
#ganized rule governed activity (the Competition) in order t
#o achieve some advantageous outcome (often the Prize).  Ran
#k and Score are different criteria by which the degree of a
#chievement of the advantageous outcome is judged.   

def applicability_condition(self,agent,Place=-1):
	if not checkCapability(agent,self.id):
		return FAILURE
	return SUCCESS

def preparatory_spec(self,agent,Place=-1):
	return SUCCESS

def execution_steps(self,agent,Place=-1):
	return {'PRIMITIVE':('play',{'agents':agent,'objects':(Place)})}

def culmination_condition(self,agent,Place=-1):
	if finishedAction(self.id):
		return SUCCESS
	return INCOMPLETE

