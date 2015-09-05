// $Id$
// $Locker$

// Author. Roar Thron�s.
// Modified Linux source file, 2001-2004. Based on memory.c.

/*
 *  linux/mm/memory.c
 *
 *  Copyright (C) 1991, 1992, 1993, 1994  Linus Torvalds
 */

/*
 * demand-loading started 01.12.91 - seems it is high on the list of
 * things wanted, and it should be easy to implement. - Linus
 */

/*
 * Ok, demand-loading was easy, shared pages a little bit tricker. Shared
 * pages started 02.12.91, seems to work. - Linus.
 *
 * Tested sharing by executing about 30 /bin/sh: under the old kernel it
 * would have taken more than the 6M I have free, but it worked well as
 * far as I could see.
 *
 * Also corrected some "invalidate()"s - I wasn't doing enough of them.
 */

/*
 * Real VM (paging to/from disk) started 18.12.91. Much more work and
 * thought has to go into this. Oh, well..
 * 19.12.91  -  works, somewhat. Sometimes I get faults, don't know why.
 *      Found it. Everything seems to work now.
 * 20.12.91  -  Ok, making the swap-device changeable like the root.
 */

/*
 * 05.04.94  -  Multi-page memory management added for v1.1.
 *      Idea by Alex Bligh (alex@cconcepts.co.uk)
 *
 * 16.07.99  -  Support of BIGMEM added by Gerhard Wichert, Siemens AG
 *      (Gerhard.Wichert@pdb.siemens.de)
 */

#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/smp_lock.h>
#include <linux/iobuf.h>
#include <linux/highmem.h>
#include <linux/pagemap.h>

#include <asm/pgalloc.h>
#include <asm/uaccess.h>
#include <asm/tlb.h>
#include <asm/pgtable.h>

#include <ipldef.h>
#include <phddef.h>
#include <rdedef.h>
#include <va_rangedef.h>
#include <vmspte.h>
#include <wsldef.h>
#include <exe_routines.h>
#include <misc_routines.h>
#include <mmg_routines.h>
#include <fcbdef.h>

pgprot_t x_to_prot(int x)
{
    pgprot_t y;
    y.pgprot=x;
    return y;
}

unsigned long max_mapnr;
unsigned long num_physpages;
void * high_memory;
struct page *highmem_start_page;

struct vm_area_struct * find_vma2(struct mm_struct * mm, unsigned long addr)
{
    panic("findvma\n");
}

/*
 * We special-case the C-O-W ZERO_PAGE, because it's such
 * a common occurrence (no need to read the page to know
 * that it's zero - better for the cache and memory subsystem).
 */
static inline void copy_cow_page(struct page * from, struct page * to, unsigned long address)
{
    if (from == ZERO_PAGE(address))
    {
        clear_user_highpage(to, address);
        return;
    }
    copy_user_highpage(to, from, address);
}

mem_map_t * mem_map;

/*
 * If a p?d_bad entry is found while walking page tables, report
 * the error, before resetting entry to p?d_none.  Usually (but
 * very seldom) called out from the p?d_none_or_clear_bad macros.
 */

void pgd_clear_bad(pgd_t *pgd)
{
    pgd_ERROR(*pgd);
    pgd_clear(pgd);
}

void pud_clear_bad(pud_t *pud)
{
    pud_ERROR(*pud);
    pud_clear(pud);
}

void pmd_clear_bad(pmd_t *pmd)
{
    pmd_ERROR(*pmd);
    pmd_clear(pmd);
}

#if 0
/*
 * Note: this doesn't free the actual pages themselves. That
 * has been handled earlier when unmapping all the memory regions.
 */
static void free_pte_range(struct mmu_gather *tlb, pmd_t *pmd)
{
    struct page *page = pmd_page(*pmd);
    pmd_clear(pmd);
    pte_lock_deinit(page);
    pte_free_tlb(tlb, page);
    dec_page_state(nr_page_table_pages);
    tlb->mm->nr_ptes--;
}

static inline void free_pmd_range(struct mmu_gather *tlb, pud_t *pud,
                                  unsigned long addr, unsigned long end,
                                  unsigned long floor, unsigned long ceiling)
{
    pmd_t *pmd;
    unsigned long next;
    unsigned long start;

    start = addr;
    pmd = pmd_offset(pud, addr);
    do
    {
        next = pmd_addr_end(addr, end);
        if (pmd_none_or_clear_bad(pmd))
            continue;
        free_pte_range(tlb, pmd);
    }
    while (pmd++, addr = next, addr != end);

    start &= PUD_MASK;
    if (start < floor)
        return;
    if (ceiling)
    {
        ceiling &= PUD_MASK;
        if (!ceiling)
            return;
    }
    if (end - 1 > ceiling - 1)
        return;

    pmd = pmd_offset(pud, start);
    pud_clear(pud);
    pmd_free_tlb(tlb, pmd);
}

static inline void free_pud_range(struct mmu_gather *tlb, pgd_t *pgd,
                                  unsigned long addr, unsigned long end,
                                  unsigned long floor, unsigned long ceiling)
{
    pud_t *pud;
    unsigned long next;
    unsigned long start;

    start = addr;
    pud = pud_offset(pgd, addr);
    do
    {
        next = pud_addr_end(addr, end);
        if (pud_none_or_clear_bad(pud))
            continue;
        free_pmd_range(tlb, pud, addr, next, floor, ceiling);
    }
    while (pud++, addr = next, addr != end);

    start &= PGDIR_MASK;
    if (start < floor)
        return;
    if (ceiling)
    {
        ceiling &= PGDIR_MASK;
        if (!ceiling)
            return;
    }
    if (end - 1 > ceiling - 1)
        return;

    pud = pud_offset(pgd, start);
    pgd_clear(pgd);
    pud_free_tlb(tlb, pud);
}

/*
 * This function frees user-level page tables of a process.
 *
 * Must be called with pagetable lock held.
 */
void free_pgd_range(struct mmu_gather **tlb,
                    unsigned long addr, unsigned long end,
                    unsigned long floor, unsigned long ceiling)
{
    pgd_t *pgd;
    unsigned long next;
    unsigned long start;

    /*
     * The next few lines have given us lots of grief...
     *
     * Why are we testing PMD* at this top level?  Because often
     * there will be no work to do at all, and we'd prefer not to
     * go all the way down to the bottom just to discover that.
     *
     * Why all these "- 1"s?  Because 0 represents both the bottom
     * of the address space and the top of it (using -1 for the
     * top wouldn't help much: the masks would do the wrong thing).
     * The rule is that addr 0 and floor 0 refer to the bottom of
     * the address space, but end 0 and ceiling 0 refer to the top
     * Comparisons need to use "end - 1" and "ceiling - 1" (though
     * that end 0 case should be mythical).
     *
     * Wherever addr is brought up or ceiling brought down, we must
     * be careful to reject "the opposite 0" before it confuses the
     * subsequent tests.  But what about where end is brought down
     * by PMD_SIZE below? no, end can't go down to 0 there.
     *
     * Whereas we round start (addr) and ceiling down, by different
     * masks at different levels, in order to test whether a table
     * now has no other vmas using it, so can be freed, we don't
     * bother to round floor or end up - the tests don't need that.
     */

    addr &= PMD_MASK;
    if (addr < floor)
    {
        addr += PMD_SIZE;
        if (!addr)
            return;
    }
    if (ceiling)
    {
        ceiling &= PMD_MASK;
        if (!ceiling)
            return;
    }
    if (end - 1 > ceiling - 1)
        end -= PMD_SIZE;
    if (addr > end - 1)
        return;

    start = addr;
    pgd = pgd_offset((*tlb)->mm, addr);
    do
    {
        next = pgd_addr_end(addr, end);
        if (pgd_none_or_clear_bad(pgd))
            continue;
        free_pud_range(*tlb, pgd, addr, next, floor, ceiling);
    }
    while (pgd++, addr = next, addr != end);

    if (!(*tlb)->fullmm)
        flush_tlb_pgtables((*tlb)->mm, start, end);
}

