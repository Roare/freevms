/*
	****************************************************************

		Copyright (c) 1992, Carnegie Mellon University

		All Rights Reserved

	Permission  is  hereby  granted   to  use,  copy,  modify,  and
	distribute  this software  provided  that the  above  copyright
	notice appears in  all copies and that  any distribution be for
	noncommercial purposes.

	Carnegie Mellon University disclaims all warranties with regard
	to this software.  In no event shall Carnegie Mellon University
	be liable for  any special, indirect,  or consequential damages
	or any damages whatsoever  resulting from loss of use, data, or
	profits  arising  out of  or in  connection  with  the  use  or
	performance of this software.

	****************************************************************
*/
//TITLE "TCP - major routines for servicing TCP connections"
/*
Facility:

	TCP - Main routines for implementing Internet TCP (RFC 792).

Abstract:
!!!HACK!!// is this accurate?
	Handles tasks which must be "scheduled" in some way, as opposed to
	event driven tasks such as user calls (handled in USER, TCP_USER)
	or network input (handled in TCP_SEGIN).

Language:

	BLISS-32

System:

	VAX/VMS

Author:

	Original version by Stan C. Smith, Summer 1981
	This version by	Vince Fuller, CMU-CSD, Spring/Summer, 1986
	Copyright (c) 1986,1987, Vince Fuller and Carnegie-Mellon University

Module Modification History:

*** Begin USBR change log ***

8.2	05-Dec-1991	Henry W. Miller		USBR
	Set Window_Default to 8192.
	Set ACK_Threshold to 1024.
	Rework arithmetic concerning SND_PPTR.  (Unsigned vrs. signed).

8.1	14-Nov-1991	Henry W. Miller		USBR
	Change arithmetic, printout on timers from signed to unsigned.
	Added routine TCP$Enqueue_ACK() to queue up an ACK for future
	delivery.  (Currently only sets the PENDING_ACK flag, which now
	appears to work due to the correct arithmmetic comparisons).
	In TCP$Send_Data(), check to see if we received an ICMP Source
	Quench packet, and if so, delay delivery

8.0h	17-Sep-1991	Henry W. Miller		USBR
	In TCP$Send_Data(), rework Nagle algorithim as per RFC1122.
	Also add hook for "de-Nagle'ing" the connection.

8.0g	26-Jul-1991	Henry W. Miller		USBR
	In CHECK_TCB(), do sanity check to see if TCB is valid, i.e.,
	not equal to 0.

8.0f	17-Jul-1991	Henry W. Miller		USBR
	In TCP$TCB_SET_STATE(), if changing from CS$NAMELOOK and lookup's
	pending, cancel pending requests.

8.0e	06-Jun-1991	Henry W. Miller		USBR
	Set default ACK_THRESHOLD to 4096.
	Set default WINDOW_DEFAULT to 32768.

8.0d	25-Jan-1991	Henry W. Miller		USBR
	Make WINDOW_DEFAULT and ACK_THRESHOLD configurable variables.

8.0c	15-Jan-1991	Henry W. Miller		USBR
	Changed SND_WND, RCV_WND, Old_RCV_WND, SND_NXT and RCV_NXT to
	unsigned long values as recommended by RFC1122.

8.0b	13-Jan-1991	Henry W. Miller		USBR
	Make TCPTTL a configurable variable.

8.0a	11-Jan-1991	Henry W. Miller		USBR
	Cleanup printout in TCP$Service_Connections().

*** Begin CMU change log ***

8.0	06-Feb-1990	Bruce R. Miller		CMU NetDev
	This module used to be TCP_Main, the main module
	for the IPACP.  I chopped out all of the ACP init
	stuff and changed the name to TCP.Bli.  The ACP
	Init stuff is now in MAIN.BLI.

	20-Oct-1989	Bruce R. Miller		CMU NetDev
	Added a bit of code to initialize the activity log.

	30-Aug-1989	Bruce R. Miller		CMU NetDev
	Made Conect Table into a hash table structure.

	07-Aug-1989	Bruce R. Miller		CMU NetDev
	Added NML$Cancel call to Inactivate_TCB routine.  When a TCB
	is inactivated any outstanding NQE's should be deleted.  If this
	were not done, and a new TCB with the old address were created,
	the the Name Query reply from the first TCB would affect the
	new TCB (which would be totally BOGUS!).

	03-Jul-1989	Bruce R. Miller		CMU NetDev
	Fixed DUMP_TCB to handle unknown TCP state.

	16-MAR-1989	Dale Moore	CMU-CS/RI
	If connection timeout is zero, then don't timeout connection.

	20-OCT-1988	Dale Moore	CMU-CS/RI
	Changed Keep_Alives (Do_Probe) to be runtime configurable.

	04-OCT-1988	Dale Moore	CMU-CS/RI
	Changed TOS from %O"15" to 0.  Evidently some busted
	implementations that couldn't take something in this
	field that wasn't what they expected.

7.10  22-JUN-1988, Dale Moore	CMU-CS/RI
	Change the call on retransmit of SYN to send size of options
	as long	rather than octets.  Thanks to Charles Lane of
	Drexel University.

7.9  10-FEB-1988, Dale Moore
	Modify the Maximum segment size to be dependent upon whether
	the remote address is local or not.

7.8  18-Nov-87, Edit by VAF
	Know about IP$SEND returning failure when no route available. Make
	SEND_CTL propagate this information back to its caller. This allows
	the TCP user interface to detect this condition immediately when
	sending a SYN and abort the connection appropriately.

7.7   4-Aug-87, Edit by VAF
	Remove CQ_DEQUEUE and CQ_ENQUEUE - they are now in MACLIB.
	Use $$KCALL macro to do $CMKRNL stuff.
	Disable Resource Wait Mode at startup so the ACP crashes instead of
	waiting for resources.
	Rewrite send options handling so it can be easily made more general.

7.6  28-Jul-87, Edit by VAF
	Know about TVT's when trying to move data in main processing loop.

7.5  22-Jul-87, Edit by VAF
	Know about internal TCB's in timeout processing.

7.4  20-Jul-87, Edit by VAF
	Clean up handling of circular queues a bit.

7.3  23-Mar-87, Edit by VAF
	Make allocated buffer sizes and maximum segment size dynamic.
	Save and reset process name when ACP starts/stops.
	Call new IP initialization routine.

7.2   3-Mar-87, Edit by VAF
	Flush I/O request tags from all requests. New calling sequence for
	User$Post_IO_Status.

7.1  24-Feb-87, Edit by VAF
	"Net message queue" has been flushed - remove references. Replace with
	call to CHECK_ERRMSG_Q at end of main TCP processing loop.

7.0  19-Feb-87, Edit by VAF
	Use new NMLOOK module entry points for name lookups.

6.9  18-Feb-87, Edit by VAF
	Use OPR$FAO and FATAL$FAO instead of old error handling stuff.

6.8  17-Feb-87, Edit by VAF
	Fix spelling of "Exception_Handler".

6.7  12-Feb-87, Edit by VAF
	Add support for domain service.

6.6   9-Feb-87, Edit by VAF
	Flush external reference to Error_Processor.

6.5   6-Feb-87, Edit by VAF
	Print modification date of ACP during startup.

6.4  10-Dec-86, Edit by VAF
	Change order of arguments to Gen_Checksum.

6.3   9-Dec-86, Edit by VAF
	Don't log "Sending data for TCB..." under LOG$TCP.

6.2  12-Nov-86, Edit by VAF
	Flush SYN_WAIT_LIST and WKS global structures. Add external call to
	TIMEOUT_SYN_WAIT_LIST in main processing loop.

6.1  11-Nov-86, Edit by VAF
	Add FQ_MAX cell set by config routines & used by SEGIN.

6.0   3-Nov-86, Edit by VAF
	Set Base_RT_Timeout (initial RX time value) to 5 seconds. The idea of
	setting it to 1 second was a kluge to get around ARP eating the first
	packet. Make a minor change in the calculation of TCB->RX_TIMER.

5.9  24-Oct-86, Edit by VAF
	Additions to debugging code.
	When enqueuing user data on send queue, know about retransmission
	count in determining amount of space left on queue.

5.8  30-Sep-86, Edit by VAF
	In INACTIVATE_TCB - don't inactivate an already inactive connection -
	log an error if this occurs. This is to detect cases (like the RX
	timeout bug) where TCB's are being inactivated repeatedly.
	Rewrite Check_Rexmit_Queue to be able to build a segment with both
	data and control information.

5.7  29-Aug-86, Edit by VAF
	Call ARP_INIT in main initialization routine.

5.6  14-Aug-86, Edit by VAF
	Make sure name lookup cancel is done NOINT.

5.5  13-Aug-86, Edit by VAF
	In Build_Header turn on ACK if IS_SYNCHED is set, not by state
	GEQ CS$ESTABLISHED.

5.4  12-Aug-86, Edit by VAF
	Add CS$NAMELOOK - Name lookup wait state. Make CLOSE_TCB know about
	this state (no others should ever see it). Make CLOSE_TCB know about
	NMLook_Flag and do a GREEN_CANCEL on the TCB if it is set.
	Use GREEN_GETNAME to get Local_Name value.

5.3   9-Aug-86, Edit by VAF
	Get and store local host name in LOCAL_NAME descriptor.

5.2   6-Aug-86, Edit by VAF
	Modify retransmission code to use send circular queue.

5.1   6-Aug-86, Edit by VAF
	Modify SEND_DATA to use circular queue of buffered send data.

5.0   5-Aug-86, Edit by VAF
	Add routines for enq/deq into circular queues.
	N.B. Do not use these at AST level, since they don't do locking.

4.9  29-Jul-86, Edit by VAF
	Minor rearrangements for UDP's sake...
	Move TCP_Protocol definition out of here, change names of TTL,TOS,DF.

4.8  25-Jul-86, Edit by VAF
	Add ts$qfull_drops counter.

4.7  18-Jul-86, Edit by VAF
	Add ts$abort_drops counter.

4.6  17-Jul-86, Edit by VAF
	Pass reference to TCB pointer in CLOSE_TCB so it can clear the
	pointer when it deletes the connection.

4.5  15-Jul-86, Edit by VAF
	Add status cell ts$badseq.
	Rearrange change log.

4.4   9-Jul-86, Edit by VAF
	Add status cell ts$future_dups.

4.3   1-Jul-86, Edit by VAF
	Add some more status cells.

4.2  25-Jun-86, Edit by VAF
	Don't ever use index 0 into VALID_TCB table.

4.1  16-Jun-86, Edit by VAF
	Don't go into FIN-WAIT-1 when setting Pending_close - wait until
	the FIN is really sent.

4.0  12-Jun-86, Edit by VAF
	Simplify buffering of user send requests.

3.9  10-Jun-86, Edit by VAF
	Implement buffering of user send requests.

3.8  10-Jun-86, Edit by VAF
	Add new counters for user send queue in preparation for implementation
	of send buffering.

3.7  22-May-86, Edit by VAF
	Use VMS error message facility.

3.6   9-May-86, Edit by VAF
	Make SEND_DATA return count sent. When sending an ACK, we need to make
	sure that SEND_DATA actually sent something.

3.5   8-May-86, Edit by VAF
	Add CLOSE_TIMEOUT. Implement blocking close (USER,SEGIN)
	Add LAST_ACK state.
	Know about U$CLOSE and M$CANCEL function timeouts.

3.4   5-May-86, Edit by VAF
	Implement the "Nagle algorithm" (RFC896) in SEND_DATA.

3.3   1-May-86, Edit by VAF
	Allow delivery of user data when in FIN_WAIT_1 or FIN_WAIT_2.
	Have Service_Connections and Check_ReXmit_Queue return next service
	time values to simplify Wait_For_Something_2_Do.
	Add code to compress retransmission queue when doing retransmisson.

3.2  30-Apr-86, Edit by VAF
	Debugging code for indicating packet origination.
	Turn off pending_ack flag in Build_Header when sending ACKs.

3.1  29-Apr-86, Edit by VAF
	Add some debugging code for logging TCB servicing.
	Remove device restart stuff from here.
	Make retransmission timeout shorter and distinct from connection
	timeout.

3.0  22-Apr-86, Edit by VAF
	Phase II of flushing XPORT - use $FAO for output formatting.
	Remove much of the VMS-specific code to IOUTIL module.

2.9  21-Apr-86, Edit by VAF
	Open timeout is now split into Active (short) and Passive (*long*)

2.8  18-Apr-86, Edit by VAF
	Checksum routine shouldn't do byteswap on IP addresses...

2.7  17-Apr-86, Edit by VAF
	Log segment when doing retransmission.
	Increase probe timer to 1 minute, connection timeout to 10 minutes.

2.6  11-Apr-86, Edit by VAF
	Specifiy and do not exceed a minimum retransmission time of 1 sec.
	Set initial retransmission time to 1 sec to speed things up when
	ARP drops initial connection packets.

2.5   8-Apr-86, Edit by VAF
	Make INACTIVATE_TCB and SET_TCB_STATE into routines. The Macros were
	a little obscure to read and didn't save much (these events do not
	happen often enough for the routine-calling overhead to matter).

2.4   7-Apr-86, Edit by VAF
	First shot at more general logging facility.

2.3  24-Mar-86, Edit by VAF
	Start adding ICMP support.
	Many small changes between previous entry and this one.

2.2  17-Mar-86, Edit by VAF
	Redo connection timeouts, add connection probing code.
	Move a lot of code here from USER.BLI.

2.1   7-Mar-86, Edit by VAF
	Phase I of flushing XPORT: replace all $XPO... calls with real RMS
	calls. Phase II will flush all of the XPORT string handling...

2.0  21-Feb-86, Edit by VAF
	Flush "known_hosts" table, add hooks for new hostname module.
	Rename most logicals from TCP$xxx to INET$xxx.
	Flush "myinternetnames" crud, my_internet_adrs, etc.

*** End CMU change log ***

1.0	[10-15-81] stan
	original version

1.2	[5-5-83] stan
	vms operator interface routines

1.3	[5-20-83] stan
	error processor routine.

1.4	[6-12-83] stan
	Support for SMTP (Simple Mail Transfer Protocol) as a weel-known server.

1.5	[6-13-83] stan
	During connection timeout processing: check if process owning connection
	has sub-processes.  If true, then update connection timeout as this
	could be an ftp cmd connection attempting to timeout during a LONG, SLOW
	data transfer.

1.6	[7-15-83] stan
	modified global literal definitions to be byte-size.

1.7	[9-13-83] stan
	rewrote routine "read_known_hosts_file" to clean up code and utilize the
	host-alias table.  Alias no longer utilize an entry in the known-hosts
	table.

1.8	[5-1-85] noelan olson
	Replace obsolete system services.

1.9	[5-30-85] noelan olson
	Use a table to store multiple internet addresses for the purpose of
	gatewaying between networks.
*/


