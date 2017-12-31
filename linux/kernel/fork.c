// $Id$
// $Locker$

// Author. Roar Thron�s.
// Modified Linux source file, 2001-2004

/*
 *  linux/kernel/fork.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

/*
 *  'fork.c' contains the help-routines for the 'fork' system call
 * (see also entry.S and others).
 * Fork is rather simple, once you get the hang of it, but the memory
 * management can be a bitch. See 'mm/memory.c': 'copy_page_range()'
 */

#include <linux/config.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/unistd.h>
#include <linux/smp_lock.h>
#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/completion.h>
#include <linux/personality.h>

#include <asm/pgtable.h>
#include <asm/pgalloc.h>
#include <asm/uaccess.h>
#include <asm/mmu_context.h>

#include <sysgen.h>
#include <phddef.h>
#include <pridef.h>
#include <rdedef.h>
#include <secdef.h>
#include <statedef.h>
#include <evtdef.h>
#include <system_data_cells.h>
#include <misc_routines.h>
#include <exe_routines.h>
#include <sch_routines.h>
#include <queue.h>
#include <internals.h>
#include <ipldef.h>
#include <vfddef.h>
#include <wsldef.h>

/* The idle threads do not count.. */
int nr_threads;
int nr_running;

int max_threads;
unsigned long total_forks;	/* Handle normal Linux uptimes. */
int last_pid;

void fastcall add_wait_queue(wait_queue_head_t *q, wait_queue_t * wait)
{
    unsigned long flags;

    wait->flags &= ~WQ_FLAG_EXCLUSIVE;
    wq_write_lock_irqsave(&q->lock, flags);
    __add_wait_queue(q, wait);
    wq_write_unlock_irqrestore(&q->lock, flags);
}

void fastcall add_wait_queue_exclusive(wait_queue_head_t *q, wait_queue_t * wait)
{
    unsigned long flags;

    wait->flags |= WQ_FLAG_EXCLUSIVE;
    wq_write_lock_irqsave(&q->lock, flags);
    __add_wait_queue_tail(q, wait);
    wq_write_unlock_irqrestore(&q->lock, flags);
}

void fastcall remove_wait_queue(wait_queue_head_t *q, wait_queue_t * wait)
{
    unsigned long flags;

    wq_write_lock_irqsave(&q->lock, flags);
    __remove_wait_queue(q, wait);
    wq_write_unlock_irqrestore(&q->lock, flags);
}

void __init fork_init(unsigned long mempages)
{
    /*
     * The default maximum number of threads is set to a safe
     * value: the thread structures can take up at most half
     * of memory.
     */
    max_threads = mempages / (THREAD_SIZE/PAGE_SIZE) / 8;

    init_task.rlim[RLIMIT_NPROC].rlim_cur = max_threads/2;
    init_task.rlim[RLIMIT_NPROC].rlim_max = max_threads/2;
}

/* Protects next_safe and last_pid. */
spinlock_t lastpid_lock = SPIN_LOCK_UNLOCKED;

/*static*/
inline int dup_mmap(struct mm_struct * mm)
{
    struct vm_area_struct * mpnt, *tmp, **pprev;
    int retval;

    flush_cache_mm(current->mm);
    mm->locked_vm = 0;
    mm->mmap = NULL;
    mm->mmap_cache = NULL;
    mm->map_count = 0;
    mm->rss = 0;
    mm->cpu_vm_mask = 0;
    mm->swap_address = 0;
    pprev = &mm->mmap;

    /*
     * Add it to the mmlist after the parent.
     * Doing it this way means that we can order the list,
     * and fork() won't mess up the ordering significantly.
     * Add it first so that swapoff can see any swap entries.
     */
    spin_lock(&mmlist_lock);
#if 0
    list_add(&mm->mmlist, &current->mm->mmlist);
#endif
    mmlist_nr++;
    spin_unlock(&mmlist_lock);

    retval = 0;

fail_nomem:
    flush_tlb_mm(current->mm);
    return retval;
}

