// $Id$
// $Locker$

// Author. Roar Thron�s.
// Modified Linux source, 2001-2004. Based on decnet files.

#define ETH_P_MYSCS 0x6009

#include <linux/config.h>
#define CONFIG_DECNET
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/socket.h>
#include <linux/in.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/string.h>
#include <linux/sockios.h>
#include <linux/net.h>
#include <linux/inet.h>
#include <linux/route.h>
#include <linux/netfilter.h>
#include <net/sock.h>
#include <asm/segment.h>
#include <asm/system.h>
#include <asm/ioctls.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/stat.h>
#include <linux/init.h>
#include <linux/poll.h>
#include <linux/vmalloc.h>

#include <cdldef.h>
#include <cdrpdef.h>
#include <chdef.h>
#include <cwpsdef.h>
#include <dyndef.h>
#include <iodef.h>
#include <pbdef.h>
#include <pdtdef.h>
#include <prcpoldef.h>
#include <rddef.h>
#include <rdtdef.h>
#include <sbdef.h>
#include <sbnbdef.h>
#include <ssdef.h>
#include <system_data_cells.h>
#include <vcdef.h>
#include <starlet.h>
#include <queue.h>
#include <exe_routines.h>
#include <misc_routines.h>

struct _pb mypb;
struct _sb mysb =
{
sb$t_nodename : "MYNODE"
};
struct _pdt mypdt;
struct _pb otherpb;
struct _sb othersb;

void scs_lower_level_send(struct _cdrp * cdrp, struct _scs * scs);
void * find_free_cdt(void);
void * scs_register_name(char * c1, char * c2);
void __init scs_dev_init(void);
void __exit scs_dev_cleanup(void);

/*
 * scs_address is kept in network order, scs_ether_address is kept
 * as a string of bytes.
 */

unsigned char scs_ether_address[ETH_ALEN] =
{ 0xAA, 0x00, 0x04, 0x00, 0x00, 0x00 };

struct _cdt *scs_sklist_find_listener2(char *addr)
{
    struct _cdt **skp = (addr);
    struct _cdt *sk;
    struct _sbnb * sbnb = scs$gq_local_names;
    struct _sbnb * tmp = sbnb->sbnb$l_flink;

    while (tmp != sbnb)
    {
        if (0 == strncmp(&tmp->sbnb$b_procnam, addr, 16))
            goto out;
        tmp = tmp->sbnb$l_flink;
    }
out:
    if (tmp == sbnb)
        return 0;
    return tmp;
}

// whoa...
int scs_std$connect(void (*msgadr)(), void (*dgadr)(), void (*erradr)(),
                    void *rsysid, void *rstadr, void *rprnam, void *lprnam, int initcr,
                    int minscr, int initdg, int blkpri, void *condat, void *auxstr,
                    void (*badrsp)(), void (*movadr)(), int load_rating,
                    int (*req_fast_recvmsg)(), void (*fast_recvmsg_pm)(),
                    void (*change_aff)(), void (*complete)(), int connect_parameter)
{
    struct _cdt *cdt = find_free_cdt();
    struct _cdt *sock = cdt;
    struct _cdt *sk = sock;
    struct _cdt *scp = sk;
    int err = -EISCONN;

    struct _sbnb *s = scs_register_name(lprnam, "");
    s->sbnb$w_local_index = cdt->cdt$l_lconid;
    cdt->cdt$w_state = CDT$C_CLOSED;

    sk->cdt$l_msginput = msgadr;
    sk->cdt$l_dginput = dgadr;

    sk->cdt$w_state = CDT$C_CON_SENT;

    scs_nsp_send_conninit2(sk, SCS$C_CON_REQ, rprnam, lprnam, condat);

#if 0
    for(;; )
    {
        volatile state=sk->cdt$w_state;
        if (state==CDT$C_OPEN) goto out2;
    }
#endif

out2:

    if (auxstr)
        *((unsigned long **) auxstr) = cdt; // wrong, but temporary

    err = 0;
out:
    return err;
}

//static int scs_accept(struct _cdt *sock, struct _cdt *newsock, int flags)

