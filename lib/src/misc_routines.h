#ifndef MISC_ROUTINES_H
#define MISC_ROUTINES_H

#include <linux/ext2_fs.h>
#include <linux/kdev_t.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>

#include <atrdef.h>
#include <cdtdef.h>
#include <crbdef.h>
#include <ddbdef.h>
#include <ddtdef.h>
#include <dptdef.h>
#include <fcbdef.h>
#include <fdtdef.h>
#include <fh2def.h>
#include <iosbdef.h>
#include <mscpdef.h>
#include <nisca.h>
#include <pfndef.h>
#include <rabdef.h>
#include <scsdef.h>
#include <ucbnidef.h>
#include <ttyvecdef.h>

#include <../../f11x/src/xqp.h>

#include <descrip.h>

char * do_translate(char * from);
struct _fcb * exttwo_search_fcb(struct _vcb * vcb, struct _fiddef * fid);
struct _fcb * exttwo_search_fcb2(struct _vcb * vcb, struct _fiddef * fid);
struct _fcb * e2_search_fcb(void * v);
inline unsigned long * getefcp(struct _pcb * p, unsigned long efn);
inline unsigned long * getefc(struct _pcb * p, unsigned long efn);
inline unsigned long * getefcno(unsigned long efn);
int alloc_ipid(void);
void init_p1pp_long(unsigned long addr, signed long offset, signed long val);
void init_p1pp_long_long(unsigned long addr, signed long offset, signed long long val);
void init_p1pp_data(struct _pcb * pcb, struct _phd * phd, signed long offset);
void init_sys_p1pp(void);
void init_p1pp(struct _pcb * pcb, struct _phd * phd);
int init_fork_p1pp(struct _pcb * pcb, struct _phd * phd, struct _pcb * oldpcb, struct _phd * oldphd);
int shell_init_other(struct _pcb * pcb, struct _pcb * oldpcb, long addr, long oldaddr);
int user_spaceable(void);
int user_spaceable_addr(void * addr);
int inline insrde(struct _rde * elem, struct _rde * head);
inline void *find_task_by_pid(int pid);

inline void ini_fdt_act(struct _fdt * f, unsigned long long mask, void * fn, unsigned long type);
inline void ini_fdt_end(struct _fdt * fdt);
inline void ini_dpt_name(struct _dpt * d, char * n);
inline void ini_dpt_adapt(struct _dpt * d, unsigned long type);
inline void ini_dpt_flags(struct _dpt * d, unsigned long type);
inline void ini_dpt_maxunits(struct _dpt * d, unsigned long type);
inline void ini_dpt_ucbsize(struct _dpt * d, unsigned long type);
inline void ini_dpt_struc_init(struct _dpt * d, unsigned long type);
inline void ini_dpt_struc_reinit(struct _dpt * d, unsigned long type);
inline void ini_dpt_ucb_crams(struct _dpt * d, unsigned long type);
inline void ini_dpt_defunits(struct _dpt * d, unsigned long type);
inline void ini_dpt_unload(struct _dpt * d, unsigned long type);
inline void ini_dpt_adptype(struct _dpt * d, long l);
inline void ini_dpt_vector(struct _dpt * d, long l);
inline void ini_dpt_end(struct _dpt * d);
inline void ini_ddt_ctrlinit(struct _ddt * d, unsigned long type);
inline void ini_ddt_unitinit(struct _ddt * d, unsigned long type);
inline void ini_ddt_start(struct _ddt * d, unsigned long type);
inline void ini_ddt_kp_startio(struct _ddt * d, unsigned long type);
inline void ini_ddt_kp_stack_size(struct _ddt * d, unsigned long type);
inline void ini_ddt_kp_reg_mask(struct _ddt * d, unsigned long type);
inline void ini_ddt_cancel(struct _ddt * d, unsigned long type);
inline void ini_ddt_regdmp(struct _ddt * d, unsigned long type);
inline void ini_ddt_erlgbf(struct _ddt * d, unsigned long type);
inline void ini_ddt_qsrv_helper(struct _ddt * d, unsigned long type);
inline void ini_ddt_end(struct _ddt * d);
int init_ddb(struct _ddb * ddb, struct _ddt * ddt, struct _ucb * ucb, char * sddb);
int init_ucb(struct _ucb * ucb, struct _ddb * ddb, struct _ddt * ddt, struct _crb *crb);
int init_crb(struct _crb * crb);
void insertdevlist(struct _ddb *d);
void registerdevchan(unsigned long dev, UINT16 chan);
UINT16 dev2chan(kdev_t dev);
int path_unix_to_vms(char * new, char * old);
int convert_soname(char * name);
unsigned long get_x2p_prim_fcb(void);
unsigned long get_xqp_prim_fcb(void);
int rms_kernel_read(struct file *file, unsigned long offset, char * addr, unsigned long count);
ssize_t rms_generic_file_read(struct file * filp, char * buf, size_t count, loff_t *ppos);
void xqp_init2(void);
void exttwo_init2(void * vcb);
int init_phd(struct _phd * phd);
void lnm_init_prc(struct _pcb * p);
void make_fcb(struct _fcb * inode);
struct file *rms_open_exec(const char *name);
int rms_prepare_binprm(struct linux_binprm *bprm);
struct vm_area_struct * find_vma_intersection2(struct mm_struct * mm, unsigned long start_addr, unsigned long end_addr);
int ip4_vmsinit(void);
int ide_vmsinit(void);
int ide_vmsinit(void);
int fd_vmsinit(void);
int ubd_vmsinit(void);
int ec_vmsinit(long dev);
int er_vmsinit(long dev);
int ft_vmsinit(void);
int pn_vmsinit(void);
int tz_vmsinit(void);
int mb_vmsinit(void);
int kbd_vmsinit(void);
int tty_vmsinit(void);
int con_vmsinit(void);
int kbd_vmsinit(void);
void __fl_init(void);
int is_cluster_on(void);
inline struct task_struct *find_process_by_pid(pid_t pid);
void lck$searchdlck(void);
inline void rdtscll_sync(unsigned long *tsc);
#ifdef __x86_64__
void set_rtc_mmss(unsigned long nowtime);
#else
int set_rtc_mmss(unsigned long nowtime);
#endif
void fixup_hib_pc(void * dummy);
void mycheckaddr(unsigned int ctl);
int task_on_comqueue(struct _pcb *p);
int cwps$forcex(unsigned int *pidadr, void *prcnam, unsigned int code);
void * find_a_cdt(int a);
int lck$grant_lock(struct _lkb * lck, struct _rsb * res, signed int curmode, signed int nextmode, int flags, int efn,
        signed int ggmode);
