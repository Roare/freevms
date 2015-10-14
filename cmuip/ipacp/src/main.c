#if 0
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
%TITLE 'MAIN - major routines for servicing network connections'
/*
 Facility:

 This is the top-level controller for the IPACP

 Abstract:

 Handles tasks which must be "scheduled" in some way, as opposed to
 event driven tasks such as user calls (handled in USER, TCP_USER)
 or network input (handled in TCP_SEGIN). Also responsible for the
 startup and initialization of the network ACP as a whole.

 Language:

 BLISS-32

 System:

 VAX/VMS

 Author:

 Original version by Stan C. Smith, Summer 1981
 This version by Vince Fuller, CMU-CSD, Spring/Summer, 1986
 Copyright (c) 1986,1987, Vince Fuller and Carnegie-Mellon University

 Module Modification History:

 *** Begin CMU change log ***

 8.1 29-Nov-1991 Henry W. Miller     USBR
 Use unsigned arithmetic in Wait_For_Something_2_DO().

 8.0A    16-Jul-1991 Henry W. Miller     USBR
 Added hooks for memory debugging.

 8.0 06-Feb-1990 Bruce R. Miller     CMU NetDev
 Changed TCP_Main.bli to Main.bli in an attempt to reorganize
 the IPACP.  I moved all the TCP stuff into TCP.BLI.

 20-Oct-1989 Bruce R. Miller     CMU NetDev
 Added a bit of code to initialize the activity log.

 30-Aug-1989 Bruce R. Miller     CMU NetDev
 Made Conect Table into a hash table structure.

 07-Aug-1989 Bruce R. Miller     CMU NetDev
 Added NML$Cancel call to Inactivate_TCB routine.  When a TCB
 is inactivated any outstanding NQE's should be deleted.  If this
 were not done, and a new TCB with the old address were created,
 the the Name Query reply from the first TCB would affect the
 new TCB (which would be totally BOGUS!).

 03-Jul-1989 Bruce R. Miller     CMU NetDev
 Fixed DUMP_TCB to handle unknown TCP state.

 16-MAR-1989 Dale Moore  CMU-CS/RI
 If connection timeout is zero, then don't timeout connection.

 20-OCT-1988 Dale Moore  CMU-CS/RI
 Changed Keep_Alives (Do_Probe) to be runtime configurable.

 04-OCT-1988 Dale Moore  CMU-CS/RI
 Changed TOS from %O'15' to 0.  Evidently some busted
 implementations that couldn't take something in this
 field that wasn't what they expected.

 7.10  22-JUN-1988, Dale Moore   CMU-CS/RI
 Change the call on retransmit of SYN to send size of options
 as long rather than octets.  Thanks to Charles Lane of
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
 packet. Make a minor change in the calculation of TCB[RX_TIMER].

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

 1.0 [10-15-81] stan
 original version

 1.2 [5-5-83] stan
 vms operator interface routines

 1.3 [5-20-83] stan
 error processor routine.

 1.4 [6-12-83] stan
 Support for SMTP (Simple Mail Transfer Protocol) as a weel-known server.

 1.5 [6-13-83] stan
 During connection timeout processing: check if process owning connection
 has sub-processes.  If true, then update connection timeout as this
 could be an ftp cmd connection attempting to timeout during a LONG, SLOW
 data transfer.

 1.6 [7-15-83] stan
 modified global literal definitions to be byte-size.

 1.7 [9-13-83] stan
 rewrote routine "read_known_hosts_file" to clean up code and utilize the
 host-alias table.  Alias no longer utilize an entry in the known-hosts
 table.

 1.8 [5-1-85] noelan olson
 Replace obsolete system services.

 1.9 [5-30-85] noelan olson
 Use a table to store multiple internet addresses for the purpose of
 gatewaying between networks.
 */

%sbttl 'Module Environment & Definition.'