void free_pgtables(struct mmu_gather **tlb, struct vm_area_struct *vma,
                   unsigned long floor, unsigned long ceiling)
{
    while (vma)
    {
        struct vm_area_struct *next = vma->rde$ps_va_list_flink;
        unsigned long addr = vma->rde$pq_start_va;

        /*
         * Hide vma from rmap and vmtruncate before freeing pgtables
         */
        anon_vma_unlink(vma);
        unlink_file_vma(vma);

        if (is_hugepage_only_range(vma->vm_mm, addr, HPAGE_SIZE))
        {
            hugetlb_free_pgd_range(tlb, addr, vma->rde$pq_start_va + vma->rde$q_region_size,
                                   floor, next? next->rde$pq_start_va: ceiling);
        }
        else
        {
            /*
             * Optimization: gather nearby vmas into one call down
             */
            while (next && next->rde$pq_start_va <= (vma->rde$pq_start_va + vma->rde$q_region_size) + PMD_SIZE
                    && !is_hugepage_only_range(vma->vm_mm, next->rde$pq_start_va,
                                               HPAGE_SIZE))
            {
                vma = next;
                next = vma->rde$ps_va_list_flink;
                anon_vma_unlink(vma);
                unlink_file_vma(vma);
            }
            free_pgd_range(tlb, addr, vma->rde$pq_start_va + vma->rde$q_region_size,
                           floor, next? next->rde$pq_start_va: ceiling);
        }
        vma = next;
    }
}
#endif

int __pte_alloc(struct mm_struct *mm, pmd_t *pmd, unsigned long address)
{
    struct page *new = pte_alloc_one(mm, address);
    if (!new)
        return -ENOMEM;

    pte_lock_init(new);
    spin_lock(&mm->page_table_lock);
    if (pmd_present(*pmd))      /* Another has populated it */
    {
        pte_lock_deinit(new);
        pte_free(new);
    }
    else
    {
#if 0
        mm->nr_ptes++;
        inc_page_state(nr_page_table_pages);
#endif
        pmd_populate(mm, pmd, new);
    }
    spin_unlock(&mm->page_table_lock);
    return 0;
}

int __pte_alloc_kernel(pmd_t *pmd, unsigned long address)
{
#ifdef __x86_64__
    pte_t *new = pte_alloc_one_kernel(&init_mm, address);
#else
    pte_t *new = pte_alloc_one(&init_mm, address);
#endif
    if (!new)
        return -ENOMEM;

    spin_lock(&init_mm.page_table_lock);
    if (pmd_present(*pmd))      /* Another has populated it */
        pte_free_kernel(new);
    else
        pmd_populate_kernel(&init_mm, pmd, new);
    spin_unlock(&init_mm.page_table_lock);
    return 0;
}

/*
 * Called by TLB shootdown
 */
void __free_pte(pte_t pte)
{
    struct page *page = pte_page(pte);
    if ((!VALID_PAGE(page)) || PageReserved(page))
        return;
    void fastcall set_page_dirty(struct page *page);
    if (pte_dirty(pte))
        set_page_dirty(page);
    //  free_page_and_swap_cache(page);
}


/*
 * Note: this doesn't free the actual pages themselves. That
 * has been handled earlier when unmapping all the memory regions.
 */
static inline void free_one_pmd(pmd_t * dir)
{
    pte_t * pte;

    if (pmd_none(*dir))
        return;
    if (pmd_bad(*dir))
    {
        pmd_ERROR(*dir);
        pmd_clear(dir);
        return;
    }
    pte = pte_offset(dir, 0);
    pmd_clear(dir);
    pte_free(pte);
}

static inline void free_one_pud(pud_t * dir)
{
    pmd_t * pmd;

    if (pud_none(*dir))
        return;
    if (pud_bad(*dir))
    {
        pud_ERROR(*dir);
        pud_clear(dir);
        return;
    }
    pmd = pmd_offset(dir, 0);
    pud_clear(dir);
    pmd_free(pmd);
}

static inline void free_one_pgd(pgd_t * dir)
{
    int j;
    pud_t * pud;

    if (pgd_none(*dir))
        return;
    if (pgd_bad(*dir))
    {
        pgd_ERROR(*dir);
        pgd_clear(dir);
        return;
    }
    pud = pud_offset(dir, 0);
    pgd_clear(dir);
    for (j = 0; j < PTRS_PER_PUD ; j++)
    {
        prefetchw(pud+j+(PREFETCH_STRIDE/16));
        free_one_pud(pud+j);
    }
    pud_free(pud);
}

/* Low and high watermarks for page table cache.
   The system should try to have pgt_water[0] <= cache elements <= pgt_water[1]
 */
int pgt_cache_water[2] = { 25, 50 };

/* Returns the number of pages freed */
int check_pgt_cache(void)
{
    return do_check_pgt_cache(pgt_cache_water[0], pgt_cache_water[1]);
}


/*
 * This function clears all user-level page tables of a process - this
 * is needed by execve(), so that old pages aren't in the way.
 */
void clear_page_tables(struct mm_struct *mm, unsigned long first, int nr)
{
    pgd_t * page_dir = mm->pgd;

    spin_lock(&mm->page_table_lock);
    page_dir += first;
    do
    {
        free_one_pgd(page_dir);
        page_dir++;
    }
    while (--nr);
    spin_unlock(&mm->page_table_lock);

    /* keep the page table cache within bounds */
    check_pgt_cache();
}

#define PTE_TABLE_MASK  ((PTRS_PER_PTE-1) * sizeof(pte_t))
#define PMD_TABLE_MASK  ((PTRS_PER_PMD-1) * sizeof(pmd_t))
#define PUD_TABLE_MASK  ((PTRS_PER_PUD-1) * sizeof(pud_t))

static inline int is_cow_mapping(unsigned int flags)
{
    return (flags & (VM_SHARED | VM_MAYWRITE)) == VM_MAYWRITE;
}

/*
 * This function gets the "struct page" associated with a pte.
 *
 * NOTE! Some mappings do not have "struct pages". A raw PFN mapping
 * will have each page table entry just pointing to a raw page frame
 * number, and as far as the VM layer is concerned, those do not have
 * pages associated with them - even if the PFN might point to memory
 * that otherwise is perfectly fine and has a "struct page".
 *
 * The way we recognize those mappings is through the rules set up
 * by "remap_pfn_range()": the vma will have the VM_PFNMAP bit set,
 * and the vm_pgoff will point to the first PFN mapped: thus every
 * page that is a raw mapping will always honor the rule
 *
 *  pfn_of_page == vma->vm_pgoff + ((addr - vma->rde$pq_start_va) >> PAGE_SHIFT)
 *
 * and if that isn't true, the page has been COW'ed (in which case it
 * _does_ have a "struct page" associated with it even if it is in a
 * VM_PFNMAP range).
 */