static inline int dup_phd(struct _pcb * p, struct _pcb * old)
{
    p->pcb$l_phd=kmalloc(sizeof(struct _phd),GFP_KERNEL);
    if (old->pcb$l_phd)
        memcpy(p->pcb$l_phd,old->pcb$l_phd,sizeof(struct _phd));
    else
        memset(p->pcb$l_phd,0,sizeof(struct _phd));
    qhead_init(&p->pcb$l_phd->phd$ps_p0_va_list_flink);
    if (old->pcb$l_phd)
    {
        if (old->pcb$l_phd->phd$l_wslist)
        {
            p->pcb$l_phd->phd$l_wslist=kmalloc(PHD_INT_SIZE*512,GFP_KERNEL);
            memcpy(((void*)p->pcb$l_phd->phd$l_wslist),old->pcb$l_phd->phd$l_wslist,PHD_INT_SIZE*512);
            // mark the end of the array somehow?
            struct _wsl * wsl = p->pcb$l_phd->phd$l_wslist;
            wsl[p->pcb$l_phd->phd$l_wslast].wsl$pq_va = WSL$C_UNKNOWN;
        }
        if (old->pcb$l_phd->phd$l_wslock)
        {
            p->pcb$l_phd->phd$l_wslock=kmalloc(PHD_INT_SIZE*512,GFP_KERNEL);
            memcpy(((void*)p->pcb$l_phd->phd$l_wslock),old->pcb$l_phd->phd$l_wslock,PHD_INT_SIZE*512);
        }
        if (old->pcb$l_phd->phd$l_wsdyn)
        {
            p->pcb$l_phd->phd$l_wsdyn=kmalloc(PHD_INT_SIZE*512,GFP_KERNEL);
            memcpy(((void*)p->pcb$l_phd->phd$l_wsdyn),old->pcb$l_phd->phd$l_wsdyn,PHD_INT_SIZE*512);
        }
        //p->pcb$l_phd->phd$l_wsnext=0;
        //p->pcb$l_phd->phd$l_wslast=511;
        if (old->pcb$l_phd->phd$l_pst_base_offset)
        {
            p->pcb$l_phd->phd$l_pst_base_offset=kmalloc(PROCSECTCNT*sizeof(struct _secdef),GFP_KERNEL);
            memcpy(p->pcb$l_phd->phd$l_pst_base_offset,old->pcb$l_phd->phd$l_pst_base_offset,PROCSECTCNT*sizeof(struct _secdef));
        }
    }
}

inline int dup_stuff(struct mm_struct * mm, struct _phd * phd)
{
    struct _rde * mpnt, *tmp, **pprev;
    int retval;

    pprev = &phd->phd$pq_p0_start_va;

    /*
     * Add it to the mmlist after the parent.
     * Doing it this way means that we can order the list,
     * and fork() won't mess up the ordering significantly.
     * Add it first so that swapoff can see any swap entries.
     */

    for (mpnt = current->pcb$l_phd->phd$ps_p0_va_list_flink ; mpnt!=&current->pcb$l_phd->phd$ps_p0_va_list_flink ; mpnt = mpnt->rde$ps_va_list_flink)
    {
        struct file *file;

        retval = -ENOMEM;
        if(mpnt->rde$l_flags & VM_DONTCOPY)
            continue;
        tmp = kmem_cache_alloc(vm_area_cachep, SLAB_KERNEL);
        if (!tmp)
            goto fail_nomem;
        memcpy(tmp,mpnt,sizeof(struct _rde));
        tmp->rde$l_flags &= ~VM_LOCKED;

        /*
         * Link in the new vma and copy the page table entries:
         * link in first so that swapoff can see swap entries.
         */
        pprev = &tmp->rde$ps_va_list_flink;
        spin_lock(&mm->page_table_lock);
        retval = copy_page_range(mm, current->mm, tmp);
        spin_unlock(&mm->page_table_lock);

        if (retval)
            goto fail_nomem;
        insrde(tmp,&phd->phd$ps_p0_va_list_flink);
    }
    retval = 0;

fail_nomem:
    flush_tlb_mm(current->mm);
    return retval;
}

spinlock_t mmlist_lock __cacheline_aligned = SPIN_LOCK_UNLOCKED;
int mmlist_nr;

#define allocate_mm()	(kmem_cache_alloc(mm_cachep, SLAB_KERNEL))
#define free_mm(mm)	(kmem_cache_free(mm_cachep, (mm)))

