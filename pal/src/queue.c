// $Id$
// $Locker$

// Author. Roar Thron�s.
/**
   \file queue.c
   \brief CPU queue instructions
   \author Roar Thron�s
 */

/* Not implemented: atomicity */
/* Not implemented: long and quadword alignments */

/* Hmm C is too strongly typed */

/* These are VARM based. 32 bits */

/* Author: Roar Thron�s */

#include<queue.h>

#ifdef __i386__
#define OFFSET 4
#endif

#ifdef __x86_64__
#define OFFSET 8
#endif

#include<linux/kernel.h>

long inline mycli(void);
void inline mysti(long flags);

void insque(void * entry, void * pred)
{
    if (entry==pred) panic("same\n");
    if (entry==*(long *)pred) panic("same\n");
    if (entry==*(long *)(((long)pred)+OFFSET)) panic("same\n");
    int flag=mycli();
    //mycheckaddr();
    *(void **)entry=*(void **)pred;
    *(void **)(entry+OFFSET)=pred;
    *(void **)((*(void **)pred)+OFFSET)=entry;
    *(void **)pred=entry;
    //mycheckaddr();
    mysti(flag);
}

unsigned long remque(void * entry, void * addr)
{
    int flag=mycli();
    //mycheckaddr();
    *(void **)(*(void **)(entry+OFFSET))=*(void **)entry;
    *(void **)((*(void **)entry)+OFFSET)=*(void **)(entry+OFFSET);
    addr=entry;
    //mycheckaddr();
    mysti(flag);
    return (unsigned long) addr;
}

/* These are AARM based. Still 32 bits */

void insqhi(void * entry, void * header)
{
    int flag=mycli();
    signed long tmp1=*(signed long *)header;
    signed long tmp2=(signed long)(header-entry);
    /* cli() or a semaphore/mutex? */
    *(signed long *)entry=tmp1+tmp2;
    *(signed long *)(entry+OFFSET)=tmp2;
    *(signed long *)(header+(signed long)(tmp1)+OFFSET)=-tmp1-tmp2;
    *(signed long *)header=-tmp2;
    /* sti() */
    mysti(flag);
}

void insqti (void * entry, void * header)
{
    int flag=mycli();
    signed long tmp1=*(signed long *)header;
    signed long tmp2=*(signed long *)(header+OFFSET);
    signed long tmp3=(signed long)(header-entry);
    /* cli() or a semaphore/mutex? */
    *(signed long *)entry=tmp3;
    *(signed long *)(entry+OFFSET)=tmp2+tmp3;
    if (tmp2)
        *(signed long *)(header+(signed long)(tmp2))=-tmp2-tmp3;
    else
        tmp1=-tmp3-tmp2;
    *(signed long *)(header+OFFSET)=-tmp3;
    *(signed long *)header=tmp1;
    /* sti() */
    mysti(flag);
}

unsigned long remqhi(void * header, void * addr)
{
    int flag=mycli();
    signed long tmp1=*(signed long *)header;
    signed long tmp2=(signed long)header+tmp1;
    signed long tmp3;
    /* cli() or a semaphore/mutex? */
    if (!tmp1)
        tmp3=(signed long) header;
    else
        tmp3=tmp2+*(signed long *)tmp2;
    *((signed long *)(tmp3+OFFSET))=(signed long)header-tmp3;
    *(signed long *)header=tmp3-(signed long)header;
    addr=(signed long *)tmp2;
    mysti(flag);
    return (unsigned long) addr;
    /* sti() */
}

unsigned long remqti(void * header, void * myaddr)
{
    int flag=mycli();
    /*signed long tmp1=*header;*/
    signed long tmp5=*(signed long *)(header+OFFSET);
    signed long addr=(signed long)header+tmp5;
    signed long tmp2=addr+*((signed long *)(addr+OFFSET));
    /* cli() or a semaphore/mutex? */
    *(signed long *)(header+OFFSET)=tmp2-(signed long)header;
    if (tmp2 == (signed long) header)
        *(signed long *)header=0;
    else
        *(signed long *)tmp2=(signed long)header-tmp2;
    myaddr=(signed long *) addr;
    mysti(flag);
    return (unsigned long) myaddr;
    /* sti() */
}

struct myq
{
    unsigned long flink;
    unsigned long blink;
};

/**
   \brief is absolute queue empty
   \param q queue header
*/

inline int aqempty(void * q)
{
    struct myq *m=(struct myq *) q;
    return (q==m->flink);
}

/**
   \brief is relative queue empty
   \param q queue header
*/

inline int rqempty(void * q)
{
    struct myq *m=(struct myq *) q;
    return (m->flink==0 && m->blink==0);
}

/**
   \brief insqti for use at boottime
   \param entry entry
   \param header of queue
*/

void boot_insqti (void * entry, void * header)
{
    signed long tmp1=*(signed long *)header;
    signed long tmp2=*(signed long *)(header+OFFSET);
    signed long tmp3=(signed long)(header-entry);
    /* cli() or a semaphore/mutex? */
    *(signed long *)entry=tmp3;
    *(signed long *)(entry+OFFSET)=tmp2+tmp3;
    if (tmp2)
        *(signed long *)(header+(signed long)(tmp2))=-tmp2-tmp3;
    else
        tmp1=-tmp3-tmp2;
    *(signed long *)(header+OFFSET)=-tmp3;
    *(signed long *)header=tmp1;
}

INSQUE(long x, long y)
{
    insque(x,y);
    return 1; //SS$_NORMAL;
}

REMQUE(long * e, long * a)
{
    int retval=0;
    if (e==e[1]) retval|=1;
    remque(e,0);
    if (e[0]==e[1]) retval|=2;
    *a=e;
    return retval;
}

//__PAL_INSQUEL_D

//__PAL_REMQUEL_D