//sbttl "Module Environment & Definition."

#if 0
MODULE TCP(IDENT="8.2",LANGUAGE(BLISS32),
	   ADDRESSING_MODE(EXTERNAL=LONG_RELATIVE,
			   NONEXTERNAL=LONG_RELATIVE),
	   LIST(REQUIRE,ASSEMBLY,OBJECT,BINARY),
	   OPTIMIZE,OPTLEVEL=3,ZIP)=
#endif

// Include System libraries.

#include <starlet.h>	// VMS system definitions
#include <cmuip/central/include/netxport.h>	// Bliss transportable library defs.
#include <cmuip/central/include/neterror.h>	// Network error codes
#include <cmuip/central/include/netcommon.h>// Common decls
#include "netvms.h"	// Include Local OS Dependent stuff.
#include <cmuip/central/include/nettcpip.h>	// TCP/IP protocols

#include "tcp.h"			// TCP related defintions.
#include "tcpmacros.h"		// Local (tcp) Macro defintions.
#include "structure.h"		// Structure Definitions
#include "snmp.h"			// Simple Network Management Protocol
#include <tcpmod.h>		// Modification history/version



//SBTTL "External Routines"

// MACLIB.MAR

extern int   Time_Stamp(void);
extern  Calc_CheckSum();
extern void     MovByt();
extern  void    SWAPBYTES();
extern  void    CQ_ENQUEUE();
extern  void    CQ_DEQUEUE();
extern  void    CQ_DEQCOPY();

// IOUTIL.BLI

extern  void    FATAL_FAO();
extern  void    OPR_FAO();
extern  void    LOG_FAO();
extern  void    LOG_OUTPUT();
extern  void    ASCII_Dec_Bytes();
extern  void    ASCII_Hex_Bytes();

// USER.BLI

extern void     USER$Post_IO_Status();
extern     USER$Err();
extern void     TCP$KILL_PENDING_REQUESTS();

// TCP_TELNET.BLI

extern  void    TELNET_INPUT();
extern  void    TELNET_OUTPUT();

// TCP_USER.BLI

extern void    TCP$Deliver_User_Data();

// TCP_MECH.BLI

extern void    TCB$Delete();

// MEMGR.BLI

extern     MM$Seg_Get();
extern void    MM$Seg_Free();
extern     MM$QBlk_Get();
extern void    MM$QBlk_Free();
extern void    MM$Uarg_Free();
extern     MM$Get_Mem();

// TCP_SEGIN.BLI

extern  void    SEG$Log_Segment();
extern  void    SEG$Timeout_Syn_Wait_List();

// IP.BLI

extern     IP$Send();
extern     Gen_Checksum();

// NMLOOK.BLI

extern    NML$CANCEL();



//SBTTL "External Literals & data segments"

#if 0
extern signed long LITERAL

// TCP function codes.

    M$INTERNAL:	UNSIGNED(8),
    U$OPEN:	UNSIGNED(8),
    U$CLOSE:	UNSIGNED(8),
    M$Cancel:	UNSIGNED(8);
#endif

// External Data

extern signed long
    Default_MSS,		// Default MSS
    MAX_RECV_DATASIZE,		// Max segment size
    MAX_PHYSICAL_BUFSIZE,	// Max size of device receive buffer
    MIN_PHYSICAL_BUFSIZE,	// Minimum size of device send buffer
    Log_State,
    Act_State;
extern    struct TCP_MIB_struct * TCP_MIB;   // TCP management Information Block


//SBTTL "Global Data Declarations."

unsigned long
CONN_TIMEVAL =CONN_TIMEOUT;
signed long
    TCPIPID,			// current IP ID for TCP

// TCP System Statistics

    TS$UIR = 0,	// user io requests
    TS$ACO = 0,	// active connections opened.
    TS$PCO = 0,	// passive connections opened.
    TS$DBX = 0,	// user data bytes xmitted.
    TS$DBR = 0,	// user data bytes received.
    TS$SX = 0,	// segments sent to IP.
    TS$SR = 0,	// segments received from IP.
    TS$BADSEQ = 0,	// segments dropped by Queue_Network_Data
    TS$ABORT_DROPS  = 0,// Segments dropped because TCB aborted
    TS$QFULL_DROPS  = 0,// Segments dropped because TCB NR queue full
    TS$SEG_BAD_CKSUM = 0, // segs dropped on bad checksum
    TS$DUPLICATE_SEGS = 0, // duplicate segments.
    TS$RETRANS_SEGS = 0,// Retransmitted segments
    TS$RPZ_RXQ = 0,	// Number of times RX queue repacketized
    TS$OORW_SEGS = 0,	// Out Of Recv Window segments
    TS$FUTURE_RCVD = 0,	// Segments in window but beyone RCV.NXT
    TS$FUTURE_DUPS = 0,	// Future segments that were duplicates
    TS$FUTURE_USED = 0,	// Number of future segments used
    TS$FUTURE_DROPPED = 0, // Number of future segments dropped
    TS$SERVERS_FORKED = 0, // well-known servers forked.

    ACK_THRESHOLD  = 1024,  // accumulate resources before window update.
    WINDOW_DEFAULT  = 8192, // receive window.
    TCPTTL	 = 32,	// Default TCP TTL
    TELNET_SERVICE  = 0, // Nonzero if internal TELNET server enabled
    Keep_Alive	 = 0,	// NonZero if Keep alives wanted
    FQ_MAX  = 0;	// Max segments on future queue, per TCB

