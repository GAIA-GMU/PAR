def applicability_condition(self, agent):
    return 1

def culmination_condition(self, agent):
    if finishedAction(self.id):
        return SUCCESS #Returns a success
    else:
        return 0

def execution_steps(self, agent):
    actions = {'COMPLEX':(SEQUENCE,('Fail',{'agents':agent}),
                          ('Fail',{'agents':agent}),
                          ('Success',{'agents':agent}))}
    return actions

def preparatory_spec(self,agent):
    return 1
