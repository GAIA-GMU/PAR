#include <iostream>
#include <stdlib.h>
#include <assert.h>
#include "lwnet.h"

LWNetListNode* LWNetList::patlist = 0;

//class bindSim;
//extern bindSim *bsCall;  //must be set up in the calling program.  

//
// Load/Unload functions for Dynamic Shared Object
//
static struct lwnetmod {
  char	*modname;
} cplusmod;



void LWNetList::nlistconst(void)
{
	patlist = 0;

} 

char *cplusnet_description =
"C++ Patnets -- Version 1.2 ";

int LWNetList::always_advance(void *)
{
  int errorstatus=0;
  LWNetList::advance(&errorstatus);
  //jcscene->setTimestamp();  JAN
  std::cout << "In LWNetList::always_advance" << std::endl;
  return 1;
}

extern "C" int
cplusnet_unload() {
//
  //cout << "exiting...: " << cplusmod.modname << endl;
// terminate
	LWNetList::nlistdest();	// substitute for destructor
	// Note. multiple execution of LWNetList::~LWNetList() & nlistdest()
	//       is harmless.
// delete the advance function from the simulation function
	//jcSim::unbindCallback(cplusmod.patptr);  JAN

	return 1;
};

/*
 * StateId member functions
 */
StateId StateId::getchild(int order, int numspawn) const
{
	StateId result;

	result.branch = branch | (order << nvfig);
	switch(numspawn) {
	case 2:
		result.nvfig = nvfig + 1;
		break;
	case 3:
	case 4:
		result.nvfig = nvfig + 2;
		break;
	default:
		std::cerr << "[E](StateId::getchild) not implemented\n";
		exit(1);
	}
	result.base = (order > 0) ? result.nvfig : base;

	return result;
}

/*
 * StateId::isdirectspawned()
 * 	Return:	(*this is directly spawned by parent) order in spawning
 *			(otherwise) -1
 */
int StateId::isdirectspawned(const StateId& parent, int numspawn) const
{
	int ret;
	unsigned long nvfigmask = (0x1 << parent.nvfig) - 1;

	switch(numspawn) {
	case 2:
		if(nvfig != parent.nvfig + 1)
			return -1;
		break;
	case 3:
	case 4:
		if(nvfig != parent.nvfig + 2)
			return -1;
		break;
	default:
		std::cerr << "[E](StateId::isdirectspawned) not implemented\n";
		exit(1);
	}
	if((branch & nvfigmask) == parent.branch
	 && (ret = (branch >> parent.nvfig)) < numspawn)
		return ret;
	else
		return -1;
}

bool StateId::isspawned(const StateId& parent) const
{
	unsigned long nvfigmask = (0x1 << parent.nvfig) - 1;

	if((branch & nvfigmask) == parent.branch && nvfig > parent.nvfig)
		return true;
	else
		return false;
}

bool StateId::isdependent(const StateId& ancestor) const
{
	unsigned long basemask = (0x1 << ancestor.base) - 1;
	return ((branch & basemask) == ancestor.branch)
	 && (branch != ancestor.branch);
}

void StateId::print(bool nl)
{
	std::cout << "(" << branch << ", " << nvfig << ", " << base << ")";
	if(nl)
		std::cout << '\n';
}

#ifdef DEBUG_STATEID
void main()
{
	StateId a[10];

	a[0].reset();
	a[1] = a[0].getchild(0, 3);
	a[2] = a[0].getchild(1, 3);
	a[3] = a[0].getchild(2, 3);
	a[4] = a[1].getchild(0, 2);
	a[5] = a[1].getchild(1, 2);
	a[6] = a[5].getchild(0, 2);
	a[7] = a[5].getchild(1, 2);
	a[8] = a[3].getchild(0, 2);
	a[9] = a[3].getchild(1, 2);

	for(int i = 0; i < 10; i++) {
		cout << "a[" << i << "] = "; a[i].print();
	}

	for(i = 0; i < 10; i++)
		for(int j = 0; j < 10; j++) {
//			cout << "a[" << i << "].isdirectspawned(a[" << j
//			 << "], 3): " << a[i].isdirectspawned(a[j], 3) << "\n";
			cout << "a[" << i << "].isspawned(a[" << j
			 << "]): " << a[i].isspawned(a[j]) << "\n";
//			cout << "a[" << i << "].isdependent(a[" << j
//			 << "]): " << a[i].isdependent(a[j]) << "\n";
	}
}
#endif