MODULE IPACP
(MAIN=Main,IDENT='8.1',LANGUAGE(BLISS32),
        ADDRESSING_MODE(EXTERNAL=LONG_RELATIVE,
                NONEXTERNAL=LONG_RELATIVE),
        LIST(REQUIRE,ASSEMBLY,OBJECT,BINARY),
        OPTIMIZE,OPTLEVEL=3,ZIP)=
BEGIN

! Include System libraries.

LIBRARY 'SYS$LIBRARY:STARLET';
! VMS system definitions
LIBRARY 'CMUIP_SRC:[CENTRAL]NETXPORT';
! Bliss transportable library defs.
LIBRARY 'CMUIP_SRC:[central]NETERROR';
! Network error codes
LIBRARY 'CMUIP_SRC:[CENTRAL]NETVMS';
! Include Local OS Dependent stuff.
LIBRARY 'CMUIP_SRC:[CENTRAL]NETCOMMON';
! Include common stuff.
LIBRARY 'CMUIP_SRC:[CENTRAL]NETTCPIP';
! TCP/IP definition

LIBRARY 'Structure';
! Structure Definitions
!LIBRARY 'TCP';
! TCP Related definitions
LIBRARY 'TCPMacros';
! Local (tcp) Macro defintions.
LIBRARY 'TCPMOD';
! Modification history/version

%SBTTL 'External Routines'

EXTERNAL ROUTINE

! MACLIB.MAR

#endif
extern mount_ip_device();
#if 0
Time_Stamp,
User_Requests_Avail,

! IOUTIL.BLI

FATAL_FAO :
NOVALUE,
OPR_FAO :
NOVALUE,
LOG_FAO :
NOVALUE,
LOG_OUTPUT :
NOVALUE,
Exit_Handler :
NOVALUE,
Exception_Handler :
NOVALUE,
CHECK_ERRMSG_Q :
NOVALUE,

! RPC.C

RPC$INIT,

! CONFIG.BLI

CNF$Check_Devices :
NOVALUE,
CNF$Define_IPACP_Interface :
NOVALUE,
CNF$Configure_ACP :
NOVALUE,
CNF$Get_Local_IP_addr,
CNF$Net_Device_init :
NOVALUE,

! USER.BLI

USER$Purge_All_IO :
NOVALUE,
TCP$Purge_Send_Queue :
NOVALUE,
USER$Process_User_Requests :
NOVALUE,
USER$Init_Routines :
NOVALUE,

! MEMGR.BLI

MM$Init :
NOVALUE,

! TCP_SEGIN.BLI

SEG$Process_Received_Segments :
NOVALUE,

! IP.BLI

IP$Init :
NOVALUE,

! TCP.BLI

TCP$Init :
NOVALUE,
TCP$Service_Connections,

! NMLOOK.BLI

NML$INIT :
NOVALUE,
NML$PURGE :
NOVALUE,
NML$GETNAME :
NOVALUE,

! MEM.BLI

Make_Zone :
NOVALUE;

%SBTTL 'External Literals & data segments'

! External Data
#endif

#include <stdio.h>
#include <string.h>

#include <ssdef.h>
#include <descrip.h>
#include <lnmdef.h>
#include <jpidef.h>
#include <misc.h>

#include <system_data_cells.h>

#include <netcommon.h>
#include <nettcpip.h>

#include <starlet.h>

#include "netvms.h"
#include "cmuip.h" // need this before tcpmacros
#include "tcpmacros.h"
#include "structure.h"
#include "neterror.h"

#ifndef NOKERNEL
#define sys$bintim exe$bintim
#define sys$schdwk exe$schdwk
#define sys$canwak exe$canwak
#define sys$crelnm exe$crelnm
#define sys$trnlnm exe$trnlnm
#define sys$gettim exe$gettim
#define sys$setrwm exe$setrwm
#define sys$exit exe$exit
#define sys$getjpiw exe$getjpiw
#define sys$setprn exe$setprn
// not sys$hiber is the same
#endif

//EXTERNAL
// TCP_MECH.BLI
extern struct _generic_64 Start_Time;   // Quadword time IPACP started.
extern unsigned long log_state, act_state, Begin_Lock,         // start & end address of process pages which
        End_Lock;           // are locked in the working set; see maclib.mar

