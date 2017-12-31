#ifndef nisca_h
#define nisca_h

#define NISCA$M_SRSNTDATWM    0x80
#define NISCA$M_SRLB          0x100
#define NISCA$M_RRSTSTRT      0x200
#define NISCA$M_SSTRT         0x400
#define NISCA$M_SRST          0x800
#define NISCA$M_RMDATREQ      0x1000
#define NISCA$M_SMDATREQ      0x2000
#define NISCA$M_RSNTMDAT      0x4000
#define NISCA$M_SSNTMDAT      0x8000
#define NISCA$M_SRID          0x10000
#define NISCA$M_SRDATREQ      0x20000 /* bit place, 6 bits */
#define NISCA$M_RSNTDAT       0x800000
#define NISCA$M_SSNTDAT       0x1000000
#define NISCA$M_SRMSG         0x2000000
#define NISCA$M_UNINIT_MAINT  0x4000000
#define NISCA$M_UNINIT        0x8000000
#define NISCA$M_DISAB_MAINT   0x10000000
#define NISCA$M_DISAB         0x20000000
#define NISCA$M_ENAB_MAINT    0x40000000
#define NISCA$M_ENAB          0x80000000

#define NISCA$C_HELLO                  0x00
#define NISCA$C_BYE                    0x01
#define NISCA$C_CCSTART                0x02
#define NISCA$C_VERF                   0x03
#define NISCA$C_VACK                   0x04
#define NISCA$C_DONE                   0x05
#define NISCA$C_SOLICIT_SRV            0x06
#define NISCA$C_RESERVED               0x07 /* ? */

#define NISCA$C_CC_LNG                 0x62
#define NISCA$C_CC_LNG_V11             0x2E
#define NISCA$C_CC_LNG_V12             0x3E
#define NISCA$C_CC_LNG_V13             0x62
#define NISCA$C_CMDQ_2                 0x01
#define NISCA$C_CMDQ_3                 0x00
#define NISCA$C_CMDQ_HI                0x02
#define NISCA$C_CMDQ_LO                0x03
#define NISCA$C_DELAY_ACK              0x01
#define NISCA$C_DELAY_SEQ              0x02
#define NISCA$C_DX_LNG_HDR             0x0E
#define NISCA$C_GROUP_BASE_HI          0x00000100
#define NISCA$C_GROUP_BASE_LO          0x010400AB /* ab:00:04:01 */
#define NISCA$C_HELLO_MIN_TIM          0x01
#define NISCA$C_HELLO_TIMER            0x03
#define NISCA$C_HS_TIMERS              0x01
#define NISCA$C_HS_TMO_1               0x05
#define NISCA$C_HS_TMO_MAX             0x05
#define NISCA$C_LSTN_TIMERS            0x01
#define NISCA$C_LSTN_TMO_1             0x08
#define NISCA$C_LSTN_TMO_MAX           0x08
#define NISCA$C_MAJOR                  0x01
#define NISCA$C_MAX_CACHE              0x1F
#define NISCA$C_MAX_REXMT              0x1E
#define NISCA$C_MINOR                  0x03
#define NISCA$C_NI_PROTOCOL            0x0760 /* really 0x6007, rfc 1700 */
#define NISCA$C_NODE_HW_VAX            0x00
#define NISCA$C_PIPE_QUOTA             0x08
#define NISCA$C_PPC_LNG_HDR            0x01
#define NISCA$C_SMSG_OVHD              0x18

#define NISCA$C_PPDMSG_MAX             0x16
#define NISCA$C_REQ_DAT0               0x08
#define NISCA$C_REQ_DAT1               0x09
#define NISCA$C_REQ_DAT2               0x0A
#define NISCA$C_REQ_DATM               0x07
#define NISCA$C_REQ_ID                 0x06
#define NISCA$C_RESET                  0x12
#define NISCA$C_RET_CNF                0x0F
#define NISCA$C_RET_CNFM               0x0E
#define NISCA$C_RET_CNFWM              0x16
#define NISCA$C_RET_DAT                0x0C
#define NISCA$C_RET_DATM               0x0B
#define NISCA$C_RET_DAT_LP             0x0D
#define NISCA$C_RET_ID                 0x10
#define NISCA$C_RET_LB                 0x11
#define NISCA$C_SNT_DAT                0x04
#define NISCA$C_SNT_DATM               0x03
#define NISCA$C_SNT_DATWM              0x14
#define NISCA$C_SNT_DATWM_LP           0x15
#define NISCA$C_SNT_DAT_LP             0x05
#define NISCA$C_SNT_DG                 0x00
#define NISCA$C_SNT_LB                 0x01
#define NISCA$C_SNT_SEQ                0x02
#define NISCA$C_START                  0x13

