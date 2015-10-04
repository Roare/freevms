// $Id$
// $Locker$

// Author. Roar Thron�s.

#include <ddbdef.h>
#include <descrip.h>
#include <cli$routines.h>

#include <stdio.h>
#include <string.h>

int sda$getmem(const void *src, void *dest, int n);

int show_device()
{
    printf("\n");
    printf("I/O data structures\n");
    printf("-------------------\n");
    printf("                                DDB list\n");
    printf("                                --------\n");
    printf("\n");
    printf("  Address     Controller     ACP                Driver                DPT\n");
    printf("  --------    ----------    ------    --------------------------    --------\n");
    printf("\n");

    int retlen;
    $DESCRIPTOR(p, "p1");
    char c[80];
    struct dsc$descriptor o;
    o.dsc$a_pointer=c;
    o.dsc$w_length=80;
    memset (c, 0, 80);
    int sts = cli$present(&p);
    if (sts&1)
    {
        sts = cli$get_value(&p, &o, &retlen);
        o.dsc$w_length=retlen;
    }
    extern void * ioc$gl_devlist;
    struct _ddb ddb, * kddb;
    sts = sda$getmem(&ioc$gl_devlist, &kddb, 0);
    for (; kddb; kddb=ddb.ddb$ps_link)
    {
        sts = sda$getmem(kddb, &ddb, sizeof (struct _ddb));
        printf("  %8lx    %10s        %26s       %8lx\n", (unsigned long) kddb, &ddb.ddb$t_name[1], "",
                (unsigned long) ddb.ddb$ps_dpt);
    }
    return sts;
}