/*
 * LWNet member functions
 */
void LWNet::addcurnode(const StateId& state, NodeId node)
{
	if(numstates >= MAXNUMSTATES) {
		std::cerr << "[E](LWNet::addcurnode) exceed MAXNUMSTATES\n";
		exit(1);
	}
	curnode[numstates].sid = state;
	curnode[numstates].nid = node;
	curnode[numstates].transed = true;
	curnode[numstates].finished = false;
	curnode[numstates].deleted = false;
	curnode[numstates].errorstatus=false;
	numstates++;
}

void LWNet::regdelcurnode(int curnodeindex)
{
	curnode[curnodeindex].deleted = true;
}

void LWNet::procdelcurnode(void)
{
	int nid, oid;

	for(nid = oid = 0; oid < numstates; oid++)
		if(curnode[oid].deleted == false) {
			if(nid != oid)
				curnode[nid] = curnode[oid];
			nid++;
		}
	numstates = nid;
}

void LWNet::defnormalnode(NodeId dnid, ACTFUNC dact,
 ACTFUNC dpreact, ACTFUNC dpostact)
{
	if(dnid >= numnodes)
		errwarn(1, "(defnormalnode) node:", itoa(dnid), "not exist", 0);
	if(node[dnid].type != UNDEF_NODE)
		errwarn(0, "(defnormalnode) node:", itoa(dnid), "redefined", 0);
	node[dnid].type = NORMAL_NODE;
	node[dnid].nc.numbranch = 0;
	node[dnid].nc.norm.act = dact;
	node[dnid].nc.norm.preact = dpreact;
	node[dnid].nc.norm.postact = dpostact;
}

void LWNet::defcallnode(NodeId dnid, LWNEW dfnew, ACTFUNC dpreact, ACTFUNC dpostact, void *args)
{
	if(dnid >= numnodes)
		errwarn(1, "(defcallnode) node:", itoa(dnid), "not exist", 0);
	if(node[dnid].type != UNDEF_NODE)
		errwarn(0, "(defcallnode) node:", itoa(dnid), "redefined", 0);
	node[dnid].type = CALL_NODE;
	node[dnid].nc.numbranch = 0;
	node[dnid].nc.call.fnew = dfnew;
	node[dnid].nc.call.preact = dpreact;
	node[dnid].nc.call.postact = dpostact;
	node[dnid].nc.call.args = args;
}

void LWNet::defparnode(NodeId dnid, NodeId br0, NodeId br1,
 NodeId br2, NodeId br3)
{
	if(dnid >= numnodes)
		errwarn(1, "(defparnode) node:", itoa(dnid), "not exist", 0);
	if(node[dnid].type != UNDEF_NODE)
		errwarn(0, "(defparnode) node:", itoa(dnid), "redefined", 0);
	if(br0 >= numnodes || br1 >= numnodes
	 || br2 >= numnodes || br3 >= numnodes)
		errwarn(1, "(defparnode) branching node not exist", 0);
	node[dnid].type = PAR_NODE;
	node[dnid].pr.node[0] = br0;
	node[dnid].pr.node[1] = br1;
	if(br2 >= 0) {
		node[dnid].pr.node[2] = br2;
		if(br3 >= 0) {
			node[dnid].pr.node[3] = br3;
			node[dnid].pr.numspawn = 4;
		}
		else
			node[dnid].pr.numspawn = 3;
	}
	else
		node[dnid].pr.numspawn = 2;

	node[dnid].pr.stp = new StateId;
}

void LWNet::defjoinnode(NodeId dnid, NodeId dpar, NodeId dnext)
{
	if(dnid >= numnodes)
		errwarn(1, "(defjoinnode) node:", itoa(dnid), "not exist", 0);
	if(node[dnid].type != UNDEF_NODE)
		errwarn(0, "(defjoinnode) node:", itoa(dnid), "redefined", 0);
	if((dpar >= numnodes) || node[dpar].type != PAR_NODE)
		errwarn(1, "(defjoinnode) node:", itoa(dpar), "illegal", 0);
	if(dnext >= numnodes)
		errwarn(1, "(defjoinnode) node:", itoa(dnext), "not exist", 0);
	node[dnid].type = JOIN_NODE;
	node[dnid].jik.node = dnext;
	node[dnid].jik.par = dpar;
	for(int i = 0; i < MAXNUMSPAWN; i++)
		node[dnid].jik.ended[i] = false;
}

