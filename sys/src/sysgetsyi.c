// $Id$
// $Locker$

// Author. Roar Thron�s.

#include<linux/linkage.h>

#include<syidef.h>
#include<ssdef.h>
#include<misc.h>
#include<pridef.h>
#include<sbdef.h>
#include<starlet.h>
#include <exe_routines.h>
#include <sch_routines.h>

#include<linux/sched.h>

#include<linux/version.h>

extern struct _sb mysb;

asmlinkage int exe$getsyi(unsigned int efn, unsigned int *csidadr, void *nodename, void *itmlst, struct _iosb *iosb, void (*astadr)(), unsigned long astprm)
{
    int sts;
    struct item_list_3 * it=itmlst;
    // no use of namcsid yet
    if (iosb)
    {
        // check iosb write access
        memset(iosb,0,sizeof(struct _iosb));
    }
    exe$clref(efn);
    // check ast quota
    // check all access in itemlist

    while (it->item_code)
    {
        switch (it->item_code)
        {
        case SYI$_VERSION:
            memcpy(it->bufaddr, FREEVMS_RELEASE, strlen(FREEVMS_RELEASE));
            break;

        case SYI$_SCSNODE:
            memcpy(it->bufaddr,&mysb.sb$t_nodename,15);
            break;

        case SYI$_BOOTTIME:
            memcpy(it->bufaddr,&exe$gq_boottime,8);
            break;

        case SYI$_MAXBUF:
            memcpy(it->bufaddr,&ioc$gw_maxbuf,sizeof(long));
            break;

        case SYI$_LASTFLD:
        {
            int mem=1;
            memcpy(it->bufaddr,&mem,4);
        }
        break;

        default:
            printk("unrecognized syi\n");
            break;
        }
        it++;
    }

    struct _pcb * pcb = ctl$gl_pcb;
    sch$postef(pcb->pcb$l_pid, PRI$_NULL, efn);

    if (iosb)
        iosb->iosb$w_status=SS$_NORMAL;

    return SS$_NORMAL;
}

asmlinkage int exe$getsyiw(unsigned int efn, unsigned int *csidadr, void *nodename, void *itmlst, struct _iosb *iosb, void (*astadr)(), unsigned long astprm)
{
    int status=exe$getsyi(efn,csidadr,nodename,itmlst,iosb,astadr,astprm);
    if ((status&1)==0) return status;
    return exe$synch(efn,iosb);
}

asmlinkage int exe$getsyiw_wrap(struct struct_args * s)
{
    return exe$getsyiw(s->s1,s->s2,s->s3,s->s4,s->s5,s->s6,s->s7);
}


asmlinkage int exe$getsyi_wrap(struct struct_args * s)
{
    return exe$getsyi(s->s1,s->s2,s->s3,s->s4,s->s5,s->s6,s->s7);
}
