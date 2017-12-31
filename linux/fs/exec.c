// $Id$
// $Locker$

// Author. Roar Thron�s.
// Modified Linux source file, 2001-2004

/*
 *  linux/fs/exec.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

/*
 * #!-checking implemented by tytso.
 */
/*
 * Demand-loading implemented 01.12.91 - no need to read anything but
 * the header into memory. The inode of the executable is put into
 * "current->executable", and page faults do the actual loading. Clean.
 *
 * Once more I can proudly say that linux stood up to being changed: it
 * was less than 2 hours work to get demand-loading completely implemented.
 *
 * Demand loading changed July 1993 by Eric Youngdale.   Use mmap instead,
 * current->executable is only used by the procfs.  This allows a dispatch
 * table to check for several different types  of binary formats.  We keep
 * trying until we recognize the file or we run out of supported binary
 * formats.
 */

#include <linux/config.h>
#include <linux/slab.h>
#include <linux/file.h>
#include <linux/mman.h>
#include <linux/a.out.h>
#include <linux/stat.h>
#include <linux/fcntl.h>
#include <linux/smp_lock.h>
#include <linux/init.h>
#include <linux/pagemap.h>
#include <linux/highmem.h>
#include <linux/spinlock.h>
#include <linux/personality.h>
#define __NO_VERSION__
#include <linux/module.h>

#include <asm/uaccess.h>
#include <asm/pgalloc.h>
#include <asm/mmu_context.h>

#ifdef CONFIG_KMOD
#include <linux/kmod.h>
#endif

#include<phddef.h>
#include<rdedef.h>
#include<secdef.h>
#include<sysgen.h>
#include<dyndef.h>
#include<rabdef.h>
#include<ccbdef.h>
#include<ucbdef.h>
#include <exe_routines.h>
#include <misc_routines.h>
#include <queue.h>
#include<wsldef.h>

int core_uses_pid;

static struct linux_binfmt *formats;
static rwlock_t binfmt_lock = RW_LOCK_UNLOCKED;

int register_binfmt(struct linux_binfmt * fmt)
{
    struct linux_binfmt ** tmp = &formats;

    if (!fmt)
        return -EINVAL;
    if (fmt->next)
        return -EBUSY;
    write_lock(&binfmt_lock);
    while (*tmp)
    {
        if (fmt == *tmp)
        {
            write_unlock(&binfmt_lock);
            return -EBUSY;
        }
        tmp = &(*tmp)->next;
    }
    fmt->next = formats;
    formats = fmt;
    write_unlock(&binfmt_lock);
    return 0;
}

int unregister_binfmt(struct linux_binfmt * fmt)
{
    struct linux_binfmt ** tmp = &formats;

    write_lock(&binfmt_lock);
    while (*tmp)
    {
        if (fmt == *tmp)
        {
            *tmp = fmt->next;
            write_unlock(&binfmt_lock);
            return 0;
        }
        tmp = &(*tmp)->next;
    }
    write_unlock(&binfmt_lock);
    return -EINVAL;
}

static inline void put_binfmt(struct linux_binfmt * fmt)
{
    if (fmt->module)
        __MOD_DEC_USE_COUNT(fmt->module);
}

/*
 * Note that a shared library must be both readable and executable due to
 * security reasons.
 *
 * Also note that we take the address to load from from the file itself.
 */
asmlinkage long sys_uselib(const char * library)
{
    return 0;
}

/*
 * count() counts the number of arguments/envelopes
 */
static int count(char ** argv, int max)
{
    int i = 0;

    if (argv != NULL)
    {
        for (;;)
        {
            char * p;

            if (get_user(p, argv))
                return -EFAULT;
            if (!p)
                break;
            argv++;
            if(++i > max)
                return -E2BIG;
        }
    }
    return i;
}

/*
 * 'copy_strings()' copies argument/envelope strings from user
 * memory to free pages in kernel mem. These are in a format ready
 * to be put directly into the top of new user memory.
 */