// I/O Status Block

//    IO_Status: IO_Status_Blk,



// Common macro used by all to send a TCP segment to IP.


#define    SEND_SEG(TCB,SEGADDR,SEGSIZE,DELFLAG,BUFPTR,BUFSIZE,XTIME) \
	TCPIPID = TCPIPID+1; \
	IP$SEND(TCB->Local_Host,TCB->Foreign_Host,TCPTOS,TCPTTL, \
		SEGADDR,SEGSIZE,TCPIPID,TCPDF,DELFLAG,TCP_Protocol, \
		BUFPTR,BUFSIZE)
#define    SEND_SEG1(TCB,SEGADDR,SEGSIZE,DELFLAG,BUFPTR,BUFSIZE,XTIME) \
	{ \
	if (XTIME == 0) \
	    TCB->Probe_time = PROBE_IVAL + Time_stamp(); \
	else \
	    TCB->Probe_time = PROBE_IVAL + XTIME; \
	SEND_SEG(TCB,SEGADDR,SEGSIZE,DELFLAG,BUFPTR,BUFSIZE) \
	}
#define    SEND_SEG0(TCB,SEGADDR,SEGSIZE,DELFLAG,BUFPTR,BUFSIZE,XTIME) \
	{ \
	SEND_SEG(TCB,SEGADDR,SEGSIZE,DELFLAG,BUFPTR,BUFSIZE) \
	}


void TCP$SET_TCB_STATE(struct tcb_structure * TCB, long S)
// Set the State of a TCB, Old state ==> last_state.
    {
    signed long
	LS;

// If changing from Lookup state, kill pending requests
//    NOINT;
    if (((TCB->NMLook_Flag) && (TCB->State == CS$NAMELOOK)))
	{
	TCB->NMLook_Flag = FALSE;
	NML$CANCEL(TCB, 0, 0);
	};
//    OKINT;

    if ($$LOGF(LOG$TCBSTATE))
	LOG$FAO("!%T TCB !XL state change from !SL to !SL!/",
		0,TCB,TCB->State,S);
    LS = TCB->Last_State = TCB->State;
    TCB->State = S;

    // SNMP accounting
    if ((S == CS$SYN_SENT) && ((LS == CS$CLOSED) || (LS == CS$NAMELOOK)))
	TCP_MIB->MIB$tcpActiveOpens = TCP_MIB->MIB$tcpActiveOpens + 1;

    if ((LS == CS$LISTEN) && (S == CS$SYN_RECV))
	TCP_MIB->MIB$tcpPassiveOpens = TCP_MIB->MIB$tcpPassiveOpens + 1;

    if ((S == CS$ESTABLISHED) || (S == CS$CLOSE_WAIT))
	TCP_MIB->MIB$tcpCurrEstab = TCP_MIB->MIB$tcpCurrEstab + 1;

    if ((LS == CS$ESTABLISHED) || (LS == CS$CLOSE_WAIT))
	TCP_MIB->MIB$tcpCurrEstab = TCP_MIB->MIB$tcpCurrEstab - 1;

    if ((S == CS$CLOSED))
	{
	if ((LS == CS$SYN_SENT) || (LS == CS$SYN_RECV))
	   TCP_MIB->MIB$tcpAttemptFails = TCP_MIB->MIB$tcpAttemptFails + 1;
	if ((LS == CS$ESTABLISHED) || (LS == CS$CLOSE_WAIT))
	   TCP_MIB->MIB$tcpEstabResets = TCP_MIB->MIB$tcpEstabResets + 1;
	};
    };

void TCP$INACTIVATE_TCB(struct tcb_structure * TCB,long err)

// Mark a TCB as inactive & set the user error code in the TCB.  When user
// accesses this connection the reason for the inactivation is returned
// instead of connection-does-not-exist error.

    {
    if ((TCB->State != CS$INACTIVE))
	{
        signed long
            MNLQs_Outstanding;

        if ((MNLQs_Outstanding = NML$Cancel(TCB,0,0)) > 0)
            {
            XLOG$FAO(LOG$TCPERR,"!%T TCB !XL inactivated with !ZB// NQE outstanding!/",
                0,TCB,MNLQs_Outstanding);
            XLOG$FAO(LOG$TCP,"!%T TCB !XL inact. with !ZB NQEs outstanding!/",
                0,TCB,MNLQs_Outstanding);
            };

	TCP$SET_TCB_STATE(TCB,CS$INACTIVE);
	XLOG$FAO(LOG$TCBSTATE,
		 "!%T TCB !XL inactivated, reason=!SL!/",0,TCB,err);
	TCB->Inactive_Code = err;
	TCB->Inactive_Timeout = Time_Stamp() + Max_Seg_LifeTime/2;
	}
    else
	XLOG$FAO(LOG$TCPERR,"!%T TCB !XL inactive in TCP$Inactivate_TCB!/",0,TCB);
    };

//SBTTL "TCP$CLOSE - Close a network connection"
/*

    Initiate close of a TCP connection.
    Returns:
	    0 on success, close in progress, TCB state advanced.
	non-0 on failure, error code to return to user.
*/

    TCP$Send_CTL();