#define NISCA$C_TIMER                  0x00989680

#define NISCA$C_TR_LNG_HDR             0x09
#define NISCA$C_TR_LNG_HDR_V10         0x06
#define NISCA$C_TR_LNG_HDR_V13         0x09

#define NISCA$M_TR_DATA                0x01
#define NISCA$M_TR_SEQ                 0x02
#define NISCA$M_TR_NAK                 0x04
#define NISCA$M_TR_ACK                 0x08
#define NISCA$M_TR_RSVP                0x10
#define NISCA$M_TR_REXMT               0x20
#define NISCA$M_TR_CTL                 0x40
#define NISCA$M_TR_CCFLG               0x80

#define NISCA$M_AUTHORIZE              0x10
#define NISCA$M_MAINT                  0x80000008 /* really 08000080 */
#define NISCA$M_OK_XMT                 0x20
#define NISCA$M_PORT_FCN               0x83FF0180
#define NISCA$M_SRV_RCHK               0x02
#define NISCA$M_SRV_XCHK               0x01
#define NISCA$M_STA_INFO               0x0400
#define NISCA_M_SRV_NOSUP              0xFFFFFFFC
#define NISCA_M_SRV_SUP                0x00000003
#define NISCA$S_DEVICE_NAME            0x10
#define NISCA$S_LAN_HW_ADDR            0x06
#define NISCA$S_NODENAME               0x08
#define NISCA$S_SERVICE                0x10

#define NISCA$V_SRV_XCHK               0x00
#define NISCA$V_SRV_RCHK               0x01
#define NISCA$V_AUTHORIZE              0x04
#define NISCA$V_OK_XMT                 0x05
#define NISCA$V_NO_XMT                 0x06
#define NISCA$V_CCFLG                  0x07
#define NISCA$V_TR_ACK                 0x03
#define NISCA$V_TR_DATA                0x00
#define NISCA$V_TR_REXMT               0x05
#define NISCA$V_TR_RSVP                0x04
#define NISCA$V_TR_SEQ                 0x01

struct _nisca_intro
{
    // oops seems this is the dx
    unsigned short length;
    unsigned char dst[6];
    unsigned short group;
    unsigned char src[6];
};

struct _nisca
{
    /* and before is: ( I think )
       unsigned short length excluding this 16 b?
       unsigned char dst[6] ab000401+word
       unsigned word scssystemid? fffe?
       unsigned char src[6] i think
    */

    union
    {
        struct
        {
            unsigned long nisca$l_dx_dst_lo;
            unsigned short nisca$w_dx_dst_hi;
            unsigned short nisca$w_dx_group;
            unsigned long nisca$l_dx_src_lo;
            unsigned short nisca$w_dx_src_hi;
        };
        struct
        {
            union
            {
                unsigned char nisca$b_msg;
                unsigned char nisca$b_msg_flags;
            };
            unsigned char nisca$b_reason;
            unsigned long nisca$l_maint;
            unsigned char nisca$b_minor;
            unsigned char nisca$b_major;
            unsigned short nisca$w_eco;
            char nisca$t_nodename[8];
            union
            {
                unsigned long nisca$l_port_fcn;
                unsigned long nisca$l_fcn_bitmasks;
            };
            unsigned long nisca$l_sta_info;
            union
            {
                unsigned long nisca$l_sta_info1;
                struct
                {
                    char fill;
                    unsigned m : 1;
                    unsigned ps: 2;
                    unsigned sys_state:21;
                };
            };
            char nisca$t_rst_port[6];
            unsigned char nisca$b_reserved0;
            unsigned char nisca$b_pipe_quota;
            unsigned short nisca$w_p_srv;
            unsigned short nisca$w_r_srv;
            unsigned long long nisca$q_authorize;
            char nisca$t_service[16]; /* obsolete? */
            unsigned short nisca$w_lcl_chan_seq_num;
            unsigned short nisca$w_buf_buffer_size;
            unsigned long long nisca$q_transmit_time;
            unsigned char nisca$ab_device_name[16];
            unsigned char nisca$ab_lan_hw_addr[6];
            unsigned char nisca$b_device_type;
            unsigned char nisca$b_node_hw_type;
        };
        struct
        {
            unsigned srv_xchk:1;
            unsigned srv_rchk:1;
        };
        struct
        {
            unsigned char nisca$b_tr_flag;
            unsigned char nisca$b_tr_pad;
            unsigned short nisca$w_tr_ack;
            unsigned short nisca$w_tr_seq;
            unsigned short nisca$w_rmt_chan_seq_num;
            unsigned char nisca$b_tr_pad_data_len;
        };
        struct
        {
            unsigned char nisca$b_ppc_opc;
        };
    };
};

#endif