/*static*/ struct mm_struct * mm_init(struct mm_struct * mm)
{
    atomic_set(&mm->mm_users, 1);
    atomic_set(&mm->mm_count, 1);
    init_rwsem(&mm->mmap_sem);
    mm->page_table_lock = SPIN_LOCK_UNLOCKED;
    mm->pgd = pgd_alloc(mm);
    mm->def_flags = 0;
    if (mm->pgd)
        return mm;
    free_mm(mm);
    return NULL;
}


/*
 * Allocate and initialize an mm_struct.
 */
struct mm_struct * mm_alloc(void)
{
    struct mm_struct * mm;

    mm = allocate_mm();
    if (mm)
    {
        memset(mm, 0, sizeof(*mm));
        return mm_init(mm);
    }
    return NULL;
}

/*
 * Called when the last reference to the mm
 * is dropped: either by a lazy thread or by
 * mmput. Free the page directory and the mm.
 */
inline void fastcall __mmdrop(struct mm_struct *mm)
{
    if (mm == &init_mm) BUG();
    pgd_free(mm->pgd);
    destroy_context(mm);
    free_mm(mm);
}

/*
 * Decrement the use count and release all resources for an mm.
 */
void mmput(struct mm_struct *mm)
{
    if (atomic_dec_and_lock(&mm->mm_users, &mmlist_lock))
    {
        extern struct mm_struct *swap_mm;
        if (swap_mm == mm)
            swap_mm = list_entry(mm->mmlist.next, struct mm_struct, mmlist);
#if 0
        list_del(&mm->mmlist);
#endif
        mmlist_nr--;
        spin_unlock(&mmlist_lock);
        exit_mmap(mm);
        mmdrop(mm);
    }
}

/* Please note the differences between mmput and mm_release.
 * mmput is called whenever we stop holding onto a mm_struct,
 * error success whatever.
 *
 * mm_release is called after a mm_struct has been removed
 * from the current process.
 *
 * This difference is important for error handling, when we
 * only half set up a mm_struct for a new process and need to restore
 * the old one.  Because we mmput the new mm_struct before
 * restoring the old one. . .
 * Eric Biederman 10 January 1998
 */
void mm_release(void)
{
    struct task_struct *tsk = current;
    struct completion *vfork_done = tsk->vfork_done;

    /* notify parent sleeping on vfork() */
    if (vfork_done)
    {
        tsk->vfork_done = NULL;
        complete(vfork_done);
    }
}

extern int mydebug6;
extern int mydebug4;

static int copy_mm(unsigned long clone_flags, struct task_struct * tsk)
{
    struct mm_struct * mm, *oldmm;
    int retval;

    tsk->min_flt = tsk->maj_flt = 0;
    tsk->cmin_flt = tsk->cmaj_flt = 0;
    tsk->nswap = tsk->cnswap = 0;

    if (mydebug4)
    {
        printk("copy_mm %x %x %x %x\n",tsk,tsk->pcb$l_pid,tsk->mm,tsk->active_mm);
        {
            int j;
            for(j=0; j<1000000000; j++) ;
        }
    }
    tsk->mm = NULL;
    tsk->active_mm = NULL;

    /*
     * Are we cloning a kernel thread?
     *
     * We need to steal a active VM for that..
     */
    oldmm = current->mm;
    if (!oldmm)
        return 0;

    goto dont;
    if (clone_flags & CLONE_VM)
    {
        atomic_inc(&oldmm->mm_users);
        mm = oldmm;
        goto good_mm;
    }
dont:
    retval = -ENOMEM;
    mm = allocate_mm();
    if (!mm)
        goto fail_nomem;

    /* Copy the current MM stuff.. */
    memcpy(mm, oldmm, sizeof(*mm));
    if (!mm_init(mm))
        goto fail_nomem;

    down_write(&oldmm->mmap_sem);
    retval = dup_mmap(mm);
    retval = dup_stuff(mm,tsk->pcb$l_phd);
    up_write(&oldmm->mmap_sem);

    if (retval)
        goto free_pt;

    /*
     * child gets a private LDT (if there was an LDT in the parent)
     */
    copy_segments(tsk, mm);

    if (init_new_context(tsk,mm))
        goto free_pt;

good_mm:
    tsk->mm = mm;
    tsk->active_mm = mm;
    return 0;

free_pt:
    mmput(mm);
fail_nomem:
    return retval;
}