int maybe_blkast(struct _rsb * res, struct _lkb * lck);
int find_highest(struct _lkb * lck, struct _rsb * res);
int grant_queued(struct _rsb * res, int ggmode_not, int docvt, int dowt);
int inline forklock(int i, signed int j);
void inline forkunlock(int i, signed int j);
void lnm$lock(void);
void lnm$unlock(void);
void lnm$lockr(void);
void lnm$unlockr(void);
void lnm$lockw(void);
void lnm$unlockw(void);
void lnm$lock(void);
void lnm$unlock(void);
void lnm$lockr(void);
void lnm$unlockr(void);
void lnm$lockw(void);
void lnm$unlockw(void);
int lnm$check_prot(void);
int mb$chanunwait(struct _ucb * u, struct _ccb * c);
int ods2_block_read_full_page3(struct _wcb * wcb, struct page *page, unsigned long pageno);
inline int copy_fs(unsigned long clone_flags, struct task_struct * tsk);
int copy_files(unsigned long clone_flags, struct task_struct * tsk);
inline int copy_sighand(unsigned long clone_flags, struct task_struct * tsk);
int lnm$hash(const int length, const unsigned char * log, const unsigned long mask, unsigned long * myhash);
unsigned long findpte_new(struct mm_struct *mm, unsigned long address);
struct mm_struct * mm_init(struct mm_struct * mm);
inline int dup_stuff(struct mm_struct * mm, struct _phd * phd);
void kernel_puts(const char *s);
int poison_packet(char * packet, int size, int deall);
int check_packet(char * packet, int size, int deall);
int ods2_block_write_full_page3(struct _wcb * wcb, struct page *page, unsigned long pageno);
int block_prepare_write2(struct _fcb *inode, struct page *page, unsigned from, unsigned to, unsigned long pageno);
int generic_commit_write2(struct _fcb * inode, struct page *page, unsigned from, unsigned to, unsigned long pageno);
int ext2_direct_IO(int rw, struct _fcb * inode, struct kiobuf * iobuf, unsigned long blocknr, int blocksize);
int ext2_get_block(struct _vcb * vcb, struct _fcb *inode, long iblock, signed long *bh_result, int create, struct _fcb * fcb);
int block_commit_write2(struct _fcb * inode, struct page *page, unsigned from, unsigned to, unsigned long pageno);
#if 0
int ext2_sync_inode (struct _vcb * vcb, struct _fcb *inode);
#endif
void mypfncheckaddr(void);
void vms_submit_bh(int rw, struct buffer_head * bh);
signed long e2_map_vbn(struct _fcb * fcb, signed long vbn);
void myqio(long rw, long data, long size, long blocknr, kdev_t dev, long block_factor);
int block_read_full_page3(struct _fcb * fcb, struct page *page, unsigned long pageno);
int block_write_full_page3(struct _fcb * fcb, struct page *page, unsigned long pageno);
int phyio_read(unsigned handle, unsigned block, unsigned length, char *buffer);
int phyio_write(unsigned handle, unsigned block, unsigned length, char *buffer);
int du_readblk(struct _irp * i, struct _ucb * u, struct _mscp_basic_pkt * m);
int du_writeblk(struct _irp * i, struct _ucb * u, struct _mscp_basic_pkt * m);
int du_rw(struct _irp * i, struct _mscp_ucb * u, struct _transfer_commands * m);
void * find_mscp_cdt(void);
void insertfillist(struct _ucb *u, char *s);
inline BLISSIF(int i);
inline BLISSIFNOT(int i);
long CH$PTR(long);
long CH$MOVE(long, long, long);
void * scs_find_name(char * c1);
int swapbytes(long, long);
long du_iodbunit_vmsinit(struct _ddb * ddb, int unitno, void * d);
void du_iodb_clu_vmsinit(struct _ucb * u);
long du_iodb_vmsinit(void);
long ide_iodb_vmsinit(int mscp);
long ide_iodbunit_vmsinit(struct _ddb * ddb, int unitno, void * dsc);
void * find_free_cdt(void);
char * do_file_translate(char * from);
void __fl_init(void);
int phyio_init(int devlen, char *devnam, struct file **handle, struct phyio_info *info, struct _dt_ucb * ucb);
void * du_init(char *s);
struct _ucb * fl_init(char * s);
void __du_init(void);
void configure_init(void);
int scs_rcv2(char * bufh, char * buf);
int scs_from_myself(struct _nisca * msg, unsigned short sr_hi, unsigned long sr_lo);
int scs_neigh_endnode_hello(struct sk_buff *skb);
int scs_rcv(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt);
int scs_rcv2(char * bufh, char * buf);
void scs_fill_dx(struct _nisca * msg, unsigned short dx_hi, unsigned long dx_lo, unsigned short sr_hi, unsigned long sr_lo);
void scs_msg_ctl_comm(struct _cdt *sk, unsigned char msgflg);
void scs_nsp_send_conninit2(struct _cdt *sk, unsigned char msgflg, char * rprnam, char * lprnam, char * condat);
void scs_msg_ctl_fill(char * buf, struct _cdt * cdt, unsigned char msgflg);
void scs_msg_fill(char * buf, struct _cdt * cdt, unsigned char msgflg, struct _scs * newscs);
void scs_msg_fill_more(char * buf, struct _cdt * cdt, struct _cdrp * cdrp, int bufsiz);
void scs_receive(int i);
inline void scs_nsp_send2(char * buf, int len);
int scs_startdev(scs_int2, setflag, setaddr);
//scs_startdev ( long, long, long);
unsigned long inline buffer_offset(unsigned long buffer, unsigned long offset);
int rms$get_idx(struct _rabdef * rab);
int search_log_repl(char * name, char ** retname, int * retsize);
int search_ent(struct _fcb * fcb, struct dsc$descriptor * fibdsc, struct dsc$descriptor * filedsc, UINT16 *reslen,
        struct dsc$descriptor * resdsc, unsigned eofblk, unsigned action);