int scs_std$accept(void (*msgadr)(), void (*dgadr)(), void (*erradr)(),
                   void *rsysid, void *rstadr, void *rprnam, void *lprnam, int initcr,
                   int minscr, int initdg, int blkpri, void *condat, void *auxstr,
                   void (*badrsp)(), void (*movadr)(), int load_rating,
                   int (*req_fast_recvmsg)(), void (*fast_recvmsg_pm)(),
                   void (*change_aff)(), void (*complete)(), struct _cdt * cdt,
                   int connect_parameter)
{
    struct _cdt *sk = cdt, *newsk, *newsock;
    struct _cdt *cb;
    unsigned char menuver;
    int err = 0;
    unsigned char type;
    int flags;

    return err;
}

//static struct _cdt * cdl$l_freecdt;
extern struct _cdt cdtl[1024];
struct _rdt rdt;
extern struct _scs_rd rdtl[128];
struct _cdl cdl;

extern int du_msg();

void * find_mscp_cdt(void)
{
    /* remember to fix cdldef */
    int i;
    //  return &cdtl[5]; // gross gross hack
    for (i = 0; i < 100; i++)
    {
        if (cdtl[i].cdt$l_rconid && cdtl[i].cdt$l_msginput == du_msg)
            return &cdtl[i];
        //if (cdtl[i].cdt$l_rconid) return &cdtl[i];
    }
    return 0;
}

void * find_a_cdt(int a)
{
    /* remember to fix cdldef */
    int i;
    for (i = 0; i < 100; i++)
    {
        if (cdtl[i].cdt$l_rconid && cdtl[i].cdt$l_msginput == a)
            return &cdtl[i];
        //if (cdtl[i].cdt$l_rconid) return &cdtl[i];
    }
    return 0;
}

void * find_free_cdt(void)
{
    /* remember to fix cdldef */
    struct _cdt * c = cdl.cdl$l_freecdt;
    cdl.cdl$l_freecdt = ((struct _cdt *) cdl.cdl$l_freecdt)->cdt$l_link;
    c->cdt$l_rconid = 0;
    qhead_init(&c->cdt$l_waitqfl);
    qhead_init(&c->cdt$l_crwaitqfl);
    return c;
}

int scs_std$alloc_rspid(struct _cdt *cdt_p, struct _pdt *pdt_p,
                        struct _cdrp * c, void (*ravail_p)())
{
    /* remember to fix rdtdef */
    struct _scs_rd * r = rdt.rdt$l_freerd;
    rdt.rdt$l_freerd = r->rd$l_link;
    r->rd$w_state = 1;
    r->rd$l_cdrp = c;
    return r->rd$w_seqnum;
}

int scs_std$find_rdte(int rspid, struct _scs_rd **rdte_p)
{
    int i;
    struct _scs_rd * rd = &rdtl[rspid];
    *rdte_p = 0;
    if (rspid > 100)
        return 0;
    if (rd->rd$w_seqnum != rspid)
        return 0;
    //if (rd->rd$v_busy) return 0;
    *rdte_p = rd;
    return 1;
}

int scs_std$deall_rspid(struct _cdrp *cdrp_p)
{
    int seq = cdrp_p->cdrp$l_rspid;
    struct _scs_rd * rd;
    if (scs_std$find_rdte(seq, &rd))
    {
        //rd->rd$w_seqnum++;
        rd->rd$w_state = 0;
        rd->rd$l_link = rdt.rdt$l_freerd;
        rdt.rdt$l_freerd = rd;
        return 1;
    }
    else
    {
        return 0;
    }
}

void * scs_register_name(char * c1, char * c2)
{
    struct _sbnb * s = kmalloc(sizeof(struct _sbnb), GFP_KERNEL);
    memset(s, 0, sizeof(struct _sbnb));

    memcpy(&s->sbnb$b_procnam, c1, min(16, strlen(c1)));

    insque(s, scs$gq_local_names);

    return s;
}

void * scs_find_name(char * c1)
{
    struct _sbnb * head = &scs$gq_local_names;
    struct _sbnb * tmp = head->sbnb$l_flink;
    while (tmp != head)
    {
        if (strcmp(c1, &tmp->sbnb$b_procnam) == 0)
            return tmp;
        tmp = tmp->sbnb$l_flink;
    }
    return 0;
}

//static int scs$listen(struct _cdt *sock, int backlog)
//sock is about-ish lprnam;

int scs_std$listen(
    void (*msgadr)(void *msg_buf, struct _cdt **cdt, struct _pdt *pdt),
    void (*erradr)(unsigned int err_status, unsigned int reason,
                   struct _cdt *cdt, struct _pdt *pdt), void *lprnam, void *prinfo,
    struct _cdt **cdt)
{
    int err = -EINVAL;

