#ifndef cxbdef_h
#define cxbdef_h

#define		CXB$M_RESP		0x1
#define		CXB$L_NI_ALTXMT		28
#define		CXB$C_AGENT_SCRATCH_LEN	52

#define		CXB$K_LENGTH		92
#define		CXB$C_LENGTH		92

#define		CXB$M_FLTR_MCA		0x1
#define		CXB$M_FLTR_CTL		0x2
#define		CXB$M_FLTR_SRC		0x4
#define		CXB$M_FLTR_STARTUP	0x1
#define		CXB$M_FLTR_INTXMIT	0x2
#define		CXB$T_R_DATA		128
#define		CXB$W_R_LEN_802		140
#define		CXB$T_R_USER_ETH	142
#define		CXB$W_R_SIZE		142
#define		CXB$X_R_CTL		144
#define		CXB$G_R_PID		145
#define		CXB$T_R_USER_802E	150
#define		CXB$C_DLL		52
#define		CXB$C_DLL_SCRATCH_LEN	52

#define		CXB$K_HEADER		144
#define		CXB$C_HEADER		144
#define		CXB$C_TRAILER		4
#define		CXB$K_OVERHEAD		148
#define		CXB$C_OVERHEAD		148

struct _cxb
{
    union
    {
        struct
        {
            struct _cxb *cxb$l_fl;
            struct _cxb *cxb$l_bl;
        };
        struct
        {
            void *cxb$ps_pktdata;
            void *cxb$ps_uva32;
        };
    };
    unsigned short int cxb$w_size;
    unsigned char cxb$b_type;
    union
    {
        unsigned char cxb$b_flag;
        struct
        {
            unsigned cxb$v_resp : 1;
            unsigned cxb$v_fill_16_ : 7;
        };
    };
    unsigned short int cxb$w_boff;
    unsigned short int cxb$w_bcnt;
    void *cxb$l_data_chain;
    unsigned long long cxb$q_station;
    unsigned short int cxb$w_ctl;
    unsigned char cxb$b_ctl_size;
    unsigned char cxb$b_dsap;
    unsigned long long cxb$q_reserved;
    union
    {
        char cxb$t_agent_scratch [52];
        union
        {
            unsigned char cxb$t_agent_decnet [52];
            struct
            {
                unsigned short int cxb$w_length;
                unsigned short int cxb$w_offset;
                unsigned char cxb$b_code;
                unsigned char cxb$b_sts;
                unsigned short int cxb$w_channel;
                void *cxb$l_link;
                struct _irp *cxb$l_irp;
                int (*cxb$l_end_action)();
                void *cxb$l_r_nsp_msg;
                unsigned int cxb$l_r_data_size;
                unsigned short int cxb$w_r_seg_num;
                short int cxb$w_filler_1;
                struct _irp *cxb$l_user_irp;
                struct _dcb *cxb$l_last_dcb;
                void *cxb$pq_uva64;
            };
        };
    };
    union
    {
        char cxb$t_dll [52];
        union
        {
            unsigned char cxb$t_dll_ni802 [52];
            struct
            {
                unsigned char cxb$b_ni_func;
                unsigned char cxb$b_r_flags;
                union
                {
                    unsigned short int cxb$w_ni_rid;
                    struct
                    {
                        unsigned char cxb$b_ni_slot;
                        unsigned char cxb$b_ni_ring;
                    };
                };
                struct _irp *cxb$l_t_irp;
                unsigned short int cxb$w_r_nchain;
                unsigned short int cxb$w_r_lenerr;
                struct _ucb *cxb$l_r_ucb;
                unsigned short int cxb$w_hdr_size;
                unsigned char cxb$b_r_fmt;
                union
                {
                    union
                    {
                        unsigned char cxb$b_r_filter;
                        struct
                        {
                            unsigned cxb$v_fltr_mca : 1;
                            unsigned cxb$v_fltr_ctl : 1;
                            unsigned cxb$v_fltr_src : 1;
                            unsigned cxb$v_fill_17_ : 5;
                        };
                    };
                    union
                    {
                        unsigned char cxb$b_t_filter;
                        struct
                        {
                            unsigned cxb$v_fltr_startup : 1;
                            unsigned cxb$v_fltr_intxmit : 1;
                            unsigned cxb$v_fill_18_ : 6;
                        };
                    };
                };
            };
        };
        union
        {
            unsigned char cxb$t_dll_ni802xmt [52];
            struct
            {
                char cxbdef$$_nixmt_fill [38];
                unsigned char cxb$t_t_data [14];
            };
        };
        union
        {
            unsigned char cxb$t_dll_ni802rcv [52];
            struct
            {
                char cxbdef$$_nircv_fill1 [36];
                unsigned short int cxb$g_r_dest [3];
                unsigned short int cxb$g_r_src [3];
                unsigned short int cxb$w_r_ptype;
                unsigned char cxb$b_r_dsap;
                unsigned char cxb$b_r_ssap;
            };
        };
    };
    int cxbdef$$_fill_1;
    char cxb$b_fill_19_ [4];
};

#define CXB$S_CXBDEF 152

#endif