//%SBTTL 'Literals'

//LITERAL
#define MAXPRCLEN 15        // Max length of process name

//%SBTTL 'Global Data Declarations.'

static char mynamestr[128], local_namestr[MAX_HNAME];

//GLOBAL
signed long bogus1[512], intdf = -1,    // Interrupt-defer count (-1 means OKINT)
        bogus2[512], ast_in_progress = FALSE, // AST service rtn executing.
        Time_2_Exit = FALSE, // Main processing loop control.
        sleeping = FALSE;  // Nap control.
struct dsc$descriptor myname_ =
    { dsc$w_length : 128, dsc$a_pointer :
    mynamestr };
struct dsc$descriptor * myname = &myname_;
struct dsc$descriptor Local_Name =
    { dsc$w_length:
    MAX_HNAME, dsc$a_pointer:
    local_namestr };
#if 0
DESC$STR_ALLOC(MyName,128), // eqv for logical "tek$network_name"
DESC$STR_ALLOC(Local_Name,MAX_HNAME),
#endif
signed long max_recv_datasize = OPT$MAX_RECV_DATASIZE,  // Max segment size
        default_mss = DEFAULT_DATA_SIZE,     // Max segment size
        max_physical_bufsize,  // Max size of device receive buffer
        min_physical_bufsize;  // Minimum size of device send buffer

// Segment Input from IP Queue header.
// Initialize Queue header to point at itself (ie, empty state).

struct queue_header_structure(si_fields) segin_ =
    { si_qhead:
    &segin_, si_qtail:
    &segin_ }, *segin = &segin_;

// Storage for original process name

static signed long myprcname[MAXPRCLEN], myprclen;

void MAIN$NOINT(void)
{
    NOINT
}

void MAIN$OKINT(void)
{
    OKINT
}

//SBTTL 'Initialize ACP'
/*

 Function:

 Initialize TCP & IP.  We are at user access mode in case of errors
 in processing we don't clobber the entire system.
 *** warning ***
 configure_acp routine MUST be called before init_memory_manager
 as configure_acp sets memory manager parameters.

 Inputs:

 None.

 Outputs:

 None.

 Side Effects:

 Host table is initialized (from INET$HOSTS, via HOST_INIT())
 Device configuration table "dev_config" and memory manager are
 initialized from logical filename "INET$CONFIG".
 */

extern void ACPINI_ADLOOK_DONE();

void Initialize_ACP(void)
{
    int cidx, lclnlen;

    if (log_state > 0)     // Open log, if requested
        if (!LOG_OPEN())
            log_state = 0;
    if (act_state > 0)     // Open activity log, if requested
        if (!ACT_OPEN())
            act_state = 0;

// Calculate standard buffer sizes. The MAX_PHYSICAL_BUFSIZE is the size of all
// allocated receive buffers and is the maximum size of the IP fragmentation
// buffer.

    max_physical_bufsize = DEVICE_HEADER + TCP_HEADER_SIZE + IP_HDR_BYTE_SIZE +
    OPT$MAX_RECV_DATASIZE;
//    IF (.MAX_PHYSICAL_BUFSIZE MOD 4) NEQ 0 THEN
//  FATAL$FAO('Initialize_ACP - Max buffer size not divisible by 4');
    min_physical_bufsize = DEVICE_HEADER + TCP_HEADER_SIZE + IP_HDR_BYTE_SIZE +
    DEFAULT_DATA_SIZE;

    // First define the IPACP interface for any other modules we may load...
    CNF$Define_IPACP_Interface();

    RPC$INIT();             // initialize RPC module (& PortMapper)

    CNF$Configure_ACP();        // read/process file "config.txt"

    mm$init();              // see memgr.bli module.
    CNF$Net_Device_Init();      // Initialize Network device(s).
    ip$init();              // Initialize IP
    user$init_routines();       // Initialize stuff in USER module.
    tcp$init();

// Initialize name lookup service

    NML$INIT();

// Get the local host name

    NML$GETNAME(CNF$Get_Local_IP_addr(), ACPINI_ADLOOK_DONE, 0);
}

