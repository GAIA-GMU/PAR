def applicability_condition(self, agent):
    return 1

def culmination_condition(self, agent):
    if finishedAction(self.id):
        return 1 #Returns a Success
    else:
        return 0

def execution_steps(self, agent):
    actions = {'PRIMITIVE':("Success",{'agents':agent})}
    return actions

def preparatory_spec(self,agent):
    return 1