    struct _cdt *c = find_free_cdt();
    struct _sbnb *s;
    //

    c->cdt$l_msginput = msgadr;
    c->cdt$l_erraddr = erradr;

    s = scs_register_name(lprnam, prinfo);
    s->sbnb$w_local_index = c->cdt$l_lconid;
    c->cdt$w_state = CDT$C_LISTEN;

    if (cdt && *cdt)
        *cdt = c;
    return SS$_NORMAL;
}

void mydirerr()
{
}

int dir_listen(void * packet, struct _cdt * c, struct _pdt * p)
{

}

extern void cf_listen(void * packet, struct _cdt * c, struct _pdt * p);

void cf_myerr()
{
}

int /*__init*/scs_init(void)
{
    int i;
    char myname[] = "scs$directory";
    char myinfo[] = "directory srv";

    struct file * file = 0;
    unsigned long long pos = 0;

    memset(cdtl, 0, 1024 * sizeof(sizeof(struct _cdt)));
    memset(rdtl, 0, 128 * sizeof(sizeof(struct _rdt)));

    scs$gl_cdl = &cdl;
    //cdl$l_freecdt=&cdtl;

    scs$gl_rdt = &rdt;

    cdl.cdl$w_maxconidx = scs$gw_cdtcnt - 1;
    cdl.cdl$l_freecdt = &cdtl[0];
    cdl.cdl$b_type = DYN$C_SCS;
    cdl.cdl$b_subtyp = DYN$C_SCS_CDL;
    cdl.cdl$l_nocdt_cnt = 0;

    for (i = 0; i < scs$gw_cdtcnt - 1; i++)
    {
        cdtl[i].cdt$b_type = DYN$C_SCS;
        cdtl[i].cdt$b_subtyp = DYN$C_SCS_CDT;
        cdtl[i].cdt$l_link = &cdtl[i + 1];
        cdtl[i].cdt$l_lconid = i;
        cdtl[i].cdt$l_rconid = i;
        cdtl[i].cdt$w_state = 0;
        qhead_init(&cdtl[i].cdt$l_waitqfl);
        qhead_init(&cdtl[i].cdt$l_crwaitqfl);
    }

    qhead_init(&rdt.rdt$l_waitfl);
    rdt.rdt$b_type = DYN$C_SCS;
    rdt.rdt$b_subtyp = DYN$C_SCS_RDT;
    rdt.rdt$l_freerd = &rdtl[0];
    rdt.rdt$l_qrdt_cnt = 0;
    rdt.rdt$l_maxrdidx = scs$gw_rdtcnt;
    for (i = 0; i < scs$gw_rdtcnt - 1; i++)
    {
        struct _scs_rd * r = &rdtl[i];
        r->rd$l_link = &rdtl[i + 1];
        r->rd$w_state = 0;
        r->rd$w_seqnum = i; // ?
    }

    scs_std$listen(dir_listen, mydirerr, myname, myinfo, 0);

    scs_std$listen(cf_listen, cf_myerr, "configure", "hw conf", 0);

    extern int mount_root_vfs;
    file = rms_open_exec("[vms$common.sysexe]params.dat");
    if (!IS_ERR(file))
    {
        char * c, *b, *n;
        char buf[1024];
        if (file == 0)
            goto out2;
#if 0
        int size=rms_generic_file_read(file,buf,1024,&pos);
#else
        asmlinkage ssize_t sys_read(unsigned int fd, char * buf, size_t count);
        int size = sys_read(file, buf, 1024);
#endif
        int i = 0;
        b = buf;
        while (b < (buf + size))
        {
            c = strchr(b, '=');
            n = strchr(b, '\n');
            //printk("Q %s %x %x %x %x\n",b,b,n,c,c-b);
            if (0 == strncmp(b, "SCSNODE", c - b))
            {
                memcpy(&mysb.sb$t_nodename, c + 1, n - c - 1);
                goto out;
            }
            if (0 == strncmp(b, "SCSDEVICE", c - b))
            {
                extern int scs_init_done;
                scs_init_done = 1;
                memcpy(&mysb.sb$t_hwtype, c + 1, n - c - 1); // borrowing t_hwtype
                goto out;
            }
out:
            b = n + 1;
        }
        if (mysb.sb$t_nodename == 0)
            memcpy(&mysb.sb$t_nodename, "NONAME", 6);
end:
        filp_close(file, 0);
    }
    mypb.pb$l_pdt = &mypdt;
out2:
    {
    }
}

