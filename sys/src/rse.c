// $Id$
// $Locker$

#include<linux/sched.h>
#include "../../freevms/sys/src/asmlink.h"
#include<asm/hw_irq.h>

#include"../../freevms/sys/src/sysgen.h"
#include"../../freevms/sys/src/rse.h"
#include"../../freevms/lib/src/pridef.h"
#include"../../freevms/lib/src/evtdef.h"
#include"../../freevms/lib/src/statedef.h"
#include"../../freevms/lib/src/cpudef.h"
#include"../../freevms/sys/src/system_data_cells.h"
#include"../../freevms/sys/src/internals.h"
#include"../../freevms/pal/src/queue.h"

extern int mydebug;

int sch$qend(struct _pcb * p) {
  int cpuid=smp_processor_id();
  struct _cpu * cpu=smp$gl_cpu_data[cpuid];
  vmslock(&SPIN_SCHED,-1);

  p->pcb$l_pixhist=p->pcb$l_pixhist << 1;
  p->phd$w_quant = -QUANTUM;
  p->pcb$l_onqtime=exe$gl_abstim_tics;
  p->pcb$l_sts=p->pcb$l_sts & ( ~ PCB$M_INQUAN );
  if (ffs(sch$gl_comoqs)) {
    p->pcb$b_pri=p->pcb$b_prib;
    /* cpu->cpu$b_cur_pri=
       not quite finished
     */
  }
  //  p->need_resched = 1;
  if (mydebug) printk("quend %x %x\n",p->pid,p->need_resched);
  {
    //    struct list_head * tmp;
    struct _pcb * e, * next;
    unsigned char c;
    int tmppri;
    //    list_for_each(tmp, &runqueue_head) {
    //      e = list_entry(tmp, struct task_struct, run_list);
    //      if (e->pcb$b_pri <= c) c=e->pcb$b_pri, next=e;
    //    }
    //SOFTINT_RESCHED_VECTOR;
    // return;
    tmppri=ffs(sch$gl_comqs);
    tmppri--;
    if (tmppri<=p->pcb$b_pri) {
	if (p->pcb$b_pri != p->pcb$b_prib) ++p->pcb$b_pri;
	//	 sch$resched(); /*no interrupt yet*/ /*did not work*/
	//SOFTINT_RESCHED_VECTOR;
	 p->need_resched = 1;       
	 /* lacks some sch stuff */
    }
    else
      {
	p->need_resched=1;
	//	SOFTINT_RESCHED_VECTOR; a bit too early. get scheduling in interrupt and crash
      }
  }
  vmsunlock(&SPIN_SCHED,-1);
  return 1;
}

int sch$pixscan(void) {
  unsigned long tmp;
  struct _pcb *tmp2;
  unsigned long comqs=sch$gl_comqs & 0x7fff0000;
  int tmppri,i,scanned=sgn$gw_pixscan;

  //printk("pixscan 1\n");

  if (!sgn$gw_pixscan) return;
  /* get sched spinlock */
  vmslock(&SPIN_SCHED,-1);
  if (!((sch$gl_comqs & 0x7fff0000) || (sch$gl_comoqs & 0x7fff0000))) goto out;
  tmppri=ffs(comqs);
  tmppri--;
  
  //printk("pri %x|",tmppri);
  goto out; /* not this yet */
  for(i=30;i>tmppri,scanned;i--) {
    tmp=sch$aq_comh[i];
    if(*(unsigned long *)tmp == tmp) {; } else {
      tmp2=tmp;
      do {
	//printk("tmp2 %x|",tmp2);
	if (!(tmp2>=&sch$aq_comh[0] && tmp2<=&sch$aq_comh[33])) {
	  if (tmp2->pcb$l_onqtime+100*sch$gw_dormantwait<exe$gl_abstim_tics) {
	    tmp2->pcb$l_pixhist|=1;
	    sch$chsep(tmp2,tmppri);
	    scanned--;
	    if (!scanned) goto out;
	  } else {
	    goto out;
	  }
	}
	tmp2=tmp2->pcb$l_sqfl;
      } while (tmp2!=tmp);
    }
  }
  out:
    /* release spinlock */
  //printk("\n2\n");
  vmsunlock(&SPIN_SCHED,-1);
  return;
}