void LWNet::defindynode(NodeId dnid, NodeId dpar, NodeId dnext)
{
	if(dnid >= numnodes)
		errwarn(1, "(defindynode) node:", itoa(dnid), "not exist", 0);
	if(node[dnid].type != UNDEF_NODE)
		errwarn(0, "(defindynode) node:", itoa(dnid), "redefined", 0);
	if((dpar >= numnodes) || node[dpar].type != PAR_NODE)
		errwarn(1, "(defindynode) node:", itoa(dpar), "illegal", 0);
	if(dnext >= numnodes)
		errwarn(1, "(defindynode) node:", itoa(dnext), "not exist", 0);
	node[dnid].type = INDY_NODE;
	node[dnid].jik.node = dnext;
	node[dnid].jik.par = dpar;
}

void LWNet::defkldpnode(NodeId dnid, NodeId dnext)
{
	if(dnid >= numnodes)
		errwarn(1, "(defkldpnode) node:", itoa(dnid), "not exist", 0);
	if(node[dnid].type != UNDEF_NODE)
		errwarn(0, "(defkldpnode) node:", itoa(dnid), "redefined", 0);
	if(dnext >= numnodes)
		errwarn(1, "(defkldpnode) node:", itoa(dnext), "not exist", 0);
	node[dnid].type = KLDP_NODE;
	node[dnid].jik.node = dnext;
}

void LWNet::defhaltnode(NodeId dnid)
{
	if(dnid >= numnodes)
		errwarn(1, "(defhaltnode) node:", itoa(dnid), "not exist", 0);
	if(node[dnid].type != UNDEF_NODE)
		errwarn(0, "(defhaltnode) node:", itoa(dnid), "redefined", 0);
	node[dnid].type = HALT_NODE;
}

void LWNet::defexitnode(NodeId dnid)
{
	if(dnid >= numnodes)
		errwarn(1, "(defexitnode) node:", itoa(dnid), "not exist", 0);
	if(node[dnid].type != UNDEF_NODE)
		errwarn(0, "(defexitnode) node:", itoa(dnid), "redefined", 0);
	node[dnid].type = EXIT_NODE;

}

void LWNet::deffailexitnode(NodeId dnid)
{
	if(dnid >= numnodes)
		errwarn(1, "(deffailexitnode) node:", itoa(dnid), "not exist", 0);
	if(node[dnid].type != UNDEF_NODE)
		errwarn(0, "(deffailexitnode) node:", itoa(dnid), "redefined", 0);
	node[dnid].type = FAIL_EXIT_NODE;

}
void LWNet::deftrans(NodeId dnid, CONDFUNC dcond, NodeId dnext)
{
	if(dnid >= numnodes)
		errwarn(1, "(deftrans) node:", itoa(dnid), "not exist", 0);
	if(node[dnid].type != NORMAL_NODE && node[dnid].type != CALL_NODE)
		errwarn(1, "(deftrans) node:", itoa(dnid), "illegal type", 0);
	if(dnext >= numnodes)
		errwarn(1, "(deftrans) node:", itoa(dnext), "not exist", 0);
	register int nb = node[dnid].nc.numbranch;
	if(nb >= MAXNUMBRANCH)
		errwarn(1, "(deftrans) MAXNUMBRANCH overflow", 0);
	node[dnid].nc.trans[nb].cond = dcond;
	node[dnid].nc.trans[nb].stype = TNST_NodeId;
	node[dnid].nc.trans[nb].nextid = dnext;
	node[dnid].nc.numbranch++;
}

void LWNet::deftrans(NodeId dnid, CONDFUNC dcond, NODEBRNCFUNC dnbf)
{
	if(dnid >= numnodes)
		errwarn(1, "(deftrans) node:", itoa(dnid), "not exist", 0);
	if(node[dnid].type != NORMAL_NODE && node[dnid].type != CALL_NODE)
		errwarn(1, "(deftrans) node:", itoa(dnid), "illegal type", 0);
	register int nb = node[dnid].nc.numbranch;
	if(nb >= MAXNUMBRANCH)
		errwarn(1, "(deftrans) MAXNUMBRANCH overflow", 0);
	node[dnid].nc.trans[nb].cond = dcond;
	node[dnid].nc.trans[nb].stype = TNST_NodeFunc;
	node[dnid].nc.trans[nb].nbf = dnbf;
	node[dnid].nc.numbranch++;
}