/*static*/ inline struct fs_struct *__copy_fs_struct(struct fs_struct *old)
{
    struct fs_struct *fs = kmem_cache_alloc(fs_cachep, GFP_KERNEL);
    /* We don't need to lock fs - think why ;-) */
    if (fs)
    {
        atomic_set(&fs->count, 1);
        fs->lock = RW_LOCK_UNLOCKED;
        fs->umask = old->umask;
        read_lock(&old->lock);
        fs->rootmnt = mntget(old->rootmnt);
        fs->root = dget(old->root);
        fs->pwdmnt = mntget(old->pwdmnt);
        fs->pwd = dget(old->pwd);
        if (old->altroot)
        {
            fs->altrootmnt = mntget(old->altrootmnt);
            fs->altroot = dget(old->altroot);
        }
        else
        {
            fs->altrootmnt = NULL;
            fs->altroot = NULL;
        }
        read_unlock(&old->lock);
    }
    return fs;
}

struct fs_struct *copy_fs_struct(struct fs_struct *old)
{
    return __copy_fs_struct(old);
}

/*static*/ inline int copy_fs(unsigned long clone_flags, struct task_struct * tsk)
{
    if (clone_flags & CLONE_FS)
    {
        atomic_inc(&current->fs->count);
        return 0;
    }
    tsk->fs = __copy_fs_struct(current->fs);
    if (!tsk->fs)
        return -1;
    return 0;
}

static int count_open_files(struct files_struct *files, int size)
{
    int i;

    /* Find the last open fd */
    for (i = size/(8*sizeof(long)); i > 0; )
    {
        if (files->open_fds->fds_bits[--i])
            break;
    }
    i = (i+1) * 8 * sizeof(long);
    return i;
}

/*static*/ int copy_files(unsigned long clone_flags, struct task_struct * tsk)
{
    struct files_struct *oldf, *newf;
    struct file **old_fds, **new_fds;
    int open_files, nfds, size, i, error = 0;

    /*
     * A background process may not have any files ...
     */
    oldf = current->files;
    if (!oldf)
        goto out;

    if (clone_flags & CLONE_FILES)
    {
        atomic_inc(&oldf->count);
        goto out;
    }

    tsk->files = NULL;
    error = -ENOMEM;
    newf = kmem_cache_alloc(files_cachep, SLAB_KERNEL);
    if (!newf)
        goto out;

    atomic_set(&newf->count, 1);

    newf->file_lock	    = RW_LOCK_UNLOCKED;
    newf->next_fd	    = 0;
    newf->max_fds	    = NR_OPEN_DEFAULT;
    newf->max_fdset	    = __FD_SETSIZE;
    newf->close_on_exec = &newf->close_on_exec_init;
    newf->open_fds	    = &newf->open_fds_init;
    newf->fd	    = &newf->fd_array[0];

    /* We don't yet have the oldf readlock, but even if the old
           fdset gets grown now, we'll only copy up to "size" fds */
    size = oldf->max_fdset;
    if (size > __FD_SETSIZE)
    {
        newf->max_fdset = 0;
        write_lock(&newf->file_lock);
        error = expand_fdset(newf, size-1);
        write_unlock(&newf->file_lock);
        if (error)
            goto out_release;
    }
    read_lock(&oldf->file_lock);

    open_files = count_open_files(oldf, size);

    /*
     * Check whether we need to allocate a larger fd array.
     * Note: we're not a clone task, so the open count won't
     * change.
     */
    nfds = NR_OPEN_DEFAULT;
    if (open_files > nfds)
    {
        read_unlock(&oldf->file_lock);
        newf->max_fds = 0;
        write_lock(&newf->file_lock);
        error = expand_fd_array(newf, open_files-1);
        write_unlock(&newf->file_lock);
        if (error)
            goto out_release;
        nfds = newf->max_fds;
        read_lock(&oldf->file_lock);
    }

    old_fds = oldf->fd;
    new_fds = newf->fd;

    memcpy(newf->open_fds->fds_bits, oldf->open_fds->fds_bits, open_files/8);
    memcpy(newf->close_on_exec->fds_bits, oldf->close_on_exec->fds_bits, open_files/8);

    for (i = open_files; i != 0; i--)
    {
        struct file *f = *old_fds++;
        if (f)
            get_file(f);
        *new_fds++ = f;
        struct vms_fd * vms_fd = f;
        if (vms_fd)
            vms_fd->vfd$l_refcnt++;
    }
    read_unlock(&oldf->file_lock);

    /* compute the remainder to be cleared */
    size = (newf->max_fds - open_files) * sizeof(struct file *);

    /* This is long word aligned thus could use a optimized version */
    memset(new_fds, 0, size);

    if (newf->max_fdset > open_files)
    {
        int left = (newf->max_fdset-open_files)/8;
        int start = open_files / (8 * sizeof(unsigned long));

        memset(&newf->open_fds->fds_bits[start], 0, left);
        memset(&newf->close_on_exec->fds_bits[start], 0, left);
    }

    tsk->files = newf;
    error = 0;
out:
    return error;

out_release:
    free_fdset (newf->close_on_exec, newf->max_fdset);
    free_fdset (newf->open_fds, newf->max_fdset);
    kmem_cache_free(files_cachep, newf);
    goto out;
}