TCP$TCB_CLOSE(struct tcb_structure * * TCBPT)  // check
    {
      struct tcb_structure * TCB;
    TCB = *TCBPT;

// Cancel any name lookup that might be in progess

    NOINT;
    if (TCB->NMLook_Flag)
	{
	TCB->NMLook_Flag = FALSE;
	NML$CANCEL(TCB, 0, 0);
	};
    OKINT;

    switch (TCB->State)
      {
    case CS$NAMELOOK: case CS$LISTEN: case CS$SYN_SENT:
	{
	TCP$KILL_PENDING_REQUESTS(TCB,NET$_CC); // Error: Connection Closing.
	TCB$Delete(TCB);
	*TCBPT = 0;
	};
    break;

    case CS$SYN_RECV:
	Send_Fin(TCB);
	break;

    case CS$ESTABLISHED:
	{

// If all send data has been sent then send a "FIN" OTHERWISE flag we need
// to send a "FIN".  TCP$Send_Data rtn will see the flag after all data has been
// sent.

	if (TCB->SND_Q_count > 0)
	  TCB->Pending_Close = TRUE;
	else
	    {
	    Send_FIN(TCB);
	    TCP$SET_TCB_STATE(TCB,CS$FIN_WAIT_1);
	    };
	};
	break;

    case CS$FIN_WAIT_1: case CS$CLOSING: case CS$FIN_WAIT_2: case CS$LAST_ACK:
      //~~~	RETURN 0;		// Close in progress
      return NET$_CC;		//~~~ Already closed
      break;

    case CS$TIME_WAIT:
	return NET$_CC;		// Already closed
	break;

    case CS$CLOSE_WAIT:
	{

// If user data remains then flag a FIN needs to be sent after ALL the
// user data has been sent.

	if (TCB->SND_Q_count > 0)
	    {
	    TCB->Pending_Close = TRUE;
	    TCP$SET_TCB_STATE(TCB,CS$CLOSING);
	    }
	else
	    {
	    Send_FIN(TCB);
 	    TCP$SET_TCB_STATE(TCB,CS$LAST_ACK);
	    };
	}
	break;
    default:
     return 0;
     };


//SBTTL "Send_TCP_Options - Set TCP options when sending SYN segment"

#define    SEND_OPT_SIZE 4		// Total number of bytes of options we set
#define    SEND_OPT_OFFSET (SEND_OPT_SIZE/4) // TCP header offset for it

void Send_TCP_Options(TCB,SEG)
     struct tcb_structure * TCB;
     struct SEGMENT_STRUCTURE * Seg;
{
  extern IP_IsLocal();
  struct TCP$OPT_BLOCK * OPTR;

// Point at the start of the TCP data

    OPTR = Seg->SH$DATA;

// Insert the maximum segment size option

    OPTR->TCP$OPT_KIND = TCP$OPT_KIND_MSS;
    OPTR->TCP$OPT_LENGTH = TCP$OPT_LENGTH_MSS;
//    OPTR->TCP$OPT_DWORD = MAX_RECV_DATASIZE;
    IF IP_IsLocal (TCB [Foreign_Host]) == -1
	THEN OPTR [TCP$OPT_DWORD] = Default_MSS
	else OPTR [TCP$OPT_DWORD] = Max_Recv_DataSize;

    SWAPWORD(OPTR->TCP$OPT_DWORD);

// Insert other options here. Be sure to update SEND_OPTION_SIZE and to make
// sure that the maximum TCP header size is not exceeded.

    };

//SBTTL "Compute_RTT: Compute Round Trip Time"
/*

Function:

	Check if "ACK" segment acks any segments currently on the retransmission
	queue. If so then delete the segment from the queue.

Inputs:

	TCB = TCB pointer.

Implicit Inputs:

	TCB->Round_Trip_Time & TCB->Calculated_RTO.

Outputs:

	Time for next retransmission

Side Effects:

	None.

*/

signed long	 TCP$Compute_RTT(struct tcb_structure * tcb)
    {

    LITERAL
	ALPHA = 90,			// Smoothing constant, per RFC973, pg 41
	BETA = 150,			// Delay variance factor
	ALPHACOMP = (100 - ALPHA),	// "Compliment" of ALPHA (i.e. 1 - ALPHA)
	MINRTT = 5,			// Minimum reasonable RTT
	MINSRTT = 10;			// Minimum smoothed RTT

    signed long
	newrto	:	UNSIGNED,
	newrtt	:	UNSIGNED,
	crtt	:	UNSIGNED,
	oldrtt	:	UNSIGNED ;

// Compute round trip time for adaptive retransmission.

    oldrtt = TCB->round_trip_time;
    crtt = MAXU((Time_Stamp() - TCB->Xmit_Start_Time), MINRTT);
//    crtt = Time_Stamp() - TCB->Xmit_Start_Time ;

// Compute smoothed round trip time, see RFC793 (TCP) page 41 for details.

    if ((TCB->RX_Count == 0))
	{
	newrtt = (((oldrtt * ALPHA) + (crtt * ALPHACOMP)) / 100) ;
	}
    else
	{
	newrtt = oldrtt ;
	} ;

    newrto = (BETA * newrtt) / 100;
    TCB->round_trip_time = newrtt;
    TCB->calculated_rto = MINU(MAX_RT_TIMEOUT,MAXU(newrto,MIN_RT_TIMEOUT));

    XLOG$FAO(LOG$TCP,
	    "!%T Compute_RTT: TCB=!XL, Prev SRTT=!UL, Cur RTT=!UL, New SRTT=!UL, RTO=!UL!/",
 		 0, TCB, oldrtt, crtt,
		TCB->round_trip_time, TCB->calculated_rto);

    RETURN (NewRTO) ;

    } ;

//SBTTL "Check Retransmission Queue"
/*

Function:

	Scan this TCB's retransmission list.  If any segments have timed out
	then retransmit the segment. If a segment has been on the queue too
	long, then timeout the connection.
	N.B. Currently only considers the first entry on the RX queue.

Inputs:

	TCB = TCB pointer.

Outputs:

	Time of next pending retransmission (returns very large value if no
	segments on retransmission queue)

Side Effects:

	Segments on the retransmission queue maybe retransmitted.
	TCB may be inactivated if RX timeout has occurred.
*/

FORWARD ROUTINE
 void    Build_Header;

TCP$Check_Rexmit_Queue(struct tcb_structure * tcb)
    {
    signed long
	now	:	UNSIGNED,
	nxtime	:	UNSIGNED,
	Delta: UNSIGNED,
	Tmp;

    Now = Time_Stamp();		// Get the time.

// See if anything is on the queue

    if ((TCB->SRX_Q_count > 0) || (TCB->RX_CTL != 0))
	{

// See if this connection has timed-out.

	if (TCB->RX_Timeout LSSU Now)
	    {			// Max RX exceeded - conn. timeout
	    XLOG$FAO(LOG$TCBSTATE,"!%T TCB !XL killed by RX timeout!/",0,TCB);
	    TCP$KILL_PENDING_REQUESTS(TCB,NET$_cto);
	    TCP$Inactivate_TCB(TCB,NET$_CTO);
	    RETURN TCB->Inactive_timeout;
	    };

// See if it is time to retransmit.

	if (TCB->RX_Timer LSSU Now)
	    {
	    signed long
		optsize,
		EOLF,SYNF,ACKF,FINF,
		Datasize,Dataptr,
		SEQsize,Segsize,
		Bufsize,Buf,
		struct Segment_Structure * Seg;

// Set segment initial transmit time if this is the first time

	    if (TCB->RX_Count == 0)
		{
		TCB->Xmit_Start_Time = Now;
		TCB->MAX_EFF_Data_Size = TCB->MAX_Seg_Data_Size ;
		} ;

	    TCB->RX_Count = TCB->RX_Count + 1;
!	    if (TCB->RX_COUNT > 0)
!		{
!		TCB->MAX_EFF_Data_Size = TCB->MAX_EFF_Data_Size / TCB->RX_COUNT ;
!		TCB->MAX_EFF_Data_Size = MAX(TCB->MAX_EFF_Data_Size, 512) ;
!		} ;

	    ts$retrans_segs = ts$retrans_segs+1;

// Compute new retrans time out based on old round trip time plus fudge factor

!	    Delta = TCB->Round_Trip_time + 
!		    ( (TCB->Round_Trip_Time * TCB->RX_Count) / 2);
!	    Delta = MAXU(MINU(Max_RT_TimeOut,Delta),Min_RT_Timeout);
!	    Delta = MINU(MAXU(TCP$Compute_RTT(TCB),Min_RT_Timeout) *
!			 TCB->RX_count, Max_RT_Timeout) ;
	    Delta = MINU(MAXU(TCP$Compute_RTT(TCB),Min_RT_Timeout),
			Max_RT_Timeout) ;
	    now = Time_Stamp();
	    TCB->RX_Timer = now + Delta;
	    XLOG$FAO(LOG$TCBCHECK,
		     "!%T Check_rexmit TCB !XL delta !UL!/",0,TCB, Delta);

// Now, build and send a segment.
// Calculate buffer size & get control flag values

	    SYNF = (TCB->RX_CTL == M$SYN) || (TCB->RX_CTL == M$SYN_ACK);
	    ACKF = TCB->RX_CTL == M$SYN_ACK;
	    FINF = (TCB->RX_CTL == M$FIN);
	    if (SYNF)
		optsize = SEND_OPT_SIZE
	    else
		optsize = 0;
!	    Datasize = MIN(TCB->SRX_Q_count,TCB->MAX_Seg_Data_Size);
	    Datasize = MIN(TCB->SRX_Q_count, TCB->MAX_EFF_Data_Size);
	    Segsize = Datasize + optsize + TCP_Header_Size;

// Select network buffer size & allocate it

	    bufsize = Device_Header+IP_Hdr_Byte_Size+.Segsize;
	    SELECTONE TRUE OF
	    SET
	    [Bufsize <= MIN_PHYSICAL_BUFSIZE]:
		bufsize = MIN_PHYSICAL_BUFSIZE;
	    [Bufsize <= MAX_PHYSICAL_BUFSIZE]:
		bufsize = MAX_PHYSICAL_BUFSIZE;
	    [OTHERWISE]:
		bufsize = segsize + device_header + ip_hdr_byte_size;
	    TES;
	    Buf = MM$Seg_Get(Bufsize);
	    Seg = Buf + device_header + ip_hdr_byte_size;
	    Dataptr = Seg->SH$Data;

// Set control flags and options for the segment

	    Seg->SH$Control_Flags = 0;
	    Seg->SH$C_ACK = ACKF;
	    if (SYNF)
		{
		SYNF = Seg->SH$C_SYN = TRUE;
		Send_TCP_Options(TCB,Seg);
		Dataptr = Dataptr + SEND_OPT_SIZE;
		};
	    if (FINF)
		Seg->SH$C_FIN = TRUE;
	    EOLF = TCB->SND_Push_Flag AND
!		   ((TCB->SND_PPtr - TCB->RX_SEQ) > 0);
		   (TCB->SND_PPtr GTRU TCB->RX_SEQ);

// Now, insert the retransmission data, if any.

	    XLOG$FAO(LOG$TCP,
		"!%T !UL RX TCB !XL,PTR=!XL,RXQ=!XL/!XL,CNT=!SL,CTL=!SL,new RXTO=!SL!/",
		0,now,TCB,TCB->SRX_Q_DEQP,TCB->SRX_Q_BASE,TCB[SRX_Q_}],
		TCB->RX_count,TCB->RX_CTL,TCB->RX_Timer);
	    if (Datasize > 0)
		{
		CQ_DEQCOPY(TCB->SRX_Q_Queue,Dataptr,Datasize);
		TCP_MIB->MIB$tcpRetransSegs = TCP_MIB->MIB$tcpRetransSegs + 1;
		};
// Build the rest of the header

	    Build_Header(TCB,Seg,Segsize,Datasize,TCB->RX_SEQ,EOLF,
			 FALSE,optsize / 4);

// Send the segment to IP

	    Send_Seg1(TCB,Seg,Segsize,TRUE,Buf,Bufsize);

	    RETURN TCB->RX_Timer;	// Next time retransmit for this TCB
	    };
!	RETURN TCB->RX_Timer;	// Next time retransmit for this TCB
	};

//    RETURN %X"7FFFFFFF";	// Nothing on retransmission queue
    RETURN (Now + (20*CSEC));	// Nothing on retransmission queue

    };

//SBTTL "Queue Segment for ReTransmission."
/*

Function:

	Place segment on this TCB's retranmission queue.  Actually a Queue
	block is enqueued which in turn points to the segment & contains
	the timeout & last sequence # of the segment.  If the segment has
	never been transmitted then force the segment to time-out when it
	reaches the head of the RT queue.

Inputs:

	TCB = points to current TCB
	Buf = segment buffer ptr, segment is sub-structure of buffer.
	bufsize = buffer size in bytes.
	Seg = pointer to segment structure
	size = size in bytes of segment
	Xmit Count = # of times segment has been transmitted.

Implicit Inputs:

	TCB->Snd_Nxt has been updated to point to next byte past end of this
	segment.  Used to flag end of segment field in queue block, aids in
	ACKing the segment on the ReTransmission queue.

Outputs:

	None.

Side Effects:

	Segment is on this TCB's retransmission queue (RT_Qhead) & ready
	to be sent directly to IP should the timeout value expires.
	Segment round trip timer is started.
*/

