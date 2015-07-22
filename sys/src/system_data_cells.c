// $Id$
// $Locker$

// Author. Roar Thron�s.
/**
 \file system_data_cells.c
 \brief most of our global variables
 \author Roar Thron�s
 */

#include <linux/vmalloc.h>
#ifdef __x86_64__
#include <linux/init.h>
#endif
#include <system_data_cells.h>
#include <dyndef.h>
#include <queue.h>
#include <phddef.h>
#include <rddef.h>
#include <cdtdef.h>
#include <syidef.h>
#include <exe_routines.h>
#include <sch_routines.h>
#include <misc_routines.h>
#include <linux/slab.h>
#include <spldef.h>
#include <cpbdef.h>

/* Author: Roar Thron�s */

/* based on grep and cut from AAA_SYSTEM_CELLS.H
 as a gross approx everything is unsigned long
 (most is that anyway */

/* some may not belong, but will be moved later */

unsigned long acme$gq_mutex;
unsigned long acme$gq_request_queue;
unsigned long acme$gq_server;
unsigned long acp$gb_baseprio;
unsigned long acp$gb_datachk;
unsigned long acp$gb_maxread;
unsigned long acp$gb_swapflgs;
unsigned long acp$gb_window;
unsigned long acp$gb_writback;
unsigned long acp$gw_dindxcache;
unsigned long acp$gw_dircache;
unsigned long acp$gw_extcache;
unsigned long acp$gw_extlimit;
unsigned long acp$gw_fidcache;
unsigned long acp$gw_hdrcache;
unsigned long acp$gw_mapcache;
unsigned long acp$gw_quocache;
unsigned long acp$gw_sysacc;
unsigned long acp$gw_workset;
unsigned long arch$gq_ptolemy_cell;
unsigned long atm$gq_complete_queue;
unsigned long atm$gq_request_queue;
unsigned long boo$c_sysparsz;
unsigned long boo$ga_hwrpb;
unsigned long boo$gl_nextpfn;
unsigned long bug$l_bugchk_flags;
unsigned long bug$l_fatal_spsav;
unsigned long cache$gl_blockcntmax;
unsigned long cache$gl_flags;
unsigned long cache$gl_protocol_ver;
unsigned long cache$gl_ptes;
unsigned long cache$gl_state;
unsigned long cfs$ga_dataptr;
unsigned long cia$gl_mutex;
unsigned long cia$gq_intruder;
unsigned long clu$ar_cluevt;
unsigned long clu$gb_cluver;
unsigned long clu$gb_niscs_comm;
unsigned long clu$gb_qdisk;
unsigned long clu$gb_quorum_lost;
unsigned long clu$gb_vaxcluster;
unsigned long clu$gl_allocls;
unsigned long clu$gl_club;
unsigned long clu$gl_cluster_credits;
unsigned long clu$gl_clusvec;
unsigned long clu$gl_csp_entries;
unsigned long clu$gl_csp_mbx;
unsigned long clu$gl_join_tblstart;
unsigned long clu$gl_mscp_buffer;
unsigned long clu$gl_mscp_cmd_tmo;
unsigned long clu$gl_mscp_credits;
unsigned long clu$gl_mscp_load;
unsigned long clu$gl_mscp_serve_all;
unsigned long clu$gl_niscs_auth;
unsigned long clu$gl_niscs_group;
unsigned long clu$gl_niscs_lan_ovrhd;
unsigned long clu$gl_niscs_max_pktsz;
unsigned long clu$gl_niscs_port_serv;
unsigned long clu$gl_remov_tblstart;
unsigned long clu$gl_sd_allocls;
unsigned long clu$gl_served_io;
unsigned long clu$gl_sgn_flags;
unsigned long clu$gl_smci_group;
unsigned long clu$gl_tape_allocls;
unsigned long clu$gl_tmscp_load;
unsigned long clu$gl_tmscp_serve_all;
unsigned long clu$gq_niscs_auth;
unsigned long clu$gq_smci_auth;
unsigned long clu$gs_niscs_comm;
unsigned long clu$gw_exp_votes;
unsigned long clu$gw_lckdirwt;
unsigned long clu$gw_maxindex;
unsigned long clu$gw_qdskinterval;
unsigned long clu$gw_qdskvotes;
unsigned long clu$gw_quorum;
unsigned long clu$gw_recnxint;
unsigned long clu$gw_votes;
unsigned long cnx$gq_debug;
unsigned long cpu$gb_arch_type;
unsigned long cpu$gt_arch_name;
#if 0
// moved to shell
unsigned long ctl$a_common;
unsigned long ctl$a_dispvec;
unsigned long ctl$a_evi_edata;
unsigned long ctl$a_evi_kdata;
unsigned long ctl$a_evi_sdata;
unsigned long ctl$a_evi_udata;
unsigned long ctl$a_imgreg;
unsigned long ctl$a_initial_pkta;
unsigned long ctl$a_krp;
unsigned long ctl$a_kspini;
unsigned long ctl$a_nsa_idt;
unsigned long ctl$a_prcprm_edata;
unsigned long ctl$a_prcprm_kdata;
unsigned long ctl$a_region_table;
unsigned long ctl$ag_clidata;
unsigned long ctl$ag_climage;
unsigned long ctl$ag_clitable;
unsigned long ctl$al_clicalbk;
unsigned long ctl$al_finalexc[4];
unsigned long ctl$aq_excvec[8];
unsigned long ctl$aq_stack;
unsigned long ctl$aq_stacklim;
unsigned long ctl$ar_dass;
unsigned long ctl$ar_dce;
unsigned long ctl$ar_files_64;
unsigned long ctl$ar_forge_word;
unsigned long ctl$ar_im_semaphore;
unsigned long ctl$ar_lfs;
unsigned long ctl$ar_persona_array;
unsigned long ctl$ar_persona_ringbuffer;
unsigned long ctl$ar_registry;
unsigned long ctl$ar_registry_extra;
unsigned long ctl$ar_xfs_client;
unsigned long ctl$ar_xfs_server;
unsigned long ctl$c_clidatasz;
unsigned long ctl$c_krp_count;
unsigned long ctl$c_krp_size;
unsigned long ctl$c_p1ptr_size;
#endif
struct _ccb ctl$ga_ccb_table[CHANNELCNT];
#if 0
//continues
struct _ccb * ctl$gl_ccbbase=ctl$ga_ccb_table;
unsigned long ctl$ga_les_process;
unsigned char ctl$gb_deflang;
unsigned char ctl$gb_msgmask;
unsigned char ctl$gb_pwrmode;
unsigned char ctl$gb_ssfilter;
unsigned long ctl$gl_arb_support;
unsigned long ctl$gl_bpd_ptr;
unsigned long ctl$gl_buffer_items;
unsigned long ctl$gl_catchall_extension;
unsigned long ctl$gl_catchall_extension_cnt;
unsigned long ctl$gl_cdsptr;
unsigned long ctl$gl_chindx=CHANNELCNT;
unsigned long ctl$gl_clintown;
unsigned long ctl$gl_cmsupr;
unsigned long ctl$gl_cmuser;
unsigned long ctl$gl_creprc_flags;
unsigned long ctl$gl_ctlbasva;
unsigned long ctl$gl_cur_xscb;
unsigned long ctl$gl_dclprsown;
unsigned long ctl$gl_def_arb_support;
unsigned long ctl$gl_dnsptr;
unsigned long ctl$gl_f11bxqp;
unsigned long ctl$gl_fcb_listhead;
unsigned long ctl$gl_finalsts;
unsigned long ctl$gl_fixuplnk;
unsigned long ctl$gl_fred;
unsigned long ctl$gl_free_index;
unsigned long ctl$gl_getmsg;
unsigned long ctl$gl_iafexe;
unsigned long ctl$gl_iaflast;
unsigned long ctl$gl_iaflink;
unsigned long ctl$gl_iaflnkptr;
unsigned long ctl$gl_iafperm;
unsigned long ctl$gl_ibiocnt;
unsigned long ctl$gl_icputim;
unsigned long ctl$gl_idiocnt;
unsigned long ctl$gl_ifaultio;
unsigned long ctl$gl_ifaults;
unsigned long ctl$gl_imghdrbf;
unsigned long ctl$gl_imglstptr;
unsigned long ctl$gl_imgreg_size;
unsigned long ctl$gl_initial_epid;
unsigned long ctl$gl_initial_ktb;
unsigned long ctl$gl_initial_pid;
unsigned long ctl$gl_ipagefl;
unsigned long ctl$gl_ivolumes;
unsigned long ctl$gl_iwspeak;
unsigned long ctl$gl_knownfil;
unsigned long ctl$gl_krpbl;
unsigned long ctl$gl_krpfl;
unsigned long ctl$gl_kt_area_pages;
unsigned long ctl$gl_kt_count;
unsigned long ctl$gl_kt_high;
unsigned long ctl$gl_license_cnt;
unsigned long ctl$gl_lnmdirect; /* really p1 shell */
unsigned long ctl$gl_lnmdirseq;
unsigned long ctl$gl_lnmhash; /* really p1 shell */
unsigned long ctl$gl_logfails;
unsigned long ctl$gl_login_flags;
unsigned long ctl$gl_multithread;
unsigned long ctl$gl_native_to_tie;
unsigned long ctl$gl_network_protocol;
unsigned long ctl$gl_nmioch;
unsigned long ctl$gl_nsab;
unsigned long ctl$gl_oss_flags;
unsigned long ctl$gl_p1merge;
unsigned long ctl$gl_pbiocnt;
struct _pcb * ctl$gl_pcb;
unsigned long ctl$gl_pcputim;
unsigned long ctl$gl_pdiocnt;
unsigned long ctl$gl_persona_freeindex;
unsigned long ctl$gl_persona_maxindex;
unsigned long ctl$gl_persona_usercount;
unsigned long ctl$gl_pfaultio;
unsigned long ctl$gl_pfaults;
unsigned long ctl$gl_phd;
unsigned long ctl$gl_pimgcnt;
unsigned long ctl$gl_posix_1;
unsigned long ctl$gl_posix_2;
unsigned long ctl$gl_powerast;
unsigned long ctl$gl_ppmsg[2];
unsigned long ctl$gl_prcallcnt;
unsigned long ctl$gl_prcprm_kdata2;
unsigned long ctl$gl_pstflags;
unsigned long ctl$gl_psx_argptr;
unsigned long ctl$gl_pvolumes;
unsigned long ctl$gl_pvpcputim;
unsigned long ctl$gl_rdiptr;
unsigned long ctl$gl_report_buffer;
unsigned long ctl$gl_report_method;
unsigned long ctl$gl_report_user_faults;
unsigned long ctl$gl_rms_ruj_count;
unsigned long ctl$gl_rms_rujlck_blink;
unsigned long ctl$gl_rms_rujlck_flink;
unsigned long ctl$gl_rmsbase;
unsigned long ctl$gl_ruf;
unsigned long ctl$gl_security_exec_1;
unsigned long ctl$gl_security_exec_2;
unsigned long ctl$gl_sitespec;
unsigned long ctl$gl_subsys_rights;
unsigned long ctl$gl_thcount[3];
unsigned long ctl$gl_thexec;
unsigned long ctl$gl_thsupr;
unsigned long ctl$gl_uaf_flags;
unsigned long ctl$gl_uaictx;
unsigned long ctl$gl_used_index;
unsigned long ctl$gl_usrundwn;
unsigned long ctl$gl_usrundwn_exec;
unsigned long ctl$gl_vectors;
unsigned long ctl$gl_virtpeak;
unsigned long ctl$gl_volumes;
unsigned long ctl$gl_wspeak;
unsigned long ctl$gl_xlated_img_cnt;
unsigned long ctl$gl_xpcb;
unsigned long ctl$gl_xsig;
unsigned long ctl$gpq_gsd_lock_pte;
unsigned long long ctl$gq_acmerb_list;
unsigned long ctl$gq_allocreg[2];
unsigned long long ctl$gq_clusyictx;
unsigned long long ctl$gq_common;
unsigned long long ctl$gq_dbgarea;
unsigned long long ctl$gq_emulate_count;
unsigned long long ctl$gq_emulate_data;
unsigned long long ctl$gq_emulate_flags;
unsigned long long ctl$gq_emulate_pc_ring;
unsigned long long ctl$gq_emulate_ring_index;
unsigned long long ctl$gq_emulate_signal_mask;
unsigned long long ctl$gq_emulate_signal_max;
unsigned long long ctl$gq_exception_flags;
unsigned long long ctl$gq_exh_mutex;
unsigned long long ctl$gq_exit_thread;
unsigned long long ctl$gq_glx_lock_reg_cnt;
unsigned long long ctl$gq_glx_lock_reg_id;
unsigned long long ctl$gq_glx_lock_regions;
unsigned long long ctl$gq_helpflags;
unsigned long long ctl$gq_ieee_fp_control;
unsigned long long ctl$gq_ims_counter;
unsigned long long ctl$gq_istart;
unsigned long long ctl$gq_kt_region_base;
unsigned long long ctl$gq_kt_region_id;
unsigned long long ctl$gq_lastlogin_i;
unsigned long long ctl$gq_lastlogin_n;
unsigned long long ctl$gq_lnmtblcache; /* really p1 shell */
unsigned long long ctl$gq_login;
unsigned long long ctl$gq_misc_p1_flags;
unsigned long long ctl$gq_mountlst;
unsigned long long ctl$gq_nt_events;
unsigned long long ctl$gq_object_context;
unsigned long long ctl$gq_orb_lockdb;
unsigned long long ctl$gq_p0alloc;
unsigned long long ctl$gq_persona_handle;
unsigned long long ctl$gq_pktavec;
unsigned long long ctl$gq_pool_mutex;
unsigned long long ctl$gq_prcterm_listhead;
unsigned long long ctl$gq_procpriv;
unsigned long long ctl$gq_pstart;
unsigned long long ctl$gq_remote_address;
unsigned long long ctl$gq_remote_fullname;
unsigned long long ctl$gq_remote_node;
unsigned long long ctl$gq_remote_uid;
unsigned long long ctl$gq_report_buffer;
unsigned long long ctl$gq_rmcb_que;
unsigned long long ctl$gq_rms_rurec_lksb;
unsigned long long ctl$gq_security_exec_1;
unsigned long long ctl$gq_ssi_interlock;
unsigned long long ctl$gq_sspi_root;
unsigned long long ctl$gq_sspi_synch;
unsigned long long ctl$gq_termchar;
unsigned long long ctl$gq_tm_callbacks;
unsigned long long ctl$gq_virtpeak;
unsigned long long ctl$gq_wsl;
unsigned long long ctl$gq_xcb_que;
unsigned long ctl$gt_cli_namestring;
unsigned long ctl$gt_cliname;
unsigned long ctl$gt_simple_tabname;
unsigned long ctl$gt_spawncli;
unsigned long ctl$gt_spawntable;
unsigned long ctl$gt_tablename;
unsigned short ctl$gw_ppmsgchn;
unsigned char ctl$t_account[8];
unsigned char ctl$t_nodeaddr[7];
unsigned char ctl$t_nodename[7];
unsigned char ctl$t_remoteid[17];
unsigned char ctl$t_username[12];
#endif
unsigned long dass$ar_control_block;
unsigned long ddtm$ar_performance_cells;
unsigned long decc$$clocale_s;
unsigned long decc$$ga___ctypet;
unsigned long decc$$gl___ctypea;
unsigned long decc$$gl___isclocale;
unsigned long decc$$gl___mb_cur_max;
unsigned long decc$ga___ctype;
unsigned long decc$ga_randx;
unsigned long decw$gl_vector;
unsigned char erl$gb_bufflag;
unsigned char erl$gb_buftim;
unsigned long erl$gl_erlpid;
unsigned long erl$gl_sequence;
unsigned long exe$a_id_upcase;
unsigned long exe$a_sysparam;
unsigned long exe$ab_hextab;
unsigned long exe$al_erlbufadr;
unsigned long exe$aq_erlmbx;
unsigned long exe$aq_files_64_control;
unsigned long exe$aq_lfs_control;
unsigned long exe$aq_xfs_client_control;
unsigned long exe$aq_xfs_server_control;
unsigned long exe$ar_arbc_hash_table;
unsigned long exe$ar_console_dispatch;
unsigned long exe$ar_crd_feet;
unsigned long exe$ar_decps_vectors;
unsigned long exe$ar_dumpdev_validation_data;
unsigned long exe$ar_ewdata;
unsigned long exe$ar_fastpath_block;
unsigned long exe$ar_file_ocb;
unsigned long exe$ar_files_64;
unsigned long exe$ar_forge_db;
unsigned long exe$ar_fork_wait_queue;
unsigned long exe$ar_ftss_vectors;
unsigned long exe$ar_glxgrp_global_ocb;
unsigned long exe$ar_glxsys_global_ocb;
unsigned long exe$ar_group_global_ocb;
unsigned long exe$ar_io_routines_data;
unsigned long exe$ar_ipc_data;
unsigned long exe$ar_lfs;
unsigned long exe$ar_mchk_blocks;
unsigned long exe$ar_mchk_errcnt;
unsigned long exe$ar_mvmslbas;
unsigned long exe$ar_mvwbqfl;
unsigned long exe$ar_npool_data;
unsigned long exe$ar_npool_data_mon;
unsigned long exe$ar_ntbs;
unsigned long exe$ar_object_server_ucb;
unsigned long exe$ar_perm_db_end;
unsigned long exe$ar_perm_db_start;
unsigned long exe$ar_powerdwn_done;
unsigned long exe$ar_pxrb_array;
unsigned long exe$ar_qman_ocb;
unsigned long exe$ar_registry;
unsigned long exe$ar_registry_extra;
unsigned long exe$ar_sd_anchor;
unsigned long exe$ar_snap_vector;
unsigned long exe$ar_system_event;
unsigned long exe$ar_system_global_ocb;
unsigned long exe$ar_system_rights;
unsigned long exe$ar_tqenorept;
unsigned long exe$ar_uafc_hash_table;
unsigned long exe$ar_unicode_upcase_dat;
unsigned long exe$ar_upcase_dat;
unsigned long exe$ar_volume_ocb;
unsigned long exe$ar_xfc_anchor;
unsigned long exe$ar_xfc_data;
unsigned long exe$ar_xfs_client;
unsigned long exe$ar_xfs_server;
unsigned long exe$c_sysparsz;
unsigned long exe$exceptable;
unsigned long exe$ga_hwname;
unsigned long exe$ga_hwtype;
unsigned long exe$ga_les_table;
unsigned long exe$ga_les_vector;
unsigned long exe$ga_rad_npp_lsthds;
unsigned long exe$ga_wp_cre;
unsigned long exe$ga_wp_del;
unsigned long exe$ga_wp_wpre;
unsigned long exe$gb_erlbufpagelets;
unsigned long exe$gb_vp_load;
unsigned long exe$gl_abstim = 0;
unsigned long exe$gl_abstim_tics = 0;
unsigned long exe$gl_abstim_utics;
unsigned long exe$gl_aclmtx;
unsigned long exe$gl_acmflags;
unsigned long exe$gl_affinity;
unsigned long exe$gl_archflag;
unsigned long exe$gl_badacv_c;
unsigned long exe$gl_badacv_t;
unsigned long exe$gl_blakhole;
unsigned long exe$gl_boot_style;
unsigned long exe$gl_bootcb;
unsigned long exe$gl_brkmsk;
unsigned long exe$gl_buffer_items;
unsigned long exe$gl_cdsptr;
unsigned long exe$gl_cebmtx;
unsigned long exe$gl_clitabl;
unsigned long exe$gl_clock_drift;
unsigned long exe$gl_confreg;
unsigned long exe$gl_confregl;
unsigned long exe$gl_cpunodsp;
unsigned long exe$gl_crd_control;
unsigned long exe$gl_decnet_version;
unsigned long exe$gl_defflags;
unsigned long exe$gl_dnsptr;
unsigned long exe$gl_dtssflags;
unsigned long exe$gl_dtssptr;
unsigned long exe$gl_dumpmask;
unsigned long exe$gl_dynamic_flags;
unsigned long exe$gl_enqmtx;
unsigned long exe$gl_erasepb;
unsigned long exe$gl_eraseppt;
unsigned long exe$gl_flags;
unsigned long exe$gl_fpexcvec;
unsigned long exe$gl_free_index;
unsigned long exe$gl_ft_flags;
unsigned long exe$gl_gh_exec_code;
unsigned long exe$gl_gh_exec_data;
unsigned long exe$gl_gh_res_code;
unsigned long exe$gl_gh_res_data;
unsigned long exe$gl_gh_rsrvpgcnt;
unsigned long exe$gl_glxgrpbl;
unsigned long exe$gl_glxgrpfl;
unsigned long exe$gl_glxsysbl;
unsigned long exe$gl_glxsysfl;
unsigned long exe$gl_gsddelfl;
unsigned long exe$gl_gsddelbl;
unsigned long exe$gl_gsdgrpfl;
unsigned long exe$gl_gsdgrpbl;
unsigned long exe$gl_gsdmtx;
unsigned long exe$gl_gsdsysfl;
unsigned long exe$gl_gsdsysbl;
unsigned long exe$gl_hbs_cip;
unsigned long exe$gl_hbs_ptr;
unsigned long exe$gl_hwname_length;
unsigned long exe$gl_hwtype_length;
unsigned long exe$gl_imgreg_pages;
unsigned long exe$gl_itb_entries;
unsigned long exe$gl_known_files = 0;
unsigned long exe$gl_ldr_cnt;
unsigned long exe$gl_ldr_seq;
unsigned long exe$gl_limbolen;
unsigned long exe$gl_limbomax;
unsigned long exe$gl_limbothr;
unsigned long exe$gl_lnm_ocb;
unsigned long exe$gl_lntorb_lock;
unsigned long exe$gl_lockrtry;
unsigned long exe$gl_max_psxrt_prio;
unsigned long exe$gl_max_vms_prio;
unsigned long exe$gl_mchkerrs;
unsigned long exe$gl_memerrs;
unsigned long exe$gl_min_psxrt_prio;
unsigned long exe$gl_min_vms_prio;
unsigned long exe$gl_mme_flags;
unsigned long exe$gl_mmecb;
unsigned long exe$gl_mmg_flags;
unsigned long exe$gl_mp;
unsigned long exe$gl_msgflags;
unsigned long exe$gl_msgvec;
unsigned long exe$gl_mvmslbas;
unsigned long exe$gl_nonpaged;
unsigned long exe$gl_npag_bap_max;
unsigned long exe$gl_npag_bap_max_pa;
unsigned long exe$gl_npag_bap_min;
unsigned long exe$gl_npag_bap_min_pa;
unsigned long exe$gl_npagedyn;
unsigned long exe$gl_npagevir;
unsigned long exe$gl_ns_flags;
unsigned long exe$gl_num_xmi_nexus;
unsigned long exe$gl_numnexus;
unsigned long exe$gl_objcls_mutex;
unsigned long exe$gl_old_sid;
unsigned long exe$gl_paged;
unsigned long exe$gl_pcb_cache;
unsigned long exe$gl_pfatim;
unsigned long exe$gl_pgdynmtx;
unsigned long exe$gl_poolcheck;
unsigned long exe$gl_psx_ctx_buffer_length;
unsigned long exe$gl_public_vector_symvec;
unsigned long exe$gl_public_vector_symvec_end;
unsigned long exe$gl_pxrb_count;
unsigned long exe$gl_report_buffer;
unsigned long exe$gl_report_match_table;
unsigned long exe$gl_report_sys_faults;
unsigned long exe$gl_rsdm_mutex;
unsigned long exe$gl_rtbitmap;
unsigned long exe$gl_s0_paging;
unsigned long exe$gl_saved_embs;
unsigned long exe$gl_savedump;
unsigned long exe$gl_scb;
unsigned long exe$gl_scb_reservation;
unsigned long exe$gl_scbpfn;
unsigned long exe$gl_sched_flags;
unsigned long exe$gl_secptr;
unsigned long exe$gl_security_policy;
unsigned long exe$gl_shadow_max_copy;
unsigned long exe$gl_shadow_mbr_timeout;
unsigned long exe$gl_shadow_remove_1;
unsigned long exe$gl_shadow_remove_2;
unsigned long exe$gl_shadow_sys_disk;
unsigned long exe$gl_shadow_sys_tmo;
unsigned long exe$gl_shadow_sys_unit;
unsigned long exe$gl_shadow_sys_wait;
unsigned long exe$gl_shadowing;
unsigned long exe$gl_shblist;
unsigned long exe$gl_sitespec;
unsigned long exe$gl_snap_fork_vector;
unsigned long exe$gl_snap_pid;
unsigned long exe$gl_snap_sequence;
unsigned long exe$gl_snap_state;
unsigned long exe$gl_soft_tick;
unsigned long exe$gl_state;
unsigned long exe$gl_state_flags;
unsigned long exe$gl_static_flags;
unsigned long exe$gl_svapte;
unsigned long exe$gl_sys_code_base;
unsigned long exe$gl_sys_code_end;
unsigned long exe$gl_sys_data_base;
unsigned long exe$gl_sys_data_end;
unsigned long exe$gl_sys_specific;
unsigned long exe$gl_sys_symvec;
unsigned long exe$gl_sys_symvec_end;
unsigned long exe$gl_sysid_lock;
unsigned long exe$gl_sysmsg;
unsigned long exe$gl_systick;
unsigned long exe$gl_sysucb;
unsigned long exe$gl_sysuic;
unsigned long exe$gl_tenusec;
unsigned long exe$gl_tickadjust;
#ifdef __i386
unsigned long exe$gl_ticklength=100000;
#endif
#ifdef __x86_64__
unsigned long exe$gl_ticklength = 100000;
#endif
unsigned long exe$gl_tickwidth;
unsigned long exe$gl_time_control;
unsigned long exe$gl_time_deviation;
unsigned long exe$gl_timeadjust;
unsigned long exe$gl_tmv_svabuf;
unsigned long exe$gl_tmv_svapte;
struct _tqe * exe$gl_tqfl;
#ifdef __i386__
unsigned long long tqehead;
#else
unsigned long long tqehead[2];
#endif
unsigned long exe$gl_transition_year;
unsigned long exe$gl_ubdelay;
unsigned long exe$gl_unicode_upcase_version;
unsigned long exe$gl_used_index;
unsigned long exe$gl_usrchme;
unsigned long exe$gl_usrchmk;
unsigned long exe$gl_usrundwn;
unsigned long exe$gl_usrundwn_exec;
unsigned long exe$gl_vaxexcvec;
unsigned long exe$gl_volshad_perdev_licelkid;
unsigned long exe$gl_volume_mutex;
unsigned long exe$gl_vp_margin;
unsigned long exe$gl_window_system;
unsigned long exe$gl_xmi_csr_array;
unsigned long exe$gl_xmi_nexus_array;
unsigned long exe$gl_xmi_structure_array;
unsigned long exe$gl_xpca;
unsigned long exe$gpl_hwrpb_l;
unsigned long exe$gpq_fru_table;
unsigned long exe$gpq_hwrpb;
unsigned long exe$gpq_rad_info_header;
unsigned long exe$gpq_swrpb;
unsigned long long exe$gq_1st_time;
unsigned long exe$gq_bap_max_pa_registered;
unsigned long exe$gq_bap_max_request_size;
unsigned long exe$gq_bap_min_pa_registered;
unsigned long exe$gq_bap_min_request_size;
unsigned long exe$gq_bap_num_registrations;
unsigned long exe$gq_bap_variable;
unsigned long exe$gq_basimgmtx;
unsigned long exe$gq_bootcb_d;
unsigned long long exe$gq_boottime;
unsigned long exe$gq_bugcheck_exclude_pfn;
unsigned long exe$gq_bugchk_dump_remove_list;
unsigned long exe$gq_bugchk_process_list;
unsigned long exe$gq_cputype;
unsigned long exe$gq_debug_class_list;
unsigned long exe$gq_debug_psb_list;
unsigned long exe$gq_debug_rights_list;
unsigned long exe$gq_deleted_kfe;
unsigned long exe$gq_kfe_lcknam;
unsigned long exe$gq_limboq;
unsigned long exe$gq_mbz;
unsigned long exe$gq_object_class_list;
unsigned long exe$gq_pfailtime;
unsigned long exe$gq_pqbiq;
unsigned long exe$gq_pwrdone;
unsigned long exe$gq_pwrintvl;
unsigned long exe$gq_pxrb_mutex;
unsigned long exe$gq_real_cputype;
unsigned long exe$gq_rightslist;
unsigned long exe$gq_rsdm_list;
unsigned long exe$gq_saved_hwclock;
unsigned long exe$gq_security_domain;
unsigned long exe$gq_snap_bitmap;
unsigned long exe$gq_snap_fork_list;
unsigned long exe$gq_sysdisk;
unsigned long long exe$gq_systime;
unsigned long exe$gq_systype;
unsigned long exe$gq_syswcbiq;
unsigned long exe$gq_tdf;
unsigned long exe$gq_todcbase;
unsigned long exe$gq_wcbdeliq;
unsigned long exe$gr_system_data_cells;
unsigned long exe$gs_bap_base_lsthds;
unsigned long exe$gs_bap_npool;
unsigned long exe$gs_npp_base_lsthds;
unsigned long exe$gs_npp_npool;
unsigned long exe$gt_hwname;
unsigned long exe$gt_startup;
unsigned long exe$gw_clkint;
unsigned long exe$gw_clkuticr;
unsigned long exe$gw_clkutics;
unsigned long exe$gw_cpumodel;
unsigned long exe$gw_erlbufhead;
unsigned long exe$gw_erlbuftail;
unsigned long exe$gw_pgfl_fid;
unsigned long exe$gw_saved_embs_count;
unsigned long fil$gq_cache;
unsigned long fil$gt_dddev;
unsigned long fil$gt_ddstring;
unsigned long fil$gt_topsys;
unsigned long flt$gq_debug;
unsigned long glx$gl_communityid_lock;
unsigned long glx$gl_cpucom_fkb_flags;
unsigned long glx$gl_ev_lp_count;
unsigned long glx$gl_gnode;
unsigned long glx$gl_is_member;
unsigned long glx$gl_max_instances;
unsigned long glx$gl_max_shm_cpp_count;
unsigned long glx$gl_max_shm_reg_count;
unsigned long glx$gl_shm_cpp_max_frag;
unsigned long glx$gl_shm_reg_attach_fkb;
unsigned long glx$gl_shm_reg_attached_count;
unsigned long glx$gl_shm_reg_detach_fkb;
unsigned long glx$gl_shmem_flags;
unsigned long glx$gl_shmem_pmap_count;
unsigned long glx$gl_spi_available;
unsigned long glx$gl_system_features_h;
unsigned long glx$gl_system_features_l;
unsigned long glx$go_galaxy_id;
unsigned long glx$gpq_active_set;
unsigned long glx$gpq_config_header;
unsigned long glx$gpq_config_root; /* error? glx$gpq_config_; */
unsigned long glx$gpq_config_tree;
unsigned long glx$gpq_cpucom_ktb;
unsigned long glx$gpq_gmdb_base;
unsigned long glx$gpq_gmdb_base_copy;
unsigned long glx$gpq_gmdb_node_block;
unsigned long glx$gpq_gmdb_node_block_copy;
unsigned long glx$gpq_packet_dispatch;
unsigned long glx$gpq_part_node_flags;
unsigned long glx$gpq_primary_cpu_array;
unsigned long glx$gpq_shm_cpp_array;
unsigned long glx$gpq_shm_desc_array;
unsigned long glx$gpq_shm_reg_array;
unsigned long glx$gpq_shm_tag_array;
unsigned long glx$gpq_shmem_gmdb;
unsigned long glx$gpq_shmem_pmap_array;
unsigned long glx$gq_community_handle;
unsigned long glx$gq_community_id;
unsigned long glx$gq_glcktbl_fkbq;
unsigned long glx$gq_glock_fkbq;
unsigned long glx$gq_gmdb_locking;
unsigned long glx$gq_hw_root_handle;
unsigned long glx$gq_init_inprog;
unsigned long glx$gq_lock_reg_cnt;
unsigned long glx$gq_lock_regions;
unsigned long glx$gq_lock_tables;
unsigned long glx$gq_partition_handle;
unsigned long glx$gq_partition_id;
unsigned long glx$gq_shm_cpp_size;
unsigned long glx$gq_shm_reg_size;
unsigned long glx$gq_smci_ipint_queue;
unsigned long glx$gq_smlan_ipint_queue;
unsigned long glx$gq_system_features;
unsigned long glx$gt_community_id_name;
unsigned long iac$al_imgactbuf;
unsigned long iac$al_vecaddr;
unsigned long iac$al_vecopcod;
unsigned long iac$gl_first_icb;
unsigned long iac$gl_icbfl;
unsigned long iac$gl_imagctx;
unsigned long iac$gl_image_list;
unsigned long iac$gl_main_icb;
unsigned long iac$gl_procctx;
unsigned long iac$gl_stack_size;
unsigned long iac$gl_work_list;
unsigned long iac$gq_image_ids;
unsigned long icc$gl_icc_pdb_vector;
unsigned long icc$gl_icc_rand;
unsigned long icc$gl_icc_trace_area;
unsigned long img$gl_image_region;
unsigned long img$gl_imcb_list = 0;
unsigned long io$gl_scb_int0;
unsigned long io$gl_uba_int0;
unsigned long io$gl_unexp_fbus_intr;
unsigned long io$gl_unexp_xmi_intr;
unsigned long ioc$gl_adplist;
unsigned long ioc$gl_aqblist;
unsigned long ioc$gl_cram_avail;
unsigned long ioc$gl_cram_in_use;
unsigned long ioc$gl_cram_total;
unsigned long ioc$gl_cramh_blink;
unsigned long ioc$gl_cramh_flink;
unsigned long ioc$gl_cramh_pages;
unsigned long ioc$gl_crbtmout;
struct _ddb * ioc$gl_devlist = 0; ///< ddb list
unsigned long ioc$gl_dptlist;
unsigned long ioc$gl_dtn_list;
unsigned long ioc$gl_du_cddb;
unsigned long ioc$gl_ens_base;
unsigned long ioc$gl_event_database;
unsigned long ioc$gl_hirt;
unsigned long ioc$gl_interrupts;
unsigned long ioc$gl_mpdev_d1;
unsigned long ioc$gl_mpdev_flags;
unsigned long ioc$gl_mpdev_lcretries;
unsigned long ioc$gl_naming;
unsigned long ioc$gl_naming_p;
unsigned long ioc$gl_pac_id_list;
unsigned long ioc$gl_paclist_b;
unsigned long ioc$gl_paclist_f;
unsigned long ioc$gl_shdw_wrk;
unsigned long ioc$gl_spdt_list;
unsigned long ioc$gl_spi_connect;
unsigned long ioc$gl_tr_number;
unsigned long ioc$gl_tu_cddb;
unsigned long ioc$gl_wwidtbl;
unsigned long ioc$gq_brdcst;
unsigned long ioc$gq_ddma_base_pa;
unsigned long ioc$gq_dev_wwid_list;
unsigned long ioc$gq_min_bap_window;
unsigned long ioc$gq_mountlst;
unsigned long ioc$gq_mutex = 0;
unsigned long long ioc$gq_postiq = 0;
unsigned long ioc$gw_lamapreg;
unsigned long ioc$gw_maxbuf = MAXBUF; // check
unsigned long ioc$gw_mbxbfquo;
unsigned long ioc$gw_mbxmxmsg;
unsigned long ioc$gw_mvtimeout;
unsigned long ioc$gw_tape_mvtimeout;
unsigned long ioc$gw_xfmxrate;
unsigned long ioc$intdisp;
//unsigned long lck$al_dwncvt_tbl;
//unsigned long lck$al_ggmax_tbl;
//unsigned long lck$al_ggred_tbl;
//unsigned long lck$al_valblk_tbl;
//unsigned long lck$ar_compat_tbl;
unsigned long lck$ar_poolzone_region;
unsigned long lck$gl_act_thrsh;
unsigned long lck$gl_activity;
unsigned long lck$gl_cache;
unsigned long lck$gl_cspin;
unsigned long lck$gl_dirvec;
unsigned long lck$gl_dlck_incmplt;
unsigned long lck$gl_extrastk;
unsigned long lck$gl_flags;
unsigned long lck$gl_htblcnt;
unsigned long lck$gl_htblsiz;
unsigned long lck$gl_idtblmax;
unsigned long lck$gl_idtblsiz;
unsigned long lck$gl_lckcnt;
unsigned long lck$gl_lckmgr_flags;
unsigned long lck$gl_lkb_miss;
unsigned long lck$gl_lkbcache;
unsigned long lck$gl_lkidcnt;
unsigned long lck$gl_lkidfree;
unsigned long lck$gl_maxdepth;
unsigned long lck$gl_maxid;
unsigned long lck$gl_nxtid;
unsigned long lck$gl_prcmap;
unsigned long lck$gl_rebld_state;
unsigned long lck$gl_rsb_miss;
unsigned long lck$gl_rsbcnt;
unsigned long lck$gl_stallfl;
unsigned long lck$gl_stallreqs;
unsigned long lck$gl_sys_thrsh;
unsigned long lck$gl_waittime;
unsigned long lck$gq_bitmap_exp;
unsigned long lck$gq_bitmap_explcl;
unsigned long lck$gq_debug;
void * lck$gl_hashtbl;
void * lck$gl_idtbl;
unsigned long lck$gq_lckcpu_list;
unsigned long lck$gq_lkb_head;
unsigned long lck$gq_lkb_tail;
unsigned long lck$gq_local_anchor;
unsigned long lck$gq_lstfree;
unsigned long lck$gq_reserved1;
unsigned long lck$gq_reserved2;
unsigned long lck$gq_reserved3;
unsigned long lck$gq_reserved4;
unsigned long lck$gq_reserved5;
unsigned long lck$gq_reserved6;
unsigned long lck$gl_rrsfl;
unsigned long lck$gl_rrsbl; /* unofficial */
unsigned long lck$gq_rsb_head;
unsigned long lck$gq_rsb_tail;
unsigned long lck$gq_shared_anchor;
unsigned long long lck$gq_timoutq;
unsigned long ldr$gq_free_s0s1_pt;
unsigned long ldr$gq_free_s2_pt;
unsigned long ldr$gq_hpdesc;
unsigned long ldr$gq_image_list;
unsigned long lmf$ar_grouptbl;
unsigned long lmf$gl_reserved;
long * lnm$al_dirtbl[2];
unsigned long lnm$al_hashtbl;
unsigned long lnm$aq_mutex = 0;
unsigned long lnm$ar_cluster_lnmb;
unsigned long lnm$ar_syscluster_lnmb;
unsigned long lnm$ar_system_directory;
unsigned long lnm$gl_cw_buffer;
unsigned long lnm$gl_cw_last_msg;
unsigned long lnm$gl_cw_server;
unsigned long lnm$gl_cw_workq_count;
unsigned long lnm$gl_htblsizp;
unsigned long lnm$gl_htblsizs;
unsigned long lnm$gl_sysdirseq;
unsigned long lnm$gq_cw_names;
unsigned long lnm$gq_cw_workq;
unsigned long lnm$gq_debug;
unsigned long lnm_ar_system_dir_lnmth;
unsigned long mb$ar_ddb;
unsigned long mb$ar_ddt;
unsigned long mb$ar_dpt;
unsigned long mb$ar_orb0;
unsigned long mb$ar_orb1;
unsigned long mb$ar_orb2;
unsigned long mb$ar_orb3;
unsigned long mb$ar_orb4;
unsigned long mb$ar_ucb0;
unsigned long mb$ar_ucb1;
unsigned long mb$ar_ucb2;
unsigned long mb$ar_ucb3;
unsigned long mb$ar_ucb4;
unsigned long mmg$a_sysparam;
unsigned long mmg$ar_cvtprt_atov;
unsigned long mmg$ar_cvtprt_vtoa;
unsigned long mmg$ar_nullpfl;
unsigned long mmg$ar_syspcb;
unsigned long mmg$ga_page_files;
unsigned long mmg$gb_ctlflags;
unsigned long mmg$gl_bwp_mask;
unsigned long mmg$gl_bwp_width;
unsigned long mmg$gl_clone_clump;
unsigned long mmg$gl_color_count;
unsigned long mmg$gl_color_mask;
unsigned long mmg$gl_ctlbasva;
unsigned long mmg$gl_dzro_pte;
unsigned long mmg$gl_dzro_va;
unsigned long mmg$gl_fpemul_base;
unsigned long mmg$gl_frewflgs;
unsigned long mmg$gl_gblpagfil;
unsigned long mmg$gl_gblsecfnd;
unsigned long mmg$gl_initial_bad_pages;
unsigned long mmg$gl_io_memory_map;
unsigned long mmg$gl_l1_index;
unsigned long mmg$gl_level_width;
unsigned long mmg$gl_max_mem_fragments;
unsigned long mmg$gl_max_node_pfn;
unsigned long mmg$gl_max_shared_pfn;
unsigned long mmg$gl_maxbobmem_pagelets;
unsigned long mmg$gl_maxbobmem_pages;
unsigned long mmg$gl_maxbobs0s1_pagelets;
unsigned long mmg$gl_maxbobs0s1_pages;
unsigned long mmg$gl_maxbobs2_pagelets;
unsigned long mmg$gl_maxbobs2_pages;
unsigned long mmg$gl_maxmem;
unsigned long mmg$gl_maxpfidx = 0;
unsigned long mmg$gl_maxpfn;
unsigned long mmg$gl_memsize;
unsigned long mmg$gl_min_node_pfn;
unsigned long mmg$gl_min_shared_pfn;
unsigned long mmg$gl_minpfn;
unsigned long mmg$gl_next_color_index;
unsigned long mmg$gl_next_free_s0s1_va;
unsigned long mmg$gl_npagedyn;
unsigned long mmg$gl_npagnext;
unsigned long mmg$gl_page_size;
unsigned long mmg$gl_page_to_vbn;
unsigned long mmg$gl_pagedyn;
unsigned long mmg$gl_pagswpvc;
unsigned long mmg$gl_pfn_memory_map;
unsigned long mmg$gl_pfnlock_pages;
unsigned long mmg$gl_phymem;
unsigned long mmg$gl_phypgcnt;
unsigned long mmg$gl_present_color_mask;
unsigned long mmg$gl_prvpfn_blink;
unsigned long mmg$gl_prvpfn_flink;
unsigned long mmg$gl_prvpfn_reclaimable;
unsigned long mmg$gl_prvpfn_reclaimed;
unsigned long mmg$gl_pte_offset_to_va;
unsigned long mmg$gl_ptes_per_page;
unsigned long mmg$gl_reclaim_flags;
unsigned long mmg$gl_res_mem_blink;
unsigned long mmg$gl_res_mem_flink;
unsigned long mmg$gl_rmsbase;
unsigned long mmg$gl_rsrvpagcnt;
unsigned long mmg$gl_sbiconf;
unsigned long mmg$gl_sbr;
//extern pgd_t swapper_pg_dir[1024];
unsigned long mmg$gl_sptbase; //=swapper_pg_dir;
unsigned long mmg$gl_syi_pfn_memory_map;
unsigned long mmg$gl_sysloa_base;
unsigned long mmg$gl_sysphd;
unsigned long mmg$gl_system_l1pte_index;
unsigned long mmg$gl_va_bits;
unsigned long mmg$gl_va_to_pte_offset;
unsigned long mmg$gl_va_to_vpn;
unsigned long mmg$gl_vaxemul_base;
unsigned long mmg$gl_vpn_to_va;
unsigned long mmg$gl_zero_list_hi_lim;
unsigned long mmg$gpq_page_swap_vector;
unsigned long mmg$gq_64p1_shift;
unsigned long mmg$gq_64sys_shift;
unsigned long mmg$gq_bap;
unsigned long mmg$gq_bwp_mask;
unsigned long mmg$gq_bwp_width;
unsigned long mmg$gq_free_gpt = 0;
unsigned long mmg$gq_gap_hi_va;
unsigned long mmg$gq_gap_lo_va;
unsigned long mmg$gq_gpt_base = 0;
unsigned long mmg$gq_l1_base;
unsigned long mmg$gq_l2_base;
unsigned long mmg$gq_level_width;
unsigned long mmg$gq_max_gpte;
unsigned long mmg$gq_next_free_s0s1_va;
unsigned long mmg$gq_next_free_s2_va;
unsigned long mmg$gq_non_pt_mask;
unsigned long mmg$gq_non_va_mask;
unsigned long mmg$gq_p2_base_va;
unsigned long mmg$gq_page_size;
unsigned long mmg$gq_pagefile_allocs;
unsigned long mmg$gq_pagefile_pages;
unsigned long mmg$gq_pagefile_refs;
unsigned long mmg$gq_pooltrace;
unsigned long mmg$gq_process_space_limit;
unsigned long mmg$gq_process_va_pages;
unsigned long mmg$gq_pt_base;
unsigned long mmg$gq_pte_offset_to_va;
unsigned long mmg$gq_ptes_per_page;
unsigned long mmg$gq_s0s1base_pte_address;
unsigned long mmg$gq_s0s1base_pte_offset;
unsigned long mmg$gq_shared_va_ptes;
unsigned long mmg$gq_sptbase;
unsigned long mmg$gq_svapte_pc;
unsigned long mmg$gq_svapte_pte;
unsigned long mmg$gq_svapte_va;
unsigned long mmg$gq_swapfile_allocs;
unsigned long mmg$gq_swapfile_pages;
unsigned long mmg$gq_system_l1pt;
unsigned long mmg$gq_system_virtual_base;
unsigned long mmg$gq_syswsl;
unsigned long mmg$gq_va_bits;
unsigned long mmg$gq_vpn_to_va;
unsigned long mmg$gq_window2_pte_pfn;
unsigned long mmg$gq_window2_va;
unsigned long mmg$gq_window_pte_pfn;
unsigned long mmg$gq_window_va;
unsigned long mmg$gq_zeroed_list_count;
unsigned long mmg$gw_minpfidx;
unsigned long mmg$imghdrbuf;
unsigned long mpw$ar_perfstats;
unsigned long mpw$aw_initval;
unsigned long mpw$gb_iolim;
unsigned long mpw$gb_prio;
unsigned long mpw$gl_badpagtotal;
unsigned long mpw$gl_hilim;
unsigned long mpw$gl_iopagcnt;
unsigned long mpw$gl_lolim;
unsigned long mpw$gl_lowaitlim;
unsigned long mpw$gl_thresh;
unsigned long mpw$gl_waitlim;
unsigned long mpw$gq_debug;
unsigned long mpw$gw_mpwpfc;
unsigned long mpw$l_count;
unsigned long mtx$gq_debug;
unsigned long net$ar_ctf_vector;
unsigned long net$ar_debug_vector;
unsigned long net$ar_emaa_ert;
unsigned long net$ar_emaa_vector;
unsigned long net$ar_lan_vector;
unsigned long net$ar_nbi_vector;
unsigned long net$ar_nrl_vector;
unsigned long net$ar_reserved1;
unsigned long net$ar_reserved2;
unsigned long net$ar_reserved3;
unsigned long net$ar_reserved4;
unsigned long net$ar_ssrv_vector;
unsigned long net$ar_wcb;
unsigned long net$ga_register_id;
unsigned long net$gl_atm_fwdpkt;
unsigned long net$gl_atm_rcvpkt;
unsigned long net$gl_atm_xmtpkt;
unsigned long net$gl_diag_buf;
unsigned long net$gl_nsa_fwdpkt;
unsigned long net$gq_ctf_reg_q;
unsigned long net$gq_ctf_tb_q;
unsigned long net$gq_ctf_wrk_q;
unsigned long net$gq_node_param;
unsigned long nl$ar_ddb;
unsigned long nl$ar_ddt;
unsigned long nl$ar_dpt;
unsigned long nl$ar_orb0;
unsigned long nl$ar_ucb0;
unsigned long no$gl_dpt;
unsigned long nsa$a_scratch;
unsigned long nsa$ar_alarm_failure;
unsigned long nsa$ar_alarm_vector;
unsigned long nsa$ar_audit_failure;
unsigned long nsa$ar_audit_vector;
unsigned long nsa$gl_audit_server_pid;
unsigned long nsa$gl_audit_summary;
unsigned long nsa$gl_server_wake_addr;
unsigned long nsa$gq_audit_queue;
unsigned long nsa$gq_deaccess_audit;
unsigned long op$ar_dpt;
unsigned long opa$ar_crb;
unsigned long opa$ar_ddb;
unsigned long opa$ar_idb;
unsigned long opa$ar_orb0;
unsigned long opa$ar_spl;
unsigned long opa$ar_ucb0;
unsigned long opa$ar_vector;
unsigned long ots$div_data;
unsigned long pdp$ga_cntrlblk;
unsigned long pdp$gl_status;
unsigned long pfm$c_bufcnt;
unsigned long pfn$al_color_head;
unsigned long pfn$al_color_tail;
unsigned long pfn$al_count;
unsigned long pfn$al_head[3];
unsigned long pfn$al_hilimit;
unsigned long pfn$al_lolimit;
unsigned long pfn$al_mfylsthd;
unsigned long pfn$al_tail[3];
unsigned long pfn$gl_phypgcnt;
unsigned long pfn$pq_database;
unsigned long phv$gl_pixbas;
unsigned long phv$gl_refcbas_lw;
unsigned long pio$a_rms_piobase;
unsigned long pio$a_rms_pioend;
unsigned long pio$al_rmsexh;
unsigned long pio$gb_dfmbc;
unsigned long pio$gb_dfmbfhsh;
unsigned long pio$gb_dfmbfidx;
unsigned long pio$gb_dfmbfrel;
unsigned long pio$gb_dfmbfsdk;
unsigned long pio$gb_dfmbfsmt;
unsigned long pio$gb_dfmbfsur;
unsigned long pio$gb_dfnbc;
unsigned long pio$gb_jnl_stall_cnt;
unsigned long pio$gb_rmsheuristic;
unsigned long pio$gb_rmsprolog;
unsigned long pio$gb_ruf_flags;
unsigned long pio$gb_vccdfw;
unsigned long pio$gl_alloc_table;
unsigned long pio$gl_asb_list;
unsigned long pio$gl_asb_slots;
unsigned long pio$gl_cur_asb;
unsigned long pio$gl_dircache;
unsigned long pio$gl_dirclals;
unsigned long pio$gl_fmlh;
unsigned long pio$gl_free_iio_rub_blink;
unsigned long pio$gl_free_iio_rub_flink;
unsigned long pio$gl_free_pio_rub_blink;
unsigned long pio$gl_free_pio_rub_flink;
unsigned long pio$gl_iiofsplh;
unsigned long pio$gl_lnkcshadr;
unsigned long pio$gl_nt0_rm_id;
unsigned long pio$gl_nxtirbseq;
unsigned long pio$gl_reserved3;
unsigned long pio$gl_reserved4;
unsigned long pio$gl_reserved5;
unsigned long pio$gl_reserved6;
unsigned long pio$gl_rms_active;
unsigned long pio$gl_rms_local_charset;
unsigned long pio$gl_rms_nql;
unsigned long pio$gl_ru_failure_count;
unsigned long pio$gl_ru_handler_id;
unsigned long pio$gl_ru_wait_q_blink;
unsigned long pio$gl_ru_wait_q_flink;
unsigned long pio$gl_rub_blink;
unsigned long pio$gl_rub_flink;
unsigned long pio$gq_free_iio_rub_qh;
unsigned long pio$gq_free_pio_rub_qh;
unsigned long pio$gq_iiodefault;
unsigned long pio$gq_ntrub_qh;
unsigned long pio$gq_reserved0;
unsigned long pio$gq_reserved1;
unsigned long pio$gq_reserved2;
unsigned long pio$gq_rub_qh;
unsigned long pio$gq_ruf_tsb_qh;
unsigned long pio$gq_urew_fplh;
unsigned long pio$gq_urew_iio_seg;
unsigned long pio$gq_urew_iiodefault;
unsigned long pio$gt_ddstring;
unsigned long pio$gt_endstr;
unsigned long pio$gw_dflrl;
unsigned long pio$gw_dfprot;
unsigned long pio$gw_dircalloc;
unsigned long pio$gw_dircsiz;
unsigned long pio$gw_iioimpa;
unsigned long pio$gw_pioimpa;
unsigned long pio$gw_rmsextend;
unsigned long pio$gw_status;
unsigned long pms$al_crf;
unsigned long pms$al_crfio;
unsigned long pms$al_transflt;
unsigned long pms$gb_prompt;
unsigned long pms$gl_2pcack_in;
unsigned long pms$gl_2pcack_out;
unsigned long pms$gl_2pccnc_in;
unsigned long pms$gl_2pccnc_out;
unsigned long pms$gl_2pcrdy_in;
unsigned long pms$gl_2pcrdy_out;
unsigned long pms$gl_2pcreq_in;
unsigned long pms$gl_2pcreq_out;
unsigned long pms$gl_arrlocpk;
unsigned long pms$gl_arrtrapk;
unsigned long pms$gl_blk_in;
unsigned long pms$gl_blk_loc;
unsigned long pms$gl_blk_out;
unsigned long pms$gl_bufio;
unsigned long pms$gl_bufobj_pages;
unsigned long pms$gl_bufobj_pages_peak;
unsigned long pms$gl_bufobj_pages_s0s1;
unsigned long pms$gl_bufobj_pages_s0s1_peak;
unsigned long pms$gl_bufobj_pages_s2;
unsigned long pms$gl_bufobj_pages_s2_peak;
unsigned long pms$gl_chme;
unsigned long pms$gl_chmk;
unsigned long pms$gl_compat;
unsigned long pms$gl_crelnm;
unsigned long pms$gl_crelnt;
unsigned long pms$gl_cwps_bytes_in;
unsigned long pms$gl_cwps_bytes_out;
unsigned long pms$gl_cwps_getjpi_in;
unsigned long pms$gl_cwps_getjpi_out;
unsigned long pms$gl_cwps_msgs_in;
unsigned long pms$gl_cwps_msgs_out;
unsigned long pms$gl_cwps_pcntrl_in;
unsigned long pms$gl_cwps_pcntrl_out;
unsigned long pms$gl_cwps_rsrc_recv;
unsigned long pms$gl_cwps_rsrc_send;
unsigned long pms$gl_dellnm;
unsigned long pms$gl_deplocpk;
unsigned long pms$gl_deq_in;
unsigned long pms$gl_deq_loc;
unsigned long pms$gl_deq_out;
unsigned long pms$gl_dir_in;
unsigned long pms$gl_dir_out;
unsigned long pms$gl_dirio;
unsigned long pms$gl_dlckfnd;
unsigned long pms$gl_dlckmsgs_in;
unsigned long pms$gl_dlckmsgs_out;
unsigned long pms$gl_dlcksrch;
unsigned long pms$gl_dostats;
unsigned long pms$gl_dptscn;
unsigned long pms$gl_dzroflts;
unsigned long pms$gl_enqcvt_in;
unsigned long pms$gl_enqcvt_loc;
unsigned long pms$gl_enqcvt_out;
unsigned long pms$gl_enqnew_in;
unsigned long pms$gl_enqnew_loc;
unsigned long pms$gl_enqnew_out;
unsigned long pms$gl_enqnotqd;
unsigned long pms$gl_enqwait;
unsigned long pms$gl_exec;
unsigned long pms$gl_exec_dispatch_vector;
unsigned long pms$gl_failed_trnlnm;
unsigned long pms$gl_faults;
unsigned long pms$gl_flags;
unsigned long pms$gl_foe_flts;
unsigned long pms$gl_for_flts;
unsigned long pms$gl_fow_flts;
unsigned long pms$gl_gblpagcnt;
unsigned long pms$gl_gblpagmax;
unsigned long pms$gl_gblsectcnt;
unsigned long pms$gl_gblsectmax;
unsigned long pms$gl_gvalid;
unsigned long pms$gl_hit;
unsigned long pms$gl_inter;
unsigned long pms$gl_iopfmpdb;
unsigned long pms$gl_iopfmseq;
unsigned long pms$gl_kernel;
unsigned long pms$gl_kernel_dispatch_vector;
unsigned long pms$gl_ldpctx;
unsigned long pms$gl_lognam;
unsigned long pms$gl_lrgrwp;
unsigned long pms$gl_mbreads;
unsigned long pms$gl_mbwrites;
unsigned long pms$gl_nostdtrm;
unsigned long pms$gl_npagdynexpf;
unsigned long pms$gl_npagdynexps;
unsigned long pms$gl_npagdynf;
unsigned long pms$gl_npagdynfpages;
unsigned long pms$gl_npagdynreq;
unsigned long pms$gl_npagdynreqf;
unsigned long pms$gl_pagdynf;
unsigned long pms$gl_pagdynfpages;
unsigned long pms$gl_pagdynreq;
unsigned long pms$gl_pagdynreqf;
unsigned long pms$gl_pages;
unsigned long pms$gl_passall;
unsigned long pms$gl_preadio;
unsigned long pms$gl_preads;
unsigned long pms$gl_proccntmax;
unsigned long pms$gl_pwrites;
unsigned long pms$gl_pwritio;
unsigned long pms$gl_qlen_scans;
unsigned long pms$gl_qlen_toctr;
unsigned long pms$gl_qlen_toint;
unsigned long pms$gl_rcvbuffl;
unsigned long pms$gl_rdflts;
unsigned long pms$gl_readcnt;
unsigned long pms$gl_reserved1;
unsigned long pms$gl_reserved10;
unsigned long pms$gl_reserved11;
unsigned long pms$gl_reserved12;
unsigned long pms$gl_reserved13;
unsigned long pms$gl_reserved14;
unsigned long pms$gl_reserved15;
unsigned long pms$gl_reserved16;
unsigned long pms$gl_reserved17;
unsigned long pms$gl_reserved18;
unsigned long pms$gl_reserved2;
unsigned long pms$gl_reserved3;
unsigned long pms$gl_reserved4;
unsigned long pms$gl_reserved5;
unsigned long pms$gl_reserved6;
unsigned long pms$gl_reserved7;
unsigned long pms$gl_reserved8;
unsigned long pms$gl_reserved9;
unsigned long pms$gl_rm_acquire;
unsigned long pms$gl_rm_better;
unsigned long pms$gl_rm_finish;
unsigned long pms$gl_rm_more_act;
unsigned long pms$gl_rm_msg_rcv;
unsigned long pms$gl_rm_msg_sent;
unsigned long pms$gl_rm_quota_wait;
unsigned long pms$gl_rm_rbld_rcvd;
unsigned long pms$gl_rm_rbld_sent;
unsigned long pms$gl_rm_req_nak;
unsigned long pms$gl_rm_single;
unsigned long pms$gl_rm_unload;
unsigned long pms$gl_rwp;
unsigned long pms$gl_rwpnostd;
unsigned long pms$gl_rwpsum;
unsigned long pms$gl_sch_0_page;
unsigned long pms$gl_split;
unsigned long pms$gl_super;
unsigned long pms$gl_switch;
unsigned long pms$gl_trcnglos;
unsigned long pms$gl_treads;
unsigned long pms$gl_tty_code1;
unsigned long pms$gl_tty_code2;
unsigned long pms$gl_twrites;
unsigned long pms$gl_user;
unsigned long pms$gl_wrtcnt;
unsigned long pms$gl_xrpfail;
unsigned long pms$gq_idle_loop;
unsigned long pms$gq_tqe_sysub;
unsigned long pms$gq_tqe_timer;
unsigned long pms$gq_tqe_total;
unsigned long pms$gq_tqe_wakeup;
unsigned long pms$gw_batch;
unsigned long pms$gw_intjobs;
unsigned long pql$ab_flag;
unsigned long pql$al_default;
unsigned long pql$al_min;
unsigned long pql$ar_syspql;
unsigned long pql$gdastlm;
unsigned long pql$gdbiolm;
unsigned long pql$gdbytlm;
unsigned long pql$gdcpulm;
unsigned long pql$gddiolm;
unsigned long pql$gdenqlm;
unsigned long pql$gdfillm;
unsigned long pql$gdjtquota;
unsigned long pql$gdpgflquota_pagelets;
unsigned long pql$gdpgflquota_pages;
unsigned long pql$gdprclm;
unsigned long pql$gdtqelm;
unsigned long pql$gdwsdefault_pagelets;
unsigned long pql$gdwsdefault_pages;
unsigned long pql$gdwsextent_pagelets;
unsigned long pql$gdwsextent_pages;
unsigned long pql$gdwsquota_pagelets;
unsigned long pql$gdwsquota_pages;
unsigned long pql$gl_syspqllen;
unsigned long pql$gmastlm;
unsigned long pql$gmbiolm;
unsigned long pql$gmbytlm;
unsigned long pql$gmcpulm;
unsigned long pql$gmdiolm;
unsigned long pql$gmenqlm;
unsigned long pql$gmfillm;
unsigned long pql$gmjtquota;
unsigned long pql$gmpgflquota_pagelets;
unsigned long pql$gmpgflquota_pages;
unsigned long pql$gmprclm;
unsigned long pql$gmtqelm;
unsigned long pql$gmwsdefault_pagelets;
unsigned long pql$gmwsdefault_pages;
unsigned long pql$gmwsextent_pagelets;
unsigned long pql$gmwsextent_pages;
unsigned long pql$gmwsquota_pagelets;
unsigned long pql$gmwsquota_pages;
unsigned long prf$gq_debug;
unsigned long psx$ga_fs_dataptr;
unsigned long psx$ga_p1dataptr;
unsigned long psx$gl_posix;
unsigned long psx$gl_process_flags;
unsigned long psx$gl_spare;
unsigned long psx$gl_state;
unsigned long qman$ar_qdbtable;
unsigned long qman$gl_getqui_default;
unsigned long qman$gl_getqui_seed;
unsigned long qman$gl_mutex;
unsigned long qman$gl_qmgrs_used;
unsigned long qman$gq_getqui_context;
unsigned long qman$gq_message_list;
unsigned long qsrv$gl_p0_root;
unsigned long qsrv$gl_pimgcnt;
unsigned long rms$gl_forklock_blk_status;
unsigned long rms$gq_local_charset_queue;
unsigned long rms$gq_reserved01;
unsigned long rms$gq_reserved02;
unsigned long rms$gq_reserved03;
unsigned long rms$gq_reserved04;
unsigned long rms$gq_reserved05;
unsigned long rms$gq_reserved06;
unsigned long rms$gq_reserved07;
unsigned long rms$gq_reserved08;
unsigned long rms$gq_reserved09;
unsigned long rms$gq_reserved10;
unsigned long sch$al_cpu_cap[32];
#ifdef __i386__
unsigned long long sch$aq_comh[33];
unsigned long long sch$aq_comoh[33];
#else
unsigned long long sch$aq_comh[33][2];
unsigned long long sch$aq_comoh[33][2];
#endif
#ifdef __i386__
unsigned long long * sch$aq_comot;
unsigned long long * sch$aq_comt;
#endif
struct _wqh sch$aq_wqhdr[12];
unsigned long sch$ar_cap_priv;
unsigned long sch$ar_class_name;
unsigned long sch$ar_class_pcblink;
unsigned long sch$ar_current_quant;
unsigned long sch$ar_nullpcb;
unsigned long sch$ar_original_quant;
unsigned long sch$ar_swppcb;
unsigned long sch$ar_time_restrict;
unsigned long sch$gb_maxclasspri;
unsigned long sch$gb_minclasspri;
unsigned long sch$gb_minprpri;
unsigned long sch$gb_priority_offset;
unsigned long sch$gb_rescan;
unsigned long sch$gl_affinity_skip = 2; // check. affinity_skip gen value?
unsigned long sch$gl_affinity_time;
unsigned long sch$gl_astdel_k;
unsigned long sch$gl_awsmin_pagelets;
unsigned long sch$gl_awsmin_pages;
unsigned long sch$gl_awstime;
unsigned long sch$gl_borrowlim;
unsigned long sch$gl_capability_sequence = 0;
unsigned long sch$gl_class_sched_mutex;
unsigned long sch$gl_cpu_cap_sum;
unsigned long sch$gl_ctlflags;
unsigned long sch$gl_def_rad_skip_cnt;
unsigned long sch$gl_default_cpu_cap = CPB$M_RUN; // add quorum later
unsigned long sch$gl_default_process_cap = CPB$M_RUN; // ditto
unsigned long sch$gl_exit_queue_rad;
unsigned long sch$gl_freecnt = 0;
unsigned long sch$gl_freelim = 64;
unsigned long sch$gl_freereq;
unsigned long sch$gl_growlim;
unsigned long sch$gl_idle_cpus;
unsigned long sch$gl_max_sched_class;
unsigned long sch$gl_maxpix = MAXPROCESSCNT;
unsigned long sch$gl_mfycnt = 0;
unsigned long sch$gl_mfylim = 64;
unsigned long sch$gl_mfylimsv;
unsigned long sch$gl_mfylolim;
unsigned long sch$gl_mfylosv;
unsigned long sch$gl_no_ktb_cpus;
unsigned long sch$gl_off_rad_fallback;
unsigned long sch$gl_off_rad_skip;
unsigned long sch$gl_on_rad;
unsigned long sch$gl_pcbvec;
unsigned long sch$gl_pfrath;
unsigned long sch$gl_pfratl;
unsigned long sch$gl_pixlast;
unsigned long sch$gl_pixwidth;
unsigned long sch$gl_resmask = 0;
unsigned long sch$gl_seqvec;
unsigned long sch$gl_sip = 0;
unsigned long sch$gl_swppid;
unsigned long sch$gl_swprate;
unsigned long sch$gl_tbsh;
unsigned long sch$gl_tbsoh;
unsigned long sch$gl_tbswh;
unsigned long sch$gl_transient_capabilities;
unsigned long sch$gl_wsdec_pagelets;
unsigned long sch$gl_wsdec_pages;
unsigned long sch$gl_wsinc_pagelets;
unsigned long sch$gl_wsinc_pages;
unsigned long sch$gl_active_priority = 0;
;
unsigned long sch$al_cpu_priority[32] =
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
unsigned long sch$gq_affinity_time_int;
unsigned long sch$gq_cc_per_hardtick;
unsigned long sch$gq_cc_per_quant;
unsigned long long sch$gq_cebhd;
unsigned long sch$gq_colpgwq;
unsigned long sch$gl_comoqs;
unsigned long sch$gl_comqs;
unsigned long sch$gq_fpgwq;
unsigned long sch$gq_hibowq;
unsigned long sch$gq_hibwq;
unsigned long sch$gq_idle_cpus;
unsigned long sch$gq_io_vs_compute;
unsigned long sch$gq_lefowq;
unsigned long sch$gq_lefwq;
unsigned long sch$gq_mwait;
unsigned long sch$gq_pfwq;
unsigned long sch$gq_reserved00;
unsigned long sch$gq_reserved01;
unsigned long sch$gq_reserved02;
unsigned long sch$gq_reserved_user_caps;
unsigned long sch$gq_susp;
unsigned long sch$gq_suspo;
unsigned long sch$gw_cebcnt;
unsigned long sch$gw_delphdct;
unsigned long sch$gw_dormantwait = DORMANTWAIT;
unsigned long sch$gw_iota;
unsigned long sch$gw_localnode;
unsigned long sch$gw_longwait;
unsigned long sch$gw_maxpixseq;
unsigned long sch$gw_proccnt;
unsigned long sch$gw_proclim;
unsigned long sch$gw_quan;
unsigned long sch$gw_swpfail;
unsigned long sch$gw_swpfcnt;
unsigned long scs$ar_localsb;
unsigned long scs$ar_pingpong_counters;
unsigned long scs$ar_pingpong_ping_cdt;
unsigned long scs$ar_pingpong_pong_cdt;
unsigned long scs$ar_qsrv_kclient;
unsigned long scs$ar_qsrv_kserver;
unsigned long scs$ga_exists;
unsigned long scs$gb_discon_limit;
unsigned long scs$gb_log;
unsigned long scs$gb_nodename;
unsigned long scs$gb_pamxport;
unsigned long scs$gb_panopoll;
unsigned long scs$gb_panpoll;
unsigned long scs$gb_pasanity;
unsigned long scs$gb_systemid;
unsigned long scs$gb_systemidh;
unsigned long scs$gb_udaburst;
unsigned long scs$gl_available_port;
unsigned long scs$gl_bdlt;
unsigned long scs$gl_cdl;
unsigned long scs$gl_init_timeout;
unsigned long scs$gl_load_share_interval;
unsigned long scs$gl_load_share_pdt_table;
unsigned long scs$gl_max_bdlt_index;
unsigned long scs$gl_mcadr;
unsigned long scs$gl_mclen;
unsigned long scs$gl_min_conn_wait_interval;
unsigned long scs$gl_min_port_wait_interval;
unsigned long scs$gl_mscp;
unsigned long scs$gl_mscp_mv;
unsigned long scs$gl_mscp_newdev;
unsigned long scs$gl_nonpaged_ddma;
unsigned long scs$gl_pdt;
unsigned long scs$gl_pingpong_num_trips;
unsigned long scs$gl_pingpong_ping_error;
unsigned long scs$gl_pingpong_pong_error;
unsigned long scs$gl_pingpong_test_done;
unsigned long scs$gl_pingpong_trip_count;
unsigned long scs$gl_rdt;
unsigned long scs$gl_red_threshold;
unsigned long scs$gl_reserved1;
unsigned long scs$gl_reserved2;
unsigned long scs$gl_share_enable;
unsigned long scs$gl_tmscp;
unsigned long scs$gl_tmscp_mv;
unsigned long scs$gl_tolerance;
unsigned long scs$gl_yellow_threshold;
unsigned long scs$gq_config;
unsigned long scs$gq_ddma_base;
unsigned long scs$gq_ddma_len;
unsigned long scs$gq_direct;
unsigned long scs$gq_disconnect;
unsigned long scs$gq_listhead_ddma;
unsigned long long scs$gq_local_names;
unsigned long scs$gq_pingpong_start_time;
unsigned long scs$gq_pingpong_stop_time;
unsigned long scs$gq_poll;
unsigned long scs$gq_reuse_cdt;
unsigned long scs$gq_worse_path;
unsigned long scs$gw_bdtcnt;
unsigned long scs$gw_cdtcnt = 1024;
unsigned long scs$gw_flowcush;
unsigned long scs$gw_maxdg;
unsigned long scs$gw_maxmsg;
unsigned long scs$gw_next_index;
unsigned long scs$gw_papolint;
unsigned long scs$gw_papoolin;
unsigned long scs$gw_pappddg;
unsigned long scs$gw_pastmout;
unsigned long scs$gw_prcpolint;
unsigned long scs$gw_rdtcnt = 128;
unsigned long scs$gw_timvcfail;
unsigned long sevms$ar_alarm_vector;
unsigned long sevms$ar_audit_vector;
unsigned long sevms$gl_flags;
unsigned long sgn$gb_multiprocessing;
unsigned long sgn$gb_pgtbpfc_pagelets;
unsigned long sgn$gb_pgtbpfc_pages;
unsigned long sgn$gb_qbus_mult_intr;
unsigned long sgn$gb_scsicluster_p1;
unsigned long sgn$gb_scsicluster_p2;
unsigned long sgn$gb_scsicluster_p3;
unsigned long sgn$gb_scsicluster_p4;
unsigned long sgn$gb_startup_p1;
unsigned long sgn$gb_startup_p2;
unsigned long sgn$gb_startup_p3;
unsigned long sgn$gb_startup_p4;
unsigned long sgn$gb_startup_p5;
unsigned long sgn$gb_startup_p6;
unsigned long sgn$gb_startup_p7;
unsigned long sgn$gb_startup_p8;
unsigned long sgn$gb_syspfc_pagelets;
unsigned long sgn$gb_syspfc_pages;
unsigned long sgn$gb_tailored;
unsigned long sgn$gl_arb_support;
unsigned long sgn$gl_auto_dlight_sav;
unsigned long sgn$gl_balsetct;
unsigned long sgn$gl_brkmsk;
unsigned long sgn$gl_cwcreprc_enable;
unsigned long sgn$gl_dnvosi1;
unsigned long sgn$gl_dr_unit_base;
unsigned long sgn$gl_dsb_upcalls;
unsigned long sgn$gl_dump_style;
unsigned long sgn$gl_extracpu;
unsigned long sgn$gl_exusrstk;
unsigned long sgn$gl_f64ctl1;
unsigned long sgn$gl_f64ctl2;
unsigned long sgn$gl_f64ctld3;
unsigned long sgn$gl_f64ctld4;
unsigned long sgn$gl_fast_path;
unsigned long sgn$gl_fast_path_ports;
unsigned long sgn$gl_file_cache;
unsigned long sgn$gl_file_cache_max;
unsigned long sgn$gl_file_cache_min;
unsigned long sgn$gl_freegoal = 1024;
unsigned long sgn$gl_freelim;
unsigned long sgn$gl_galaxy;
unsigned long sgn$gl_gblpagfil;
unsigned long sgn$gl_glx_inst_tmo;
unsigned long sgn$gl_glx_shm_reg;
unsigned long sgn$gl_jobctld;
unsigned long sgn$gl_kstackpag;
unsigned long sgn$gl_lan_flags;
unsigned long sgn$gl_lckmgr_flags;
unsigned long sgn$gl_loadflags;
unsigned long sgn$gl_maxgpgct_pagelets;
unsigned long sgn$gl_maxgpgct_pages;
unsigned long sgn$gl_maxvpgct_pagelets;
unsigned long sgn$gl_maxvpgct_pages;
unsigned long sgn$gl_maxwscnt_pagelets;
unsigned long sgn$gl_maxwscnt_pages;
unsigned long sgn$gl_mc_services_p0;
unsigned long sgn$gl_mc_services_p1;
unsigned long sgn$gl_mc_services_p2;
unsigned long sgn$gl_mc_services_p3;
unsigned long sgn$gl_mc_services_p4;
unsigned long sgn$gl_mc_services_p5;
unsigned long sgn$gl_mc_services_p6;
unsigned long sgn$gl_mc_services_p7;
unsigned long sgn$gl_mc_services_p8;
unsigned long sgn$gl_mc_services_p9;
unsigned long sgn$gl_minwscnt;
unsigned long sgn$gl_multithread;
unsigned long sgn$gl_npag_aggressive;
unsigned long sgn$gl_npag_gentle;
unsigned long sgn$gl_npag_interval;
unsigned long sgn$gl_npag_ring_size;
unsigned long sgn$gl_npagecalc;
unsigned long sgn$gl_npagedyn = 0x800000;
unsigned long sgn$gl_npagerad;
unsigned long sgn$gl_npagevir;
unsigned long sgn$gl_p1lwcnt;
unsigned long sgn$gl_pagedyn;
unsigned long sgn$gl_pe1;
unsigned long sgn$gl_pe2;
unsigned long sgn$gl_pe3;
unsigned long sgn$gl_pe4;
unsigned long sgn$gl_pe5;
unsigned long sgn$gl_pe6;
unsigned long sgn$gl_pfn_color_count;
unsigned long sgn$gl_pseudoloa;
unsigned long sgn$gl_pu_options;
unsigned long sgn$gl_rad_support;
unsigned long sgn$gl_s2_size;
unsigned long sgn$gl_smci_flags;
unsigned long sgn$gl_smci_ports;
unsigned long sgn$gl_smp_cpus;
unsigned long sgn$gl_smp_cpush;
unsigned long sgn$gl_smp_lngspinwait;
unsigned long sgn$gl_smp_sanity_cnt;
unsigned long sgn$gl_smp_spinwait;
unsigned long sgn$gl_smp_tick_cnt;
unsigned long sgn$gl_sysdwsct_pagelets;
unsigned long sgn$gl_sysdwsct_pages;
unsigned long sgn$gl_system_check;
unsigned long sgn$gl_user3;
unsigned long sgn$gl_user4;
unsigned long sgn$gl_userd1;
unsigned long sgn$gl_userd2;
unsigned long sgn$gl_vms5;
unsigned long sgn$gl_vms6;
unsigned long sgn$gl_vms7;
unsigned long sgn$gl_vms8;
unsigned long sgn$gl_vmsd1;
unsigned long sgn$gl_vmsd2;
unsigned long sgn$gl_vmsd3;
unsigned long sgn$gl_vmsd4;
unsigned long sgn$gl_wptte_size;
unsigned long sgn$gl_xqpctl2;
unsigned long sgn$gl_xqpctl4;
unsigned long sgn$gl_xqpctl6;
unsigned long sgn$gl_xqpctl8;
unsigned long sgn$gl_xqpctld1;
unsigned long sgn$gl_xqpctld3;
unsigned long sgn$gl_xqpctld5;
unsigned long sgn$gl_xqpctld7;
unsigned long sgn$gw_ctlimglim;
unsigned long sgn$gw_ctlpagelets;
unsigned long sgn$gw_dfpfc_pagelets;
unsigned long sgn$gw_dfpfc_pages;
unsigned long sgn$gw_erlbufcnt;
unsigned long sgn$gw_gblsecnt;
unsigned long sgn$gw_imgiocnt;
unsigned long sgn$gw_maxprcct;
unsigned long sgn$gw_maxpstct;
unsigned long sgn$gw_pagfilct;
unsigned long sgn$gw_pchancnt;
unsigned long sgn$gw_piopagelets;
unsigned long sgn$gw_pixscan = 1;
unsigned long sgn$gw_swpfilct;
unsigned long sgn$gw_swpfiles;
unsigned long sgn$gw_tpwait;
unsigned long sgn$gw_wpre_size;
unsigned long sgn$gw_wslmxskp;
unsigned long smp$al_iplvec;
unsigned long smp$ar_busywait_scc;
unsigned long smp$ar_busywait_ticks;
unsigned long smp$ar_cpu_ring_pointer;
unsigned long smp$ar_spinlock_data;
unsigned long smp$ar_spnlkvec;
unsigned long smp$gb_pfork_pool_size;
unsigned long smp$gl_active_cpus;
unsigned long smp$gl_astsr_ack;
unsigned long smp$gl_available_port_cpus;
unsigned long smp$gl_bug_done;
unsigned long smp$gl_bugchkcp;
unsigned long long smp$gq_capabilities[32];
unsigned short smp$gw_affinity_count[32];
unsigned long smp$gl_compat_level;
unsigned long smp$gl_cpu_auto_start;
struct _cpu * smp$gl_cpu_data[64];
unsigned long smp$gl_cpu_transition;
unsigned long smp$gl_cpu_upd_defer;
unsigned long smp$gl_cpuconf;
unsigned long smp$gl_cpus_powered;
unsigned long smp$gl_cpus_present = 1;
unsigned long smp$gl_cpus_tested;
unsigned long smp$gl_dynamic_busywait;
unsigned long smp$gl_flags;
unsigned long smp$gl_last_cpu_assigned;
unsigned long smp$gl_max_cpuid;
unsigned long smp$gl_override;
unsigned long smp$gl_pfork_pool;
unsigned long smp$gl_potential_set;
unsigned long smp$gl_primid;
unsigned long smp$gl_tbi_ack_mask;
unsigned long smp$gl_tbi_ack_mask_mmg;
unsigned long smp$gpq_ctd_listhead;
unsigned long smp$gpq_failover_gnode;
unsigned long smp$gq_abuse_count;
unsigned long smp$gq_cpu_mutex;
unsigned long smp$gq_ctd_wait_queue;
unsigned long smp$gq_debug;
unsigned long smp$gq_invalid;
unsigned long smp$gq_invalid_mmg;
unsigned long smp$gq_mpsynch_flags;
unsigned long smp$gq_new_hwclock;
unsigned long smp$gq_primary_workq;
unsigned long smp$gq_proposed_hwclock;
unsigned long smp$gq_tree_upd_count;
unsigned long smp$gw_min_index;
unsigned long smp$gw_spnlkcnt;
unsigned long swi$gl_fqbl;
unsigned long swi$gl_fqfl;
unsigned long swp$al_swapper_stack;
unsigned long swp$c_kstack;
unsigned long swp$c_kstack_wsl;
unsigned long swp$c_l1pt;
unsigned long swp$c_l2pt;
unsigned long swp$c_shlp1pt;
unsigned long swp$c_shlproc;
unsigned long swp$gb_iswpri;
unsigned long swp$gb_prio;
unsigned long swp$gl_balbase;
unsigned long swp$gl_balspt;
unsigned long swp$gl_bslotsz;
unsigned long swp$gl_hiswpcnt;
unsigned long swp$gl_hoswpcnt;
unsigned long swp$gl_inpcb;
unsigned long swp$gl_ispagcnt;
unsigned long swp$gl_iswpcnt;
unsigned long swp$gl_iswppages;
unsigned long swp$gl_map;
unsigned long swp$gl_min_shellpfil;
unsigned long swp$gl_oswpcnt;
unsigned long swp$gl_phdbasva;
unsigned long swp$gl_shell;
unsigned long swp$gl_shellbas;
unsigned long swp$gl_shellpfil;
unsigned long swp$gl_shellsiz;
unsigned long swp$gl_slotcnt;
unsigned long swp$gl_swap_image_size_max;
unsigned long swp$gl_swapper_stack_size;
unsigned long swp$gl_swppgcnt_pagelets;
unsigned long swp$gl_swppgcnt_pages;
unsigned long swp$gl_swtime;
unsigned long swp$gl_troll_time;
unsigned long swp$gq_bal_end;
unsigned long swp$gq_balbase;
unsigned long swp$gw_balcnt;
unsigned long swp$gw_emptpte;
unsigned long swp$gw_fredpte;
unsigned long swp$gw_ibalsetx;
unsigned long swp$gw_swpinc;
unsigned long swp$gw_wslpte;
unsigned long sys$ar_audsrvmbx;
unsigned long sys$ar_bootddb;
unsigned long sys$ar_bootorb;
unsigned long sys$ar_bootucb;
unsigned long sys$ar_dcpi_mbx;
unsigned long sys$ar_jobctlmb;
unsigned long sys$ar_oprmbx;
unsigned long sys$ar_qiosrvmbx;
unsigned long sys$c_jobctlmb;
unsigned long sys$gb_brk_lim;
unsigned long sys$gb_callouts;
unsigned long sys$gb_defpri;
unsigned long sys$gb_defquepri;
unsigned long sys$gb_dfmbc;
unsigned long sys$gb_dfmbfhsh;
unsigned long sys$gb_dfmbfidx;
unsigned long sys$gb_dfmbfrel;
unsigned long sys$gb_dfmbfsdk;
unsigned long sys$gb_dfmbfsmt;
unsigned long sys$gb_dfmbfsur;
unsigned long sys$gb_dfnbc;
unsigned long sys$gb_maxquepri;
unsigned long sys$gb_net_callouts;
unsigned long sys$gb_pwd_tmo;
unsigned long sys$gb_retry_lim;
unsigned long sys$gb_retry_tmo;
unsigned long sys$gb_rmsheuristic;
unsigned long sys$gb_rmsprolog;
unsigned long sys$gb_vccdfw;
unsigned long sys$gl_bjobcnt;
unsigned long sys$gl_brk_tmo;
unsigned long sys$gl_erlbufpagelets;
unsigned long sys$gl_exitret;
unsigned long sys$gl_fallback;
unsigned long sys$gl_hid_tim;
unsigned long sys$gl_ijobcnt;
unsigned long sys$gl_njobcnt;
unsigned long sys$gl_rms_nql;
unsigned long sys$gl_rmsd1;
unsigned long sys$gl_uis;
unsigned long sys$gl_uis_flags;
unsigned long sys$gl_uisbg_epid;
unsigned long sys$gl_version;
unsigned long sys$gq_pwd;
unsigned long sys$gq_version;
unsigned long sys$gw_bjoblim;
unsigned long sys$gw_dflrl;
unsigned long sys$gw_fileprot;
unsigned long sys$gw_ijoblim;
unsigned long sys$gw_mbxucbsiz;
unsigned long sys$gw_njoblim;
unsigned long sys$gw_rjoblim;
unsigned long sys$gw_rmsextend;
unsigned long sys$k_version;
unsigned long sys$k_version_01;
unsigned long sys$k_version_02;
unsigned long sys$k_version_03;
unsigned long sys$k_version_04;
unsigned long sys$k_version_05;
unsigned long sys$k_version_06;
unsigned long sys$k_version_07;
unsigned long sys$k_version_08;
unsigned long sys$k_version_09;
unsigned long sys$k_version_10;
unsigned long sys$k_version_11;
unsigned long sys$k_version_12;
unsigned long sys$k_version_13;
unsigned long sys$k_version_14;
unsigned long sys$k_version_15;
unsigned long sys$k_version_16;
unsigned long sys$k_version_17;
unsigned long sys$k_version_18;
unsigned long sys$k_version_19;
unsigned long sys$version_begin;
unsigned long tr$gq_debug;
unsigned long tty$gb_autochar;
unsigned long tty$gb_defspeed;
unsigned long tty$gb_dialtyp;
unsigned long tty$gb_parity;
unsigned long tty$gb_rspeed;
unsigned long tty$gb_silotime;
unsigned long tty$gl_asian_dpt;
unsigned long tty$gl_defchar;
unsigned long tty$gl_defchar2;
unsigned long tty$gl_defport;
unsigned long tty$gl_delta;
unsigned long tty$gl_dpt = 0;
unsigned long tty$gl_jobctlmb;
unsigned long tty$gl_timeout;
unsigned long tty$gw_altalarm;
unsigned long tty$gw_altypahd;
unsigned long tty$gw_classnam;
unsigned long tty$gw_defbuf;
unsigned long tty$gw_dmasize;
unsigned long tty$gw_typahdsz = SYI$_TTY_TYPAHDSZ;
unsigned long uid$gl_ieee_address;
unsigned long uid$gw_ieee_addressh;
unsigned long uis$gl_ltrc_buf;
unsigned long uis$gl_ltrc_end;
unsigned long uis$gl_ltrc_ptr;
unsigned long uis$gl_ltrc_spare;
unsigned long uis$gl_usb;
unsigned long vcc$gl_max_cache;
unsigned long vcc$gl_max_cache_pages;
unsigned long vcc$gl_max_io_size;
unsigned long vcc$gl_max_locks;
unsigned long vcc$gl_readahead;
unsigned long vcc$gl_write_delay;
unsigned long vcc$gl_writebehind;
unsigned long vms$gl_license_version;
unsigned long vms$gq_license_date;
unsigned long wbm$gl_data;
unsigned long wbm$gl_interval;
unsigned long wbm$gl_lower_threshold;
unsigned long wbm$gl_shadow_max_unit;
unsigned long wbm$gl_upper_threshold;
unsigned long wbm$gl_wbm_opcom_lvl;
unsigned long wbm$gq_wbmb_queue;
unsigned long xdt$al_tk_net_info;
unsigned long xdt$gl_benign_cpus;
unsigned long xdt$gl_interlock;
unsigned long xdt$gl_owner_id;
unsigned long xdt$gl_scb;
unsigned long xdt$gl_scratch_pages;
unsigned long xdt$gq_scratch_memory;
unsigned long xqp$ar_xqp_private;
unsigned long xqp$gl_alloc_blocks;
unsigned long xqp$gl_dzro;
unsigned long xqp$gl_fileserv_entry;
unsigned long xqp$gl_fileserver;
unsigned long xqp$gl_request_callout;
unsigned long xqp$gl_sections;