/*static*/ inline int copy_sighand(unsigned long clone_flags, struct task_struct * tsk)
{
    struct signal_struct *sig;

    if (clone_flags & CLONE_SIGHAND)
    {
        atomic_inc(&current->sig->count);
        return 0;
    }
    sig = kmem_cache_alloc(sigact_cachep, GFP_KERNEL);
    tsk->sig = sig;
    if (!sig)
        return -1;
    spin_lock_init(&sig->siglock);
    atomic_set(&sig->count, 1);
    memcpy(tsk->sig->action, current->sig->action, sizeof(tsk->sig->action));
    return 0;
}

static inline void copy_flags(unsigned long clone_flags, struct task_struct *p)
{
    unsigned long new_flags = p->flags;

    new_flags &= ~(PF_SUPERPRIV | PF_USEDFPU);
    new_flags |= PF_FORKNOEXEC;
    if (!(clone_flags & CLONE_PTRACE))
        p->ptrace = 0;
    p->flags = new_flags;
}

#ifdef __x86_64__
long kernel_thread(int (*fn)(void *), void * arg, unsigned long flags)
{
    struct task_struct *task = current;
    unsigned old_task_dumpable;
    long ret;

    /* lock out any potential ptracer */
    task_lock(task);
    if (task->ptrace)
    {
        task_unlock(task);
        return -EPERM;
    }

#if 0
    // not yet?
    old_task_dumpable = task->task_dumpable;
    task->task_dumpable = 0;
#endif
    task_unlock(task);

    ret = arch_kernel_thread(fn, arg, flags);

    /* never reached in child process, only in parent */
#if 0
    current->task_dumpable = old_task_dumpable;
#endif

    return ret;
}
#endif

/*
 *  Ok, this is the main fork-routine. It copies the system process
 * information (task[nr]) and sets up the necessary registers. It also
 * copies the data segment in its entirety.  The "stack_start" and
 * "stack_top" arguments are simply passed along to the platform
 * specific copy_thread() routine.  Most platforms ignore stack_top.
 * For an example that's using stack_top, see
 * arch/ia64/kernel/process.c.
 */
int do_fork(unsigned long clone_flags, unsigned long stack_start,
            struct pt_regs *regs, unsigned long stack_size)
{
    int retval;
    struct task_struct *p;
    struct completion vfork;

    retval = -EPERM;
    //	printk("do_fork\n");
    /*
     * CLONE_PID is only allowed for the initial SMP swapper
     * calls
     */
    if (clone_flags & CLONE_PID)
    {
        if (current->pcb$l_pid)
            goto fork_out;
    }

    retval = -ENOMEM;
    vmslock(&SPIN_SCHED, IPL$_SCHED);
    vmslock(&SPIN_MMG, IPL$_MMG);
    p = alloc_task_struct();
    if (!p)
        goto fork_out;

    *p = *current;