signed int f11b_map_vbn(unsigned int vbn, struct _wcb *wcb);
void f11b_write_attrib(struct _fcb * fcb, struct _atrdef * atrp);
signed int f11b_map_idxvbn(struct _vcb * vcb, unsigned int vbn);
void iosbret(struct _irp * i, int sts);
int add_wcb(struct _fcb * fcb, unsigned short * map);
struct _fcb * getmapfcb(struct _vcb * vcb);
void * f11b_search_fcb(struct _vcb * vcb, struct _fiddef * fid);
int writechunk(struct _fcb * fcb, unsigned long vblock, char * buff);
int mounte2(unsigned flags, unsigned devices, char *devnam[], char *label[], struct _vcb **retvcb);
int f11b_access(struct _vcb * vcb, struct _irp * irp);
int f11b_delete(struct _vcb * vcb, struct _irp * irp);
int f11b_create(struct _vcb *vcb, struct _irp * i);
int f11b_read_writevb(struct _irp * i);
int f11b_io_done(struct _irp * i);
int f11b_extend(struct _fcb *fcb, unsigned blocks, unsigned contig);
void *fcb_create2(struct _fh2 * head, unsigned *retsts);
void *f11b_read_header(struct _vcb *vcb, struct _fiddef *fid, struct _fcb * fcb, unsigned long * retsts);
void vms_ll_rw_block(int rw, int nr, struct buffer_head * bhs[], kdev_t dev);
void vms_mark_buffer_dirty(struct buffer_head * bh);
int e2_fcb_wcb_add_one(struct _fcb * fcb, signed long vbn, signed long result);
int ext2_delete_entry2(struct _vcb * vcb, struct ext2_dir_entry_2 * dir, struct page * page, struct _fcb * inode);
void *exttwo_fcb_create(struct _fcb * inode, int i_ino, unsigned *retsts);
int exttwo_read_writevb(struct _irp * i);
int exttwo_delete(struct _vcb * vcb, struct _irp * irp);
int exttwo_modify(struct _vcb * vcb, struct _irp * irp);
int exttwo_io_done(struct _irp * i);
int exttwo_create(struct _vcb *vcb, struct _irp * i);
struct _ucb * finducb(struct _fcb * fcb);
int exe$altquepkt(struct _irp * i, struct _pcb * p, struct _ucb * u);
int GET_DCL(struct _ucb * ucb);
int VERIFY_SENSE(int *outsize, int **buf, struct _irp * i, struct _pcb * p, struct _ucb * u, struct _ccb * c);
int jobctl_unsolicit(struct dsc$descriptor * term);
int tty$getnextchar(int * chr, int * CC, struct _ucb * u);
int tty$putnextchar(int * chr, int * CC, struct _ucb * u);
void con$startio(int R3, struct _ucb * u, signed int CC);