void sch$chsep(struct _pcb * p,unsigned char newpri) {
  struct _pcb * p2, *p3 , *dummy;
  p3=p->pcb$l_sqfl;
  p2=remque(p,dummy);
  if (p3==p3->pcb$l_sqfl) {
      if (p2->pcb$w_state==SCH$C_COM) {
	sch$gl_comqs&=(~(1 << p2->pcb$b_pri));
      } else if (p2->pcb$w_state==SCH$C_COMO) {
	sch$gl_comoqs&=(~(1 << p2->pcb$b_pri));
      }
  }
  p->pcb$b_pri=newpri;
  //  if (!(p->pcb$l_sts & PCB$M_RES)) { not yet used
  if (0) {
    sch$swpwake();
    p2->pcb$w_state=SCH$C_COMO;
    p2->state=TASK_RUNNING;
    insque(p2,sch$aq_comot[newpri]);
    sch$gl_comoqs|=(1 << newpri);
    return;
  }
  //  SOFTINT_RESCHED_VECTOR; not yet?
  p2->pcb$w_state=SCH$C_COM;
  p2->state=TASK_RUNNING;
  if (!task_on_comqueue(p2)) insque(p2,sch$aq_comt[newpri]);
  sch$gl_comqs|=(1 << newpri);
}

// need more statequeues before sch$chse*
  void sch$chse(struct _pcb * p, unsigned char class) {
    unsigned char pri=0,newpri;

    switch (class) {
    case PRI$_NULL:
      pri=0;
      break;
    case PRI$_IOCOM:
      pri=2;
      break;
    case PRI$_TIMER:
    case PRI$_RESAVL:
      pri=3;
      break;
    case PRI$_TOCOM:
      pri=4;
      break;
    case PRI$_TICOM:
      pri=6;
      break;
    }
    newpri=p->pcb$b_prib-pri;
    if (newpri>p->pcb$b_pri) newpri=p->pcb$b_pri;
    if (p->pcb$b_pri<16) p->pcb$b_pri=p->pcb$b_prib;
    sch$chsep(p,newpri);
  }

void sch$wake(unsigned long pid) {
  struct _pcb * p=find_process_by_pid(pid);
  if (!p) return;
  p->pcb$l_sts |= PCB$M_WAKEPEN;
  sch$rse(p,PRI$_RESAVL,EVT$_WAKE);
}

void sch$swpwake(void) {
  if (sch$gl_comoqs) goto wake;

  return;
 wake:
  sch$wake(sch$gl_swppid); /*not yet set*/
  return;
}

void sch$rse(struct _pcb * p, unsigned char class, unsigned char event) {
  unsigned long dummy;
  struct _pcb *p2;

  switch (event) {
  case EVT$_AST:
    switch (p->pcb$w_state) {
    case SCH$C_SUSP:
    case SCH$C_SUSPO:
    case SCH$C_COM:
    case SCH$C_COMO:
    case SCH$C_CUR:
      break;
    default:
      goto ast;
    }

  case EVT$_EVENT: {
    if (p->pcb$w_state==SCH$C_LEF) goto event;
    if (p->pcb$w_state==SCH$C_CEF) goto cef;
    if (p->pcb$w_state==SCH$C_LEFO) goto lefo;
  }

  case EVT$_CEF: {
    if (p->pcb$w_state==SCH$C_CEF) goto cef;
    if (p->pcb$w_state==SCH$C_LEFO) goto lefo;
  }

  case EVT$_LEFO:
    if (p->pcb$w_state==SCH$C_LEFO) goto lefo;

  case EVT$_FPGA:
    if (p->pcb$w_state==SCH$C_FPG) goto fpga;

  case EVT$_WAKE:
    switch (p->pcb$w_state) {
    case SCH$C_HIB:
    case SCH$C_HIBO:
      goto wake;
    }     

  case EVT$_RESUME:
    switch (p->pcb$w_state) {
    case SCH$C_SUSP:
    case SCH$C_SUSPO:
      goto resume;
    }

  case EVT$_PFCOM:
    if (p->pcb$w_state==SCH$C_PFW) goto pfcom;

  case EVT$_SETPRI:
    switch (p->pcb$w_state) {
    case SCH$C_COM:
    case SCH$C_COMO:
      goto setpri;
    }

  case EVT$_SWPOUT:
    switch (p->pcb$w_state) {
    case SCH$C_COM:
    case SCH$C_HIB:
    case SCH$C_LEF:
    case SCH$C_SUSP:
      goto swpout;
    }
  }

  /* bugcheck */
  return;

 cef:
  if (!(p->pcb$l_sts & PCB$M_RES)) goto lefo;
 event:
  /* add stuff to pc no use here */
  goto lefo;

 lefo:
  goto common;

 ast:
 colpga:
 fpga:
 wake:
 resume:
 pfcom:
 setpri:

 common:
  //  p2=remque(p,dummy);
  //  sch$aq_wqhdr[p->pcb$w_state].wqh$l_wqcnt--;
  p->pcb$l_onqtime+=(exe$gl_abstim_tics-p->pcb$l_waitime);
  sch$unwait(p);
  sch$chse(p,class);
  return;

 swpout:
  if (p->pcb$w_state!=SCH$C_COM) {
    p2=remque(p,dummy);
    sch$aq_wqhdr[p->pcb$w_state].wqh$l_wqcnt--;
    (p->pcb$w_state)++;
    insque(p,sch$aq_wqhdr[p->pcb$w_state].wqh$l_wqfl);
    sch$aq_wqhdr[p->pcb$w_state].wqh$l_wqcnt++;
  } else {
    int tmppri=p->pcb$b_pri;
    unsigned long qhead;

    p2=remque(p,dummy);
    if (sch$aq_comh[tmppri]==((struct _pcb *) sch$aq_comh[tmppri])->pcb$l_sqfl)
      sch$gl_comqs=sch$gl_comqs & (~(1 << tmppri));
    (p->pcb$w_state)++;
    sch$gl_comoqs=sch$gl_comoqs | (1 << tmppri);
    qhead=sch$aq_comot[tmppri];
    insque(p,qhead);
  }
  return;
}