struct page *vm_normal_page(struct vm_area_struct *vma, unsigned long addr, pte_t pte)
{
    unsigned long pfn = pte_pfn(pte);

    if (vma->rde$l_flags & VM_PFNMAP)
    {
        unsigned long off = (addr - ((long)vma->rde$pq_start_va)) >> PAGE_SHIFT;
        if (pfn == 0 /*vma->vm_pgoff*/ + off) // check
            return NULL;
        if (!is_cow_mapping(vma->rde$l_flags))
            return NULL;
    }

    /*
     * Add some anal sanity checks for now. Eventually,
     * we should just do "return pfn_to_page(pfn)", but
     * in the meantime we check that we get a valid pfn,
     * and that the resulting page looks ok.
     *
     * Remove this test eventually!
     */
#if 0
    if (unlikely(!pfn_valid(pfn)))
    {
        print_bad_pte(vma, pte, addr);
        return NULL;
    }
#endif

    /*
     * NOTE! We still have PageReserved() pages in the page
     * tables.
     *
     * The PAGE_ZERO() pages and various VDSO mappings can
     * cause them to exist.
     */
    return pfn_to_page(pfn);
}

/*
 * copy one vm_area from one task to the other. Assumes the page tables
 * already present in the new task to be cleared in the whole range
 * covered by this vma.
 */

static inline void
copy_one_pte(struct mm_struct *dst_mm, struct mm_struct *src_mm,
             pte_t *dst_pte, pte_t *src_pte, struct vm_area_struct *vma,
             unsigned long addr, int *rss)
{
    unsigned long vm_flags = ((struct _rde *)vma)->rde$l_flags;
    pte_t pte = *src_pte;
    struct page *page;
    unsigned long did_cow=0;
    struct page *ptepage = pte_page(pte);
    unsigned long address = addr;

    /* pte contains position in swap or file, so copy. */
    if (unlikely(!pte_present(pte)))
    {
        if (!pte_file(pte))
        {
#if 0
            swap_duplicate(pte_to_swp_entry(pte));
#endif
            /* make sure dst_mm is on swapoff's mmlist. */
            if (unlikely(list_empty(&dst_mm->mmlist)))
            {
                spin_lock(&mmlist_lock);
                if (list_empty(&dst_mm->mmlist))
                    list_add(&dst_mm->mmlist,
                             &src_mm->mmlist);
                spin_unlock(&mmlist_lock);
            }
        }
        goto out_set_pte;
    }

    /*
     * If it's a COW mapping, write protect it both
     * in the parent and the child
     */
    if (is_cow_mapping(vm_flags))
    {
        ptep_set_wrprotect(src_mm, addr, src_pte);
        pte = *src_pte;
        did_cow = 1;
    }

    /*
     * If it's a shared mapping, mark it clean in
     * the child
     */
    if (vm_flags & VM_SHARED)
        pte = pte_mkclean(pte);
    pte = pte_mkold(pte);

    page = vm_normal_page(vma, addr, pte);
    if (page)
    {
        // get_page(page); // check. bugged?
#if 0
        page_dup_rmap(page);
        rss[!!PageAnon(page)]++;
#endif
        ptepage->pfn$l_refcnt++;
    }

    if (did_cow==0 && (ptepage->pfn$q_bak&PTE$M_TYP0))
    {
        pte_t * mypte=&pte;
        signed long page = mmg$allocpfn();
        unsigned long address2 = (*(unsigned long *)src_pte)&0xfffff000;

        ptepage->pfn$l_refcnt--;
#if 0
#ifdef __i386__
        // check debug
        ptepage->pfn$l_pt_pfn=address2;
#endif
#endif
        mem_map[page]=*ptepage;
        *(unsigned long *)mypte=((unsigned long)(page*PAGE_SIZE))|((*(unsigned long *)mypte)&0xfff);
        memcpy(__va(page*PAGE_SIZE),__va(address2),PAGE_SIZE);
    }
    else
    {
        static int mydebugg = 0;
        if (mydebugg)
        {
            printk("%x %x %x %x\n",address,*dst_pte,*src_pte,src_pte);
        }
    }

out_set_pte:
    set_pte_at(dst_mm, addr, dst_pte, pte);
}

static int copy_pte_range(struct mm_struct *dst_mm, struct mm_struct *src_mm,
                          pmd_t *dst_pmd, pmd_t *src_pmd, struct vm_area_struct *vma,
                          unsigned long addr, unsigned long end)
{
    pte_t *src_pte, *dst_pte;
#if 0
    spinlock_t *src_ptl, *dst_ptl;
#endif
    int progress = 0;
    int rss[2];

again:
    rss[1] = rss[0] = 0;
#if 0
    dst_pte = pte_alloc_map_lock(dst_mm, dst_pmd, addr, &dst_ptl);
#else
    dst_pte = pte_alloc(dst_mm, dst_pmd, addr);
#endif
    if (!dst_pte)
        return -ENOMEM;
#if 0
    src_pte = pte_offset_map_nested(src_pmd, addr);
    src_ptl = pte_lockptr(src_mm, src_pmd);
    spin_lock(src_ptl);
#else
    src_pte = pte_offset(src_pmd, addr);
#endif

    do
    {
        /*
         * We are holding two locks at this point - either of them
         * could generate latencies in another task on another CPU.
         */
        if (progress >= 32)
        {
            progress = 0;
#if 0
            if (need_resched() ||
                    need_lockbreak(src_ptl) ||
                    need_lockbreak(dst_ptl))
                break;
#endif
        }
        if (pte_none(*src_pte))
        {
            progress++;
            continue;
        }
        copy_one_pte(dst_mm, src_mm, dst_pte, src_pte, vma, addr, rss);
        progress += 8;
    }
    while (dst_pte++, src_pte++, addr += PAGE_SIZE, addr != end);

#if 0
    spin_unlock(src_ptl);
#endif
    pte_unmap_nested(src_pte - 1);
#if 0
    add_mm_rss(dst_mm, rss[0], rss[1]);
#endif
#if 0
    pte_unmap_unlock(dst_pte - 1, dst_ptl);
#endif
#if 0
    cond_resched();
#endif
    if (addr != end)
        goto again;
    return 0;
}

static inline int copy_pmd_range(struct mm_struct *dst_mm, struct mm_struct *src_mm,
                                 pud_t *dst_pud, pud_t *src_pud, struct vm_area_struct *vma,
                                 unsigned long addr, unsigned long end)
{
    pmd_t *src_pmd, *dst_pmd;
    unsigned long next;

    dst_pmd = pmd_alloc(dst_mm, dst_pud, addr);
    if (!dst_pmd)
        return -ENOMEM;
    src_pmd = pmd_offset(src_pud, addr);
    do
    {
        next = pmd_addr_end(addr, end);
        if (pmd_none_or_clear_bad(src_pmd))
            continue;
        if (copy_pte_range(dst_mm, src_mm, dst_pmd, src_pmd,
                           vma, addr, next))
            return -ENOMEM;
    }
    while (dst_pmd++, src_pmd++, addr = next, addr != end);
    return 0;
}

