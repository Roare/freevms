#ifndef AQBDEF_H
#define AQBDEF_H

#include <vms_types.h>

#define AQB$M_UNIQUE 0x1
#define AQB$M_DEFCLASS 0x2
#define AQB$M_DEFSYS 0x4
#define AQB$M_CREATING 0x8
#define AQB$M_XQIOPROC 0x10
#define AQB$K_UNDEFINED 0
#define AQB$K_F11V1 1
#define AQB$K_F11V2 2
#define AQB$K_MTA 3
#define AQB$K_NET 4
#define AQB$K_REM 5
#define AQB$K_HBS 6
#define AQB$K_F11V3 7
#define AQB$K_F11V4 8
#define AQB$K_F64 9
#define AQB$K_UCX 10
#define AQB$K_F11V5 11
#define AQB$K_F11V6 12
#define AQB$C_MAXACP 12
#define AQB$K_LENGTH 40
#define AQB$C_LENGTH 40
#define AQB$S_AQBDEF 40

struct _aqb
{
    struct _irp *aqb$l_acpqfl;
    struct _irp *aqb$l_acpqbl;
    UINT16 aqb$w_size;
    UINT8 aqb$b_type;
    UINT8 aqb$b_mntcnt;
    UINT32 aqb$l_acppid;
    struct _aqb *aqb$l_link;
    union
    {
        UINT8 aqb$b_status;
        struct
        {
            unsigned aqb$v_unique : 1;
            unsigned aqb$v_defclass : 1;
            unsigned aqb$v_defsys : 1;
            unsigned aqb$v_creating : 1;
            unsigned aqb$v_xqioproc : 1;
            unsigned aqb$v_fill_0_ : 3;
        };
    };
    UINT8 aqb$b_acptype;
    UINT8 aqb$b_class;
    INT8 aqbdef$$_fill_1;
    void *aqb$l_bufcache;
    UINT32 aqb$l_mount_count;
    struct _vcb *aqb$l_orphaned_vcb;
    void (*aqb$l_astadr)(void);
};

#endif