//static int scs_sendmsg(struct _cdt *sock, struct msghdr *msg, int size,struct scm_cookie *scm)

int scs_std$senddg(int disposition_flag, int dg_msg_length, struct _cdrp *cdrp)
{
    struct _scs * scs = kmalloc(sizeof(struct _scs), GFP_KERNEL);
    memset(scs, 0, sizeof(struct _scs));
    struct _scs1 * scs1 = scs;
    struct _ppd * ppd = scs;
    struct _cdt * cdt = cdrp->cdrp$l_cdt;
    ppd->ppd$b_opc = PPD$C_SNDDG;
    scs->scs$w_mtype = SCS$C_APPL_DG;
    scs1->scs$l_lconid = cdt->cdt$l_lconid;
    scs->scs$l_src_conid = cdt->cdt$l_lconid;
    scs->scs$l_dst_conid = cdt->cdt$l_rconid;
    scs1->scs$l_rspid = cdrp->cdrp$l_rspid;
    scs1->scs$l_xct_len = cdrp->cdrp$l_xct_len;
    scs_lower_level_send(cdrp, scs);
}

void scs_lower_level_send(struct _cdrp * cdrp, struct _scs * scs)
{
    struct sk_buff *skb = NULL;
    struct _cdt * sk = cdrp->cdrp$l_cdt;

    char * buf = kmalloc(1600, GFP_KERNEL);
    memset(buf, 0, 1600);

    scs_msg_fill(buf, sk, 0, scs);

    scs_msg_fill_more(buf, sk, cdrp, 600);

    scs_nsp_send2(buf, 1500);
}

static int scs_std$sendmsg(int msg_buf_len, struct _pdt *pdt_p,
                           struct _cdrp *cdrp_p, void (*complete)(void))
{
    struct _cdt *sk = cdrp_p->cdrp$l_cdt;
    struct _cdt *scp = sk;
    struct msghdr *msg;
    struct _cdt *sock;
    int size;
    int mss;
    struct sk_buff_head *queue = &scp->cdt$l_waitqfl;
    int flags = msg->msg_flags;
    int err = 0;
    int sent = 0;
    int addr_len = msg->msg_namelen;
    struct sockaddr_dn *addr = (struct sockaddr_dn *) msg->msg_name;
    struct _cdt *cb;
    unsigned char msgflg;
    unsigned char *ptr;
    unsigned short ack;
    int len;
    unsigned char fctype;

    return sent ? sent : err;
}

static int scs_device_event(struct notifier_block *this, unsigned long event,
                            void *ptr)
{
    return NOTIFY_DONE;
}

static struct notifier_block scs_dev_notifier =
{
notifier_call:
    scs_device_event,
};

extern int scs_rcv(struct sk_buff *, struct net_device *, struct packet_type *);

static struct packet_type scs_dix_packet_type =
{
type:
    __constant_htons(ETH_P_MYSCS),
dev:
    NULL, /* All devices */
func:
    scs_rcv,
data:
    (void*)1,
};

#define IS_NOT_PRINTABLE(x) ((x) < 32 || (x) > 126)

static struct net_proto_family scs_family_ops =
{
family:
    AF_DECnet, create: 0,
};

static char banner[] __initdata = KERN_INFO "%%KERNEL-I-STARTUP, NET5: MYSCS, based on DECnet for Linux: V.2.4.15-pre5s (C) 1995-2001 Linux DECnet Project Team\n";

int scs_init2(void)
{
    printk(banner);

    scs_startdev(0, 0, 0);

    scs_dev_init();

    configure_init();
    extern void configure();
    kernel_thread(configure, NULL, CLONE_FS | CLONE_FILES | CLONE_SIGNAL);

    /*
     * Prevent DECnet module unloading until its fixed properly.
     * Requires an audit of the code to check for memory leaks and
     * initialisation problems etc.
     */
    MOD_INC_USE_COUNT;

    return 0;

}

static void __exit scs_exit(void)
{
    scs_dev_cleanup();
}