static inline int copy_pud_range(struct mm_struct *dst_mm, struct mm_struct *src_mm,
                                 pgd_t *dst_pgd, pgd_t *src_pgd, struct vm_area_struct *vma,
                                 unsigned long addr, unsigned long end)
{
    pud_t *src_pud, *dst_pud;
    unsigned long next;

    dst_pud = pud_alloc(dst_mm, dst_pgd, addr);
    if (!dst_pud)
        return -ENOMEM;
    src_pud = pud_offset(src_pgd, addr);
    do
    {
        next = pud_addr_end(addr, end);
        if (pud_none_or_clear_bad(src_pud))
            continue;
        if (copy_pmd_range(dst_mm, src_mm, dst_pud, src_pud,
                           vma, addr, next))
            return -ENOMEM;
    }
    while (dst_pud++, src_pud++, addr = next, addr != end);
    return 0;
}

int copy_page_range(struct mm_struct *dst_mm, struct mm_struct *src_mm,
                    struct vm_area_struct *vma)
{
    pgd_t *src_pgd, *dst_pgd;
    unsigned long next;
    unsigned long addr = ((struct _rde *)vma)->rde$pq_start_va;
    unsigned long end = ((struct _rde *)vma)->rde$pq_start_va + ((struct _rde *)vma)->rde$q_region_size;

    /*
     * Don't copy ptes where a page fault will fill them correctly.
     * Fork becomes much lighter when there are big shared or private
     * readonly mappings. The tradeoff is that copy_page_range is more
     * efficient than faulting.
     */
#if 0
    // not yet?
    if (!(vma->vm_flags & (VM_HUGETLB|VM_NONLINEAR|VM_PFNMAP|VM_INSERTPAGE)))
    {
        if (!vma->anon_vma)
            return 0;
    }
#endif

#if 0
    // check
    if (is_vm_hugetlb_page(vma))
        return copy_hugetlb_page_range(dst_mm, src_mm, vma);
#endif

    dst_pgd = pgd_offset(dst_mm, addr);
    src_pgd = pgd_offset(src_mm, addr);
    do
    {
        next = pgd_addr_end(addr, end);
        if (pgd_none_or_clear_bad(src_pgd))
            continue;
        if (copy_pud_range(dst_mm, src_mm, dst_pgd, src_pgd,
                           vma, addr, next))
            return -ENOMEM;
    }
    while (dst_pgd++, src_pgd++, addr = next, addr != end);
    return 0;
}

#if 0
// old stuff
/*
 * copy one vm_area from one task to the other. Assumes the page tables
 * already present in the new task to be cleared in the whole range
 * covered by this vma.
 *
 * 08Jan98 Merged into one routine from several inline routines to reduce
 *         variable count and make things faster. -jj
 *
 * dst->page_table_lock is held on entry and exit,
 * but may be dropped within pmd_alloc() and pte_alloc().
 */
int copy_page_range(struct mm_struct *dst, struct mm_struct *src,
                    struct vm_area_struct *vma)
{
    pgd_t * src_pgd, * dst_pgd;
    unsigned long address = ((struct _rde *)vma)->rde$pq_start_va;
    unsigned long end = ((struct _rde *)vma)->rde$pq_start_va + ((struct _rde *)vma)->rde$q_region_size;
    unsigned long cow = (((struct _rde *)vma)->rde$l_flags & (VM_SHARED | VM_MAYWRITE)) == VM_MAYWRITE;
    unsigned long did_cow=0;

    src_pgd = pgd_offset(src, address)-1;
    dst_pgd = pgd_offset(dst, address)-1;

    for (;;)
    {
        pud_t * src_pud, * dst_pud;

        src_pgd++;
        dst_pgd++;

        /* copy_pud_range */

        if (pgd_none(*src_pgd))
            goto skip_copy_pud_range;
        if (pgd_bad(*src_pgd))
        {
            pgd_ERROR(*src_pgd);
            pgd_clear(src_pgd);
skip_copy_pud_range:
            address = (address + PGDIR_SIZE) & PGDIR_MASK;
            if (!address || (address >= end))
                goto out;
            continue;
        }

        src_pud = pud_offset(src_pgd, address);
        dst_pud = pud_alloc(dst, dst_pgd, address);
        if (!dst_pud)
            goto nomem;

        do
        {
            pte_t * src_pte, * dst_pte;

            /* copy_pte_range */

            if (pud_none(*src_pud))
                goto skip_copy_pte_range;
            if (pud_bad(*src_pud))
            {
                pud_ERROR(*src_pud);
                pud_clear(src_pud);
skip_copy_pte_range:
                address = (address + PUD_SIZE) & PUD_MASK;
                if (address >= end)
                    goto out;
                goto cont_copy_pud_range;
            }

            src_pte = pte_offset(src_pmd, address);
            dst_pte = pte_alloc(dst, dst_pmd, address);
            if (!dst_pte)
                goto nomem;

            spin_lock(&src->page_table_lock);
            do
            {
                pte_t pte = *src_pte;
                struct page *ptepage;

                /* copy_one_pte */

                if (pte_none(pte))
                    goto cont_copy_pte_range_noset;
                if (!pte_present(pte))
                {
#if 0
                    swap_duplicate(pte_to_swp_entry(pte));
#endif
                    goto cont_copy_pte_range;
                }
                ptepage = pte_page(pte);
                if ((!VALID_PAGE(ptepage)) ||
                        PageReserved(ptepage))
                    goto cont_copy_pte_range;

                /* If it's a COW mapping, write protect it both in the parent and the child */
                if (cow && pte_write(pte))
                {
                    ptep_set_wrprotect(src_pte);// drop this a while because it makes the stack readonly
                    pte = *src_pte;
                    did_cow=1;
                    if (0)
                    {
                        pte_t * mypte=&pte;
                        signed long page = mmg$allocpfn();
                        unsigned long address2 = (*(unsigned long *)src_pte)&0xfffff000;
#if 0
                        mem_map[page].virtual=__va(page*PAGE_SIZE);
#endif
                        *(unsigned long *)mypte=(__va(page*PAGE_SIZE));
                        *(unsigned long *)mypte|=_PAGE_PRESENT;
                        *(unsigned long *)mypte|=_PAGE_RW|_PAGE_USER|_PAGE_ACCESSED|_PAGE_DIRTY;
                        //flush_tlb_range(current->mm, address2, address2 + PAGE_SIZE);
                        memcpy(__va(page*PAGE_SIZE),address2,PAGE_SIZE);
                        //map(address2,(__va(page*PAGE_SIZE)),PAGE_SIZE,1,1,1);
                        //*(unsigned long *)pte|=1;
                    }
                }
                else
                {
                    did_cow=0;
                }

                /* If it's a shared mapping, mark it clean in the child */
                if (((struct _rde *)vma)->rde$l_flags & VM_SHARED)
                    pte = pte_mkclean(pte);
                pte = pte_mkold(pte);
                //get_page(ptepage); bugged
                ptepage->pfn$l_refcnt++;
                dst->rss++;

                if (did_cow==0 && (ptepage->pfn$q_bak&PTE$M_TYP0))
                {
                    pte_t * mypte=&pte;
                    signed long page = mmg$allocpfn();
                    unsigned long address2 = (*(unsigned long *)src_pte)&0xfffff000;
#if 0
                    struct _wsl * wsl = srcphd->phd$l_wslist;
                    struct _wsl * wsle = &wsl[ptepage->pfn$l_wslx_qw];
#endif

                    ptepage->pfn$l_refcnt--;
#if 0
#ifdef __i386__
                    // check debug
                    ptepage->pfn$l_pt_pfn=address2;
#endif
#endif
                    mem_map[page]=*ptepage;
#if 0
                    wsle->wsl$v_valid=1;
                    wsle->wsl$v_pagtyp=mem_map[page].pfn$v_pagtyp;
                    mem_map[page].virtual=__va(page*PAGE_SIZE);
                    ((unsigned long)wsle->wsl$pq_va)|=(unsigned long)mem_map[page].virtual;
#endif
                    *(unsigned long *)mypte=((unsigned long)(page*PAGE_SIZE))|((*(unsigned long *)mypte)&0xfff);
                    //*(unsigned long *)mypte|=_PAGE_PRESENT;
                    //*(unsigned long *)mypte|=_PAGE_RW|_PAGE_USER|_PAGE_ACCESSED|_PAGE_DIRTY;
                    if (0 && __va(page*PAGE_SIZE)>0xc0500000) goto there;
                    if (0 && ( address2<0x100000 || __va(page*PAGE_SIZE)<0x100000))
                    {
there:
                        printk("%x %x %x %x %x\n",address2,page,__va(page*PAGE_SIZE),src_pte,*src_pte);
#ifdef __arch_i386__
                        sickinsque(4,4);
#endif
                        panic("die is cast\n");
                    }
                    memcpy(__va(page*PAGE_SIZE),__va(address2),PAGE_SIZE);
                }
                else
                {
                    static int mydebugg = 0;
                    if (mydebugg)
                    {
                        printk("%x %x %x %x\n",address,*dst_pte,*src_pte,src_pte);
                    }
                }

cont_copy_pte_range:
                set_pte(dst_pte, pte);
                if (0)
                {
                    unsigned long * l=dst_pte;
                    *l&=0xfffff000;
                    *l|=((*(unsigned long *)src_pte)&0xfff);
                }
cont_copy_pte_range_noset:
                address += PAGE_SIZE;
                if (address >= end)
                    goto out_unlock;
                src_pte++;
                dst_pte++;
            }
            while ((unsigned long)src_pte & PTE_TABLE_MASK);
            spin_unlock(&src->page_table_lock);

cont_copy_pmd_range:
            src_pmd++;
            dst_pmd++;
        }
        while ((unsigned long)src_pmd & PMD_TABLE_MASK);
    }
out_unlock:
    spin_unlock(&src->page_table_lock);
out:
    return 0;
nomem:
    return -ENOMEM;
}
#endif

