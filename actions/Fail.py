def applicability_condition(self, agent):
    return 1

def culmination_condition(self, agent):
    if finishedAction(self.id):
        return 2 #Returns a failure
    else:
        return 0

def execution_steps(self, agent):
    actions = {'PRIMITIVE':("Fail",{'agents':agent})}
    return actions

def preparatory_spec(self,agent):
    return 1
