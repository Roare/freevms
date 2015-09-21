#ifndef KPBDEF_H
#define KPBDEF_H

#include <vms_types.h>

#define KPBDBG$K_PC_VEC_CNT 8

struct _kpbdbg
{
    UINT32 kpbdbg$is_start_time;
    UINT32 kpbdbg$is_start_count;
    UINT32 kpbdbg$is_restart_count;
    UINT32 kpbdbg$is_vec_index;
    int (*kpbdbg$is_pc_vec [8])(void);
};

#define KPBDBG$K_LENGTH 48
#define KPBDBG$S_KPBDBGDEF 48

struct _kpbspl
{
    void (*kpbspl$ps_spl_stall_rtn)(void);
    void (*kpbspl$ps_spl_restrt_rtn)(void);
};
#define KPBSPL$K_LENGTH 8
#define KPBSPL$S_KPBSPLDEF 8

struct _kpbsch
{
    void (*kpbsch$ps_sch_stall_rtn)(void);
    void (*kpbsch$ps_sch_restrt_rtn)(void);
    struct _fkb *kpbsch$ps_fkblk;
    char kpbsch$t_align_3 [4];
    union
    {
        struct
        {
            struct _fkb *kpbsch$ps_fqfl;
            struct _fkb *kpbsch$ps_fqbl;
            UINT16 kpbsch$iw_fkb_size;
            UINT8 kpbsch$ib_fkb_type;
            UINT8 kpbsch$ib_flck;
            void (*kpbsch$ps_fpc)(void);
            INT64 kpbsch$q_fr3;
            INT64 kpbsch$q_fr4;
        };
        struct
        {
            struct _tqe *kpbsch$ps_tqfl;
            struct _tqe *kpbsch$ps_tqbl;
            UINT16 kpbsch$iw_tqe_size;
            UINT8 kpbsch$ib_tqe_type;
            UINT8 kpbsch$ib_rqtype;
            void (*kpbsch$ps_tqe_fpc)(void);
            INT64 kpbsch$q_tqe_fr3;
            INT64 kpbsch$q_tqe_fr4;
            INT64 kpbsch$iq_time;
            INT64 kpbsch$q_fill_delta;
            INT32 kpbsch$l_fill_rmod;
            INT32 kpbsch$l_fill_efn;
            INT32 kpbsch$l_fill_rqpid;
            INT32 kpbsch$l_fill_cputim;
        };
    };
};

#define KPBSCH$K_LENGTH 80
#define KPBSCH$S_KPBSCHDEF 80
#define KP$M_VEST 0x4
#define KP$M_SPLOCK 0x20
#define KP$M_DEBUG 0x40
#define KP$M_DEALLOC_AT_END 0x100
#define KP$M_IO 292
#define KP$M_LKMGR 4

struct _kpflags
{
    union
    {
        UINT32 kp$is_flags;
        struct
        {
            unsigned kp$v_fill1 : 2;
            unsigned kp$v_vest : 1;
            unsigned kp$v_fill2 : 2;
            unsigned kp$v_splock : 1;
            unsigned kp$v_debug : 1;
            unsigned kp$v_fill3 : 1;
            unsigned kp$v_dealloc_at_end : 1;
            unsigned kp$v_fill4 : 3;
            unsigned kp$v_fill_2_ : 4;
        };
    };
};

#define KP$S_KPDEF 4
#define KPB$K_MIN_IO_STACK 8192
#define KPB$K_MAX_MPW_STACK 8192
#define KPB$M_VALID 0x1
#define KPB$M_ACTIVE 0x2
#define KPB$M_VEST 0x4
#define KPB$M_DELETING 0x8
#define KPB$M_SCHED 0x10
#define KPB$M_SPLOCK 0x20
#define KPB$M_DEBUG 0x40
#define KPB$M_PARAM 0x80
#define KPB$M_DEALLOC_AT_END 0x100
#define KPB$M_BYPASS_CACHE 0x200
#define KPB$M_HLL_MASK 0x400
#define KPB$M_RESERVED_CACHE 0x800
#define KPREG$K_MIN_REG_MASK 738258944
#define KPREG$K_MIN_IO_REG_MASK 738259004
#define KPREG$K_ERR_REG_MASK -738263037
#define KPREG$K_HLL_REG_MASK 738263036
#define KPB$K_LENGTH 56
#define KPB$K_TQE_LENGTH 64