int copy_strings(int argc,char ** argv, struct linux_binprm *bprm)
{
    while (argc-- > 0)
    {
        char *str;
        int len;
        unsigned long pos;

        if (get_user(str, argv+argc) || !(len = strnlen_user(str, bprm->p)))
            return -EFAULT;
        if (bprm->p < len)
            return -E2BIG;

        bprm->p -= len;
        /* XXX: add architecture specific overflow check here. */

        pos = bprm->p;
        while (len > 0)
        {
            char *kaddr;
            int i, new, err;
            struct page *page;
            int offset, bytes_to_copy;

            offset = pos % PAGE_SIZE;
            i = pos/PAGE_SIZE;
            page = bprm->page[i];
            new = 0;
            if (!page)
            {
                page = alloc_page(GFP_HIGHUSER);
                bprm->page[i] = page;
                if (!page)
                    return -ENOMEM;
                new = 1;
            }
            kaddr = kmap(page);

            if (new && offset)
                memset(kaddr, 0, offset);
            bytes_to_copy = PAGE_SIZE - offset;
            if (bytes_to_copy > len)
            {
                bytes_to_copy = len;
                if (new)
                    memset(kaddr+offset+len, 0, PAGE_SIZE-offset-len);
            }
            err = copy_from_user(kaddr + offset, str, bytes_to_copy);
            kunmap(page);

            if (err)
                return -EFAULT;

            pos += bytes_to_copy;
            str += bytes_to_copy;
            len -= bytes_to_copy;
        }
    }
    return 0;
}

/*
 * Like copy_strings, but get argv and its values from kernel memory.
 */
int copy_strings_kernel(int argc,char ** argv, struct linux_binprm *bprm)
{
    int r;
    mm_segment_t oldfs = get_fs();
    set_fs(KERNEL_DS);
    r = copy_strings(argc, argv, bprm);
    set_fs(oldfs);
    return r;
}

/*
 * This routine is used to map in a page into an address space: needed by
 * execve() for the initial stack and environment pages.
 *
 * tsk->mmap_sem is held for writing.
 */
void put_dirty_page(struct task_struct * tsk, struct page *page, unsigned long address)
{
    pgd_t * pgd;
    pud_t * pud;
    pmd_t * pmd;
    pte_t * pte;

    if (page_count(page) != 1)
        printk(KERN_ERR "mem_map disagrees with %p at %08lx\n", page, address);
    pgd = pgd_offset(tsk->mm, address);

    spin_lock(&tsk->mm->page_table_lock);
    pud = pud_alloc(tsk->mm, pgd, address);
    if (!pud)
        goto out;
    pmd = pmd_alloc(tsk->mm, pud, address);
    if (!pmd)
        goto out;
    pte = pte_alloc(tsk->mm, pmd, address);
    if (!pte)
        goto out;
#if 0
    // check. temp fix. leaky
    if (!pte_none(*pte))
        goto out;
#endif
    flush_dcache_page(page);
    flush_page_to_ram(page);
    set_pte(pte, pte_mkdirty(pte_mkwrite(mk_pte(page, PAGE_COPY))));
    tsk->mm->rss++;
    spin_unlock(&tsk->mm->page_table_lock);

    /* no need for flush_tlb */
    return;
out:
    spin_unlock(&tsk->mm->page_table_lock);
    __free_page(page);
#if 0
    // check. related to CLI supervisor
    force_sig(SIGKILL, tsk);
#endif
    return;
}

int setup_arg_pages(struct linux_binprm *bprm)
{
    unsigned long stack_base;
    struct _rde *mpnt;
    int i;

#define STACK_TOP1 STACK_TOP
#define STACK_TOP1 0x7ffe0000

    // check. related to CLI supervisor
#ifdef __i386__
    if (strstr(bprm->filename, "dcl"))
        goto supervisor;
#endif
    stack_base = STACK_TOP1 - MAX_ARG_PAGES*PAGE_SIZE;
    goto g;
supervisor:
    stack_base = 0x7ff80000 - MAX_ARG_PAGES*PAGE_SIZE;
g:

    bprm->p += stack_base;
    if (bprm->loader)
        bprm->loader += stack_base;
    bprm->exec += stack_base;

    mpnt = kmem_cache_alloc(vm_area_cachep, SLAB_KERNEL);
    if (!mpnt)
        return -ENOMEM;

    down_write(&current->mm->mmap_sem);
    {
        mpnt->rde$pq_start_va = PAGE_MASK & (unsigned long) bprm->p;
        mpnt->rde$q_region_size = STACK_TOP1 - (unsigned long) mpnt->rde$pq_start_va;
        mpnt->rde$q_region_size = 0x1000;
        mpnt->rde$r_regprot.regprt$l_region_prot = _PAGE_PRESENT | _PAGE_USER | _PAGE_ACCESSED;//PAGE_COPY;
        mpnt->rde$l_flags = VM_STACK_FLAGS;
        insrde(mpnt,&current->pcb$l_phd->phd$ps_p0_va_list_flink);
        //flush_tlb_range(current->mm, mpnt->rde$pq_start_va, mpnt->rde$pq_start_va + PAGE_SIZE);
    }

    for (i = 0 ; i < MAX_ARG_PAGES ; i++)
    {
        struct page *page = bprm->page[i];
        if (page)
        {
            bprm->page[i] = NULL;
            put_dirty_page(current,page,stack_base);
        }
        stack_base += PAGE_SIZE;
    }
    up_write(&current->mm->mmap_sem);

    return 0;
}