//SBTTL 'ACPINI_ADLOOK_DONE - Set local host name when name lookup finished'

void ACPINI_ADLOOK_DONE( DUMMY, STATUS, NAMLEN, NAMPTR)
{

// Check the status

    if (!STATUS)
        return;

// Copy the name into the local name buffer

    Local_Name.dsc$w_length = NAMLEN;
    memcpy(Local_Name.dsc$a_pointer, NAMPTR, NAMLEN);
}

//SBTTL 'Wait-For-Something-2-DO: Snooze if we have the time.'
/*

 Function:

 Determine if we have any work to do.  IF no work then snooze for an
 hour or until we are wakened by VMS (ie, user IO request queue) or
 a network segment arrives (ie, IP's AST routine which runs in response
 to AST delivered from PI-13 driver).  If there are active connections
 then skip the nap.


 Inputs:

 Minimum time to sleep, derived from TCB examination.

 Outputs:

 None.

 Side Effects:

 If we are taking a long nap (ie, nothing to do) then purge the working
 set before hibernation to reduce system impact.
 */

void wait_for_something_2_do(unsigned long long nxtime)
{
    //    BIND
    // Range of pages to purge from WS.
#if 0
    PageRange0 = UPlit(1,Begin_lock-%O'1000'),
            PageRange1 = UPlit(End_Lock+%O'1000',%X'7FFFFFFF'),
#endif
    $DESCRIPTOR(Long_Nap, "0 12:0:0.0");     // hours.
    //    LITERAL
#define SHORT_NAP (20*CSEC)
#define SHORTER_NAP (2*CSEC)
    extern tcb_count;
#if 0
    REGISTER
    qb:
    REF Queue_Blk_Structure(QB_RT_Fields);
#endif
    //    LOCAL
    unsigned long long now;
    unsigned rto, delay;
    char Big_Sleep = FALSE;
    struct _generic_64 BTime;   // Binary rep of time.

// Skip this snooze if we have network segments to process or IP has datagrams
// to send or it's Time to exit.

    if (queue_empty(&segin->si_qhead) && (!Time_2_Exit))
    {

// Any valid Connections around? (TCB_Count > 0). If none take a restful nap.
// Otherwise, use next TCB service time provided by TCB scan.

        if (tcb_count > 0)
        {
            now = Time_Stamp();
//      RTO = MINU(SHORT_NAP,nxtime-now);
            if (nxtime > now)
            {
                delay = nxtime - now;
            }
            else
            {
//      Delay = 100 ;
                delay = 0;
            }
            rto = MINU(SHORTER_NAP, delay);
            if (rto <= 0)
            {
                XLOG$FAO(LOG$TCBCHECK, "!%T WFS2D: No sleep, RTO=!UL!/", 0, rto);
                rto = 0;
            }
            else
            {
                XLOG$FAO(LOG$DEBUG, "!%T WFS2D: Short sleep, RTO=!UL!/", 0, rto);
                XLOG$FAO(LOG$TCBCHECK, "!%T WFS2D: Short sleep, RTO=!UL!/", 0, rto);
            }

            BTime.gen64$r_quad_overlay.gen64$q_quadword =  -1 * rto * CSEC_TIMER_DELTA; // make it delta time, nap size.
        }
        else            // No TCB's
        {
            sys$bintim(&Long_Nap, &BTime);
            Big_Sleep = TRUE;
            XLOG$FAO(LOG$TCBCHECK, "!%T WFS2D: Long sleep!/", 0);
        }

// Check again for segment arrival & no TCB actions

        if (queue_empty(&segin->si_qhead) && (((int *) &BTime)[0] != 0))
        {

// If we are doing a BIG sleep then purge the working set to reduce system impact.

//      IF .Big_Sleep THEN
//      BEGIN
//      $PurgWS(INadr=PageRange0);
//      $PurgWS(INadr=PageRange1);
//      END;

// Snoooooozzzzzz!!!!!!!!!!!!

            sys$schdwk(0, 0, &BTime, 0); // Schedule a wake-up.
            sleeping = TRUE;    // So AST Handler can wake us.
            sys$hiber();

// If we received an Attention AST from the PI device (network mesg avail)
// Then sleeping will be set to false.  This implies that there is still an
// outstanding wake-up (SCHWAK) because we were forced awake by the AST service
// routine.  ****** WARNING:  DO NOT place $canwak in AST service routine as
// strange things begin to happen (TCP just stops).  There appears to be some
// kind of timing related nonsense between AST delivery & XPORT/RMS output
// processing.

            sleeping = FALSE;
            sys$canwak(0, 0);
        }
        else
        {
            XLOG$FAO(LOG$TCBCHECK, "!%T WFS2D: Not sleeping!/", 0);
        }
    }
}