Rexmit_Enqueue(TCB,SEQsize,CTLTYPE) : NOVALUE (void)
    {
    MAP
	struct tcb_structure * tcb;
    REGISTER
	struct Queue_Blk_Structure * QB(QB_RT_Fields);
    signed long
	now: UNSIGNED;

    now = Time_Stamp();		// get current time

// If this is the first data item on queue, set retransmit pointer

    XLOG$FAO(LOG$TCP,%STRING(
	     "!%T RX-ENQ: Time=!UL, TCB=!XL, RXTO=!SL, MXTO=!SL!/",
	     "!%T RX-ENQ: PTR=!XL/!XL, SEQ=!XL, SIZ=!SL, CNT=!SL, CTL=!SL, RXC=!SL!/"),
	     0,Now,TCB,TCB->RX_Timer,TCB->RX_Timeout,
	     0,TCB->SRX_Q_DEQP,TCB->SND_Q_DEQP,TCB->RX_SEQ,SEQsize,
	     TCB->SRX_Q_count,TCB->RX_CTL, TCB->RX_COUNT);

    if ((TCB->SRX_Q_count == 0))
	{
	TCB->SRX_Q_DEQP = TCB->SND_Q_DEQP;

// If really the first thing on the queue, set sequence # and timers.
!~~~ Questionable code here, using RX_CTL seems like a kluge
// (Should probably use RX_COUNT - HWM)

!	if (TCB->RX_CTL == 0)
!	if ((TCB->RX_SEQ LSSU TCB->SND_NXT))
	if (TCB->RX_Count == 0)
	    {
	    TCB->RX_SEQ = TCB->SND_NXT;
	    TCB->RX_Count = 1;
	    TCB->Xmit_Start_Time = Now;	// Start round-trip-timer
	    TCB->RX_Timer = Now + TCB->Calculated_RTO;
	    TCB->RX_Timeout = Now + RX_TIMEVAL; // Max time on RX queue
	    };
	};

// Add data count for the new seg and set flags

//    TCB->RX_Timeout = Now + RX_TIMEVAL; // Max time on RX queue
    TCB->SRX_Q_count = TCB->SRX_Q_count + SEQsize;
    TCB->RX_CTL = CTLTYPE;
    XLOG$FAO(LOG$TCP,%STRING(
	     "!%T RX-ENQ: Time=!UL, TCB=!XL, RXTO=!SL, MXTO=!SL!/",
	     "!%T RX-ENQ: PTR=!XL/!XL, SEQ=!XL, SIZ=!SL, CNT=!SL, CTL=!SL, RXC=!SL!/"),
	     0,Now,TCB,TCB->RX_Timer,TCB->RX_Timeout,
	     0,TCB->SRX_Q_DEQP,TCB->SND_Q_DEQP,TCB->RX_SEQ,SEQsize,
	     TCB->SRX_Q_count,TCB->RX_CTL, TCB->RX_COUNT);
    };

//SBTTL "Debug rtn: Display Specified TCB"
/*

Function:

	display contents of a TCB
	N.B. LOG_TCBDUMP is assumed to be on in log_state.
Inputs:

	TCB = address of a TCB data structure.

Outputs:

	none.

Side Effects:

	None.
*/
void TCP$Dump_TCB ( struct tcb_structure * tcb ) (void)
    {
    signed long
	now	:	UNSIGNED,
	rxtime	:	UNSIGNED,
	stastr,
	DESC$STR_ALLOC(fhstr,20),
	DESC$STR_ALLOC(lhstr,20);

    if (TCB->SRX_Q_count > 0)
	{
	now = Time_Stamp();
	rxtime = TCB->RX_Timer;
	}
    else
	{
	rxtime = 0;
	now = 0;
	};
    SELECTONE TCB->State OF
	SET
	[CS$CLOSED]: stastr = %ASCID"Closed(CMU).";
	[CS$LISTEN]: stastr = %ASCID"Listen.";
	[CS$SYN_SENT]: stastr = %ASCID"SYN_SENT.";
	[CS$SYN_RECV]: stastr = %ASCID"SYN_RECV.";
	[CS$ESTABLISHED]: stastr = %ASCID"Established.";
	[CS$FIN_WAIT_1]: stastr = %ASCID"FIN_Wait_1.";
	[CS$FIN_WAIT_2]: stastr = %ASCID"FIN_Wait_2.";
	[CS$TIME_WAIT]: stastr = %ASCID"Time_Wait.";
	[CS$CLOSE_WAIT]: stastr = %ASCID"Close_Wait.";
	[CS$CLOSING]: stastr = %ASCID"Closing.";
	[CS$LAST_ACK]: stastr = %ASCID"Last_ACK.";
	[CS$RESET]: stastr = %ASCID"Reset(CMU).";
	[CS$INACTIVE]: stastr = %ASCID"Inactive(CMU).";
	[CS$NAMELOOK]: stastr = %ASCID"NameLook(CMU).";
	[OTHERWISE]: stastr = %ASCID"Unknown state."
	TES;

    ASCII_Dec_Bytes(fhstr,4,TCB->foreign_host,fhstr->DSC$W_LENGTH);
    ASCII_Dec_Bytes(lhstr,4,TCB->local_host,lhstr->DSC$W_LENGTH);

    LOG$FAO(%STRING("!_PID:!_!XL!_State:!AS(!XL)!/",
		    "!_FH:!_!AS!_FP:!_!XL (!UL)!/",
		    "!_LH:!_!AS!_LP:!_!XL (!UL)!/",
		    "!_SND.WND: !UL!_RCV.WND: !UL!/",
		    "!_SND.NXT: !XL (!UL)!_RCV.NXT: !XL (!UL)!/",
		    "!_SND.UNA: !XL (!UL)!/",
		    "!_RX time: !UL (now + !UL)!/"),
		    TCB->User_ID,stastr,TCB->State,
		    fhstr,TCB->foreign_port,TCB->foreign_port,
		    lhstr,TCB->local_port,TCB->local_port,
		    TCB->snd_wnd,TCB->rcv_wnd,
		    TCB->snd_nxt,TCB->snd_nxt,TCB->rcv_nxt,TCB->rcv_nxt,
		    TCB->snd_una,TCB->snd_una,rxtime,rxtime-.now);
    };



//SBTTL "Service-Connections - Look for things to do."
/*

Function:

	Examine a known connections & look for things which need to be
	done.  

Inputs:

	None.

Outputs:

	Next time that Service_Connections needs to be called.

Side Effects:

	Data may be sent or delivered to the user.  Connection may be
	deleted due to timeouts.  Segment retransmission queue is examined.
*/

FORWARD ROUTINE
    TCP$Send_Data,
    TCP$Enqueue_ACK: NOVALUE,
    TCP$Send_ACK: NOVALUE,
    Do_Probe: NOvalue;

Check_TCB ( struct tcb_structure * tcb , Idx , now : UNSIGNED, nxtime : UNSIGNED)
    {

    if ($$LOGF(LOG$TCBCHECK+LOG$TCBDUMP))
	LOG$FAO("!%T Servicing TCB !XL (now=!XL,nxtime=!XL)!/",0,TCB,
		.now,..nxtime);

// TCB sanity check
    if ((TCB == 0))
	RETURN 0 ;

    if ($$LOGF(LOG$TCBDUMP))
	TCP$Dump_TCB(TCB);

    SELECTONE TRUE OF
    SET
 
 // If connection is inactive & the user has not accessed the connection
 // within the timeout period then delete the connection.
 
    [(TCB->State == CS$INACTIVE) AND
     (TCB->Inactive_Timeout LSSU NOW)]:
	{
	TCP$KILL_PENDING_REQUESTS(TCB,TCB->inactive_code);
	XLOG$FAO(LOG$TCBSTATE,"!%T Deleting inactive connection !XL!/",
		 0,TCB);
	TCB$Delete(TCB);
	};

 // If the connection has not received any packets at all, despite our
 // probes for too long, then we inactivate it here.

    [(TCB->State != CS$INACTIVE) AND
     ((TCB->Connection_timeout LSSU now) && (TCB->Connection_Timeout != 0)) OR
     ((TCB->User_timeout != 0) && (TCB->User_timeout LSS now))]:
	{
	TCP$KILL_PENDING_REQUESTS(TCB,NET$_cto);
	SELECTONE TCB->State OF
	SET
	[CS$SYN_SENT, CS$SYN_RECV, CS$ESTABLISHED, CS$FIN_WAIT_1,
	 CS$FIN_WAIT_2, CS$CLOSE_WAIT]:
	    Send_RST(TCB);
	TES;
	if ($$LOGF(LOG$TCBSTATE))
	    {
	    IF (TCB->Connection_timeout LSSU now) && 
		(TCB->Connection_Timeout != 0) THEN
		LOG$FAO("!%T Conn !XL inactivated - conn timeout!/",
			0,TCB)
	    else
		LOG$FAO("!%T Conn !XL inactivated - user timeout!/",
			0,TCB);
	    };
	TCP$KILL_PENDING_REQUESTS(TCB,NET$_cto);
	TCP$Inactivate_TCB(TCB,NET$_cto);
	};

// Is there a pending IO function being held until a network event occurs?
// IF so, check the timer & clobber(inactivate) the connection.

    [TCB->pending_io && (TCB->function_timer LSSU now)]:
	{
	TCB->pending_io = false;
	if (TCB->Curr_User_Function == M$INTERNAL)
	    {

// For internal TCB"s (i.e. TVT"s), just call the timeout handler.

	    (TCB->Timeout_Routine)(TCB);
	    }
	else
	    {
	    if (TCB->argblk != 0)
		USER$Err(TCB->argBlk,NET$_fto);

// Process According to TCP function requested.

	    SELECTONE TCB->Curr_User_Function OF
	    SET
	    [u$open,u$close,m$cancel]:
		{
		TCP$KILL_PENDING_REQUESTS(TCB,NET$_FTO);
 
// RESET just in case.

		SELECTONE TCB->State OF
		SET
		[CS$SYN_SENT TO CS$FIN_WAIT_2,CS$CLOSE_WAIT]:
		    Send_RST(TCB);
		TES;
		XLOG$FAO(LOG$TCBSTATE,
		    "!%T Function timeout: FCN=!XL, TCB=!XL!/",
		     0,TCB->Curr_User_Function,TCB);
		TCP$Inactivate_TCB(TCB,NET$_FTO); // Inactivate connection
		};
	    TES;
	    };
	};
 
// Test for time-wait timeout.  If true then delete the connection.
 
    [(TCB->state == CS$TIME_WAIT) AND
     (TCB->Time_Wait_Timer LSSU now)]:
	{
	TCP$KILL_PENDING_REQUESTS(TCB,NET$_TWT);
	XLOG$FAO(LOG$TCBSTATE,"!%T Time-wait expired, conn=!XL!/",
		 0,TCB);
	TCB$Delete(TCB);
	};
 
// Connection is valid, Check retransmission queue & see if we can move
// some data. Better check SYN-wait queue also.
 
    [OTHERWISE]:
	{
	signed long
		Delay	:	UNSIGNED,
		Min_Time:	UNSIGNED,
		RX_Time	:	UNSIGNED ;

!	XLOG$FAO(LOG$DEBUG,
!		 "!%T Rexmit_Queue : count = !SL!/",
!		    0,TCB->SRX_Q_count);
	RX_time = TCP$Check_Rexmit_Queue(TCB);
	XLOG$FAO(LOG$DEBUG,
		 "!%T TCP$Check_Rexmit_Queue : RVal = !XL!/",
		    0,RX_Time);

	SELECT TCB->state OF
	SET
	[CS$ESTABLISHED,CS$CLOSE_WAIT]:
	    {	// Try to send some data
	    XLOG$FAO(LOG$TCBCHECK,
		     "!%T Sending data for TCB !XL!/",0,TCB);
	    TCP$Send_Data(TCB);
	    };
	[CS$ESTABLISHED]:
	    {
	    if ((TCB->ack_Timer LSSU now))
		{
		XLOG$FAO(LOG$TCP,
			 "!%T Sending spontaneous ACK, TCB=!XL!/",
			 0,TCB);
!		TCP$Send_ACK(TCB);
		TCP$Enqueue_ACK(TCB);
		};
	    if ((TCB->probe_time LSSU now) && Keep_Alive)
		DO_Probe(TCB); // Time to send another probe
	    };

	[CS$ESTABLISHED,CS$FIN_WAIT_1,CS$FIN_WAIT_2]:
	    {	// Try to receive some data
	    if (TCB->IS_TVT)
		{
		TELNET_INPUT(TCB);
		TELNET_OUTPUT(TCB);
		}
	    else
		TCP$Deliver_User_Data(TCB);
	    };
	TES;

	XLOG$FAO(LOG$DEBUG,
		 "!%T TCP$Check_Rexmit_Queue : RX_Time = !XL!/",
		    0,RX_Time);
	XLOG$FAO(LOG$DEBUG,
		 "!%T TCP$Check_Rexmit_Queue : delay = !XL!/",
		    0,TCB->SND_Delay_Timer);
	XLOG$FAO(LOG$DEBUG,
		 "!%T TCP$Check_Rexmit_Queue : nxtime = !XL!/",
		    0,..nxtime);

	if ((TCB->state NEQU CS$LISTEN) && (TCB->SND_Q_count GTRU 0))
	    {
	    Delay = MINU((TCB->Round_Trip_Time / 2), Base_RT_Timeout) ;
	    Delay = MAXU(Delay, 33) ;
	    Min_Time = Now + Delay ;
!	    IF ((TCB->SND_Delay_Timer LSSU Now) && (TCB->SND_Q_count > 0))THEN
	    if ((TCB->SND_Q_count > TCB->MAX_EFF_DATA_SIZE))
		Min_Time = TCB->SND_Delay_Timer ;
	    Min_Time = MAXU(Min_Time, Now) ;
	    Min_Time = MINU(RX_Time, Min_Time) ;
!	    nxtime = MINU(..nxtime,RX_Time);
	    nxtime = MINU(..nxtime, Min_Time);
	    XLOG$FAO(LOG$DEBUG,
		     "!%T TCP$Check_Rexmit_Queue : nxtime = !XL!/",
		     0,..nxtime);

	    if (TCB->Pending_ACK)
		{
		XLOG$FAO(LOG$TCP,
		         "!%T Sending pending ACK, TCB=!XL!/",0,TCB);
!		TCP$Send_ACK(TCB) // Give the window update/ACK
		TCP$Enqueue_ACK(TCB) // Give the window update/ACK
		}
	    }
	};
    TES;

    1
    };