void LWNet::defmonitor(CONDFUNC mcond, ACTFUNC mact)
{
	if(nummonitrs >= MAXNUMMONITRS)
		errwarn(1, "(defmonitor) MAXNUMMONITRS overflow", 0);
	monitr[nummonitrs].mcond = mcond;
	monitr[nummonitrs].mact = mact;
	nummonitrs++;
}

void LWNet::markfinished(void)
{
	curnode[cnsub].finished = true;
}

void LWNet::markerror(void)
{
	curnode[cnsub].errorstatus = true;
}

void LWNet::unmarkerror(void)
{
	curnode[cnsub].errorstatus = false;
}

bool LWNet::defaultcond(void) const
{
	return true;
}

bool LWNet::finishcond(void) const
{
	return curnode[cnsub].finished;
}

bool LWNet::errorcond(void) const
{
	return curnode[cnsub].errorstatus;
}

StateId LWNet::getcursid(void) const
{
	return curnode[cnsub].sid;
}

void LWNet::procrequest(void)
{
	numjobreqs = 0;
}

LWNet::LWNet(int sz, NodeId start)
{
  //cout << "LWNet constructor: " << (int)this << endl;
	numstates = 1;
	curnode[0].sid.reset();
	curnode[0].nid = start;
	curnode[0].transed = true;
	curnode[0].finished = false;
	curnode[0].deleted = false;
	curnode[0].errorstatus = false;
	node = new struct lwn_node[numnodes = sz];
	for(int i = 0; i < numnodes; i++)
		node[i].type = UNDEF_NODE;
	nummonitrs = 0;
	shdexit = false;
	exitcode = 0;
	numjobreqs = 0;
}

LWNet::~LWNet()
{
  //cout << "LWNet destructor: " << (int)this << endl;
	for(int s = 0; s < numstates; s++)
		if(node[curnode[s].nid].type == CALL_NODE)
			delete node[curnode[s].nid].nc.call.net;
	for(int i = 0; i < numnodes; i++)
		if(node[i].type == PAR_NODE)
			delete node[i].pr.stp;
	delete[] node;
}