//SBTTL 'TCP Mainline rtn.'

void INIT_PROCNAME(void);

#ifdef NOKERNEL
void main (void)
{
#else
void Main(void)
{
#endif
#ifndef NOKERNEL
    //current->pslstk[current->pslindex-1]|=(3<<2 | 3);
    setpslmod(3); // check. pretend user mode
#endif
    $DESCRIPTOR(logical_netname_, "INET$NETWORK_NAME");
#if 0
    BUILTIN
    fp;         // VAX Frame Pointer.
    MAP
    fp:
    REF VECTOR[1];
#endif
    long rc;
    unsigned char eqv_name[255];
    unsigned short eqv_len;
    struct item_list_3 items[2] =
        {
            { 255,         // output buffer length
                LNM$_STRING, // item code
                eqv_name, // output buffer
                &eqv_len, // word for translated string length
            },
            { 0, 0, 0, 0 } };  // list terminator.
    long exitblk[4],    // exit handler arg blk.
            exit_status;       // exit status code( SS$_xxxxxx).
    $DESCRIPTOR(tabnam_, "LNM$SYSTEM_TABLE"); //check shorten later
    struct dsc$descriptor * logical_netname = &logical_netname_, *tabnam = &tabnam_;

#ifndef NOKERNEL
    // some extra temp stuff here
    xqp_init2();
    extern void * global_e2_vcb;
    exttwo_init2(global_e2_vcb);
#endif
    if (1)
    {
        int sts;
        $DESCRIPTOR(mytabnam_, "LNM$SYSTEM_TABLE");
        struct dsc$descriptor * mytabnam = &mytabnam_;
        $DESCRIPTOR(dev_, "INET$DEVICE");
        $DESCRIPTOR(pty_, "INET$PTY");
        $DESCRIPTOR(pty_term_, "INET$PTY_TERM");
        struct dsc$descriptor * dev = &dev_;
        struct dsc$descriptor * pty = &pty_;
        struct dsc$descriptor * pty_term = &pty_term_;
        struct item_list_3 itm[2];
#ifndef NOKERNEL
        lnm_init_prc(ctl$gl_pcb); // needs this extra one
#endif

        itm[0].item_code = LNM$_STRING;
        itm[0].buflen = 4;
        itm[0].bufaddr = "ipa0";
        bzero(&itm[1], sizeof(struct item_list_3));
        sts = sys$crelnm(0, mytabnam, dev, 0, itm);

        itm[0].item_code = LNM$_STRING;
        itm[0].buflen = 4;
        itm[0].bufaddr = "pna0";
        sts = sys$crelnm(0, mytabnam, pty, 0, itm);

        itm[0].item_code = LNM$_STRING;
        itm[0].buflen = 3; // should be 2
        itm[0].bufaddr = "tza"; // should be tz
        sts = sys$crelnm(0, mytabnam, pty_term, 0, itm);

    }
    // end of extra temp stuff

// Translate logical network name to equivalence string (myname_buf,desc=myname)
// provide a default name in case tcp$network_name is undefined.

//    Make_Zone() ;
    rc = sys$trnlnm(0, tabnam, logical_netname, 0, items);
    if (rc == SS$_NOLOGNAM || !rc)
    {
        myname->dsc$w_length = 14; // set the proper length since fixed string.
        myname->dsc$a_pointer = "Noname Network";
    }
    else
    {
        myname->dsc$w_length = eqv_len;
        myname->dsc$a_pointer = eqv_name;
    }

// Record starting time.

    sys$gettim(&Start_Time);

// Set process name

    INIT_PROCNAME();

// Install an execption handler to catch those nasty execptions & clean up
// user IO on a TCP crash.  Use an exit handler to double check that all
// user IO has been posted.

#if 0
    Fp[0] = Exception_Handler;  // Declare an exception handler.
    ExitBlk[0] = 0;// VMS uses this for a pointer.
    ExitBlk[1] = Exit_Handler;// Routine to field the exit.
    ExitBlk[2] = 1;// # of args for handler.
    ExitBlk[3] = Exit_Status;// adrs of where to store exit reason.
    $DCLEXH(desblk=ExitBlk);// Declare the exit handler.
#endif

// Disable resource wait mode

    sys$setrwm(1);

    // borrow from tcpmod.req until we get tcpmod.h

#define    IPACP_Version_String "IP_ACP(V6.6-5.001) "  //Becomes ACP process name.
#define    IPACP_Version_Name IPACP_Version_String
#define    IPACP_Date_String "9-Apr-2007"   // Date of last change
#define    IPACP_Who_String "roart" // Author of last change

// Announce that we exist.

    char s[256];
    sprintf(s, "%s%s%s%s%s%s\n", IPACP_Version_String, "[", IPACP_Who_String, "/",
    IPACP_Date_String, "] starting");
    OPR$FAO(s);

//    OPR$FAO("NET$_CC = !XL",NET$_CC);

//    Signal(NET$_CC);

// Mount the network virtual device "IP".

    if (!(rc = $$KCALL(mount_ip_device)))
        FATAL$FAO("Failed to mount virtual device IP ,RC = %x\n", rc);

// Perform tcp/acp initialization

    Initialize_ACP();

// Tell the operator that we are alive and well.

    OPR$FAO("Network online");

// Main processing loop....till forever or time 2 Exit.
    do
    {
        long long nxtime;

        SEG$Process_Received_Segments();
        user$process_user_requests();
        if (intdf < -1)
            OPR$FAO("Intdf went negative...");
        nxtime = tcp$service_connections();
        CHECK_ERRMSG_Q();
        wait_for_something_2_do(nxtime);
    }
    while (Time_2_Exit == 0);

// Exit in an orderly fashion.

    NML$PURGE(NET$_TE);     // Purge the name server queue
    user$purge_all_io();    // Post any pending user IO.
    LOG$FAO("!%T IPACP exit.!/", 0);
    sys$exit(SS$_NORMAL);   // say goodbye...
}

//SBTTL 'Set and reset process name'

void INIT_PROCNAME(void)
{
//
// Set our process name to the special IPACP version name. If duplicate name
// is found, we will punt, assuming another ACP already exists. We also save
// our original name so that it can be restored when we exit.
//
    signed long rc;
    struct item_list_3 itmlist[2] =
        {
            { MAXPRCLEN, JPI$_PRCNAM, myprcname, &myprclen },
            { 0, 0, 0, 0 } };
#if 0
    BIND
    PROCESS_NAME = IPACP_Version_Name; // see tcp.mod file.
#endif
    $DESCRIPTOR(process_name_, "IPACP");
    struct dsc$descriptor * process_name = &process_name_;

// First, obtain our original process name

    rc = sys$getjpiw(0, 0, 0, &itmlist, 0, 0, 0);
    if (!(rc & 1))
    {
        myprclen = 0;
        WARN$FAO("$GETJPIW failure for process name, RC = %x", rc);
    }

// Now, try to set our name as the IPACP.

    rc = sys$setprn(process_name);
    if (!(rc & 1))
    {
        if (rc == SS$_DUPLNAM)
            FATAL$FAO("Duplicate network ACP -- exiting");
        else
            FATAL$FAO("$SETPRN failure, RC = %x", rc);
    }
}

void RESET_PROCNAME(void)
{
    struct dsc$descriptor namdsc;

// Build descriptor for our original name buffer.

    namdsc.dsc$b_class = DSC$K_CLASS_Z;
    namdsc.dsc$b_dtype = DSC$K_DTYPE_Z;
    namdsc.dsc$w_length = myprclen;
    namdsc.dsc$a_pointer = myprcname;

// Reset our process name.

    sys$setprn(&namdsc);
}

void ip4acp_main(void)
{
    int sts;
    struct item_list_3 items[2];
    int rc;
    char eqv_name[LNM$C_NAMLENGTH];
    int eqv_len;
    $DESCRIPTOR(tabnam, "LNM$SYSTEM_TABLE"); // cut _TABLE some time
    $DESCRIPTOR(lognam, "INET$NETWORK_NAME");
    struct _generic_64 Start_Time;
    struct dsc$descriptor myname;

    items[0].item_code = LNM$_STRING;
    items[0].buflen = LNM$C_NAMLENGTH;
    items[0].bufaddr = eqv_name;
    items[0].retlenaddr = &eqv_len;
    bzero(&items[1], sizeof(struct item_list_3));

    rc = sys$trnlnm(0, &tabnam, &lognam, 0, items);

    if (rc == SS$_NOLOGNAM || rc == 0)
    {
        myname.dsc$w_length = 14; // set the proper length since fixed string.
        myname.dsc$a_pointer = "Noname Network";
    }
    else
    {
        myname.dsc$w_length = eqv_len;
        myname.dsc$a_pointer = eqv_name;
    }

// Record starting time.

    sys$gettim(&Start_Time);

// Set process name

    INIT_PROCNAME();

// Install an execption handler to catch those nasty execptions & clean up
// user IO on a TCP crash.  Use an exit handler to double check that all
// user IO has been posted.

#if 0
    Fp[0] = Exception_Handler;  // Declare an exception handler.
    ExitBlk[0] = 0;// VMS uses this for a pointer.
    ExitBlk[1] = Exit_Handler;// Routine to field the exit.
    ExitBlk[2] = 1;// # of args for handler.
    ExitBlk[3] = Exit_Status;// adrs of where to store exit reason.
    $DCLEXH(desblk=ExitBlk);// Declare the exit handler.
#endif

// Disable resource wait mode

    sys$setrwm(1); // check

// Announce that we exist.

#if 0
    OPR$FAO(%STRING(IPACP_Version_String," [',IPACP_Who_String,'/",
                    IPACP_Date_String,"] starting"));
#endif

//    OPR$FAO("NET$_CC = !XL",NET$_CC);

//    Signal(NET$_CC);

// Mount the network virtual device "IP".

#if 0
//check wait check
    IF NOT(rc=$$KCALL(mount_ip_device)) THEN
    FATAL$FAO("Failed to mount virtual device IP ,RC = %x\n",rc);
#endif

// Perform tcp/acp initialization

    Initialize_ACP();

// Tell the operator that we are alive and well.

    OPR$FAO("Network online\n");

// Main processing loop....till forever or time 2 Exit.

    do
    {
        unsigned long long nxtime;

        SEG$Process_Received_Segments();
        user$process_user_requests();
        if (intdf <= -1)
            OPR$FAO("Intdf went negative...\n");
        nxtime = tcp$service_connections();
        CHECK_ERRMSG_Q();
        wait_for_something_2_do(nxtime);
    }
    while (Time_2_Exit == 0);

// Exit in an orderly fashion.

    NML$PURGE(NET$_TE);     // Purge the name server queue
    user$purge_all_io();    // Post any pending user IO.
    LOG$FAO("!%T IPACP exit.!/");
    sys$exit(SS$_NORMAL);   // say goodbye...
}

