/*
 * lwnet.h
 */

#ifndef _LWNET_H
#define _LWNET_H

#include <iostream>


/*
 *
 */
#ifndef sqr
#define sqr(x) ((x) * (x))
#endif

char* itoa(int n);
void errwarn(int severity ...);

/*
 * class declaration
 */
class StateId;
class JobReport;
class LWNet;
class LWNetList;

/*
 * node identifier & type
 */
typedef int	NodeId;
enum NodeType {
				UNDEF_NODE,		// undefined
				NORMAL_NODE,	// normal node
				CALL_NODE,		// (subnet) call node
				PAR_NODE,		// par node
				JOIN_NODE,		// (par-)join node
				INDY_NODE,		// (par-)indy node
				KLDP_NODE,		// kill-dependent node
				HALT_NODE,		// halt node
				EXIT_NODE,		// exit node
                                FAIL_EXIT_NODE,    // exit node on failure
				};

/*
 * action/condition/new functions
 */
typedef void (LWNet::*ACTFUNC)(void);	// action function
typedef bool (LWNet::*CONDFUNC)(void);	// condition function
typedef NodeId (LWNet::*NODEBRNCFUNC)(void);	// node branch function
typedef LWNet* (LWNet::*LWNEW)(void *); 

/*
 * constants for LWNet
 */
#define MAXNUMSTATES	20	// maximum number of simultaneous states
#define MAXNUMBRANCH	8	// maximum number of conditional branches
#define MAXNUMSPAWN		4	// maximum number of spawining in PAR_NODE
#define	MAXNUMJOBREQS	8	// maximum number of job requests
#define	MAXNUMMONITRS	8	// maximum number of monitors

/*
 * class StateId - Id among simultanous states in LWNet
 */
class StateId {
private:
	/* unsigned long depth;	// depth in LWNet */
	unsigned long branch;	// id in branching in LWNet
	int nvfig;	// number of valid figures in branch
	int base;	// number of significant (nonzero) figures in branch
public:
	void reset(void)	{ branch = nvfig = base = 0; }
	int operator==(const StateId &a) const	{ return
	 (branch == a.branch && nvfig == a.nvfig && base == a.base); }
	StateId getchild(int order, int numspawn) const;
										// for PAR_NODE
	int isdirectspawned(const StateId& parent, int numspawn) const;
										// for JOIN_NODE

	bool isspawned(const StateId& parent) const;
										// for INDY_NODE
	bool isdependent(const StateId& ancestor) const;
										// for KLDP_NODE
	void print(bool nl = true);			// for debug
};

/*
 * class JobReport - job report from requested net to requesting net
 */
class JobReport {
public:
	virtual void reset() = 0;		// reset JobReport as UNREPORTED
	virtual int isreported() = 0;	// true if reported; false otherwise
};

/*
 * structures in LWNet
 */
struct lwn_state {
  StateId sid;			// state id;
  NodeId nid;			// node id;
  bool transed;			// flag: just after transition or not
  bool finished;		// flag: current node finished or not
  bool deleted;			// flag: should be deleted or not
  bool errorstatus;		// flag: error occured during execution of node
};

enum TransNodeSpecType {
						TNST_NodeId,
						TNST_NodeFunc
						};

struct lwn_trans {
	CONDFUNC cond;			// condition id for transition
	TransNodeSpecType stype; // transition node spec. type
	union {
		NodeId nextid;		// following node if cond is true
		NODEBRNCFUNC nbf;	// branch function returning NodeId
	};
};

struct lwn_node {
	NodeType type;			// node type

	union {
	/* NORMAL_NODE & CALL_NODE */
		struct {
			int numbranch;	// size of transitioni table
			struct lwn_trans trans[MAXNUMBRANCH];
							// transition table from node to node
			union {
			/* NORMAL_NODE */
				struct {
					ACTFUNC act;	// action
					ACTFUNC preact;	// pre-action
					ACTFUNC postact; // post-action
				} norm;
			/* CALL_NODE */
				struct {
					LWNEW fnew;		// "new" function for LWNet
					LWNet* net;		// called sub-net;
					ACTFUNC preact;	// pre-action
					ACTFUNC postact; // post-action
				        void *args;
				} call;
			};
		} nc;
	/* PAR_NODE */
		struct {
			int numspawn;			// number of spawning
			NodeId node[MAXNUMSPAWN];	// next nodes to be spawned
			StateId* stp;			// incoming state id
									// (NEEDS INITIALIZATION !!!)
		} pr;
	/* JOIN_NODE & INDY_NODE & KLDP_NODE */
		struct {
			NodeId node;			// next node
		/* JOIN_NODE & INDY_NODE only */
			NodeId par;				// corresponding PAR_NODE
		/* JOIN_NODE only */
			bool ended[MAXNUMSPAWN]; // status table for incoming nodes
									// (NEEDS INITIALIZATION !!!)
		} jik;
	};
};