#define KPB$K_NON_VEST_LENGTH 136

#define KPB$K_KEEP 1
#define KPB$K_RELEASE 2
#define KPB$K_LOW 3
#define KPB$K_HIGH 4
#define KPB$K_SCH_LENGTH 184

#define KPB$K_SPL_LENGTH 192
#define KPB$S_KPBDEF 192

struct _kpb
{
    struct _kpb *kpb$ps_flink;
    struct _kpb *kpb$ps_blink;
    UINT16 kpb$iw_size;
    UINT8 kpb$ib_type;
    UINT8 kpb$ib_subtype;
    UINT32 kpb$is_stack_size;
    union
    {
        union
        {
            UINT32 kpb$is_flags;
            struct
            {
                unsigned kpb$v_valid : 1;
                unsigned kpb$v_active : 1;
                unsigned kpb$v_vest : 1;
                unsigned kpb$v_deleting : 1;
                unsigned kpb$v_sched : 1;
                unsigned kpb$v_splock : 1;
                unsigned kpb$v_debug : 1;
                unsigned kpb$v_param : 1;
                unsigned kpb$v_dealloc_at_end : 1;
                unsigned kpb$v_bypass_cache : 1;
                unsigned kpb$v_hll_mask : 1;
                unsigned kpb$v_reserved_cache : 1;
                unsigned kpb$v_fill_5_ : 4;
            };
        };
    };
    void *kpb$ps_saved_sp;
    UINT32 kpb$is_reg_mask;
    void *kpb$ps_stack_base;
    void *kpb$ps_stack_sp;
    void *kpb$ps_sch_ptr;
    void *kpb$ps_spl_ptr;
    void *kpb$ps_dbg_ptr;
    void *kpb$ps_prm_ptr;
    UINT32 kpb$is_prm_length;

    void (*kpb$ps_sch_stall_rtn)(void);
    void (*kpb$ps_sch_restrt_rtn)(void);
    struct _fkb *kpb$ps_fkblk;
    void (*kpb$ps_sch_end_rtn)(void);

    union
    {
        struct
        {
            struct _fkb *kpb$ps_fqfl;
            struct _fkb *kpb$ps_fqbl;
            UINT16 kpb$iw_fkb_size;
            UINT8 kpb$ib_fkb_type;
            UINT8 kpb$ib_flck;
            void (*kpb$ps_fpc)(void);
            INT64 kpb$q_fr3;
            INT64 kpb$q_fr4;
        };
        struct
        {
            struct _tqe *kpb$ps_tqfl;
            struct _tqe *kpb$ps_tqbl;
            UINT16 kpb$iw_tqe_size;
            UINT8 kpb$ib_tqe_type;
            UINT8 kpb$ib_rqtype;
            void (*kpb$ps_tqe_fpc)(void);
            INT64 kpb$q_tqe_fr3;
            INT64 kpb$q_tqe_fr4;
            INT64 kpb$iq_time;
            INT64 kpb$q_fill_delta;
            INT32 kpb$l_fill_rmod;
            INT32 kpb$l_fill_efn;
            INT32 kpb$l_fill_rqpid;
            INT32 kpb$l_fill_cputim;
        };
    };

    union
    {
        struct
        {
            struct _ucb *kpb$ps_ucb;
            struct _irp *kpb$ps_irp;

            struct
            {
                UINT32 kpb$is_timeout_time;
                UINT32 kpb$is_restore_ipl;
                UINT32 kpb$is_channel_data;
            };
            struct
            {
                void *kpb$ps_scsi_ptr1;
                void *kpb$ps_scsi_ptr2;
                void *kpb$ps_scsi_scdrp;
            };
            struct
            {
                UINT32 kpb$is_timeout;
                UINT32 kpb$is_newipl;
                struct _spl *kpb$ps_dlck;
            };
        };

        struct
        {
            struct _lkb *kpb$ps_lkb;
        };
    };
    char kpb$t_align_7 [4];
    void (*kpb$ps_spl_stall_rtn)(void);
    void (*kpb$ps_spl_restrt_rtn)(void);
};

#endif