int scs_std$reqdata(struct _pdt *pdt_p, struct _cdrp *cdrp_p,
                    void (*complete)())
{
    struct _scs * scs = kmalloc(sizeof(struct _scs), GFP_KERNEL);
    memset(scs, 0, sizeof(struct _scs));
    struct _scs1 * scs1 = scs;
    struct _ppd * ppd = scs;
    struct _cdt * cdt = cdrp_p->cdrp$l_cdt;
    ppd->ppd$b_opc = PPD$C_REQDAT;
    scs->scs$w_mtype = SCS$C_APPL_DG;
    scs1->scs$l_lconid = cdt->cdt$l_lconid;
    scs->scs$l_src_conid = cdt->cdt$l_lconid;
    scs->scs$l_dst_conid = cdt->cdt$l_rconid;
    scs1->scs$l_rspid = cdrp_p->cdrp$l_rspid;
    scs1->scs$l_xct_len = cdrp_p->cdrp$l_xct_len;
    //insque(cdrp_p,&pdt_p->pdt$q_comql);
    scs_lower_level_send(cdrp_p, scs);
}

int scs_std$request_data(struct _pdt *pdt_p, struct _cdrp *cdrp_p,
                         void (*complete)())
{
    return scs_std$reqdata(pdt_p, cdrp_p, complete);
}

int scs_std$senddata(struct _pdt *pdt_p, struct _cdrp *cdrp_p,
                     void (*complete)())
{
    struct _scs * scs = kmalloc(sizeof(struct _scs), GFP_KERNEL);
    memset(scs, 0, sizeof(struct _scs));
    struct _scs1 * scs1 = scs;
    struct _ppd * ppd = scs;
    struct _cdt * cdt = cdrp_p->cdrp$l_cdt;
    ppd->ppd$b_opc = PPD$C_SNDDAT;
    scs->scs$w_mtype = SCS$C_APPL_DG;
    scs1->scs$l_lconid = cdt->cdt$l_lconid;
    scs->scs$l_src_conid = cdt->cdt$l_lconid;
    scs->scs$l_dst_conid = cdt->cdt$l_rconid;
    scs1->scs$l_rspid = cdrp_p->cdrp$l_rspid;
    scs1->scs$l_xct_len = cdrp_p->cdrp$l_xct_len;
    //insque(cdrp_p,&pdt_p->pdt$q_comql);
    scs_lower_level_send(cdrp_p, scs);
}

int scs_std$send_data(struct _pdt *pdt_p, struct _cdrp *cdrp_p,
                      void (*complete)())
{
    return scs_std$senddata(pdt_p, cdrp_p, complete);
}

cwpsmyerr()
{
}

cwpslisten(void * packet, struct _cdt * c, struct _pdt * p)
{
    int sts;
    struct _iosb * iosb = kmalloc(sizeof(struct _iosb), GFP_KERNEL);
    memset(iosb, 0, sizeof(struct _iosb));
    struct _cdrp * cdrp;
    struct _scs * scs = packet;
    struct _cdt * cdt = &cdtl[scs->scs$l_dst_conid];
    struct _cwpssrv * cp = ((unsigned long) packet) + sizeof(*scs);
    void * next = (unsigned long) cp + sizeof(struct _cwpssrv);
    switch (cp->cwpssrv$b_subtype)
    {
    case CWPSSRV$K_FORCEX:
    {
        struct _cwpsfex * fex = next;
        exe$forcex(&cp->cwpssrv$l_sought_epid, 0, fex->cwpsfex$l_code);
    }
    break;
    default:
    {
    }
    }
}

void init_cwps(void)
{
    char myname[] = "cwps";
    char myinfo[] = "cwps service";

    //  listen(msgbuf,err,cdt,pdt,cdt);
    scs_std$listen(cwpslisten, cwpsmyerr, myname, myinfo, 0);
}

cwps$getjpi()
{
}

cwps$getjpi_pscan()
{
}

cwps$srcv_getjpi_ast()
{
}

// module cwps_service_recv