struct lnmhshs lnmhshs; /* should be one struct, will be solved later */
struct lnmhshp lnmhshp;

struct _tqe tqe, tqe2;

struct _cpu vmscpus[32]; /* max. this number should be defined */

//long long forklistheads[32][6];

struct _rsb * reshashtbl[2 * RESHASHTBL];
unsigned long lockidtbl[LOCKIDTBL];
unsigned long lockmaxid;
unsigned long locknxtid;

struct _cdt cdtl[1024]; //[scs$gw_cdtcnt];
struct _scs_rd rdtl[128]; //[scs$gw_rdtcnt];

/* spinlocks */

struct _spl SPIN_ATOMIC __cacheline_aligned =
{
spl$l_own_cpu : 0, spl$l_own_cnt :
    -1 /*, spl$l_spinlock : 0*/
}; /* for atomic operations */

/* corresponding to ipls */

struct _spl SPIN_EMB __cacheline_aligned =
{
spl$l_own_cpu : 0, spl$l_own_cnt :
    -1 /*, spl$l_spinlock : 0*/
};
struct _spl SPIN_MCHECK __cacheline_aligned =
{
spl$l_own_cpu : 0, spl$l_own_cnt :
    -1 /*, spl$l_spinlock : 0*/
};
struct _spl SPIN_MEGA __cacheline_aligned =
{
spl$l_own_cpu : 0, spl$l_own_cnt :
    -1 /*, spl$l_spinlock : 0*/
};
struct _spl SPIN_HWCLK __cacheline_aligned =
{
spl$l_own_cpu : 0, spl$l_own_cnt :
    -1 /*, spl$l_spinlock : 0*/
};
struct _spl SPIN_VIRTCONS __cacheline_aligned =
{
spl$l_own_cpu : 0, spl$l_own_cnt :
    -1 /*, spl$l_spinlock : 0*/
};
struct _spl SPIN_INVALIDATE __cacheline_aligned =
{
spl$l_own_cpu : 0, spl$l_own_cnt :
    -1 /*, spl$l_spinlock : 0*/
};
struct _spl SPIN_PERFMON __cacheline_aligned =
{
spl$l_own_cpu : 0, spl$l_own_cnt :
    -1 /*, spl$l_spinlock : 0*/
};
struct _spl SPIN_POOL __cacheline_aligned =
{
spl$l_own_cpu : 0, spl$l_own_cnt :
    -1 /*, spl$l_spinlock : 0*/
};
struct _spl SPIN_MAILBOX __cacheline_aligned =
{
spl$l_own_cpu : 0, spl$l_own_cnt :
    -1 /*, spl$l_spinlock : 0*/
};
struct _spl SPIN_PR_LK11 __cacheline_aligned =
{
spl$l_own_cpu : 0, spl$l_own_cnt :
    -1 /*, spl$l_spinlock : 0*/
};
struct _spl SPIN_IOLOCK11 __cacheline_aligned =
{
spl$l_own_cpu : 0, spl$l_own_cnt :
    -1 /*, spl$l_spinlock : 0*/
};
struct _spl SPIN_PR_LK10 __cacheline_aligned =
{
spl$l_own_cpu : 0, spl$l_own_cnt :
    -1 /*, spl$l_spinlock : 0*/
};
struct _spl SPIN_IOLOCK10 __cacheline_aligned =
{
spl$l_own_cpu : 0, spl$l_own_cnt :
    -1 /*, spl$l_spinlock : 0*/
};
struct _spl SPIN_PR_LK9 __cacheline_aligned =
{
spl$l_own_cpu : 0, spl$l_own_cnt :
    -1 /*, spl$l_spinlock : 0*/
};
struct _spl SPIN_IOLOCK9 __cacheline_aligned =
{
spl$l_own_cpu : 0, spl$l_own_cnt :
    -1 /*, spl$l_spinlock : 0*/
};
struct _spl SPIN_SCHED __cacheline_aligned =
{
spl$l_own_cpu : 0, spl$l_own_cnt :
    -1 /*, spl$l_spinlock : 0*/
};
struct _spl SPIN_MMG __cacheline_aligned =
{
spl$l_own_cpu : 0, spl$l_own_cnt :
    -1 /*, spl$l_spinlock : 0*/
};
struct _spl SPIN_JIB __cacheline_aligned =
{
spl$l_own_cpu : 0, spl$l_own_cnt :
    -1 /*, spl$l_spinlock : 0*/
};
struct _spl SPIN_TIMER __cacheline_aligned =
{
spl$l_own_cpu : 0, spl$l_own_cnt :
    -1 /*, spl$l_spinlock : 0*/
};
struct _spl SPIN_PR_LK8 __cacheline_aligned =
{
spl$l_own_cpu : 0, spl$l_own_cnt :
    -1 /*, spl$l_spinlock : 0*/
};
struct _spl SPIN_IOLOCK8 __cacheline_aligned =
{
spl$l_own_cpu : 0, spl$l_own_cnt :
    -1 /*, spl$l_spinlock : 0*/
};
struct _spl SPIN_SCS __cacheline_aligned =
{
spl$l_own_cpu : 0, spl$l_own_cnt :
    -1 /*, spl$l_spinlock : 0*/
};
struct _spl SPIN_IOPOST __cacheline_aligned =
{
spl$l_own_cpu : 0, spl$l_own_cnt :
    -1 /*, spl$l_spinlock : 0*/
}; // new in freevms