struct file *rms_open_exec(const char *name)
{
    struct file *file=0;
    int err = 0;
#if 0
    struct _fabdef fab = cc$rms_fab;
#endif
    int sts;

#ifdef __i386__
    char vms_filename[256];
#else
    char * vms_filename = kmalloc(256,GFP_KERNEL);
#endif
    path_unix_to_vms(vms_filename, name);
    convert_soname(vms_filename);

#if 0
    long prev_xqp_fcb = get_xqp_prim_fcb();
    long prev_x2p_fcb = get_x2p_prim_fcb();

    fab.fab$l_fna = vms_filename;
    fab.fab$b_fns = strlen(fab.fab$l_fna);
    if ((sts = exe$open(&fab)) & 1)
    {
        long xqp_fcb = get_xqp_prim_fcb();
        long x2p_fcb = get_x2p_prim_fcb();
        extern int mount_root_ext2;
        if (mount_root_ext2==0 || xqp_fcb!=prev_xqp_fcb)
            file=xqp_fcb;
        else
            file=x2p_fcb;
        exe$close(&fab);
    }
#ifndef __i386__
    kfree(vms_filename);
#endif
    return file;
#else
    return sys_open(vms_filename,0,0);
#endif
}

int kernel_read(struct file *file, unsigned long offset,
                char * addr, unsigned long count)
{
    mm_segment_t old_fs;
    loff_t pos = offset;
    int result = -ENOSYS;

#if 0
    if (((struct _fcb *)file)->fcb$b_type==DYN$C_FCB)
#endif
#if 0
        return rms_kernel_read(file,offset,addr,count);
#else
        asmlinkage ssize_t sys_pread(unsigned int fd, char * buf, size_t count, loff_t pos);
    return sys_pread(file,addr,count,offset);
#endif
    if (!file->f_op->read)
        goto fail;
    old_fs = get_fs();
    set_fs(get_ds());
    result = file->f_op->read(file, addr, count, &pos);
    set_fs(old_fs);
fail:
    return result;
}

#if 0
int rms_kernel_read(struct file *file, unsigned long offset,
                    char * addr, unsigned long count)
{
#if 0
    mm_segment_t old_fs;
    loff_t pos = offset;
    int result = -ENOSYS;

    old_fs = get_fs();
    set_fs(get_ds());
    result = rms_generic_file_read(file, addr, count, &pos);
    set_fs(old_fs);
fail:
    return result;
#else
    struct _rabdef * rab = file;
    rab->rab$l_ubf = addr;
    rab->rab$w_usz = count;
    int block = offset >> 9;
    rab->rab$w_rfa[0] = block & 0xffff;
    rab->rab$w_rfa[1] = block >> 16;
    rab->rab$w_rfa[2] = offset;
    int sts = exe$get(rab);
    return rab->rab$w_rsz;
#endif
}
#endif

static int exec_mmap(void)
{
    struct mm_struct * mm, * old_mm;

    old_mm = current->mm;
    if (old_mm && atomic_read(&old_mm->mm_users) == 1)
    {
        mm_release();
        exit_mmap(old_mm);
        return 0;
    }

    mm = mm_alloc();
    if (mm)
    {
        struct mm_struct *active_mm;

        if (init_new_context(current, mm))
        {
            mmdrop(mm);
            return -ENOMEM;
        }

        /* Add it to the list of mm's */
        spin_lock(&mmlist_lock);
        list_add(&mm->mmlist, &init_mm.mmlist);
        mmlist_nr++;
        spin_unlock(&mmlist_lock);

        task_lock(current);
        active_mm = current->active_mm;
        current->mm = mm;
        current->active_mm = mm;
        task_unlock(current);
        activate_mm(active_mm, mm);
        mm_release();

        if (old_mm)
        {
            if (active_mm != old_mm) BUG();
            mmput(old_mm);
            return 0;
        }
        mmdrop(active_mm);
        return 0;
    }
    return -ENOMEM;
}

