/* Not implemented: atomicity */
/* Not implemented: long and quadword alignments */

/* Hmm C is too strongly typed */

/* These are VARM based. 32 bits */

/* Author: Roar Thron�s */

#include"queue.h"

int in_insque=0;

void insque(void * entry, void * pred) {
if (in_insque) panic("aaaah\n");
in_insque=1;
*(void **)entry=*(void **)pred;
*(void **)(entry+4)=pred;
*(void **)((*(void **)pred)+4)=entry;
*(void **)pred=entry;
in_insque=0;
}

unsigned long remque(void * entry, void * addr) {
*(void **)(*(void **)(entry+4))=*(void **)entry;
*(void **)((*(void **)entry)+4)=*(void **)(entry+4);
addr=entry;
return (unsigned long) addr;
}

/* These are AARM based. Still 32 bits */

void insqhi(void * entry, void * header) {
signed long tmp1=*(signed long *)header;
signed long tmp2=(signed long)(header-entry);
/* cli() or a semaphore/mutex? */
*(signed long *)entry=tmp1+tmp2;
*(signed long *)(entry+4)=tmp2;
*(signed long *)(header+(signed long)(tmp1)+4)=-tmp1-tmp2;
*(signed long *)header=-tmp2;
/* sti() */
}

void insqti (void * entry, void * header) {
signed long tmp1=*(signed long *)header;
signed long tmp2=*(signed long *)(header+4);
signed long tmp3=(signed long)(header-entry);
/* cli() or a semaphore/mutex? */
*(signed long *)entry=tmp3;
*(signed long *)(entry+4)=tmp2+tmp3;
if (tmp2)
 *(signed long *)(header+(signed long)(tmp2))=-tmp2-tmp3;
else
 tmp1=-tmp3-tmp2;
*(signed long *)(header+4)=-tmp3;
*(signed long *)header=tmp1;
/* sti() */
}

unsigned long remqhi(void * header, void * addr) {
signed long tmp1=*(signed long *)header;
signed long tmp2=(signed long)header+tmp1;
signed long tmp3;
/* cli() or a semaphore/mutex? */
if (!tmp1)
 tmp3=(signed long) header;
else
 tmp3=tmp2+*(signed long *)tmp2;
*((signed long *)(tmp3+4))=(signed long)header-tmp3;
*(signed long *)header=tmp3-(signed long)header;
addr=(signed long *)tmp2;
return (unsigned long) addr;
/* sti() */
}

unsigned long remqti(void * header, void * myaddr) {
/*signed long tmp1=*header;*/
signed long tmp5=*(signed long *)(header+4);
signed long addr=(signed long)header+tmp5;
signed long tmp2=addr+*((signed long *)(addr+4));
/* cli() or a semaphore/mutex? */
*(signed long *)(header+4)=tmp2-(signed long)header;
if (tmp2 == (signed long) header)
 *(signed long *)header=0;
else
 *(signed long *)tmp2=(signed long)header-tmp2;
myaddr=(signed long *) addr;
return (unsigned long) myaddr;
/* sti() */
}

struct myq {
unsigned long flink;
unsigned long blink;
};

inline int aqempty(void * q) {
struct myq *m=(struct myq *) q;
return (q==m->flink);
}

inline int rqempty(void * q) {
struct myq *m=(struct myq *) q;
return (m->flink==0 && m->blink==0);
}

//__PAL_INSQUEL_D

//__PAL_REMQUEL_D