/*
 * Return indicates whether a page was freed so caller can adjust rss
 */
static inline void forget_pte(pte_t page)
{
    if (!pte_none(page))
    {
        printk("forget_pte: old mapping existed!\n");
        BUG();
    }
}

static inline int zap_pte_range(mmu_gather_t *tlb, pmd_t * pmd, unsigned long address, unsigned long size)
{
    unsigned long offset;
    pte_t * ptep;
    int freed = 0;

    if (pmd_none(*pmd))
        return 0;
    if (pmd_bad(*pmd))
    {
        pmd_ERROR(*pmd);
        pmd_clear(pmd);
        return 0;
    }
    ptep = pte_offset(pmd, address);
    offset = address & ~PMD_MASK;
    if (offset + size > PMD_SIZE)
        size = PMD_SIZE - offset;
    size &= PAGE_MASK;
    for (offset=0; offset < size; ptep++, offset += PAGE_SIZE)
    {
        pte_t pte = *ptep;
        if (pte_none(pte))
            continue;
        if (pte_present(pte))
        {
            struct page *page = pte_page(pte);
            if (VALID_PAGE(page) && !PageReserved(page))
                freed ++;
            /* This will eventually call __free_pte on the pte. */
            tlb_remove_page(tlb, ptep, address + offset);
        }
        else
        {
#if 0
            free_swap_and_cache(pte_to_swp_entry(pte));
#endif
            pte_clear(42, 42, ptep); // check
        }
    }

    return freed;
}

static inline int zap_pmd_range(mmu_gather_t *tlb, pud_t * dir, unsigned long address, unsigned long size)
{
    pmd_t * pmd;
    unsigned long end;
    int freed;

    if (pud_none(*dir))
        return 0;
    if (pud_bad(*dir))
    {
        pud_ERROR(*dir);
        pud_clear(dir);
        return 0;
    }
    pmd = pmd_offset(dir, address);
    end = address + size;
    if (end > ((address + PGDIR_SIZE) & PGDIR_MASK))
        end = ((address + PGDIR_SIZE) & PGDIR_MASK);
    freed = 0;
    do
    {
        freed += zap_pte_range(tlb, pmd, address, end - address);
        address = (address + PMD_SIZE) & PMD_MASK;
        pmd++;
    }
    while (address < end);
    return freed;
}

static inline int zap_pud_range(mmu_gather_t *tlb, pgd_t * dir, unsigned long address, unsigned long size)
{
    pud_t * pud;
    unsigned long end;
    int freed;

    if (pgd_none(*dir))
        return 0;
    if (pgd_bad(*dir))
    {
        pgd_ERROR(*dir);
        pgd_clear(dir);
        return 0;
    }
    pud = pud_offset(dir, address);
    end = address + size;
    if (end > ((address + PGDIR_SIZE) & PGDIR_MASK))
        end = ((address + PGDIR_SIZE) & PGDIR_MASK);
    freed = 0;
    do
    {
        freed += zap_pmd_range(tlb, pud, address, end - address);
        address = (address + PUD_SIZE) & PUD_MASK;
        pud++;
    }
    while (address < end);
    return freed;
}

/*
 * remove user pages in a given range.
 */
void zap_page_range(struct mm_struct *mm, unsigned long address, unsigned long size)
{
    mmu_gather_t *tlb;
    pgd_t * dir;
    unsigned long start = address, end = address + size;
    int freed = 0;
    struct _va_range inadr;

    inadr.va_range$ps_start_va=address;
    inadr.va_range$ps_end_va=address+size;

    dir = pgd_offset(mm, address);

    /*
     * This is a long-lived spinlock. That's fine.
     * There's no contention, because the page table
     * lock only protects against kswapd anyway, and
     * even if kswapd happened to be looking at this
     * process we _want_ it to get stuck.
     */
    if (address >= end)
        BUG();
    spin_lock(&mm->page_table_lock);
    exe$deltva(&inadr,0,0);
    //  exe$purgws(&inadr);
    flush_cache_range(mm, address, end);
    tlb = tlb_gather_mmu(mm);

    do
    {
        freed += zap_pud_range(tlb, dir, address, end - address);
        address = (address + PGDIR_SIZE) & PGDIR_MASK;
        dir++;
    }
    while (address && (address < end));

    /* this will flush any remaining tlb entries */
    tlb_finish_mmu(tlb, start, end);

    /*
     * Update rss for the mm_struct (not necessarily current->mm)
     * Notice that rss is an unsigned long.
     */
    if (mm->rss > freed)
        mm->rss -= freed;
    else
        mm->rss = 0;
    spin_unlock(&mm->page_table_lock);
}

/*
 * Do a quick page-table lookup for a single page.
 */
static struct page * follow_page(struct mm_struct *mm, unsigned long address, int write)
{
    pgd_t *pgd;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *ptep, pte;

    pgd = pgd_offset(mm, address);
    if (pgd_none(*pgd) || pgd_bad(*pgd))
        goto out;

    pud = pud_offset(pgd, address);
    if (pud_none(*pud) || pud_bad(*pud))
        goto out;

    pmd = pmd_offset(pud, address);
    if (pmd_none(*pmd) || pmd_bad(*pmd))
        goto out;

    ptep = pte_offset(pmd, address);
    if (!ptep)
        goto out;