    retval = -EAGAIN;
    /*
     * Check if we are over our maximum process limit, but be sure to
     * exclude root. This is needed to make it possible for login and
     * friends to set the per-user process limit to something lower
     * than the amount of processes root is running. -- Rik
     */
    if (atomic_read(&p->user->processes) >= p->rlim[RLIMIT_NPROC].rlim_cur
            && !capable(CAP_SYS_ADMIN) && !capable(CAP_SYS_RESOURCE))
        goto bad_fork_free;

    atomic_inc(&p->user->__count);
    atomic_inc(&p->user->processes);

    /*
     * Counter increases are protected by
     * the kernel lock so nr_threads can't
     * increase under us (but it may decrease).
     */
    if (nr_threads >= max_threads)
        goto bad_fork_cleanup_count;

    get_exec_domain(p->exec_domain);

    if (p->binfmt && p->binfmt->module)
        __MOD_INC_USE_COUNT(p->binfmt->module);

    p->did_exec = 0;
    p->swappable = 0;
    p->state = TASK_UNINTERRUPTIBLE;

    copy_flags(clone_flags, p);
    p->pcb$l_pid=alloc_ipid();
    {
        unsigned long *vec=sch$gl_pcbvec;
        vec[p->pcb$l_pid&0xffff]=p;
    }
    p->pcb$l_epid=exe$ipid_to_epid(p->pcb$l_pid);

    p->run_list.next = NULL;
    p->run_list.prev = NULL;

    p->p_cptr = NULL;
    init_waitqueue_head(&p->wait_chldexit);
    p->vfork_done = NULL;
    if (clone_flags & CLONE_VFORK)
    {
        p->vfork_done = &vfork;
        init_completion(&vfork);
    }
    spin_lock_init(&p->alloc_lock);

    p->sigpending = 0;
    init_sigpending(&p->pending);

    p->it_real_value = p->it_virt_value = p->it_prof_value = 0;
    p->it_real_incr = p->it_virt_incr = p->it_prof_incr = 0;
    init_timer(&p->real_timer);
    p->real_timer.data = (unsigned long) p;

    p->leader = 0;		/* session leadership doesn't inherit */
    p->tty_old_pgrp = 0;
    p->times.tms_utime = p->times.tms_stime = 0;
    p->times.tms_cutime = p->times.tms_cstime = 0;
    {
        int i;
        p->cpus_runnable = ~0UL;
        p->pcb$l_cpu_id = current->pcb$l_cpu_id;
        /* ?? should we just memset this ?? */
        for(i = 0; i < smp_num_cpus; i++)
            p->per_cpu_utime[i] = p->per_cpu_stime[i] = 0;
        spin_lock_init(&p->sigmask_lock);
    }
    p->lock_depth = -1;		/* -1 = no lock */
    p->start_time = jiffies;

    INIT_LIST_HEAD(&p->local_pages);

#if 0
    p->pcb$l_phd=kmalloc(sizeof(struct _phd),GFP_KERNEL);
    if (current->pcb$l_phd)
        memcpy(p->pcb$l_phd,current->pcb$l_phd,sizeof(struct _phd));
    else
        memset(p->pcb$l_phd,0,sizeof(struct _phd));

    // p->pcb$l_phd->phd$q_ptbr=p->mm->pgd; // wait a bit or move it?
    {
        qhead_init(&p->pcb$l_phd->phd$ps_p0_va_list_flink);
        p->pcb$l_phd->phd$l_wslist=kmalloc(PHD_INT_SIZE*512,GFP_KERNEL);
        p->pcb$l_phd->phd$l_wslock=kmalloc(PHD_INT_SIZE*512,GFP_KERNEL);
        p->pcb$l_phd->phd$l_wsdyn=kmalloc(PHD_INT_SIZE*512,GFP_KERNEL);
        memset((void*)p->pcb$l_phd->phd$l_wslist,0,PHD_INT_SIZE*512);
        memset((void*)p->pcb$l_phd->phd$l_wslock,0,PHD_INT_SIZE*512);
        memset((void*)p->pcb$l_phd->phd$l_wsdyn,0,PHD_INT_SIZE*512);
        //p->pcb$l_phd->phd$l_wsnext=0;
        //p->pcb$l_phd->phd$l_wslast=511;
        p->pcb$l_phd->phd$l_pst_base_offset=kmalloc(PROCSECTCNT*sizeof(struct _secdef),GFP_KERNEL);
        memset((void*)p->pcb$l_phd->phd$l_pst_base_offset,0,PROCSECTCNT*sizeof(struct _secdef));
        //p->pcb$l_phd->phd$l_pst_last=PROCSECTCNT-1;
        //p->pcb$l_phd->phd$l_pst_free=0;
    }
#endif

