def applicability_condition(self, agent):
    if checkCapability(agent, self.id):
        return SUCCESS
    else:
        return FAILURE

def preparatory_spec(self, agent):
    return SUCCESS

def execution_steps(self, agent):
    setProperty(agent,"obj_status","OPERATING");
    actions = {'PRIMITIVE':("Speak",{'agents':agent})};
    return actions


def culmination_condition(self, agent):
    #par_debug(" ".join([str(self.start_time),str(self.duration),str(getElapsedTime())]))
    if self.start_time+self.duration < getElapsedTime():
        setProperty(agent,"obj_status","IDLE");
        return SUCCESS

    return INCOMPLETE