/*
 * This function makes sure the current process has its own signal table,
 * so that flush_signal_handlers can later reset the handlers without
 * disturbing other processes.  (Other processes might share the signal
 * table via the CLONE_SIGNAL option to clone().)
 */

static inline int make_private_signals(void)
{
    struct signal_struct * newsig;

    if (atomic_read(&current->sig->count) <= 1)
        return 0;
    newsig = kmem_cache_alloc(sigact_cachep, GFP_KERNEL);
    if (newsig == NULL)
        return -ENOMEM;
    spin_lock_init(&newsig->siglock);
    atomic_set(&newsig->count, 1);
    memcpy(newsig->action, current->sig->action, sizeof(newsig->action));
    spin_lock_irq(&current->sigmask_lock);
    current->sig = newsig;
    spin_unlock_irq(&current->sigmask_lock);
    return 0;
}

/*
 * If make_private_signals() made a copy of the signal table, decrement the
 * refcount of the original table, and free it if necessary.
 * We don't do that in make_private_signals() so that we can back off
 * in flush_old_exec() if an error occurs after calling make_private_signals().
 */

static inline void release_old_signals(struct signal_struct * oldsig)
{
    if (current->sig == oldsig)
        return;
    if (atomic_dec_and_test(&oldsig->count))
        kmem_cache_free(sigact_cachep, oldsig);
}

/*
 * These functions flushes out all traces of the currently running executable
 * so that a new one can be started
 */

static inline void flush_old_files(struct files_struct * files)
{
    long j = -1;

    write_lock(&files->file_lock);
    for (;;)
    {
        unsigned long set, i;

        j++;
        i = j * __NFDBITS;
        if (i >= files->max_fds || i >= files->max_fdset)
            break;
        set = files->close_on_exec->fds_bits[j];
        if (!set)
            continue;
        files->close_on_exec->fds_bits[j] = 0;
        write_unlock(&files->file_lock);
        for ( ; set ; i++,set >>= 1)
        {
            if (set & 1)
            {
                sys_close(i);
            }
        }
        write_lock(&files->file_lock);

    }
    write_unlock(&files->file_lock);
}

#if 0
/*
 * An execve() will automatically "de-thread" the process.
 * Note: we don't have to hold the tasklist_lock to test
 * whether we migth need to do this. If we're not part of
 * a thread group, there is no way we can become one
 * dynamically. And if we are, we only need to protect the
 * unlink - even if we race with the last other thread exit,
 * at worst the list_del_init() might end up being a no-op.
 */
static inline void de_thread(struct task_struct *tsk)
{
    if (!list_empty(&tsk->thread_group))
    {
        write_lock_irq(&tasklist_lock);
        list_del_init(&tsk->thread_group);
        write_unlock_irq(&tasklist_lock);
    }

    /* Minor oddity: this might stay the same. */
    tsk->tgid = tsk->pcb$l_pid;
}
#endif

int flush_old_exec(struct linux_binprm * bprm)
{
    char * name;
    int i, ch, retval;
    struct signal_struct * oldsig;

    /*
     * Make sure we have a private signal table
     */
    oldsig = current->sig;
    retval = make_private_signals();
    if (retval) goto flush_failed;

    /*
     * Release all of the old mmap stuff
     */
    retval = exec_mmap();
    if (retval) goto mmap_failed;

    current->pcb$l_phd=kmalloc(sizeof(struct _phd),GFP_KERNEL);
    init_phd(current->pcb$l_phd);
    init_p1pp(current,current->pcb$l_phd);
    lnm_init_prc(current);

    /* This is the point of no return */
    release_old_signals(oldsig);

    current->sas_ss_sp = current->sas_ss_size = 0;

    if (current->euid == current->uid && current->egid == current->gid)
        current->mm->dumpable = 1;
    name = bprm->filename;
    if (current->pcb$l_pqb) goto skip_name;  // if creprc was done
    for (i=0; (ch = *(name++)) != '\0';)
    {
        if (ch == '/')
            i = 0;
        else if (i < 15)
            current->pcb$t_lname[i++] = ch;
    }
    current->pcb$t_lname[i] = '\0';
skip_name:

    flush_thread();

#if 0
    de_thread(current);
#endif

#if 0
    if (((struct _fcb *)(bprm->file))->fcb$b_type!=DYN$C_FCB)
#endif
        if (bprm->e_uid != current->euid || bprm->e_gid != current->egid ||
                1)
            current->mm->dumpable = 0;

    /* An exec changes our domain. We are no longer part of the thread
       group */

    current->self_exec_id++;

    flush_signal_handlers(current);
    flush_old_files(current->files);

    return 0;

mmap_failed:
flush_failed:
    spin_lock_irq(&current->sigmask_lock);
    if (current->sig != oldsig)
    {
        kmem_cache_free(sigact_cachep, current->sig);
        current->sig = oldsig;
    }
    spin_unlock_irq(&current->sigmask_lock);
    return retval;
}