int LWNet::advance(int *errorstatus)
{
  if(errorstatus)
    *errorstatus = 0;
//
// scan and execute monitors
//
	for(int m = 0; m < nummonitrs; m++) {
		if(monitr[m].mcond) {
			if((this->*monitr[m].mcond)()) {
				(this->*monitr[m].mact)();
				break;
			}
		}
		else
			(this->*monitr[m].mact)();
	}
/*
 * pre-action process for entering a new node
 */
	for(int s = 0; !shdexit && s < numstates; s++) {
		cnsub = s;				// for getcursid()
		if(curnode[s].transed) {
			switch(node[curnode[s].nid].type) {
			case NORMAL_NODE:
				if(node[curnode[s].nid].nc.norm.preact)
					(this->*node[curnode[s].nid].nc.norm.preact)();
						// pre-action
				break;

                        // do both preaction and intial call to patnet
                        case CALL_NODE:
                               if(node[curnode[s].nid].nc.call.preact)
	                        (this->*node[curnode[s].nid].nc.call.preact)();
                                 node[curnode[s].nid].nc.call.net
				  = (this->*node[curnode[s].nid].nc.call.fnew)(node[curnode[s].nid].nc.call.args);
						// pre-action
				break;

			// otherwize no pre-action
			}
			curnode[s].transed = false;
		}
	}
/*
 * unit action
 */
	for(int s = 0; !shdexit && s < numstates; s++) {
		cnsub = s;				// for markfinished()
		switch(node[curnode[s].nid].type) {
		case NORMAL_NODE:
			if(node[curnode[s].nid].nc.norm.act)
				(this->*node[curnode[s].nid].nc.norm.act)(); // action
			break;
		case CALL_NODE:
		  //cout << "lwnet: callnode ptr is " <<
                  //         node[curnode[s].nid].nc.call.net << endl;
		  int errstat=0;
			if(!node[curnode[s].nid].nc.call.net->advance(&errstat))
			{
			  if(errstat)
			    curnode[s].errorstatus = true; // mark error status
			  curnode[s].finished = true;	// markfinished();
                                //cout << "walknet ptr doesn't advance!" << endl;
			 };
                        break;
		// otherwize no unit-action
		}
	}
/*
 * node transition & post-action
 */
	for(int s = 0; !shdexit && s < numstates; s++) {
/*
 * node transition
 */
		cnsub = s;				// for finishcond()
		NodeId oldnode = curnode[s].nid;
		switch(node[oldnode].type) {
		case NORMAL_NODE:
			for(int i = 0; i < node[oldnode].nc.numbranch; i++) {
                          if((this->*node[oldnode].nc.trans[i].cond)()) {
			     switch(node[oldnode].nc.trans[i].stype) {
					case TNST_NodeId:
						curnode[s].nid
						 = node[oldnode].nc.trans[i].nextid;
						break;
					case TNST_NodeFunc:
						curnode[s].nid
						 = (this->*node[oldnode].nc.trans[i].nbf)();
						break;
			      }
					break;
                           }
			}
			break;

		case CALL_NODE:
                    if (curnode[s].finished == true)
                    {
	             for(int i = 0; i < node[oldnode].nc.numbranch; i++) {
			if((this->*node[oldnode].nc.trans[i].cond)()) {
				switch(node[oldnode].nc.trans[i].stype) {
					case TNST_NodeId:
						curnode[s].nid
						 = node[oldnode].nc.trans[i].nextid;
						break;
					case TNST_NodeFunc:
						curnode[s].nid
						 = (this->*node[oldnode].nc.trans[i].nbf)();
						break;
					}
					break;
				}
			}
		   };
			break;

		}
/*
 * post-action process for exiting a current node
 */
		if(curnode[s].nid != oldnode) {
			curnode[s].transed = true;		// node transition was made
			curnode[s].finished = false;
			switch(node[oldnode].type) {
			case NORMAL_NODE:
				if(node[oldnode].nc.norm.postact)
					(this->*node[oldnode].nc.norm.postact)();
						// post-action
				break;
			case CALL_NODE:
                                // do post action processing right before
                                // killing off the net
				if(node[oldnode].nc.call.postact)
                                   (this->*node[oldnode].nc.call.postact)();
		                delete node[oldnode].nc.call.net;
                               break;
			}
		}
	}
//	
	if(shdexit)
		return false;
//
/*
 *
 */
	int oldnumstates = numstates;
	bool allend = false;
	NodeId parid;
	for(int s = 0; s < oldnumstates; s++) {
		if(curnode[s].deleted)
			continue;
		NodeId selfid = curnode[s].nid;
		switch(node[selfid].type) {
		case PAR_NODE:
			*node[selfid].pr.stp = curnode[s].sid;
			for(int i = 0; i < node[selfid].pr.numspawn; i++) {
				addcurnode(
				 curnode[s].sid.getchild(i, node[selfid].pr.numspawn),
				 node[selfid].pr.node[i]);
			}
			
			regdelcurnode(s);
			break;
		case JOIN_NODE:
			parid = node[selfid].jik.par;
			int cid;
			if((cid = curnode[s].sid.isdirectspawned(*node[parid].pr.stp,
			 node[parid].pr.numspawn)) >= 0) {
				node[selfid].jik.ended[cid] = true;
				regdelcurnode(s);
			}
		/* */
			allend = true;
			for(int i = 0; allend && i < node[parid].pr.numspawn; i++)
				allend = allend && node[selfid].jik.ended[i];
			if(allend) {
				addcurnode(*node[parid].pr.stp, node[selfid].jik.node);
				for(int i = 0; i < MAXNUMSPAWN; i++)
					node[selfid].jik.ended[i] = false;
			}
			break;
		case INDY_NODE:
			parid = node[selfid].jik.par;
			for(int t = 0; t < numstates; t++)		// not oldnumstates
				if(curnode[t].sid.isspawned(*node[parid].pr.stp))
					regdelcurnode(t);
			addcurnode(*node[parid].pr.stp, node[selfid].jik.node);
			break;
		case KLDP_NODE:
			for(int t = 0; t < numstates; t++)			// not oldnumstates
				if(curnode[t].sid.isdependent(curnode[s].sid))
					regdelcurnode(t);
		// node-to-node transition
			curnode[s].nid = node[selfid].jik.node;
			curnode[s].transed = true;
			curnode[s].finished = false;
			break;
		case HALT_NODE:
			regdelcurnode(s);
			break;
		}
	}
	procdelcurnode();

	for(int s = 0; s < numstates; s++) {
		if(node[curnode[s].nid].type == EXIT_NODE) {
			if(numstates > 1)
				errwarn(0, "(advance) multi states at EXIT_NODE", 0);

			return false;
		}
		if(node[curnode[s].nid].type == FAIL_EXIT_NODE) {
			if(numstates > 1)
				errwarn(0, "(advance) multi states at FAIL_EXIT_NODE", 0);

			*errorstatus = 1;
			return false;
		}
	}

	if(numstates >= 1)
		return true;
	else
		return false;
}

