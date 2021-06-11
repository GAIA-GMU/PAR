#talk.v.02
#Chatting
#A group of people (the Interlocutors or Interlocutor_1 and 
#Interlocutor_2 together) have a conversation.  No person is
# construed as only a speaker or only an addressee. Rather, 
#it is understood that both (or all) participants do some sp
#eaking and some listening--the process is understood to be 
#symmetrical or reciprocal.  In this frame, the purpose of t
#he conversation is generally social, rather than specifical
#ly to decide something or exchange information, or to quarr
#el (see Discussion and Quarreling).  'If you ain't got noth
#in' better to do, I 'd like to shoot the breeze with you fo
#r a couple.' 'We used to chat about everything.'

def applicability_condition(self,agent,Interlocutors,Place=-1):
	if not checkCapability(agent,self.id):
		return FAILURE
	if not isSet(Interlocutors) or not checkObjectCapability(Interlocutors,self.id,0):
		return FAILURE
	return SUCCESS

def preparatory_spec(self,agent,Interlocutors,Place=-1):
	return SUCCESS

def execution_steps(self,agent,Interlocutors,Place=-1):
	return {'PRIMITIVE':('talk',{'agents':agent,'objects':(Interlocutors,Place)})}

def culmination_condition(self,agent,Interlocutors,Place=-1):
	if finishedAction(self.id):
		return SUCCESS
	return INCOMPLETE