//SBTTL "Service-Connections - Look for things to do."
/*

Function:

	Examine all known connections & look for things which need to be
	done.  This rtn is the 3rd major processing rtn called from TCP
	mainline (Start-Network).

Inputs:

	None.

Outputs:

	Next time that Service_Connections needs to be called.

Side Effects:

	Data may be sent or delivered to the user.  Connection may be
	deleted due to timeouts.  Segment retransmission queue is examined.
*/

TCP$Service_Connections (void)
    {
    EXTERNAL ROUTINE
	VTCB_Scan;
    REGISTER
	now	:	UNSIGNED,
	struct tcb_structure * tcb;
    Local
	nxtime	:	UNSIGNED,
	Count,
	IOS : IO_Status_Blk;

// Examine ALL known Connections.

    now = Time_Stamp();
    nxtime = now+daysec;	// Far into the future....

    if ($$LOGF(LOG$TCBCHECK))
	LOG$FAO("!%T !UL Servicing all TCBs, !/",0,now);

    Count = VTCB_Scan ( Check_TCB , now , nxtime );
    XLOG$FAO(LOG$DEBUG,"!%T Service_connections: now=!XL nxtime=!XL!/",0,
			.now,nxtime);

    if ($$LOGF(LOG$TCBCHECK))
	LOG$FAO("!%T !UL Serviced !SL TCBs,!/",0,now,Count);


// Examine Syn wait list for entries that have timed out.

    SEG$Timeout_Syn_Wait_List(now);

// Return calculated next time a TCB needs to be serviced

    RETURN nxtime;
    };

//SBTTL "TCP$SEND_ENQUEUE - Copy send data from user send queue to circular buffer"

TCP$SEND_ENQUEUE(TCB,Bufcount,Buf,PushF) : NOVALUE (void)
    {
    MAP
	struct tcb_structure * tcb;
    signed long
	Usedcount,
	Lbptr,
	Lbcount,
	Copycount;
    Lbptr = ..Buf;
    Lbcount = ..Bufcount;

// Compute amount that the queue can take, enqueue it.

    Usedcount = TCB->SND_Q_count + TCB->SRX_Q_count;
    Copycount = MIN(Lbcount,TCB->SND_Q_Size-.Usedcount);
    XLOG$FAO(LOG$TCP,"!%T SEND-ENQ EQ=!XL,DQ=!XL,RX=!XL,SNQ=!XL/!XL,CNT=!SL,SIZ=!SL,PSH=!SL!/",
	     0,TCB->SND_Q_ENQP,TCB->SND_Q_DEQP,TCB->SRX_Q_DEQP,
	     TCB->SND_Q_BASE,TCB[SND_Q_END],Usedcount,Copycount, PushF);
    $$KCALL(CQ_Enqueue,TCB->SND_Q_Queue,Lbptr,Copycount);

// Update user pointer and counter

    Bufcount = Lbcount - Copycount;
    Buf = Lbptr + Copycount;

// If PUSH was set, then advance the send Push pointer to end of this buffer

    if (PushF)
	{
	TCB->SND_Push_Flag = TRUE;
	TCB->SND_Pptr = TCB->SND_NXT + TCB->SND_Q_count;
	};
    };

//SBTTL "SEND DATA: Segmentize data & send segment over the network."
/*

Function:
	Read user data from send queue (SND_Qhead) & segmentize it.  Send queue
	is structured in the following format:
	Each element of the queue is a queue_block.  Each queue_Block(QB)
	contains information about the user data buffer it points to 
	(size, start address, End Of Letter & Urgent flags).

Inputs:

	TCB = TCB pointer

Implicit Inputs:

	TCB->ASM_BC = # of bytes in current user send buffer left. 1st
			element in TCB->Snd_Qhead.
	TCB->ASM_Ptr = address of user data in current send buffer.
	TCB->max_seg_data_size = max # of data bytes per segment receiver
				 will accept.

Outputs:

	0 if data not sent because of accumulation strategy, else count of
	data octets sent.

Side Effects:

	As much of the users data is sent with respect to the size of
	the send window.

*/