void LWNet::requestjob(int type, void* data, JobReport* ret)
{
	if(numjobreqs >= MAXNUMJOBREQS)
		errwarn(1, "(requestjob) numjobreqs exceed limit", 0);
	if(ret)
		ret->reset();
	jobreq[numjobreqs].type = type;
	jobreq[numjobreqs].indat = data;
	jobreq[numjobreqs].rep = ret;
	numjobreqs++;
}

void LWNet::markfinished(StateId sid)
{
	int s;
	for(s = 0; s < numstates; s++)
		if(curnode[s].sid == sid) {
			curnode[s].finished = true;	// markfinished();
			break;
		}
	if(s == numstates)	// not found
		errwarn(0, "(markfinished(StateId)) not found", 0);
}

void LWNet::stateprint()
{
	for(int s = 0; s < numstates; s++) {
		curnode[s].sid.print(false);
		std::cout << '\t' << "nid = " << curnode[s].nid << '\n';
	}
}

/*
 * LWNetList member functions
 */

void LWNetList::nlistdest(void)
{
	for(LWNetListNode* p = patlist; p; /* p = p->next */ ) {
		LWNetListNode* delnode = p;
		p = p->next;
		delnode->net->lwndest();	// destructor
		delete delnode->net;		// delete delnode->net
		delete delnode;
	}
	patlist = 0;
	//cout << "LWNetList::nlistdest()" << endl;
}

void LWNetList::addnet(LWNet* newnet)
{
      /* add to the end of the patnet list rather than
         the beginning, this allows the evaluation of
         new patnets added from called patnets, isactive to
         work, etc. */
 
        LWNetListNode* p = patlist;
	LWNetListNode* node = new LWNetListNode;
        assert(node);
         
	node->net = newnet;
        node->next = NULL;

        //cout << "lwnet: adding a patnet patlist=" << p << " node = " << node
        // << endl; 

        /*go to the end of the list */
         if (p)
         {
           while (p->next) p=p->next;
   	   p->next = node;
         }
         else
           patlist = node;
}

bool LWNetList::delnet(LWNet* dnet)
{
     LWNetListNode *temp,*prev;
      
        prev = temp = patlist;

        while (temp)
	{
          if (temp->net == dnet)
	  { 
              prev->next = temp->next;
              break;
          };

          prev = temp;
          temp = temp->next;
        };
 
	if(!temp) {
		errwarn(0, "(LL:delnet) net not found", 0);
		return false;
	};


	temp->net->lwndest();	// destructor
	delete temp->net;		// delete dnet

	return true;

}

int LWNetList::advance(void *)
{
	int numactive = 0;

// find the 1st still active net
	int errorstatus=0;
	LWNetListNode* p;
	for(p = patlist; p; /* p = p->next */ )
		if(p->net->advance(&errorstatus)) {
			numactive++;
			break;
		}
		else {	// delnet(p->net)
			LWNetListNode* delnode = p;
			p = p->next;
			delnode->net->lwndest();	// destructor
			delete delnode->net;		// delete delnode->net
                        delete delnode;
		}
	
	patlist = p;	// now p is the 1st active net

// if there is no active net, return 0 (= numactive)
	if(!p)
	{
          //cout << "exiting LWNetList::advance , returning " << numactive 
          //  << endl;
	  return numactive;
        }; // skip the 1st active node, that is already processed
// ***********In Noma's original code, lwnets.list = p was defined **********
// ********** in the next step. This gives buserror as if there is *********
// ********** only one node, then lwnets.list doesnt get reinitialized *****
// ********** to 0 correctly. So move lwnets.list=p; one step above 
// *********   if(!p)... ***********
//		lwnets.list = p;	// now p is the 1st active net

	for( /* p = lwnets.list */; p->next; /* p = p->next */ )
		if(p->next->net->advance(&errorstatus)) {
			numactive++;
			p = p->next;
		}
		else {	// delnet(p->next->net)
			LWNetListNode* delnode = p->next;
			p->next = p->next->next;
			delnode->net->lwndest();	// destructor
			delete delnode->net;		// delete delnode->net
			delete delnode;
		}

	return numactive;
}

bool
LWNetList::isActive(LWNet *net)
{
    LWNetListNode* p = patlist;
    
    while(p)
      {
	  if(p->net == net)
	    return true;

	  else
	    p = p->next;
      }

    return false;

}