void sch$change_cur_priority(struct _pcb *p, unsigned char newpri) {
  /* lacks sch$al_cpu etc stuff */
  int tmppri;
  struct _pcb * p2;
  int cpuid = smp_processor_id();
  struct _cpu * cpu=smp$gl_cpu_data[cpuid];
  cpu->cpu$b_cur_pri=newpri;
  p2=cpu->cpu$l_curpcb;
  p2->pcb$b_pri=newpri;
  /* lacks sch$al_cpu etc stuff */
  tmppri=ffs(sch$gl_comqs);
  tmppri--;
  if (newpri>=tmppri) return;
  SOFTINT_RESCHED_VECTOR; /* or set need_resched if interrupt probs */
  // interprocessor interrupt not implemented?
}

void sch$one_sec(void) {
  sch$pixscan();
    }

int sch$waitm(struct _pcb * p, struct _wqh * wq) {
  /* do no phd stuff yet */
  p->pcb$l_waitime=exe$gl_abstim_tics;
  /* check queued ast */
  if (p->phd$b_astlvl==4) {
    sch$sched(0);
    return;
  }
  // sch$sched(0); /* do it anyway until asts are implemented */
  /* check psl and ast stuff */
  if (!(p->psl_cur_mod >= p->pr_astlvl)) {
    sch$sched(0);
    return;
  }
  if (p->psl_ipl!=0) {
    sch$sched(0);
    return;
  }
  sch$rse(p,PRI$_NULL,EVT$_AST);
  sch$sched(0);
}

int sch$waitl(struct _pcb * p, struct _wqh * wq) {
  /* do a svpctx ? */
  return sch$waitm(p,wq);
}

int sch$waitk(struct _pcb * p, struct _wqh * wq) {
  p->pcb$w_state=wq->wqh$l_wqstate;
  p->state=TASK_UNINTERRUPTIBLE;
  insque(p,wq->wqh$l_wqfl);
  wq->wqh$l_wqcnt++;
  return sch$waitl(p,wq);
}

int sch$wait(struct _pcb * p, struct _wqh * wq) {
  /* remove call frame etc ? */
  return sch$waitk(p,wq);
}

void sch$chsep2(struct _pcb * p,unsigned char newpri) {
  struct _pcb * p2, *p3 , *dummy;
  p3=p->pcb$l_sqfl;
  p2=p;
  /*
  if (p3==p3->pcb$l_sqfl) {
      if (p2->state==TASK_RUNNING) {
	sch$gl_comqs&=(~(1 << p2->pcb$b_pri));
      } else if (p2->pcb$w_state==SCH$C_COMO) {
	sch$gl_comoqs&=(~(1 << p2->pcb$b_pri));
      }
  }
  */
  p->pcb$b_pri=newpri;
  /*
  if (!(p->pcb$l_sts & PCB$M_RES)) {
    sch$swpwake();
    p2->pcb$w_state=SCH$C_COMO;
    p2->state=TASK_RUNNING;
    insque(p2,sch$aq_comot[newpri]);
    sch$gl_comoqs|=(1 << newpri);
    return;
  }
  */
  //SOFTINT_RESCHED_VECTOR;
  p2->state=TASK_RUNNING;
  insque(p2,sch$aq_comt[newpri]);
  sch$gl_comqs|=(1 << newpri);
}

// need more statequeues before sch$chse*
  void sch$chse2(struct _pcb * p, unsigned char class) {
    unsigned char pri=0,newpri;

    switch (class) {
    case PRI$_NULL:
      pri=0;
      break;
    case PRI$_IOCOM:
      pri=2;
      break;
    case PRI$_TIMER:
    case PRI$_RESAVL:
      pri=3;
      break;
    case PRI$_TOCOM:
      pri=4;
      break;
    case PRI$_TICOM:
      pri=6;
      break;
    }
    newpri=p->pcb$b_prib-pri;
    if (newpri>p->pcb$b_pri) newpri=p->pcb$b_pri;
    if (p->pcb$b_pri<16) p->pcb$b_pri=p->pcb$b_prib;
    sch$chsep2(p,newpri);
  }

void sch$unwait(struct _pcb * p) {

}