    pte = *ptep;
    if (pte_present(pte))
    {
        if (!write ||
                (pte_write(pte) && pte_dirty(pte)))
            return pte_page(pte);
    }

out:
    return 0;
}

/*
 * Given a physical address, is there a useful struct page pointing to
 * it?  This may become more complex in the future if we start dealing
 * with IO-aperture pages in kiobufs.
 */

static inline struct page * get_page_map(struct page *page)
{
    if (!VALID_PAGE(page))
        return 0;
    return page;
}

/*
 * Force in an entire range of pages from the current process's user VA,
 * and pin them in physical memory.
 */
#define dprintk(x...)

int map_user_kiobuf(int rw, struct kiobuf *iobuf, unsigned long va, size_t len)
{
    int pgcount, err;
    struct mm_struct *  mm;

    /* Make sure the iobuf is not already mapped somewhere. */
    if (iobuf->nr_pages)
        return -EINVAL;

    mm = current->mm;
    dprintk ("map_user_kiobuf: begin\n");

    pgcount = (va + len + PAGE_SIZE - 1)/PAGE_SIZE - va/PAGE_SIZE;
    /* mapping 0 bytes is not permitted */
    if (!pgcount) BUG();
    err = expand_kiobuf(iobuf, pgcount);
    if (err)
        return err;

    iobuf->locked = 0;
    iobuf->offset = va & (PAGE_SIZE-1);
    iobuf->length = len;

    /* Try to fault in all of the necessary pages */
    down_read(&mm->mmap_sem);
    /* rw==READ means read from disk, write into memory area */
#if 0
    err = get_user_pages(current, mm, va, pgcount,
                         (rw==READ), 0, iobuf->maplist, NULL);
#endif
    up_read(&mm->mmap_sem);
    if (err < 0)
    {
        unmap_kiobuf(iobuf);
        dprintk ("map_user_kiobuf: end %d\n", err);
        return err;
    }
    iobuf->nr_pages = err;
    while (pgcount--)
    {
        /* FIXME: flush superflous for rw==READ,
         * probably wrong function for rw==WRITE
         */
        flush_dcache_page(iobuf->maplist[pgcount]);
    }
    dprintk ("map_user_kiobuf: end OK\n");
    return 0;
}

/*
 * Mark all of the pages in a kiobuf as dirty
 *
 * We need to be able to deal with short reads from disk: if an IO error
 * occurs, the number of bytes read into memory may be less than the
 * size of the kiobuf, so we have to stop marking pages dirty once the
 * requested byte count has been reached.
 */

void mark_dirty_kiobuf(struct kiobuf *iobuf, int bytes)
{
    int index, offset, remaining;
    struct page *page;

    index = iobuf->offset >> PAGE_SHIFT;
    offset = iobuf->offset & ~PAGE_MASK;
    remaining = bytes;
    if (remaining > iobuf->length)
        remaining = iobuf->length;

    while (remaining > 0 && index < iobuf->nr_pages)
    {
        page = iobuf->maplist[index];

#if 0
        if (!PageReserved(page))
            SetPageDirty(page);
#endif

        remaining -= (PAGE_SIZE - offset);
        offset = 0;
        index++;
    }
}

/*
 * Unmap all of the pages referenced by a kiobuf.  We release the pages,
 * and unlock them if they were locked.
 */

void unmap_kiobuf (struct kiobuf *iobuf)
{
    int i;
    struct page *map;

    for (i = 0; i < iobuf->nr_pages; i++)
    {
        map = iobuf->maplist[i];
        if (map)
        {
#if 0
            if (iobuf->locked)
                UnlockPage(map);
#endif
            /* FIXME: cache flush missing for rw==READ
             * FIXME: call the correct reference counting function
             */
            page_cache_release(map);
        }
    }

    iobuf->nr_pages = 0;
    iobuf->locked = 0;
}


/*
 * Lock down all of the pages of a kiovec for IO.
 *
 * If any page is mapped twice in the kiovec, we return the error -EINVAL.
 *
 * The optional wait parameter causes the lock call to block until all
 * pages can be locked if set.  If wait==0, the lock operation is
 * aborted if any locked pages are found and -EAGAIN is returned.
 */

int lock_kiovec(int nr, struct kiobuf *iovec[], int wait)
{
    struct kiobuf *iobuf;
    int i, j;
    struct page *page, **ppage;
    int doublepage = 0;
    int repeat = 0;

repeat:

    for (i = 0; i < nr; i++)
    {
        iobuf = iovec[i];

        if (iobuf->locked)
            continue;

        ppage = iobuf->maplist;
        for (j = 0; j < iobuf->nr_pages; ppage++, j++)
        {
            page = *ppage;
            if (!page)
                continue;

#if 0
            if (TryLockPage(page))
            {
                while (j--)
                {
                    struct page *tmp = *--ppage;
                    if (tmp)
                        UnlockPage(tmp);
                }
                goto retry;
            }
#endif
        }
        iobuf->locked = 1;
    }

    return 0;

retry:

    /*
     * We couldn't lock one of the pages.  Undo the locking so far,
     * wait on the page we got to, and try again.
     */

    unlock_kiovec(nr, iovec);
    if (!wait)
        return -EAGAIN;

    /*
     * Did the release also unlock the page we got stuck on?
     */
#if 0
    if (!PageLocked(page))
    {
        /*
         * If so, we may well have the page mapped twice
         * in the IO address range.  Bad news.  Of
         * course, it _might_ just be a coincidence,
         * but if it happens more than once, chances
         * are we have a double-mapped page.
         */
        if (++doublepage >= 3)
            return -EINVAL;

        /* Try again...  */
        //wait_on_page(page);
    }
#endif

    if (++repeat < 16)
        goto repeat;
    return -EAGAIN;
}

/*
 * Unlock all of the pages of a kiovec after IO.
 */

int unlock_kiovec(int nr, struct kiobuf *iovec[])
{
    struct kiobuf *iobuf;
    int i, j;
    struct page *page, **ppage;

    for (i = 0; i < nr; i++)
    {
        iobuf = iovec[i];

        if (!iobuf->locked)
            continue;
        iobuf->locked = 0;

        ppage = iobuf->maplist;
        for (j = 0; j < iobuf->nr_pages; ppage++, j++)
        {
            page = *ppage;
            if (!page)
                continue;
#if 0
            UnlockPage(page);
#endif
        }
    }
    return 0;
}

static inline void zeromap_pte_range(pte_t * pte, unsigned long address,
                                     unsigned long size, pgprot_t prot)
{
    unsigned long end;

    address &= ~PMD_MASK;
    end = address + size;
    if (end > PMD_SIZE)
        end = PMD_SIZE;
    do
    {
        pte_t zero_pte = pte_wrprotect(mk_pte(ZERO_PAGE(address), prot));
        pte_t oldpage = ptep_get_and_clear(42, 42, pte); // check
        set_pte(pte, zero_pte);
        forget_pte(oldpage);
        address += PAGE_SIZE;
        pte++;
    }
    while (address && (address < end));
}

static inline int zeromap_pmd_range(struct mm_struct *mm, pmd_t * pmd, unsigned long address,
                                    unsigned long size, pgprot_t prot)
{
    unsigned long end;

    address &= ~PGDIR_MASK;
    end = address + size;
    if (end > PGDIR_SIZE)
        end = PGDIR_SIZE;
    do
    {
        pte_t * pte = pte_alloc(mm, pmd, address);
        if (!pte)
            return -ENOMEM;
        zeromap_pte_range(pte, address, end - address, prot);
        address = (address + PMD_SIZE) & PMD_MASK;
        pmd++;
    }
    while (address && (address < end));
    return 0;
}

