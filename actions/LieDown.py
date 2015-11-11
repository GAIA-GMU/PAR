#lie_down.v.01
#Change_posture
#A Protagonist changes the overall position and posture of t
#he body.

def applicability_condition(self,agent,Source=-1):
        if not checkCapability(agent,self.id):
                return FAILURE
	return SUCCESS

def preparatory_spec(self,agent,Source=-1):
	return SUCCESS

def execution_steps(self,agent,Source=-1):
        parent=GetParent(self.id,"action")
        if parent is not None:
                parent_name=GetActionName(parent)
                if parent_name == "LieDown": 
                        return {'PRIMITIVE':('LieDown',{'agents':agent,'objects':(Source)})}
                elif parent_name == "Stand":
                        return {'PRIMITIVE':('Stand',{'agents':agent,'objects':(Source)})}
        return {'PRIMITIVE':('Sit',{'agents':agent,'objects':(Source)})}

def culmination_condition(self,agent,Source=-1):
        parent=GetParent(self.id,"action")
        if parent is not None:
                parent_name=GetActionName(parent)
        else:
                parent_name = ""
	if finishedAction(self.id):
                if parent_name == "LieDown":
                        setProperty(agent,"Posture","Prone")
                elif parent_name == "Stand":
                        setProperty(agent,"Posture","Standing")
                else:
                        setProperty(agent,"Posture","Sitting")
		return SUCCESS
	return INCOMPLETE

