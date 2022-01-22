DELETE from action_parent WHERE (SELECT act_id from action where act_name = 'Speak');
DELETE FROM action where act_name = "Speak";
INSERT INTO action (act_name,act_appl_cond,act_term_cond,act_prep_spec,act_exec_steps,act_obj_num) values ('Speak','../PAR/actions/Speak.py','../PAR/actions/Speak.py','../PAR/actions/Speak.py','../PAR/actions/Speak.py',-1);
INSERT INTO action_parent ((SELECT act_id from action where act_name = 'Speak'),8);