int zeromap_page_range(unsigned long address, unsigned long size, pgprot_t prot)
{
    int error = 0;
    pgd_t * dir;
    unsigned long beg = address;
    unsigned long end = address + size;
    struct mm_struct *mm = current->mm;

    dir = pgd_offset(mm, address);
    flush_cache_range(mm, beg, end);
    if (address >= end)
        BUG();

    spin_lock(&mm->page_table_lock);
    do
    {
        pmd_t *pmd = pmd_alloc(mm, dir, address);
        error = -ENOMEM;
        if (!pmd)
            break;
        error = zeromap_pmd_range(mm, pmd, address, end - address, prot);
        if (error)
            break;
        address = (address + PGDIR_SIZE) & PGDIR_MASK;
        dir++;
    }
    while (address && (address < end));
    spin_unlock(&mm->page_table_lock);
    flush_tlb_range(mm, beg, end);
    return error;
}

/*
 * maps a range of physical memory into the requested pages. the old
 * mappings are removed. any references to nonexistent pages results
 * in null mappings (currently treated as "copy-on-access")
 */
static inline void remap_pte_range(pte_t * pte, unsigned long address, unsigned long size,
                                   unsigned long phys_addr, pgprot_t prot)
{
    unsigned long end;

    address &= ~PMD_MASK;
    end = address + size;
    if (end > PMD_SIZE)
        end = PMD_SIZE;
    do
    {
        struct page *page;
        pte_t oldpage;
        oldpage = ptep_get_and_clear(42, 42, pte); // check

        page = virt_to_page(__va(phys_addr));
        if ((!VALID_PAGE(page)) || PageReserved(page))
            set_pte(pte, mk_pte_phys(phys_addr, prot));
        forget_pte(oldpage);
        address += PAGE_SIZE;
        phys_addr += PAGE_SIZE;
        pte++;
    }
    while (address && (address < end));
}

static inline int remap_pmd_range(struct mm_struct *mm, pmd_t * pmd, unsigned long address, unsigned long size,
                                  unsigned long phys_addr, pgprot_t prot)
{
    unsigned long end;

    address &= ~PGDIR_MASK;
    end = address + size;
    if (end > PGDIR_SIZE)
        end = PGDIR_SIZE;
    phys_addr -= address;
    do
    {
        pte_t * pte = pte_alloc(mm, pmd, address);
        if (!pte)
            return -ENOMEM;
        remap_pte_range(pte, address, end - address, address + phys_addr, prot);
        address = (address + PMD_SIZE) & PMD_MASK;
        pmd++;
    }
    while (address && (address < end));
    return 0;
}

static inline int remap_pud_range(struct mm_struct *mm, pud_t * pud, unsigned long address, unsigned long size,
                                  unsigned long phys_addr, pgprot_t prot)
{
    unsigned long end;

    address &= ~PGDIR_MASK;
    end = address + size;
    if (end > PGDIR_SIZE)
        end = PGDIR_SIZE;
    phys_addr -= address;
    do
    {
        pte_t * pte = pte_alloc(mm, pud, address);
        if (!pte)
            return -ENOMEM;
        remap_pte_range(pte, address, end - address, address + phys_addr, prot);
        address = (address + PUD_SIZE) & PUD_MASK;
        pud++;
    }
    while (address && (address < end));
    return 0;
}

/*  Note: this is only safe if the mm semaphore is held when called. */
int remap_page_range(unsigned long from, unsigned long phys_addr, unsigned long size, pgprot_t prot)
{
    int error = 0;
    pgd_t * dir;
    unsigned long beg = from;
    unsigned long end = from + size;
    struct mm_struct *mm = current->mm;

    phys_addr -= from;
    dir = pgd_offset(mm, from);
    flush_cache_range(mm, beg, end);
    if (from >= end)
        BUG();

    spin_lock(&mm->page_table_lock);
    do
    {
        pud_t *pud = pud_alloc(mm, dir, from);
        error = -ENOMEM;
        if (!pud)
            break;
        error = remap_pud_range(mm, pud, from, end - from, phys_addr + from, prot);
        if (error)
            break;
        from = (from + PGDIR_SIZE) & PGDIR_MASK;
        dir++;
    }
    while (from && (from < end));
    spin_unlock(&mm->page_table_lock);
    flush_tlb_range(mm, beg, end);
    return error;
}

/*
 * Establish a new mapping:
 *  - flush the old one
 *  - update the page tables
 *  - inform the TLB about the new one
 *
 * We hold the mm semaphore for reading and vma->vm_mm->page_table_lock
 */
static inline void establish_pte(struct _rde * vma, unsigned long address, pte_t *page_table, pte_t entry)
{
    set_pte(page_table, entry);
    flush_tlb_page2(current->mm, address);
    update_mmu_cache(vma, address, entry);
}

/*
 * We hold the mm semaphore for reading and vma->vm_mm->page_table_lock
 */
static inline void break_cow(struct _rde * vma, struct page * new_page, unsigned long address,
                             pte_t *page_table)
{
    flush_page_to_ram(new_page);
    flush_cache_page(vma, address);
    establish_pte(vma, address, page_table, pte_mkwrite(pte_mkdirty(mk_pte(new_page, (x_to_prot(vma->rde$r_regprot.regprt$l_region_prot))))));
}

/*
 * This routine handles present pages, when users try to write
 * to a shared page. It is done by copying the page to a new address
 * and decrementing the shared-page counter for the old page.
 *
 * Goto-purists beware: the only reason for goto's here is that it results
 * in better assembly code.. The "default" path will see no jumps at all.
 *
 * Note that this routine assumes that the protection checks have been
 * done by the caller (the low-level page fault routine in most cases).
 * Thus we can safely just mark it writable once we've done any necessary
 * COW.
 *
 * We also mark the page dirty at this point even though the page will
 * change only once the write actually happens. This avoids a few races,
 * and potentially makes it more efficient.
 *
 * We hold the mm semaphore and the page_table_lock on entry and exit
 * with the page_table_lock released.
 */
/* static */ int do_wp_page(struct mm_struct *mm, struct _rde * vma,
                            unsigned long address, pte_t *page_table, pte_t pte)
{
    struct page *old_page, *new_page;

    old_page = pte_page(pte);
    if (!VALID_PAGE(old_page))
        goto bad_wp_page;

    if (1/*!TryLockPage(old_page)*/)
    {
        int reuse = can_share_swap_page(old_page);
#if 0
        unlock_page(old_page);
#endif
        if (reuse)
        {
            flush_cache_page(vma, address);
            establish_pte(vma, address, page_table, pte_mkyoung(pte_mkdirty(pte_mkwrite(pte))));
            spin_unlock(&mm->page_table_lock);
            return 1;   /* Minor fault */
        }
    }

    /*
     * Ok, we need to copy. Oh, well..
     */
    page_cache_get(old_page);
    spin_unlock(&mm->page_table_lock);

    new_page = alloc_page(GFP_HIGHUSER);
    if (!new_page)
        goto no_mem;
    new_page->pfn$l_page_state=old_page->pfn$l_page_state;
    new_page->pfn$l_wslx_qw=old_page->pfn$l_wslx_qw;
    new_page->pfn$q_pte_index=findpte_new(mm,address);
    new_page->pfn$q_bak=old_page->pfn$q_bak;
    new_page->pfn$l_refcnt=1;
    copy_cow_page(old_page,new_page,address);

    /*
     * Re-check the pte - we dropped the lock
     */
    spin_lock(&mm->page_table_lock);
    if (pte_same(*page_table, pte))
    {
#if 0
        if (PageReserved(old_page))
            ++mm->rss;
#endif
        break_cow(vma, new_page, address, page_table);
        //lru_cache_add(new_page);

        /* Free the old page.. */
        new_page = old_page;
    }
    spin_unlock(&mm->page_table_lock);
    page_cache_release(new_page);
    page_cache_release(old_page);
    return 1;   /* Minor fault */

bad_wp_page:
    spin_unlock(&mm->page_table_lock);
    printk("do_wp_page: bogus page at address %08lx (page 0x%lx)\n",address,(unsigned long)old_page);
    return -1;
no_mem:
    page_cache_release(old_page);
    return -1;
}

