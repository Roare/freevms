// $Id$
// $Locker$

// Author. Roar Thron�s.

#include<linux/vmalloc.h>

#include<cddbdef.h>
#include<cdtdef.h>
#include<crbdef.h>
#include<ddbdef.h>
#include<ddtdef.h>
#include<devdef.h>
#include<dptdef.h>
#include<dyndef.h>
#include<fdtdef.h>
#include<iodef.h>
#include<ipldef.h>
#include<irpdef.h>
#include<mscpdef.h>
#include<pdtdef.h>
#include<ssdef.h>
#include<ucbdef.h>
#include<queue.h>

int scs_std$sendmsg(struct _irp * i, struct _pdt * p)
{
    char * buf = 0;
    printk("check this sendmsg\n");
    insque(buf,&p->pdt$q_comqh);
}

int scs_std$allocmsg()
{

}


int scs_std$deallomsg()
{

}

int scs_std$dconnect()
{

}