/*
 * We mustn't allow tracing of suid binaries, unless
 * the tracer has the capability to trace anything..
 */
static inline int must_not_trace_exec(struct task_struct * p)
{
    return (p->ptrace & PT_PTRACED) && !(p->ptrace & PT_PTRACE_CAP);
}

/*
 * Fill the binprm structure from the inode.
 * Check permissions, then read the first 128 (BINPRM_BUF_SIZE) bytes
 */
int prepare_binprm(struct linux_binprm *bprm)
{
    int mode;
    struct inode * inode = bprm->file->f_dentry->d_inode;

    mode = inode->i_mode;
    /*
     * Check execute perms again - if the caller has CAP_DAC_OVERRIDE,
     * vfs_permission lets a non-executable through
     */
    if (!(mode & 0111))	/* with at least _one_ execute bit set */
        return -EACCES;
    if (bprm->file->f_op == NULL)
        return -EACCES;

    bprm->e_uid = current->euid;
    bprm->e_gid = current->egid;

    if(!(bprm->file->f_vfsmnt->mnt_flags & MNT_NOSUID))
    {
        /* Set-uid? */
        if (mode & S_ISUID)
            bprm->e_uid = inode->i_uid;

        /* Set-gid? */
        /*
         * If setgid is set but no group execute bit then this
         * is a candidate for mandatory locking, not a setgid
         * executable.
         */
        if ((mode & (S_ISGID | S_IXGRP)) == (S_ISGID | S_IXGRP))
            bprm->e_gid = inode->i_gid;
    }

    /* We don't have VFS support for capabilities yet */
    cap_clear(bprm->cap_inheritable);
    cap_clear(bprm->cap_permitted);
    cap_clear(bprm->cap_effective);

    /*  To support inheritance of root-permissions and suid-root
         *  executables under compatibility mode, we raise all three
         *  capability sets for the file.
         *
         *  If only the real uid is 0, we only raise the inheritable
         *  and permitted sets of the executable file.
         */

    if (!issecure(SECURE_NOROOT))
    {
        if (bprm->e_uid == 0 || current->uid == 0)
        {
            cap_set_full(bprm->cap_inheritable);
            cap_set_full(bprm->cap_permitted);
        }
        if (bprm->e_uid == 0)
            cap_set_full(bprm->cap_effective);
    }

    memset(bprm->buf,0,BINPRM_BUF_SIZE);
    return kernel_read(bprm->file,0,bprm->buf,BINPRM_BUF_SIZE);
}

int rms_prepare_binprm(struct linux_binprm *bprm)
{
    int mode;
    struct _fcb * fcb = bprm->file;
#if 0
    struct inode * inode = fcb->fcb$l_primfcb;

    if (fcb->fcb$l_fill_5)
        mode = inode->i_mode;
    else
#endif
        mode = 0x755;
    /*
     * Check execute perms again - if the caller has CAP_DAC_OVERRIDE,
     * vfs_permission lets a non-executable through
     */
    if (!(mode & 0111))	/* with at least _one_ execute bit set */
        return -EACCES;

    bprm->e_uid = current->euid;
    bprm->e_gid = current->egid;

    // no nosuid mount check
    //if(!(bprm->file->f_vfsmnt->mnt_flags & MNT_NOSUID)) {}
    /* Set-uid? */
#if 0
    if (mode & S_ISUID)
        bprm->e_uid = inode->i_uid;
#endif

    /* Set-gid? */
    /*
     * If setgid is set but no group execute bit then this
     * is a candidate for mandatory locking, not a setgid
     * executable.
     */
#if 0
    if ((mode & (S_ISGID | S_IXGRP)) == (S_ISGID | S_IXGRP))
        bprm->e_gid = inode->i_gid;
#endif

    /* We don't have VFS support for capabilities yet */
    cap_clear(bprm->cap_inheritable);
    cap_clear(bprm->cap_permitted);
    cap_clear(bprm->cap_effective);

    /*  To support inheritance of root-permissions and suid-root
         *  executables under compatibility mode, we raise all three
         *  capability sets for the file.
         *
         *  If only the real uid is 0, we only raise the inheritable
         *  and permitted sets of the executable file.
         */

    if (!issecure(SECURE_NOROOT))
    {
        if (bprm->e_uid == 0 || current->uid == 0)
        {
            cap_set_full(bprm->cap_inheritable);
            cap_set_full(bprm->cap_permitted);
        }
        if (bprm->e_uid == 0)
            cap_set_full(bprm->cap_effective);
    }

    memset(bprm->buf,0,BINPRM_BUF_SIZE);
    return kernel_read(bprm->file,0,bprm->buf,BINPRM_BUF_SIZE);
}