static void vmtruncate_list(struct _rde *mpnt, unsigned long pgoff)
{
    do
    {
        struct mm_struct *mm = 0; //mpnt->vm_mm;
        unsigned long start = mpnt->rde$pq_start_va;
        unsigned long end = mpnt->rde$pq_start_va + mpnt->rde$q_region_size;
        unsigned long len = end - start;
        unsigned long diff;

        /* mapping wholly truncated? */
        if (0 /*mpnt->vm_pgoff*/ >= pgoff)
        {
            zap_page_range(mm, start, len);
            continue;
        }

        /* mapping wholly unaffected? */
        len = len >> PAGE_SHIFT;
        diff = pgoff - 0 /*mpnt->vm_pgoff*/;
        if (diff >= len)
            continue;

        /* Ok, partially affected.. */
        start += diff << PAGE_SHIFT;
        len = (len - diff) << PAGE_SHIFT;
        zap_page_range(mm, start, len);
    }
    while ((mpnt = 0 /*mpnt->vm_next_share*/) != NULL);
}

/*
 * Handle all mappings that got truncated by a "truncate()"
 * system call.
 *
 * NOTE! We have to be ready to update the memory sharing
 * between the file and the memory map for a potential last
 * incomplete page.  Ugly, but necessary.
 */
int vmtruncate(struct _fcb * inode, loff_t offset)
{
#if 0
    unsigned long pgoff;
    struct address_space *mapping = inode->i_mapping;
    unsigned long limit;

    if (inode->fcb$l_filesize < offset)
        goto do_expand;
    inode->fcb$l_filesize = offset;
    spin_lock(&mapping->i_shared_lock);
    if (!mapping->i_mmap && !mapping->i_mmap_shared)
        goto out_unlock;

    pgoff = (offset + PAGE_CACHE_SIZE - 1) >> PAGE_CACHE_SHIFT;
    if (mapping->i_mmap != NULL)
        vmtruncate_list(mapping->i_mmap, pgoff);
    if (mapping->i_mmap_shared != NULL)
        vmtruncate_list(mapping->i_mmap_shared, pgoff);

out_unlock:
    spin_unlock(&mapping->i_shared_lock);
    truncate_inode_pages(mapping, offset);
    goto out_truncate;

do_expand:
    limit = current->rlim[RLIMIT_FSIZE].rlim_cur;
    if (limit != RLIM_INFINITY && offset > limit)
        goto out_sig;
    if (offset > inode->i_sb->s_maxbytes)
        goto out;
    inode->fcb$l_filesize = offset;

out_truncate:
    if (inode->i_op && inode->i_op->truncate)
    {
        lock_kernel();
        inode->i_op->truncate(inode);
        unlock_kernel();
    }
    return 0;
out_sig:
    send_sig(SIGXFSZ, current, 0);
out:
    return -EFBIG;
#else
    return -EPERM;
#endif
}

/*
 * Allocate page upper directory.
 *
 * We've already handled the fast-path in-line, and we own the
 * page table lock.
 *
 * On a two-level page table, this ends up actually being entirely
 * optimized away.
 */
pud_t * fastcall __pud_alloc(struct mm_struct *mm, pgd_t *pgd, unsigned long address)
{
    pud_t *new = 0;

    /* "fast" allocation can happen without dropping the lock.. */
    if (!new)
    {
        spin_unlock(&mm->page_table_lock);
        new = pud_alloc_one(mm, address);
        spin_lock(&mm->page_table_lock);
        if (!new)
            return NULL;

        /*
         * Because we dropped the lock, we should re-check the
         * entry, as somebody else could have populated it..
         */
        if (!pgd_none(*pgd))
        {
            pud_free(new);
            goto out;
        }
    }
    pgd_populate(mm, pgd, new);
out:
    return pud_offset(pgd, address);
}

/*
 * Allocate page middle directory.
 *
 * We've already handled the fast-path in-line, and we own the
 * page table lock.
 *
 * On a two-level page table, this ends up actually being entirely
 * optimized away.
 */
pmd_t * fastcall __pmd_alloc(struct mm_struct *mm, pud_t *pud, unsigned long address)
{
    pmd_t *new;

    /* "fast" allocation can happen without dropping the lock.. */
    new = pmd_alloc_one_fast(mm, address);
    if (!new)
    {
        spin_unlock(&mm->page_table_lock);
        new = pmd_alloc_one(mm, address);
        spin_lock(&mm->page_table_lock);
        if (!new)
            return NULL;

        /*
         * Because we dropped the lock, we should re-check the
         * entry, as somebody else could have populated it..
         */
        if (!pud_none(*pud))
        {
            pmd_free(new);
            goto out;
        }
    }
    pud_populate(mm, pud, new);
out:
    return pmd_offset(pud, address);
}

/*
 * Allocate the page table directory.
 *
 * We've already handled the fast-path in-line, and we own the
 * page table lock.
 */
pte_t * fastcall pte_alloc(struct mm_struct *mm, pmd_t *pmd, unsigned long address)
{
    if (pmd_none(*pmd))
    {
        pte_t *new;

        /* "fast" allocation can happen without dropping the lock.. */
        new = pte_alloc_one_fast(mm, address);
        if (!new)
        {
            spin_unlock(&mm->page_table_lock);
            new = pte_alloc_one(mm, address);
            spin_lock(&mm->page_table_lock);
            if (!new)
                return NULL;

            /*
             * Because we dropped the lock, we should re-check the
             * entry, as somebody else could have populated it..
             */
            if (!pmd_none(*pmd))
            {
                pte_free(new);
                goto out;
            }
        }
        pmd_populate(mm, pmd, new);
    }
out:
    return pte_offset(pmd, address);
}

int make_pages_present(unsigned long addr, unsigned long end)
{
    int ret = 0, len, write;
    struct _rde * vma;

    //  vma = find_vma(current->mm, addr);
    vma = find_vma(current->pcb$l_phd,addr);
    write = (((struct _rde *) vma)->rde$l_flags & VM_WRITE) != 0;
    if (addr >= end)
        BUG();
    if (end > ((unsigned long long)((struct _rde *) vma->rde$pq_start_va) + (unsigned long long)((struct _rde *) vma->rde$q_region_size)))
        BUG();
    len = (end+PAGE_SIZE-1)/PAGE_SIZE-addr/PAGE_SIZE;
#if 0
    ret = get_user_pages(current, current->mm, addr,
                         len, write, 0, NULL, NULL);
#endif
    return ret == len ? 0 : -1;
}
