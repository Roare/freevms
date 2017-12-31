// $Id$
// $Locker$

// Author. Roar Thron�s.

#include<linux/linkage.h>
#include<system_data_cells.h>
#include<ucbdef.h>
#include<irpdef.h>
#include<fkbdef.h>
#include<ipldef.h>
#include<ipl.h>
#include<internals.h>
#include <linux/smp.h>
#include <asm/current.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <asm/hw_irq.h>
#include <queue.h>

asmlinkage void exe$forkdspth(int i);
void exe$fork(struct _irp * i, struct _ucb * u);
void exe$queue_fork(struct _irp * i, struct _ucb * u);

struct fork_lock_struct
{
    struct _spl * spin;
    int ipl;
};

// call this smp$al_iplvec?
static struct fork_lock_struct forklock_table[7]=
{
    { &SPIN_IOLOCK8, IPL$_IOLOCK8 },
    { 0, 0 },
    { &SPIN_IOLOCK8, IPL$_IOLOCK8 },
    { &SPIN_IOLOCK9, IPL$_IOLOCK9 },
    { &SPIN_IOLOCK10, IPL$_IOLOCK10 },
    { &SPIN_IOLOCK11, IPL$_IOLOCK11 },
    { &SPIN_MAILBOX, IPL$_MAILBOX }
};

/**
   \brief forklock - see 5.2 8.3.7
   \details forklock with spinlock and setipl
   \details not 100% according to spec
   \param i lock type
   \param j new ipl
*/

int inline forklock(int i, signed int j)
{
    if (i<6 || i == 7 || i > 12)
        panic("forklock\n");
    if (j==-2) j=forklock_table[i-6].ipl;
    return vmslock(forklock_table[i-6].spin, j);
}

/**
   \brief forkunlock - see 5.2 8.3.7
   \details forkunlock with spinunlock and setipl
   \details not 100% according to spec
   \param i lock type
   \param j new ipl
*/

void inline forkunlock(int i, signed int j)
{
    if (i<6 || i == 7 || i > 12)
        panic("forkunlock\n");
    if (j==-2) j=forklock_table[i-6].ipl;
    vmsunlock(forklock_table[i-6].spin, j);
}

/**
   \brief fork soft int - see 5.2 22.3.4
*/

asmlinkage void exe$frkipl6dsp(void)
{
    exe$forkdspth(6);
}

/**
   \brief fork soft int - see 5.2 22.3.4
*/

asmlinkage void exe$frkipl8dsp(void)
{
    exe$forkdspth(8);
}

/**
   \brief fork soft int - see 5.2 22.3.4
*/

asmlinkage void exe$frkipl9dsp(void)
{
    exe$forkdspth(9);
}

/**
   \brief fork soft int - see 5.2 22.3.4
*/

asmlinkage void exe$frkipl10dsp(void)
{
    exe$forkdspth(10);
}

/**
   \brief fork soft int - see 5.2 22.3.4
*/

asmlinkage void exe$frkipl11dsp(void)
{
    exe$forkdspth(11);
}

/**
   \brief requesting i/o completion processing - see 5.2 22.3.4
   \param i interrupt
*/

asmlinkage void exe$forkdspth(int i)
{
    void (*func)(void *,void *, void *);
    struct _fkb * f, * dummy = 0, * fq;
#ifdef __x86_64__
    if (intr_blocked(i))
        return;
    regtrap(REG_INTR, i);
#endif
    setipl(i);
    //printk("forkdspth\n");
    /** remove one fork block at a time from the appropriate fork queue */
    fq=smp$gl_cpu_data[smp_processor_id()]->cpu$q_swiqfl[i-6]; /* so far */
    while (!aqempty(fq))
    {
        f=remque(fq,dummy);
        //printk("forking entry %x\n",f);
        func=f->fkb$l_fpc;
        /** get forklock */
        int savipl = vmslock(forklock_table[i-6].spin, forklock_table[i-6].spin);
        /** call fork routine */
        func(f->fkb$l_fr3,f->fkb$l_fr4,f);
        /** unlock fork */
        vmsunlock(forklock_table[i-6].spin, savipl);
        /** next entry TODO redo maybe */
        fq=smp$gl_cpu_data[smp_processor_id()]->cpu$q_swiqfl[0]; /* so far */
    }
}

/**
   \brief servicing the device interrupt - see 5.2 22.3.3
   \param i irp
   \param u ucb
*/

void exe$iofork(struct _irp * i, struct _ucb * u)
{
    /** clear tim bit */
    u->ucb$l_sts&=~UCB$M_TIM;
    /** call exe$fork */
    exe$fork(i,u);
}

/**
   \brief servicing the device interrupt - see 5.2 22.3.3
   \param i irp
   \param u ucb
*/

void exe$fork(struct _irp * i, struct _ucb * u)
{
    u->ucb$l_fr3=i;
    /* should also popl fkb$l_fpc(r5) according to the book. */
    /** call queue_fork */
    exe$queue_fork(i,u);
}

/**
   \brief servicing the device interrupt - see 5.2 22.3.3
   \param i irp
   \param u ucb
*/

void exe$queue_fork(struct _irp * i, struct _ucb * u)
{
    int curipl;
    int newipl;
    int isempty;
    struct _fkb * f=u;
    /* I think that the below is really an fkb */
    /* need caller and caller's caller address of return again */
    // u->ucb$l_fr4=current; // wrong?
    /** get fork ipl - TODO smp/up support? */
    newipl=f->fkb$b_flck;
    /** locate fork block queue and insert fork block */
    f=smp$gl_cpu_data[smp_processor_id()]->cpu$q_swiqfl[newipl-6];
    isempty=aqempty(f);
    insque(u,f->fkb$l_fqbl);
    /** if first element request softint for it */
    if (isempty)
    {
        /* do SOFTINTS */
        switch (newipl)
        {
        case IPL$_IOLOCK11:
            SOFTINT_IOLOCK11_VECTOR;
            break;
        case IPL$_IOLOCK10:
            SOFTINT_IOLOCK10_VECTOR;
            break;
        case IPL$_IOLOCK9:
            SOFTINT_IOLOCK9_VECTOR;
            break;
        case IPL$_IOLOCK8:
            SOFTINT_IOLOCK8_VECTOR;
            break;
        case IPL$_QUEUEAST:
            SOFTINT_QUEUEAST_VECTOR;
            break;
        default:
            panic("in forkdspth\n");
        }
    }
}

void exe_std$queue_fork(struct _fkb * fkb)
{
    exe$queue_fork(0,fkb);
}

// replaces exe$fork
void exe_std$primitive_fork(long fr3, long fr4, struct _fkb * fkb)
{
    fkb->fkb$l_fr3=fr3;
    fkb->fkb$l_fr4=fr4;
    exe_std$queue_fork(fkb);
}