    retval = -ENOMEM;
    /* copy all the process information */
    if (copy_files(clone_flags, p))
        goto bad_fork_cleanup;
    if (copy_fs(clone_flags, p))
        goto bad_fork_cleanup_files;
    if (copy_sighand(clone_flags, p))
        goto bad_fork_cleanup_fs;
    dup_phd(p,current);
    if (copy_mm(clone_flags, p))
        goto bad_fork_cleanup_sighand;
    if (clone_flags & 0x00010000)
        p->active_mm = p->mm = mm_alloc();
    int uml_map = init_fork_p1pp(p,p->pcb$l_phd,current,current->pcb$l_phd);
#ifdef __x86_64__
    shell_init_other(p,ctl$gl_pcb,0x7ff80000-0x1000,0x7fffe000);
    shell_init_other(p,ctl$gl_pcb,0x7ff80000-0x2000,0x7fffe000);
    shell_init_other(p,ctl$gl_pcb,0x7ff90000-0x1000,0x7fffe000);
    shell_init_other(p,ctl$gl_pcb,0x7ff90000-0x2000,0x7fffe000);
    shell_init_other(p,ctl$gl_pcb,0x7ffa0000-0x1000,0x7fffe000);
    shell_init_other(p,ctl$gl_pcb,0x7ffa0000-0x2000,0x7fffe000);
#else
    shell_init_other(p,ctl$gl_pcb,0x7ff80000-0x1000,0x7fffe000);
    shell_init_other(p,ctl$gl_pcb,0x7ff80000-0x2000,0x7fffe000);
    shell_init_other(p,ctl$gl_pcb,0x7ff90000-0x1000,0x7fffe000);
    shell_init_other(p,ctl$gl_pcb,0x7ff90000-0x2000,0x7fffe000);
#endif
    //printk("phd %x %x %x\n",tsk,tsk->pcb$l_pid,tsk->pcb$l_phd);
    //printk("phd %x %x %x\n",current,current->pcb$l_pid,current->pcb$l_phd);
    retval = copy_thread(0, clone_flags, stack_start, stack_size, p, regs);
    if (retval)
        goto bad_fork_cleanup_mm;
    p->semundo = NULL;

    /* Our parent execution domain becomes current domain
       These must match for thread signalling to apply */

    p->parent_exec_id = p->self_exec_id;

    /* ok, now we should be set up.. */
    p->swappable = 1;
    p->exit_signal = clone_flags & CSIGNAL;
    p->pdeath_signal = 0;


    /* pcb stuff */

    p->pcb$b_prib=31-DEFPRI;
    p->pcb$b_pri=31-DEFPRI-6;
    //	if (p->pcb$b_pri<16) p->pcb$b_pri=16;
    p->pcb$w_quant=-QUANTUM;

    qhead_init(&p->pcb$l_lockqfl);

    p->pcb$l_permanent_capability = sch$gl_default_process_cap;
    p->pcb$l_capability = p->pcb$l_permanent_capability;

    qhead_init(&p->pcb$l_astqfl);
    p->pcb$b_asten=15;
    p->phd$b_astlvl=4;
    p->pr_astlvl=4;

    p->psl=0;
    //	p->pslindex=0;

    /*
     * "share" dynamic priority between parent and child, thus the
     * total amount of dynamic priorities in the system doesnt change,
     * more scheduling fairness. This is only important in the first
     * timeslice, on the long run the scheduling behaviour is unchanged.
     */

    /*
     * Ok, add it to the run-queues and make it
     * visible to the rest of the system.
     *
     * Let it rip!
     */
    retval = p->pcb$l_epid;
    INIT_LIST_HEAD(&p->thread_group);

    /* Need tasklist lock for parent etc handling! */
    write_lock_irq(&tasklist_lock);