/*
 * This function is used to produce the new IDs and capabilities
 * from the old ones and the file's capabilities.
 *
 * The formula used for evolving capabilities is:
 *
 *       pI' = pI
 * (***) pP' = (fP & X) | (fI & pI)
 *       pE' = pP' & fE          [NB. fE is 0 or ~0]
 *
 * I=Inheritable, P=Permitted, E=Effective // p=process, f=file
 * ' indicates post-exec(), and X is the global 'cap_bset'.
 *
 */

void compute_creds(struct linux_binprm *bprm)
{
    kernel_cap_t new_permitted, working;
    int do_unlock = 0;

    new_permitted = cap_intersect(bprm->cap_permitted, cap_bset);
    working = cap_intersect(bprm->cap_inheritable,
                            current->cap_inheritable);
    new_permitted = cap_combine(new_permitted, working);

    if (bprm->e_uid != current->uid || bprm->e_gid != current->gid ||
            !cap_issubset(new_permitted, current->cap_permitted))
    {
        current->mm->dumpable = 0;

        lock_kernel();
        if (must_not_trace_exec(current)
                || atomic_read(&current->fs->count) > 1
                || atomic_read(&current->files->count) > 1
                || atomic_read(&current->sig->count) > 1)
        {
            if(!capable(CAP_SETUID))
            {
                bprm->e_uid = current->uid;
                bprm->e_gid = current->gid;
            }
            if(!capable(CAP_SETPCAP))
            {
                new_permitted = cap_intersect(new_permitted,
                                              current->cap_permitted);
            }
        }
        do_unlock = 1;
    }


    /* For init, we want to retain the capabilities set
         * in the init_task struct. Thus we skip the usual
         * capability rules */
    if (current->pcb$l_pid != INIT_PID)
    {
        current->cap_permitted = new_permitted;
        current->cap_effective =
            cap_intersect(new_permitted, bprm->cap_effective);
    }

    /* AUD: Audit candidate if current->cap_effective is set */

    current->suid = current->euid = current->fsuid = bprm->e_uid;
    current->sgid = current->egid = current->fsgid = bprm->e_gid;

    if(do_unlock)
        unlock_kernel();
    current->keep_capabilities = 0;
}


void remove_arg_zero(struct linux_binprm *bprm)
{
    if (bprm->argc)
    {
        unsigned long offset;
        char * kaddr;
        struct page *page;

        offset = bprm->p % PAGE_SIZE;
        goto inside;

        while (bprm->p++, *(kaddr+offset++))
        {
            if (offset != PAGE_SIZE)
                continue;
            offset = 0;
            kunmap(page);
inside:
            page = bprm->page[bprm->p/PAGE_SIZE];
            kaddr = kmap(page);
        }
        kunmap(page);
        bprm->argc--;
    }
}

/*
 * cycle the list of binary formats handler, until one recognizes the image
 */
