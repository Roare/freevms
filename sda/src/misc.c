// $Id$
// $Locker$

// Author. Roar Thron�s.

// like memcpy

#include<starlet.h>

int memcpy(void *dest, const void *src, int n);

int sda$getmem(const void *src, void *dest, int n)
{
    if (n==0)
        n = sizeof(void *);
    long arglst[4];
    arglst[0]=3;
    arglst[1]=dest;
    arglst[2]=src;
    arglst[3]=n;
    sys$cmkrnl(memcpy, &arglst[0]);
}

int sda$getmemlong(const void *src, void *dest)
{
    return sda$getmem(src, dest, sizeof (long));
}

int sda$exit()
{
    // not yet exit(0);
    extern int do_ret;
    do_ret = 1;
}