    /* CLONE_PARENT and CLONE_THREAD re-use the old parent */
    p->p_opptr = current->p_opptr;
    p->p_pptr = current->p_pptr;
    if (!(clone_flags & (CLONE_PARENT | CLONE_THREAD)))
    {
        p->p_opptr = current;
        if (!(p->ptrace & PT_PTRACED))
            p->p_pptr = current;
    }

    if (clone_flags & CLONE_THREAD)
    {
#if 0
        p->tgid = current->tgid;
#endif
        list_add(&p->thread_group, &current->thread_group);
    }

    SET_LINKS(p);
    nr_threads++;
    write_unlock_irq(&tasklist_lock);

    if (p->ptrace & PT_PTRACED)
        send_sig(SIGSTOP, p, 1);
    //printk("fork befwak\n");
    p->state = TASK_INTERRUPTIBLE;
    p->pcb$w_state = SCH$C_HIB;
    ((struct _wqh *)sch$gq_mwait)->wqh$l_wqcnt++; // temp fix
    qhead_init(p); // move this up?
    //	vmslock(&SPIN_SCHED,IPL$_SCHED);
    sch$rse(p,PRI$_RESAVL,EVT$_WAKE); // check if this is right
    vmsunlock(&SPIN_MMG,-1);
    vmsunlock(&SPIN_SCHED,0);
    //	wake_up_process2(p,PRI$_TICOM);		/* do this last */
    ++total_forks;
    if (clone_flags & CLONE_VFORK)
        wait_for_completion(&vfork);

fork_out:
    return retval;

bad_fork_cleanup_mm:
    exit_mm(p);
bad_fork_cleanup_sighand:
    exit_sighand(p);
bad_fork_cleanup_fs:
    exit_fs(p); /* blocking */
bad_fork_cleanup_files:
    exit_files(p); /* blocking */
bad_fork_cleanup:
    put_exec_domain(p->exec_domain);
    if (p->binfmt && p->binfmt->module)
        __MOD_DEC_USE_COUNT(p->binfmt->module);
bad_fork_cleanup_count:
    atomic_dec(&p->user->processes);
    free_uid(p->user);
bad_fork_free:
    free_task_struct(p);
    goto fork_out;
}

/* SLAB cache for signal_struct structures (tsk->sig) */
kmem_cache_t *sigact_cachep;

/* SLAB cache for files_struct structures (tsk->files) */
kmem_cache_t *files_cachep;

/* SLAB cache for fs_struct structures (tsk->fs) */
kmem_cache_t *fs_cachep;

/* SLAB cache for vm_area_struct structures */
kmem_cache_t *vm_area_cachep;

/* SLAB cache for mm_struct structures (tsk->mm) */
kmem_cache_t *mm_cachep;

void __init proc_caches_init(void)
{
    sigact_cachep = kmem_cache_create("signal_act",
                                      sizeof(struct signal_struct), 0,
                                      SLAB_HWCACHE_ALIGN, NULL, NULL);
    if (!sigact_cachep)
        panic("Cannot create signal action SLAB cache");

    files_cachep = kmem_cache_create("files_cache",
                                     sizeof(struct files_struct), 0,
                                     SLAB_HWCACHE_ALIGN, NULL, NULL);
    if (!files_cachep)
        panic("Cannot create files SLAB cache");

    fs_cachep = kmem_cache_create("fs_cache",
                                  sizeof(struct fs_struct), 0,
                                  SLAB_HWCACHE_ALIGN, NULL, NULL);
    if (!fs_cachep)
        panic("Cannot create fs_struct SLAB cache");

    vm_area_cachep = kmem_cache_create("vm_area_struct",
                                       sizeof(struct vm_area_struct), 0,
                                       SLAB_HWCACHE_ALIGN, NULL, NULL);
    if(!vm_area_cachep)
        panic("vma_init: Cannot alloc vm_area_struct SLAB cache");

    mm_cachep = kmem_cache_create("mm_struct",
                                  sizeof(struct mm_struct), 0,
                                  SLAB_HWCACHE_ALIGN, NULL, NULL);
    if(!mm_cachep)
        panic("vma_init: Cannot alloc mm_struct SLAB cache");
}