struct _phd system_phd;

extern void exe$timeout(void);

void qhead_init(void * l)
{
    struct _pcb * tmp = (struct _pcb *) l; /* could be anything */
    tmp->pcb$l_sqfl = tmp;
    tmp->pcb$l_sqbl = tmp;
}

int vmstimerconf = 0;

struct myq2
{
    unsigned long flink;
    unsigned long blink;
};

unsigned long pcbvec[MAXPROCESSCNT];
unsigned long seqvec[MAXPROCESSCNT];
unsigned long csid; // have to use it like this for  now

#ifdef __i386__
char boot_cpu_stack[0x4000] __cacheline_aligned;
#endif

void __init vms_init(void)
{
    int i, j;

    sch$gl_pixwidth = 0;
    for (i = MAXPROCESSCNT; i; i = i >> 1)
        sch$gl_pixwidth++;

    for (i = 0; i < 32; i++)
    {
        smp$gl_cpu_data[i] = &vmscpus[i];
        smp$gl_cpu_data[i]->cpu$l_cpuid_mask = 1 << i;
        for (j = 0; j < 6; j++)
        {
            struct myq2 * q = &smp$gl_cpu_data[i]->cpu$q_swiqfl[j];
            q->flink = q;
            q->blink = q;
        }
        qhead_init(&smp$gl_cpu_data[i]->cpu$l_psfl);
        smp$gl_cpu_data[i]->cpu$l_phy_cpuid = i;
        smp$gl_cpu_data[i]->cpu$l_cpuid_mask = 1 << i;
#ifdef __x86_64__
#include <asm/pda.h>
        smp$gl_cpu_data[i]->cpu$ps_pda = cpu_pda + i;
#endif
    }

    smp$gl_cpu_data[0]->cpu$l_curpcb = &init_task_union;
#if 0
    smp$gl_cpu_data[0]->cpu$l_capability = CPB$M_RUN | CPB$M_PRIMARY;
#else
    sch$add_cpu_cap(0, CPB$M_RUN | CPB$M_PRIMARY, 0);
#endif
    sch$al_cpu_cap[0] = smp$gl_cpu_data[0]->cpu$l_capability;
#ifdef __i386__
    smp$gl_cpu_data[0]->cpu$l_saved_isp = &boot_cpu_stack[0x4000];
#endif

    sch$gl_idle_cpus = 0;
#ifdef __i386__
    sch$aq_comot=(void *)sch$aq_comoh+4;
    sch$aq_comt=(void *)sch$aq_comh+4;
#else
#if 0
    sch$aq_comot=(void *)sch$aq_comoh+8;
    sch$aq_comt=(void *)sch$aq_comh+8;
#endif
#endif

    for (i = 0; i < 32; i++)
    {
        struct _pcb * tmp;
        tmp = (struct _pcb *) &sch$aq_comh[i];
        tmp->pcb$l_sqfl = tmp;
        tmp->pcb$l_sqbl = tmp;
    }

    for (i = 0; i < 12; i++)
    {
        struct _wqh * tmp;
        tmp = &sch$aq_wqhdr[i];
        tmp->wqh$l_wqfl = tmp;
        tmp->wqh$l_wqbl = tmp;
        tmp->wqh$l_wqcnt = 0;
        tmp->wqh$l_wqstate = i;
    }

    sch$gq_colpgwq = &sch$aq_wqhdr[1];
    sch$gq_mwait = &sch$aq_wqhdr[2];
//sch$gq_cefwq=&sch$aq_wqhdr[3];
    sch$gq_pfwq = &sch$aq_wqhdr[4];
    sch$gq_lefwq = &sch$aq_wqhdr[5];
    sch$gq_lefowq = &sch$aq_wqhdr[6];
    sch$gq_hibwq = &sch$aq_wqhdr[7];
    sch$gq_hibowq = &sch$aq_wqhdr[8];
    sch$gq_susp = &sch$aq_wqhdr[9];
    sch$gq_suspo = &sch$aq_wqhdr[10];
    sch$gq_fpgwq = &sch$aq_wqhdr[11];

    exe$gq_systime = 40916 * 864;
    exe$gq_systime *= 100;
    exe$gq_systime += xtime.tv_sec;
    exe$gq_systime *= 10000000;

    exe$gq_boottime = exe$gq_systime;

    exe$gl_tqfl = (void *) &tqehead;
    exe$gl_tqfl->tqe$l_tqfl = exe$gl_tqfl;
    exe$gl_tqfl->tqe$l_tqbl = exe$gl_tqfl;

    tqe.tqe$l_tqfl = 0;
    tqe.tqe$l_tqbl = 0;
    tqe.tqe$w_size = 0;
    tqe.tqe$b_type = DYN$C_TQE;
    tqe.tqe$b_rqtype = TQE$C_TMSNGL | TQE$M_REPEAT; /* ???? */
    tqe.tqe$l_rqpid = 0xffffffff;
    tqe.tqe$l_cputim = 0xffffffff;
    tqe.tqe$q_time = 0x4000000000000000;
    insque(&tqe, exe$gl_tqfl);

    tqe2.tqe$l_tqfl = 0;
    tqe2.tqe$l_tqbl = 0;
    tqe2.tqe$w_size = 0;
    tqe2.tqe$b_type = DYN$C_TQE;
    tqe2.tqe$b_rqtype = TQE$C_SSREPT;
    tqe2.tqe$l_fpc = &exe$timeout;
    tqe2.tqe$l_fr3 = 0; /* something? */
    tqe2.tqe$l_fr4 = 0; /* something? */
    tqe2.tqe$q_time = 0;
    tqe2.tqe$q_delta = 10000000;
    insque(&tqe2, exe$gl_tqfl);
    //  exe$instimq(&tqe2);

    vmstimerconf = 1;

    //  qhead_init(&ioc$gq_postiq);

    qhead_init(&lck$gl_rrsfl);

    for (i = 0; i < RESHASHTBL; i++)
    {
        //    reshashtbl[i]=0;
        qhead_init(&reshashtbl[2 * i]);
    }
    lck$gl_hashtbl = &reshashtbl;
    lck$gl_idtbl = &lockidtbl;
    for (i = 0; i < LOCKIDTBL - 1; i++)
        lockidtbl[i] = i + 1;
    lck$gl_nxtid = &locknxtid;
    lck$gl_maxid = &lockmaxid;

    qhead_init(&scs$gq_local_names);

    qhead_init(&sch$gq_cebhd);

    qhead_init(&exe$gl_gsdsysfl);
    qhead_init(&exe$gl_gsdgrpfl);

    printk("Done system data cells\n");
}