int search_binary_handler(struct linux_binprm *bprm,struct pt_regs *regs)
{
    int try,retval=0;
    struct linux_binfmt *fmt;
#ifdef __alpha__
    /* handle /sbin/loader.. */
    {
        struct exec * eh = (struct exec *) bprm->buf;

        if (!bprm->loader && eh->fh.f_magic == 0x183 &&
                (eh->fh.f_flags & 0x3000) == 0x3000)
        {
            struct file * file;
            unsigned long loader;

            allow_write_access(bprm->file);
            fput(bprm->file);
            bprm->file = NULL;

            loader = PAGE_SIZE*MAX_ARG_PAGES-sizeof(void *);

            file = open_exec("/sbin/loader");
            retval = PTR_ERR(file);
            if (IS_ERR(file))
                return retval;

            /* Remember if the application is TASO.  */
            bprm->sh_bang = eh->ah.entry < 0x100000000;

            bprm->file = file;
            bprm->loader = loader;
            retval = prepare_binprm(bprm);
            if (retval<0)
                return retval;
            /* should call search_binary_handler recursively here,
               but it does not matter */
        }
    }
#endif
    /* kernel module loader fixup */
    /* so we don't try to load run modprobe in kernel space. */
    set_fs(USER_DS);
    for (try=0; try<2; try++)
                {
                    read_lock(&binfmt_lock);
                    for (fmt = formats ; fmt ; fmt = fmt->next)
                    {
                        int (*fn)(struct linux_binprm *, struct pt_regs *) = fmt->load_binary;
                        if (!fn)
                            continue;
                        if (!try_inc_mod_count(fmt->module))
                            continue;
                        read_unlock(&binfmt_lock);
                        retval = fn(bprm, regs);
                        if (retval >= 0)
                        {
                            put_binfmt(fmt);
#if 0
                            if (((struct _fcb *)(bprm->file))->fcb$b_type!=DYN$C_FCB)
                            {
#endif
                                allow_write_access(bprm->file);
                                if (bprm->file)
                                    fput(bprm->file);
#if 0
                            }
#endif
                            bprm->file = NULL;
                            current->did_exec = 1;
                            return retval;
                        }
                        read_lock(&binfmt_lock);
                        put_binfmt(fmt);
                        if (retval != -ENOEXEC)
                            break;
                        if (!bprm->file)
                        {
                            read_unlock(&binfmt_lock);
                            return retval;
                        }
                    }
                    read_unlock(&binfmt_lock);
                    if (retval != -ENOEXEC)
                    {
                        break;
#ifdef CONFIG_KMOD
                    }
                    else
                    {
#define printable(c) (((c)=='\t') || ((c)=='\n') || (0x20<=(c) && (c)<=0x7e))
                        char modname[20];
                        if (printable(bprm->buf[0]) &&
                                printable(bprm->buf[1]) &&
                                printable(bprm->buf[2]) &&
                                printable(bprm->buf[3]))
                            break; /* -ENOEXEC */
                        sprintf(modname, "binfmt-%04x", *(unsigned short *)(&bprm->buf[2]));
                        request_module(modname);
#endif
                    }
                }
    return retval;
}

extern int mydebug;

/*
 * sys_execve() executes a new program.
 */
