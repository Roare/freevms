// $Id$
// $Locker$

// Author. Roar Thron�s.

struct __xqp
{
    // unsigned long long xqp_queue;
    unsigned long xqp_head;
    unsigned long xqp_tail;
    unsigned long xqp_dispatcher;
    unsigned long code_size;
    unsigned long code_address;
    unsigned long data_size;
    unsigned long data_address;
    unsigned long prev_fp;
    unsigned long prev_stklim[2];
    unsigned long xqp_stklim[2];
    unsigned long xqp_savfp;
    unsigned long io_ccb;
    unsigned long io_channel;
    unsigned long block_lockid;
    unsigned long user_status[2];
    unsigned long io_status[2];
    unsigned long io_packet;
    unsigned long current_ucb;
    unsigned long current_vcb;
    unsigned long current_rvt;
    unsigned long current_rvn;
    unsigned short save_vc_flags;
    char stsflags;
    char block_check;
    unsigned long new_fid;
    unsigned long new_fid_rvn;
    unsigned long header_lbn;
    unsigned long bitmap_vbn;
    unsigned long bitmap_rvn;
    unsigned long bitmap_buffer;
    unsigned long save_status;
    unsigned long long privs_used;
    unsigned long acb_addr;
    unsigned long bfr_list[8];
    unsigned long bfr_credits[2];
    unsigned long bfrs_used[2];
    unsigned long cache_hdr;
    unsigned long context_start;
    unsigned long cleanup_flags;
    unsigned long file_header;
    unsigned long primary_fcb;
    unsigned long current_window;
    unsigned long current_fib;
    unsigned long curr_lckindx;
    unsigned long prim_lckindx;
    unsigned long loc_rvn;
    unsigned long loc_lbn;
    unsigned long unrec_lbn;
    unsigned long unrec_count;
    unsigned long unrec_rvn;
    char prev_link[6];
    char context_save[54];
    unsigned int lb_lockid_idx;
    unsigned long lb_lockid[5];
    unsigned long lb_basis[5];
    unsigned long lb_hdrseq[5];
    unsigned long lb_dataseq[5];
    unsigned long lb_oldhdrseg[5];
    unsigned long lb_olddataseg[5];
    unsigned long quota_dataseq;
    unsigned long quota_olddataseq;
    unsigned long dir_fcb;
    unsigned long dir_lckindx;
    unsigned long dir_record;
    char dir_context[112];
    char old_version_fid[6];
    unsigned long prev_version;
    char prev_name[81];
    char padding_0;
    char prev_iname[86];
    char super_fid[6];
    unsigned long local_fib[64];
    unsigned long second_fib[64];
    unsigned long local_arb[13];
    unsigned long l_data_end;
    unsigned long quota_record;
    unsigned long free_quota;
    unsigned long real_q_rec;
    unsigned long quota_index;
    unsigned long dummy_rec[8];
    unsigned long audit_count;
    unsigned long matching_ace[128];
    unsigned long notify_ast_addr;
    unsigned short notify_name_len;
    char notify_name_txt[89];
    unsigned long block_vcb;
    unsigned short file_spec_len;
    char full_file_spec[1022];
    unsigned long pms_tot_read;
    unsigned long pms_tot_write;
    unsigned long pms_tot_cache;
    unsigned long pms_fnc_read;
    unsigned long pms_fnc_write;
    unsigned long pms_fnc_cache;
    unsigned long pms_fnc_cpu;
    unsigned long pms_fnc_pfa;
    unsigned long pms_sub_nest;
    unsigned long pms_sub_func;
    unsigned long pms_sub_read;
    unsigned long pms_sub_write;
    unsigned long pms_sub_cache;
    unsigned long pms_sub_cpu;
    unsigned long pms_sub_pfa;
    unsigned long audit_arglist[16];
};

extern struct __xqp xqps[1]; // number of pids

extern struct __xqp * xqp;