#if 0
int ddb_transfer(struct _cdt * conf_cdt)
{
    struct _ddb * ddb;
    struct _ucb * ucb;
    char buf[512];
    memset(buf,0,512);
    char *b = buf;
    int i;
    struct _cdrp * cdrp=kmalloc(sizeof(struct _cdrp),GFP_KERNEL);
    memset(cdrp,0,sizeof(struct _cdrp));
    struct _scs_rd *r;

    __du_init(); //temp placement?

    // now to transfer ddb

    printk("initiating ddb transfer\n");

    ddb = ioc$gl_devlist;

    while (ddb)
    {
        if (ddb->ddb$ps_sb==0 || ddb->ddb$ps_sb==&mysb)
        {
            *b++=DYN$C_DDB;
            memcpy(b,&ddb->ddb$t_name,16);
            b+=16;
            *b++=DYN$C_UCB;
            ucb=ddb->ddb$ps_ucb;
            *b++=ucb->ucb$w_unit_seed;
        }
        ddb=ddb->ddb$ps_link;
    }
    *b++==0;
    *b++==0;
    *b++==0;
    *b++==0;

    cdrp->cdrp$l_rwcptr=0;
    cdrp->cdrp$l_rspid=scs_std$alloc_rspid(0,0,cdrp,0);
    scs_std$find_rdte( cdrp->cdrp$l_rspid, &r);
    r->rd$l_cdrp=cdrp;

    cdrp->cdrp$l_cdt=conf_cdt; //&cdtl[6]; // temp dummy ?
    cdrp->cdrp$w_cdrpsize=512;
    cdrp->cdrp$l_msg_buf=buf;
    cdrp->cdrp$l_xct_len=100+512;
    scs_std$senddg(0,100+512,cdrp);
    kfree(cdrp);
    printk("ending ddb transfer\n");

    // also more to transfer

}
#endif

extern int cf_msg();
extern int cf_dg();
extern int cf_err();
extern int dlm_msg();
extern int dlm_dg();
extern int dlm_err();
extern int du_msg();
extern int du_dg();
extern int du_err();
extern int forcex_msg();
extern int forcex_dg();
extern int forcex_err();

int startconnect(int none)
{
    struct _cdt * cdt;
    struct _cdt * conf_cdt;
    char * s;
    signed long long time = -10000000;

    printk("starting cluster connects\n");
    scs_std$connect(du_msg, du_dg, du_err, 0, 0, "mscp$disk", "vms$disk_cl_drv",
                    0, 0, 0, 0, 0/*s*/, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    // should be vms$disk_cl_drvr but use null-term for now

    exe$schdwk(0, 0, &time, 0);
    sys$hiber();

    scs_std$connect(cf_msg, cf_dg, cf_err, 0, 0, "configure", "configclient", 0,
                    0, 0, 0, 0/*s*/, &conf_cdt, 0, 0, 0, 0, 0, 0, 0, 0);

    exe$schdwk(0, 0, &time, 0);
    sys$hiber();

    scs_std$connect(forcex_msg, forcex_dg, forcex_err, 0, 0, "cwps",
                    "cwps$forcex", 0, 0, 0, 0, 0, &cdt, 0, 0, 0, 0, 0, 0, 0, 0);

    exe$schdwk(0, 0, &time, 0);
    sys$hiber();

    scs_std$connect(dlm_msg, dlm_dg, dlm_err, 0, 0, "dlm$dlm",
                    &current->pcb$t_lname, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    exe$schdwk(0, 0, &time, 0);
    sys$hiber();

    // mscp$disk is there by default, so it is mandatory polled
    struct _prcpol prcpol_, *prcpol = &prcpol_;
    memset(prcpol, 0, sizeof(struct _prcpol));
    memcpy(&prcpol->prcpol$b_prcnam, "mscp$disk", 9);
    memcpy(&prcpol->prcpol$t_nodnam, &othersb.sb$t_nodename[0],
           1 + othersb.sb$t_nodename[0]);
    extern short int cf_mbxchan;
    struct _iosb iosb;
    printk("just before mbx write to configure\n");
    int sts = sys$qiow(0, cf_mbxchan, IO$M_NOW | IO$_WRITEVBLK, &iosb, 0, 0,
                       prcpol, sizeof(struct _prcpol), 0, 0, 0, 0);
    printk("just after mbx write to configure %x %x\n", sts,
           iosb.iosb$w_status);

#if 0
    ddb_transfer(conf_cdt);
    {
        signed long long time=-100000000;
        //      $DESCRIPTOR(tensec,"0 00:00:10.00");
        //exe$bintim(&tensec,&time);
        //time=-time;
        exe$setimr(0,&time,ddb_transfer,conf_cdt,0);
    }
#endif
}

