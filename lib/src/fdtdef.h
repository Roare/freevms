#ifndef FDTDEF_H
#define FDTDEF_H

#include <vms_types.h>

#include <ccbdef.h>
#include <irpdef.h>
#include <pcbdef.h>
#include <ucbdef.h>

#define FDT$K_LENGTH 528

struct __fdt
{
    UINT64 fdt$q_valid;
    //    UINT64 fdt$q_ok64bit;
    UINT64 fdt$q_buffered;
    struct
    {
        UINT64 fdt$q_mask;
        int (*fdt$ps_func_rtn)(struct _irp * i, struct _pcb * p, struct _ucb * u, struct _ccb * c);
    } f[64];
    //    int (*fdt$ps_func_rtn [64])(struct _irp * i, struct _pcb * p, struct _ucb * u, struct _ccb * c);
    //    unsigned long fdt$l_qsrv_sts2 [64];
};

struct _fdt
{
    UINT64 fdt$q_valid;
    UINT64 fdt$q_buffered;
    int (*fdt$ps_func_rtn [64])(struct _irp * i, struct _pcb * p, struct _ucb * u, struct _ccb * c);
};

#endif

