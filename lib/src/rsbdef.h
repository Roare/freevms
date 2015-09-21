#ifndef RSBDEF_H
#define RSBDEF_H

#include <vms_types.h>

#define RSB$M_DIRENTRY 0x1
#define RSB$M_VALINVLD 0x2
#define RSB$M_DIR_RQD 0x4
#define RSB$M_RM_PEND 0x8
#define RSB$M_RM_IP 0x10
#define RSB$M_RM_ACCEPT 0x20
#define RSB$M_RM_RBLD 0x40
#define RSB$M_RM_WAIT 0x80
#define RSB$M_RM_DEFLECT 0x100
#define RSB$M_DIR_IP 0x200
#define RSB$M_RBLD_IP 0x400
#define RSB$M_RBLD_RQD 0x800
#define RSB$M_RBLD_ACT 0x1000
#define RSB$M_CHK_BTR 0x2000
#define RSB$M_WTFULRNG 0x4000
#define RSB$M_WTSUBRNG 0x8000
#define RSB$M_BRL 0x10000
#define RSB$M_2PC_IP 0x80000
#define RSB$M_CVTFULRNG 0x100000
#define RSB$M_CVTSUBRNG 0x200000
#define RSB$M_VALCUR 0x400000
#define RSB$M_INVPEND 0x800000
#define RSB$M_DPC 0x1000000
#define RSB$M_GGMODE_VALID 0x2000000
#define RSB$K_MAXLEN 31
#define RSB$K_LENGTH 256
#define RSB$C_LENGTH 256
#define RSB$S_RSBDEF 256

struct _rsb
{
    void *rsb$l_hshchn;
    void *rsb$l_hshchnbk;

    UINT16 rsb$w_size;
    UINT8 rsb$b_type;
    UINT8 rsb$b_depth;
    UINT8 rsb$b_fgmode;
    UINT8 rsb$b_ggmode;
    UINT8 rsb$b_cgmode;
    UINT32 rsb$l_status;
    UINT32 rsb$l_refcnt;
    void *rsb$l_grqfl;
    void *rsb$l_grqbl;
    void *rsb$l_cvtqfl;
    void *rsb$l_cvtqbl;
    void *rsb$l_wtqfl;
    void *rsb$l_wtqbl;
    UINT64 rsb$q_valblk;
    unsigned long rsb$l_csid;
    void *rsb$l_rrsfl;
    void *rsb$l_rrsbl;
    void *rsb$l_srsfl;
    void *rsb$l_srsbl;
    unsigned long rsb$l_rm_csid;
    struct _rsb *rsb$l_rtrsb;
    struct _clurcb *rsb$l_clurcb;
    UINT16 rsb$w_activity;
    UINT16 rsb$w_lckcnt;
    UINT32 rsb$l_valseqnum;
    UINT16 rsb$w_refcnt;
    UINT16 rsb$w_blkastcnt;
    UINT16 rsb$w_hashval;
    UINT16 rsb$w_rqseqnm;
    struct _rsb *rsb$l_parent;
    UINT16 rsb$w_group;
    UINT8 rsb$b_rmod;
    UINT8 rsb$b_rsnlen;
    char rsb$t_resnam [32];
};

#endif