int do_execve(char * filename, char ** argv, char ** envp, struct pt_regs * regs)
{
    struct linux_binprm bprm;
    struct file *file=0;
    int retval=0;
    int i;

    if (ctl$gl_pcb->pcb$t_terminal[0]==0)
    {
        memcpy(&ctl$gl_pcb->pcb$t_terminal,"opa0",4);
    }
    else
    {
#if 0
        short int chan;
        int sts = ptd$create(&chan,0,0,0,0,0,0,0);
        memcpy(&ctl$gl_pcb->pcb$t_terminal,"opa",3);
        struct _ccb * c=&ctl$gl_ccbbase[chan];
        ctl$gl_pcb->pcb$t_terminal[3]=48+(char)c->ccb$l_ucb->ucb$w_unit;
#endif
    }

    //printk("execve %s\n",filename);
    //	mydebug=1;

    //printk("before lnm_init_prc\n");
#if 0
    init_p1pp(current,current->pcb$l_phd);
    lnm_init_prc(current);
#endif
    //printk("after lnm_init_prc\n");

    xqp_init2();
    extern void * global_e2_vcb;
    exttwo_init2(global_e2_vcb);

    file = rms_open_exec(filename);
    if (((long)file) >= 0) goto fcb_found;

    retval = PTR_ERR(file);
    //printk("here 5\n");
    if (IS_ERR(file))
        return retval;

fcb_found:
    bprm.p = PAGE_SIZE*MAX_ARG_PAGES-sizeof(void *);
    memset(bprm.page, 0, MAX_ARG_PAGES*sizeof(bprm.page[0]));

    bprm.file = file;
    bprm.filename = filename;
    bprm.sh_bang = 0;
    bprm.loader = 0;
    bprm.exec = 0;
    if ((bprm.argc = count(argv, bprm.p / sizeof(void *))) < 0)
    {
        printk("uh oh, argv error\n");
        allow_write_access(file);
        fput(file);
        //printk("here 7 %x\n",bprm.argc);
        return bprm.argc;
    }

    if ((bprm.envc = count(envp, bprm.p / sizeof(void *))) < 0)
    {
        printk("uh oh, envp error\n");
        allow_write_access(file);
        fput(file);
        //printk("here 6\n");
        return bprm.envc;
    }

#if 0
    if (((struct _fcb *)file)->fcb$b_type==DYN$C_FCB)
#endif
        retval = rms_prepare_binprm(&bprm);
    //printk("here 4\n");
    if (retval < 0)
        goto out;

    retval = copy_strings_kernel(1, &bprm.filename, &bprm);
    //printk("here 3\n");
    if (retval < 0)
        goto out;

    bprm.exec = bprm.p;
    retval = copy_strings(bprm.envc, envp, &bprm);
    //printk("here 2\n");
    if (retval < 0)
        goto out;

    retval = copy_strings(bprm.argc, argv, &bprm);
    //printk("here 1\n");
    if (retval < 0)
        goto out;

    retval = search_binary_handler(&bprm,regs);
    if (retval >= 0)
        /* execve success */
        return retval;

out:
    /* Something went wrong, return the inode and free the argument pages*/
    printk("uh oh, something went wrong error\n");
#if 0
    if (((struct _fcb *)file)->fcb$b_type!=DYN$C_FCB)
    {
#else
    if (0)
    {
#endif
        allow_write_access(bprm.file);
        if (bprm.file)
            fput(bprm.file);
    }

    for (i = 0 ; i < MAX_ARG_PAGES ; i++)
    {
        struct page * page = bprm.page[i];
        if (page)
            __free_page(page);
    }

    return retval;
}

void set_binfmt(struct linux_binfmt *new)
{
    struct linux_binfmt *old = current->binfmt;
    if (new && new->module)
        __MOD_INC_USE_COUNT(new->module);
    current->binfmt = new;
    if (old && old->module)
        __MOD_DEC_USE_COUNT(old->module);
}

int do_coredump(long signr, struct pt_regs * regs)
{
    return -EPERM;
}

init_phd(struct _phd * phd)
{
    memset(phd,0,sizeof(struct _phd));
    // p->pcb$l_phd->phd$q_ptbr=p->mm->pgd; // wait a bit or move it?
    {
        qhead_init(&phd->phd$ps_p0_va_list_flink);
        phd->phd$l_wslist=kmalloc(PHD_INT_SIZE*512,GFP_KERNEL);
        phd->phd$l_wslock=kmalloc(PHD_INT_SIZE*512,GFP_KERNEL);
        phd->phd$l_wsdyn=kmalloc(PHD_INT_SIZE*512,GFP_KERNEL);
        memset((void*)phd->phd$l_wslist,0,PHD_INT_SIZE*512);
        memset((void*)phd->phd$l_wslock,0,PHD_INT_SIZE*512);
        memset((void*)phd->phd$l_wsdyn,0,PHD_INT_SIZE*512);
        phd->phd$l_wsquota=PQL_DWSQUOTA;
        phd->phd$l_wsauthext=PQL_DWSEXTENT;
        phd->phd$l_wsextent=PQL_DWSEXTENT;
        phd->phd$l_wsauth=PQL_DWSQUOTA;
        phd->phd$l_wssize=WSMAX;
        phd->phd$l_wsnext=0;
        phd->phd$l_wslast=511;
#if 0
        {
            struct _wsl * wsl = phd->phd$l_wslist;
            int i;
            for(i=0; i<512; i++)
                wsl[i].wsl$v_valid=1;
        }
#else
        // mark the end of the array somehow?
        struct _wsl * wsl = phd->phd$l_wslist;
        wsl[phd->phd$l_wslast].wsl$pq_va = WSL$C_UNKNOWN;
#endif
        phd->phd$l_pst_base_offset=kmalloc(PROCSECTCNT*sizeof(struct _secdef),GFP_KERNEL);
        memset((void*)phd->phd$l_pst_base_offset,0,PROCSECTCNT*sizeof(struct _secdef));
        phd->phd$l_pst_last=PROCSECTCNT-1;
        phd->phd$l_pst_free=0;
    }
}