#define FUDGEFACTOR 40587

void __init vms_init2(void)
{
    //  exe$gq_systime=40916*864;
    exe$gq_systime = FUDGEFACTOR * 864;
    exe$gq_systime *= 100;
    exe$gq_systime += xtime.tv_sec;
    exe$gq_systime *= 10000000;

    exe$gq_boottime = exe$gq_systime;

    xqp_init2();
    exttwo_init2(0);

#if 0
#ifdef CONFIG_BLK_DEV_FD_VMS
    vms_floppy_init();
#endif
#endif
}

void __init vms_init4(void)
{
    __fl_init();
    //nl_init(); /* first */
    //rnd_init();
    tty_vmsinit();
    ft_vmsinit();
    pn_vmsinit();
    tz_vmsinit();
    ip4_vmsinit();
    void exe$reclaim_pool_gentle(void * pool);
    signed long long time = -10000000 * 60;
    exe$setimr(0, &time, exe$reclaim_pool_gentle, exe$gs_npp_npool, 0);
}

void __init vms_init3(void)
{
    mmg$gq_gpt_base = kmalloc(4096, GFP_KERNEL);

    mmg$gl_sysphd = &system_phd;
    init_phd(&system_phd);

    csid = xtime.tv_sec;
}

unsigned long long inline unix_to_vms_time(unsigned long l)
{
    unsigned long long retval = FUDGEFACTOR * 864;
    retval *= 100;
    retval += l;
    retval *= 10000000;
    return retval;
}

unsigned long inline vms_to_unix_time(unsigned long long l)
{
    unsigned long long retval = FUDGEFACTOR * 864;
    retval *= 100;
    retval = l - retval;
    //  retval/=10000000;
    retval >>= 23;
    return retval;
}