TCP$SEND_DATA(struct tcb_structure * tcb)
    {
    signed long
	struct Segment_Structure * Seg,
	SEQsize,
	struct Queue_Blk_Structure * QB(QB_Send_Fields),
	Now		:	UNSIGNED,
	Delay		:	UNSIGNED,
	Delay_Flag	 = 0,
	Send_Flag	 = 0,
	Useable,
	Bufptr,
	Bufsize,
	SegSize,
	Dcount,
	EOL,
	Urg;

// Get time of day

    Now = Time_Stamp() ;

// If there is anything on the retransmission queue, then don't try to queue
// anything else up ("Nagle Algorithm")
//    if (((TCB->SND_NXT GTRU TCB->SND_UNA) && (NOT TCB->DE_NAGLE)))
!	RETURN 0;

    if (TCB->SRX_Q_count != 0)
	RETURN 0 ;
    if ((TCB->SND_Delay_Timer == 0))
	TCB->SND_Delay_Timer = Now ;
//    if ((TCB->SND_Delay_Timer LSSU Now))
!	{
!	// Timer still active, but maybe we can send anyway
!	IF ((TCB->SND_NXT GTRU TCB->SND_UNA) AND
!	    (NOT TCB->DE_NAGLE) AND
!	    ((TCB->SND_Q_COUNT + TCB->SRX_Q_COUNT) LSSU TCB->MAX_EFF_DATA_SIZE)) THEN
!		RETURN 0 ;		// Nope
!	} ;

    Useable = TCB->SND_UNA + TCB->SND_WND - TCB->SND_NXT ;
    if ((MINU(TCB->SND_Q_COUNT, Useable) GTRU TCB->MAX_EFF_DATA_SIZE))
	Send_Flag = 1 ;
    IF ((TCB->SND_NXT == TCB->SND_UNA) AND
	(MINU(TCB->SND_Q_COUNT, Useable) GEQU (TCB->SND_MAX_WND / 2))) THEN
	Send_Flag = 1 ;
    IF ((TCB->SND_NXT == TCB->SND_UNA) && (TCB->SND_PUSH_FLAG) AND
	(TCB->SND_Q_COUNT LEQU Useable)) THEN
	Send_Flag = 1 ;
    if ((TCB->SND_Delay_Timer GEQU Now))
	Send_Flag = 1 ;
    if ((TCB->SND_NXT GTRU TCB->SND_UNA))
	Send_Flag = 1 ;
    if ((NOT TCB->DE_NAGLE))
	Send_Flag = 1 ;
    if ((TCB->SND_Q_COUNT GEQU TCB->MAX_EFF_DATA_SIZE))
	Send_Flag = 1 ;
    if (((TCB->SND_NXT GTRU TCB->SND_UNA) && (NOT Send_Flag)))
	RETURN 0 ;		// Nope

// If we received a ICMP Source Quench, then see if it's time to try to
// send more data.

    if ((TCB->SQUENCH))
	{
	if ((Now LSSU TCB->SQUENCH_TIMER))
		{			//  Still in Source Quench mode
		RETURN 0 ;
		}
	else
		{
		TCB->SQUENCH = FALSE ;	//  Reset Source Quench flag
		} ;
	} ;

// Generate TCP segments from circular queue until all send requests are
// exhausted or the SEND WINDOW goes to 0.

    Dcount = 0;
    WHILE (TCB->SND_Q_count > 0) && (TCB->SND_WND GTRU 0) DO
	{
	EOL = Urg = FALSE;

// How much user data is avail to send? Up to TCB->max_seg_data_size
// Make sure it fits within the send-window.

!	SEQsize = MIN(TCB->SND_Q_count,TCB->Max_Seg_Data_Size);
	SEQsize = MIN(TCB->SND_Q_count, TCB->Max_EFF_Data_Size);
	SEQsize = MINU(SEQsize,TCB->Snd_Wnd);

// get a buffer large enough to contain: device header, IP, TCP headers and
// data. Round to nearest big buffer size.

	SegSize = SEQsize + TCP_Header_Size;
	bufsize = segsize + device_header + IP_hdr_byte_size;
	SELECTONE TRUE OF
	SET
	[bufsize <= MIN_PHYSICAL_BUFSIZE]:
	    bufsize = MIN_PHYSICAL_BUFSIZE;
	[bufsize <= MAX_PHYSICAL_BUFSIZE]:
	    bufsize = MAX_PHYSICAL_BUFSIZE;
	[OTHERWISE]:
	    bufsize = segsize + device_header + IP_hdr_byte_size;
	TES;

// Get buffer, point at segment, clear flags

	bufptr = MM$Seg_Get(BufSize);
	seg = Bufptr+device_header+IP_hdr_byte_size;
	Seg->SH$Control_Flags = 0;

// Set PUSH and advance PUSH pointer

	if (TCB->SND_Push_Flag)
!	    if ((TCB->SND_Pptr - TCB->SND_NXT) > 0)
	    if ((TCB->SND_Pptr GTRU TCB->SND_NXT))
		{
		EOL = TRUE;
!		if ((TCB->SND_NXT + SEQsize) - TCB->SND_PPtr GTRU 0)
		if (((TCB->SND_NXT + SEQsize) GTRU TCB->SND_PPtr))
		    TCB->SND_Push_Flag = FALSE;
		};

!~~~ Set URG and urgent pointer here?
// Set retransmission info
	
	ReXmit_Enqueue(TCB,SEQsize,0);

// Remove from the send circular queue into the segment

	XLOG$FAO(LOG$TCP,
	   "!%T Sending for TCB !XL,SEQ count=!SL,DQ=!XL,EQ=!XL,SNQ=!XL/!XL!/",
	   0,TCB,SEQsize,TCB->SND_Q_DEQP,TCB->SND_Q_ENQP,
	   TCB->SND_Q_BASE,TCB[SND_Q_}]);
	XLOG$FAO(LOG$TCP,
	   "!%T Sending for TCB !XL, EFF = !SL!/",
	   0, TCB, TCB->MAX_EFF_Data_Size);
	CQ_Dequeue(TCB->SND_Q_Queue,Seg->SH$DATA,SEQsize);

// Deduct from the window and add to statistics

	TCB->SND_WND = TCB->SND_WND - SEQsize;
	ts$dbx = ts$dbx + SEQsize;

// Send the segement to IP (internet Protocol handler).

	Build_Header(TCB,Seg,SegSize,SEQsize,TCB->SND_NXT,EOL,URG,0);

// Advance SND_NXT for this segment

	TCB->snd_nxt = TCB->snd_nxt + SEQsize;

// Send the segment to IP for transmission.

	Send_Seg1(TCB, Seg, Segsize, TRUE, Bufptr, Bufsize);
	ts$sx = ts$sx + 1;
	Dcount = Dcount+.SEQsize;
	TCP_MIB->MIB$tcpOutSegs = TCP_MIB->MIB$tcpOutSegs + 1;

// If we got here by virtue of expired timer, and not enough left to send
// another full sized packet, or window is too small then exit to avoid
// sending runts.
// (Nice in theory, but it doesn't work...)
!	IF (((Delay_Flag) AND
!	     (TCB->Max_SEG_Data_Size GTRU TCB->SND_Q_count)) OR
!	     (TCB->SND_WND LSSU TCB->SND_Q_count)) THEN
!		EXITLOOP ;
	};			!WHILE

// If circular queue is not full, then insert anything we have onto it.
!~~~ Warning: we count on the RX queue being empty here (known above)

    if (TCB->SND_Q_count LSS TCB->SND_Q_Size)
	{
	QB = TCB->SND_Qhead;
	WHILE QB != TCB->SND_Qhead DO
	    {
	    signed long
		NQB,
		struct User_Send_Args * Uargs;

// Enqueue as much data as possible from this user buffer

	    TCP$Send_Enqueue(TCB,QB->SN$SIZE,QB->SN$Data,QB->SN$EOL);

// If the user buffer still has data in it, then we ran out of queue space.

	    if (QB->SN$SIZE != 0)
		EXITLOOP;

// Otherwise, we're done with this buffer. Post the I/O, dequeue the block, and
// advance to the next entry on the queue.

	    Uargs = QB->SN$Uargs;
	    User$Post_IO_Status(Uargs,SS$_NORMAL,QB->SN$Size,0,0);
	    MM$UArg_Free(Uargs);
	    NQB = QB->SN$NEXT;
	    REMQUE(QB,QB);
	    MM$QBLK_Free(QB);
	    QB = NQB;
	    };
	};

// Pending Close & ALL user data sent?
// If true then send a FIN.

    if ((TCB->Pending_Close) && (TCB->SND_Q_count == 0))
	{
	TCB->Pending_Close = FALSE;
	Send_FIN(TCB);

	if (TCB->State == CS$ESTABLISHED)
	    TCP$SET_TCB_STATE(TCB,CS$FIN_WAIT_1)
	else

// Technically, this violates the spec, but we got this way when a CLOSE was
// done in CLOSE-WAIT, so we should really be in LAST-ACK (RFC793 is ambiguous
// on this point).

	    if (TCB->State == CS$CLOSING)
		TCP$SET_TCB_STATE(TCB,CS$LAST_ACK);
	};

    // Reset delay timer for next time
    Now = Time_Stamp() ;
//    Delay = (TCB->Round_Trip_Time / 10) ;
    Delay = (TCB->Round_Trip_Time / 2);
    Delay = MINU(Delay, 100) ;
    Delay = MAXU(Delay, 10) ;
    TCB->SND_Delay_Timer = Now + Delay ;

    RETURN Dcount;
    };


//SBTTL "Send-ACK: Check if we can piggyback ACK or must send a lone ACK"
/*

Function:

	Transmit an ACK.  Check to see if we can send a piggyback ACK on
	an out going data segment or must we send a lone ACK control
	segment.

Inputs:

	TCB - TCB pointer.

Outputs:

	None.

Side Effects:

	An "ACK" control segment maybe sent if there is no data to send
	for this connection or the send window is zero.
*/