#include <linux/linkage.h>
asmlinkage int exe$qiow(unsigned int efn, UINT16 chan, unsigned int func, struct _iosb *iosb, void (*astadr)(__unknown_params),
        long astprm, void*p1, long p2, long p3, long p4, long p5, long p6);
asmlinkage int exe$qio(unsigned int efn, UINT16 chan, unsigned int func, struct _iosb *iosb, void (*astadr)(__unknown_params),
        long astprm, void*p1, long p2, long p3, long p4, long p5, long p6);
asmlinkage int cmod$astexit(void);
asmlinkage long sys_unlink(const char * pathname);
asmlinkage long sys_reboot(int magic1, int magic2, unsigned int cmd, void * arg);
int exe$waitfr(unsigned int efn);
FORKLOCK(void);
FORKUNLOCK(void);
int con$fdtwrite(struct _irp * i, struct _pcb * p, struct _ucb * u, struct _ccb * c);
void myout(int tty, int p1, int p2);
int kbd$fdtread(struct _irp * i, struct _pcb * p, struct _ucb * u, struct _ccb * c);
inline CLASS_UNIT_INIT(struct _ucb * ucb,struct _tt_port * port_vector);
kfreebuf(void * d);
int lan$setmode(struct _irp * i, struct _pcb * p, struct _ucb * u, struct _ccb * c);
int lan$sensemode(struct _irp * i, struct _pcb * p, struct _ucb * u, struct _ccb * c);
int lan$setchar(struct _irp * i, struct _pcb * p, struct _ucb * u, struct _ccb * c);
int lan$sensechar(struct _irp * i, struct _pcb * p, struct _ucb * u, struct _ccb * c);
int lan$eth_type_trans(struct _ucb * u, void * data);
int lan$netif_rx(struct _ucb * u, void * bdsc);
int lan$readblk(struct _irp * i, struct _pcb * p, struct _ucb * u, struct _ccb * c);
void * lan$alloc_cxb(int len);
void * lan$alloc_xmit_buf(struct _irp * i, struct _ucbnidef * u, char *dest);
int er_vmsinit(long dev);
int ft$fdtread(struct _irp * i, struct _pcb * p, struct _ucb * u, struct _ccb * c);
int ft$fdtwrite(struct _irp * i, struct _pcb * p, struct _ucb * u, struct _ccb * c);
inline DEVICELOCK(void);
inline DEVICEUNLOCK(void);
int SEARCHDEV(struct _ddb ** ddb, char * string, int strlen);
int SEARCHUNIT(struct _ddb * ddb, int unit, struct _ucb ** ucb);
void mscp_talk_with(char * node, char * sysap);
void sig_exit(int sig, int exit_code, struct siginfo *info);
int new_thread(int nr, unsigned long clone_flags, unsigned long rsp, unsigned long unused, struct task_struct * p,
        struct pt_regs * regs);
void set_ccb_wind(short int channel, struct _fcb * fcb);
int device_lookup(unsigned devlen, char *devnam, int create, short int *retchan);
int deaccesschunk(unsigned wrtvbn, int wrtblks, int reuse);
int mount(unsigned flags, unsigned devices, char *devnam[], char *label[], struct _vcb **vcb);
int unlock_xqp(char * prefix, struct __xqp * xqp);
int serial_file(char * prefix, struct __xqp * xqp, int fid);
int volume_lock(char * prefix, char * name);

#endif /* MISC_ROUTINES_H */