struct lwn_jobreq {
	int type;	// requested job type
	void* indat;	// job input data
	JobReport* rep;	// report data
};

struct lwn_monitr {	// monitor data
	CONDFUNC mcond;	// condition for activating monitor's action
	ACTFUNC mact;	// action
};

/*
 * class LWNet
 */
class LWNet {
private:
	int numstates;			// number of states
	struct lwn_state curnode[MAXNUMSTATES];	// current nodes/states
	struct lwn_node *node;	// node table
	int nummonitrs;			// number of monitors
	struct lwn_monitr monitr[MAXNUMMONITRS];	// monitor table
	bool shdexit;			// flag: this net should exit.
	int cnsub;				// current index of curnode[]
					// (for markfinished() & finishcond() & getcursid())
	void addcurnode(const StateId& state, NodeId node);
							// add current node/state
	void regdelcurnode(int curnodeindex);
							// delete current node/state (flag only)
	void procdelcurnode(void);
							// delete current node/state (process)
protected:
	int numnodes;			// number of nodes
	int numjobreqs; 		// number of job requests;
	lwn_jobreq jobreq[MAXNUMJOBREQS];	// job request table;
	void defnormalnode(NodeId dnid, ACTFUNC dact,
	 ACTFUNC dpreact = 0, ACTFUNC dpostact = 0);
	void defcallnode(NodeId dnid, LWNEW dfnew, ACTFUNC dpreact = 0, ACTFUNC dpostact = 0, void *args = 0);
	void defparnode(NodeId dnid, NodeId br0, NodeId br1,
	 NodeId br2 = -1, NodeId br3 = -1);
	void defjoinnode(NodeId dnid, NodeId dpar, NodeId dnext);
	void defindynode(NodeId dnid, NodeId dpar, NodeId dnext);
	void defkldpnode(NodeId dnid, NodeId dnext);
	void defhaltnode(NodeId dnid);
	void defexitnode(NodeId dnid);
        void deffailexitnode(NodeId dnid);
	void deftrans(NodeId dnid, CONDFUNC dcond, NodeId dnext);
	void deftrans(NodeId dnid, CONDFUNC dcond, NODEBRNCFUNC dnbf);
	void defmonitor(CONDFUNC mcond, ACTFUNC mact);
	void markfinished(void);		// mark current node as finished
    void markerror(void); // mark that an error was encountered during the execution of this node
	void unmarkerror(void);//There are certain cases when we need to unmark an error. 
	public:
	bool defaultcond(void) const;	// default condition function
	bool finishcond(void) const;	// finished condition function
    bool errorcond(void) const;     // error condition function
	StateId getcursid(void) const;	// get current state id
								// only in (pre-/post-)action functions
	int isrequested(void)	{ return numjobreqs; }	// jobs requested?
	virtual void procrequest(void);	// process requested jobs

	LWNet(int sz, NodeId start = 0);	// constructor
						// (sz: the number of nodes; start: start node)
	~LWNet();				// destructor
	virtual void lwndest(void)	{ }		// destructor for derived class
	// NOTE: If a net of a derived class of LWNet is managed via
	//		LWNetList, its own destructor is not executed because nets
	//		in LWNetList are treated as LWNet.  Instead, a virtual
	//		function lwndest(), which is called when the net is deleted
	//		in LWNetList::delnet() or LWNetList::advance(), works as
	//		a destructor.  But note that only one lwndest() at the top
	//		level is executed even if the net is defined via several
	//		derivations.
	virtual int advance(int *errstatus=0);	// advance one step
	void requestjob(int type, void* data, JobReport* ret);
							// request job to another net
	void markfinished(StateId sid);	// mark node with sid as finished
	void exitnet(int code = 0)	{ exitcode = code; shdexit = true; }
        void failexitnet(int code = 0)	{ exitcode = code; shdexit = true; }
							// make LWNet exit
	int exitcode;			// exit code
	void stateprint();		// debug
};


/*
 * class LWNetList
 */
struct LWNetListNode {
		LWNet* net;
		LWNetListNode* next;
};

class LWNetList {
private:
	static LWNetListNode* patlist; // since addnet, et al need to
       // to access, and they are static functions, patlist must be
       // a static member variable
       // s.c.: why did this ever work when it wasn't a static member??

public:

	LWNetList()	{  std::cout << "LWNetList::LWNetList()" << std::endl; }
	static void nlistconst(void);
	static void nlistdest(void);
	~LWNetList() { nlistdest(); std::cout << "LWNetList::~LWNetList()" << std::endl; }
	static void addnet(LWNet* newnet);
	static bool delnet(LWNet* dnet);
	static bool isActive(LWNet* net);
	static int advance(void *);
    static int always_advance(void *);	// advance one step

};

#endif