TCP$Enqueue_ACK(struct tcb_structure * tcb): NOVALUE (void)
    {
    signed long
	Delay	:	UNSIGNED,
	Now	:	UNSIGNED;

// Queue an ACK up for short term delivery

    XLOG$FAO(LOG$TCP,"!%T TCP$Enqueue_ACK TCB !XL!/",
	  0, TCB);

    TCB[PENDING_ACK] = TRUE ;

// Get Time of Day

    Now = Time_Stamp() ;

    IF ((Now GEQU TCB->Delayed_ACK_Timer) OR
	((TCB->Old_RCV_NXT + (2 * TCB->MAX_EFF_DATA_SIZE)) GEQU TCB->RCV_NXT)) THEN
	{
	TCP$Send_ACK(TCB) ;
!	$DCLAST(ASTADR = TCP$Send_ACK,
!		ASTPRM = TCB);
!	TCB->Delayed_ACK_Timer = Now + Delayed_ACK_Interval;
	Delay = MINU((TCB->Round_Trip_Time / 2), Delayed_ACK_Interval) ;
	Delay = MAXU(Delay, 20);
	TCB->Delayed_ACK_Timer = Now + Delay ;
	} ;
//    else
!	{
!	if (((Now + Delayed_ACK_Interval) (TCB[Delayed_ACK_Timer)))
!		{
!		} ;
!	} ;

    };


//SBTTL "Send-ACK: Check if we can piggyback ACK or must send a lone ACK"
/*

Function:

	Transmit an ACK.  Check to see if we can send a piggyback ACK on
	an out going data segment or must we send a lone ACK control
	segment.

Inputs:

	TCB - TCB pointer.

Outputs:

	None.

Side Effects:

	An "ACK" control segment maybe sent if there is no data to send
	for this connection or the send window is zero.
*/


TCP$SEND_ACK(struct tcb_structure * tcb): NOVALUE (void)
    {

// If user data needs to be sent then piggyback the ACK.
// Otherwise send a ACK control segment.

XLOG$FAO(LOG$TCP,"!%T TCP$Send_ACK, Rcv window: !UL, ACK number: !XL (!UL)!/",
	  0, TCB->rcv_wnd, TCB->rcv_nxt, TCB->rcv_nxt);

    if (TCB->SND_Q_count == 0)
	TCP$Send_CTL(TCB,M$ACK)
    else
	{

// Data available to send, is the send window greater than 0?  Must be
// or TCP$Send_Data will not attempt to send any.

	if ((TCB->State == CS$ESTABLISHED) && (TCB->Snd_Wnd GTRU 0))
	    {
	    if (TCP$Send_Data(TCB) == 0)
		TCP$Send_CTL(TCB,M$ACK)
	    }
	else
	    TCP$Send_CTL(TCB,M$ACK); // Send an ACK control segment.
	};
    };

//SBTTL "SEND CTL: Send a Control Segment"
/*

Function:

	Send a control segment to IP.
	Control segments are always sent regardless of the size of the
	send window because we need to force the other TCP to send us it's
	current window size.  Case of foreign TCP closes window we need
	to become aware when it opens back up.

Inputs:

	TCB = Points at current TCB
	Type = Control Message identifier(M$xxx type).

Outputs:

	None.

Side Effects:

	TCP Control segment (Control_Seg_Size) is send by IP over the
	network.  If control-seg is an ACK then clear pending_ack flag in
	the TCB.

*/
TCP$SEND_CTL(struct tcb_structure * tcb,Type)
    {
    signed long
	Bufsize,
	Bufptr,
	struct Segment_Structure * Seg,
	Segsize  = TCP_Header_Size,
	Option_Offset = 0,
	SeqSpace = 1;	// Sequence Space occupied by this segment.
				// assume protected control segment (SYN or FIN)

// Fix segment size if doing SYN - need to send standard options

    if ((type == M$SYN) || (type == M$SYN_ACK))
	{
	Segsize = Segsize + SEND_OPT_SIZE; // Include room for our options
	Option_Offset = SEND_OPT_OFFSET; // And indicate how long they are
	};

// Get a buffer large enough to contain tcp header, data, ip header and device
// header.  Index into buffer & overlay segment header structure.

    Bufsize = Device_header + IP_Hdr_byte_size + Segsize;
    if (Bufsize <= MIN_PHYSICAL_BUFSIZE)
	Bufsize = MIN_PHYSICAL_BUFSIZE
    else
	if (Bufsize <= MAX_PHYSICAL_BUFSIZE)
	    Bufsize = MAX_PHYSICAL_BUFSIZE;
    Bufptr = MM$Seg_Get(BufSize);	// Carve out a control segment structure.
    Seg = BufPtr + Device_header + IP_hdr_byte_size; // point at segment start.

// build the segment

    Seg->SH$Control_Flags = 0;		// Clear control Flags.
    SELECTONE TYPE OF
    SET
    [M$ACK]:
	{
	Seg->SH$C_ACK = TRUE;
	SeqSpace = 0;
	TCB->Pending_Ack = FALSE;		// clear pending ack flag.
	};

    [M$RST]:
	{
	Seg->SH$C_RST = TRUE;
	SeqSpace = 0;
	};

// make this a SYN control segment with max receive data size option present.
// could be syn_ack.

    [M$SYN, M$syn_ack]:
	{
	Seg->SH$C_SYN = TRUE;
	if (type == M$syn_ack)
	    Seg->SH$C_ACK = TRUE;

// include max data size we will receive option.

	Send_TCP_Options(TCB,Seg);
	};

    [M$FIN]:
	Seg->SH$C_FIN = TRUE;
    TES;

// If this segment occupies sequence number space (i.e. SYN or FIN), then
// queue it up for retransmission.

    if (SeqSpace > 0)
	Rexmit_Enqueue(TCB,0,TYPE);

// 2 false parameters are for EOL & Urgent, not on control segments.

    Build_Header(TCB,Seg,SegSize,SeqSpace,TCB->SND_NXT,
		 FALSE,FALSE,Option_Offset);

// Advance SND_NXT for FIN/SYN

    TCB->SND_NXT = TCB->SND_NXT + Seqspace;

// Only ACKable segments update the probe timer.

    TS$SX = TS$SX + 1;		// track segments transmitted.
    TCP_MIB->MIB$tcpOutSegs = TCP_MIB->MIB$tcpOutSegs + 1;
    if (SeqSpace > 0)
	Send_Seg1(TCB, Seg, Segsize, TRUE, Bufptr, Bufsize)
    else
	Send_Seg0(TCB, seg, segsize, TRUE, bufptr, bufsize)
    };
%SBTTL "Send a probe packet"
/*
Function:

	Called periodically for connections which are idle to make sure
	that they are still alive. We send an empty packet which is
	intended to generate some ACK response. Currently, we will send
	an unacceptable segment with SYN and ACK on with a bogus sequence
	number. According to the TCP spec, such a segment should either
	generate an ACK with the correct sequence numbers or should generate
	an RST if the connection does not exist.
*/

void DO_PROBE(struct tcb_structure * tcb) (void)
    {
    BIND
	segsize = TCP_Header_Size,
	bufsize = Device_header+IP_HDR_Byte_Size+TCP_Header_Size;
    OWN
	buf : VECTOR[(bufsize+3)/4];
    BIND
	seg = buf+Device_header+IP_hdr_byte_size : Segment_structure;

// Fill in the segment

    seg->SH$Source_port = TCB->Local_Port;
    seg->SH$Dest_port = TCB->Foreign_Port;
    seg->SH$Seq = (TCB->SND_NXT XOR -1);
    seg->SH$Control_Flags = 0;
    seg->SH$C_ACK = TRUE;
    seg->SH$C_SYN = TRUE;
    seg->SH$ACK = TCB->RCV_NXT;
    seg->SH$Data_Offset = TCP_Data_Offset;
    seg->SH$Window = TCB->RCV_WND;
    seg->SH$Urgent = 0;
    seg->SH$Checksum = 0;

    if ($$LOGF(LOG$TCP))
	{
	LOG$FAO("!%T Sending TCB inactivity probe!/",0);
	SEG$Log_Segment(seg,segsize,false,FALSE)
	};

// Byteswap header and do checksum

    seg->SH$SEQ = ROT(seg->SH$SEQ,16);
    seg->SH$ACK = ROT(seg->SH$ACK,16);

    SwapBytes(TCP_Header_Size/2,seg);
    seg->SH$Checksum = Gen_Checksum(segsize,seg,TCB->Local_Host,
				    TCB->Foreign_Host,TCP_Protocol);
// Finally, send the packet.

    TCP_MIB->MIB$tcpOutSegs = TCP_MIB->MIB$tcpOutSegs + 1;
    Send_Seg1(TCB, seg, segsize, false, buf, bufsize);
    };

%SBTTL "Build Header: Fill in TCP segment header."
/*

Function:

	Fill in the TCP segment header fields & generate the segment
	checksum.

Inputs:

	TCB = points at current TCB
	seg = points at segment structure.
	Length = Length of TCP header & data in bytes.
	SeqSpace = Amount of segment space contained in this segment.
	EOL = End Of Letter boolean.
	URG = Urgent boolean.
	options = # of 4 byte options fields present. 0 = none.

Outputs:

	none.

Side Effects:

	TCB segment space counters are updated.
*/

Build_Header(TCB,SEG,Length,SeqSpace,SEQstart,
		     EOL,Urg,OPTOFF): NOVALUE (void)
    {
    MAP
	struct tcb_structure * tcb,
	struct Segment_Structure * SEG;
    signed long
	swap_wordSize;

    Seg->SH$Source_Port = TCB->Local_Port;
    Seg->SH$Dest_Port = TCB->Foreign_Port;
    Seg->SH$Seq = SEQstart;

// Is this a synchronized connection?  True implies ack field is valid.

    if (TCB->IS_Synched)
	{
	Seg->SH$C_ACK = TRUE;
	TCB->pending_ack = FALSE;
	};

// special case reset segs for ack bit.

    if ((seg->sh$c_rst))
	{
	if (seg->sh$seq == 0)
	    Seg->SH$C_ACK = TRUE
	else
	    Seg->SH$C_ACK = FALSE;
	};

// If ack control is asserted then update spontaneous ack timer.
// Also, update last RCV_NXT/RCV_WND that were ACK'ed

    if ((seg->sh$c_ack))
	{
	TCB->ack_timer = Time_Stamp() + ACK_INTERVAL;
	TCB->old_RCV_NXT = TCB->RCV_NXT;
	TCB->old_RCV_WND = TCB->RCV_WND;
	};

    Seg->SH$Ack = TCB->RCV_NXT;
    Seg->SH$Data_Offset = TCP_Data_Offset + OPTOFF;
    Seg->SH$Window = TCB->Rcv_Wnd;

// Process EOL(End Of Letter) & Urgent flags.

    if (eol)
	Seg->SH$C_EOL = TRUE;

    if (urg)
	{
	Seg->SH$C_URG = TRUE;
	Seg->SH$Urgent = (TCB->SND_NXT + SEQspace) - 1;
	};

    if ($$LOGF(LOG$TCP))
	SEG$Log_Segment(Seg,Length,FALSE,FALSE);

// Generate the Segment Checksum, Includes TCP Pseudo Header.

    Seg->SH$CheckSum = 0;	// For checksum routine.
//   XLOG$FAO(LOG$TCP,"Sending SEQ #: !XL (!UL)!/",Seg->SH$SEQ,Seg->SH$SEQ);

// Swap bytes within TCP header words(16-bits) so checksum is correct for
// order in which the bytes are transmitted.

    Seg->SH$SEQ = ROT(Seg->SH$SEQ,16);
    Seg->SH$ACK = ROT(Seg->SH$ACK,16); // swap 16-bit words in 32-bit fullwords

// Swap the bytes in the header. Note that options are always in network byte
// order and thus need not be swapped.

    SwapBytes(TCP_Header_Size/2,Seg);

// generate the actual checksum now that all the bytes are in the order of
// transmission.

    Seg->SH$CheckSum = Gen_Checksum(Length,Seg,TCB->Local_Host,
				    TCB->Foreign_Host,TCP_Protocol);
    };

}
ELUDOM
