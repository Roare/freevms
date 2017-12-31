// $Id$
// $Locker$

// Author. Roar Thron�s.
// Modified Linux source file, 2001-2004. Based on bootmem.c.

/*
 *  linux/mm/bootmem.c
 *
 *  Copyright (C) 1999 Ingo Molnar
 *  Discontiguous memory support, Kanoj Sarcar, SGI, Nov 1999
 *
 *  simple boot-time physical memory area allocator and
 *  free memory collector. It's used to deal with reserved
 *  system memory and memory holes as well.
 */

#include <linux/config.h>
#include <linux/mm.h>
#include <linux/kernel_stat.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/bootmem.h>
#include <linux/mmzone.h>
#include <asm/dma.h>

#include <system_data_cells.h>

#undef OLDINT
#define OLDINT

#ifdef __x86_64__
#undef OLDINT
#endif

/*
 * Access to this subsystem has to be serialized externally. (this is
 * true for the boot process anyway)
 */
unsigned long max_low_pfn;
unsigned long min_low_pfn;

/* return the number of _pages_ that will be allocated for the boot bitmap */
unsigned long __init bootmem_bootmap_pages (unsigned long pages)
{
    unsigned long mapsize;

    mapsize = (pages+7)/8;
    mapsize = (mapsize + ~PAGE_MASK) & PAGE_MASK;
    mapsize >>= PAGE_SHIFT;

    return mapsize;
}

/*
 * Called once to set up the allocator itself.
 */
static unsigned long __init init_bootmem_core (pg_data_t *pgdat,
        unsigned long mapstart, unsigned long start, unsigned long end)
{
    bootmem_data_t *bdata = pgdat->bdata;
    unsigned long mapsize = ((end - start)+7)/8;

    pgdat->node_next = pgdat_list;
    pgdat_list = pgdat;

    mapsize = (mapsize + (sizeof(long) - 1UL)) & ~(sizeof(long) - 1UL);
    bdata->node_bootmem_map = phys_to_virt(mapstart << PAGE_SHIFT);
    bdata->node_boot_start = (start << PAGE_SHIFT);
    bdata->node_low_pfn = end;

    /*
     * Initially all pages are reserved - setup_arch() has to
     * register free RAM areas explicitly.
     */
    memset(bdata->node_bootmem_map, 0xff, mapsize);

    return mapsize;
}

/*
 * Marks a particular physical memory range as unallocatable. Usable RAM
 * might be used for boot-time allocations - or it might get added
 * to the free page pool later on.
 */
static void __init reserve_bootmem_core(bootmem_data_t *bdata, unsigned long addr, unsigned long size)
{
    unsigned long i;
    /*
     * round up, partially reserved pages are considered
     * fully reserved.
     */
    unsigned long sidx = (addr - bdata->node_boot_start)/PAGE_SIZE;
    unsigned long eidx = (addr + size - bdata->node_boot_start +
                          PAGE_SIZE-1)/PAGE_SIZE;
    unsigned long end = (addr + size + PAGE_SIZE-1)/PAGE_SIZE;

    if (!size) BUG();

    if (sidx < 0)
        BUG();
    if (eidx < 0)
        BUG();
    if (sidx >= eidx)
        BUG();
    if ((addr >> PAGE_SHIFT) >= bdata->node_low_pfn)
        BUG();
    if (end > bdata->node_low_pfn)
        BUG();
    for (i = sidx; i < eidx; i++)
        if (test_and_set_bit(i, bdata->node_bootmem_map))
            printk("hm, page %08lx reserved twice.\n", i*PAGE_SIZE);
}

static void __init free_bootmem_core(bootmem_data_t *bdata, unsigned long addr, unsigned long size)
{
    unsigned long i;
    unsigned long start;
    /*
     * round down end of usable mem, partially free pages are
     * considered reserved.
     */
    unsigned long sidx;
    unsigned long eidx = (addr + size - bdata->node_boot_start)/PAGE_SIZE;
    unsigned long end = (addr + size)/PAGE_SIZE;

    if (!size) BUG();
    if (end > bdata->node_low_pfn)
        BUG();

    /*
     * Round up the beginning of the address.
     */
    start = (addr + PAGE_SIZE-1) / PAGE_SIZE;
    sidx = start - (bdata->node_boot_start/PAGE_SIZE);

    for (i = sidx; i < eidx; i++)
    {
        if (!test_and_clear_bit(i, bdata->node_bootmem_map))
            BUG();
    }
}

/*
 * We 'merge' subsequent allocations to save space. We might 'lose'
 * some fraction of a page if allocations cannot be satisfied due to
 * size constraints on boxes where there is physical RAM space
 * fragmentation - in these cases * (mostly large memory boxes) this
 * is not a problem.
 *
 * On low memory boxes we get it right in 100% of the cases.
 */

/*
 * alignment has to be a power of 2 value.
 */
static void * __init __alloc_bootmem_core (bootmem_data_t *bdata,
        unsigned long size, unsigned long align, unsigned long goal)
{
    unsigned long i, start = 0;
    void *ret;
    unsigned long offset, remaining_size;
    unsigned long areasize, preferred, incr;
    unsigned long eidx = bdata->node_low_pfn - (bdata->node_boot_start >>
                         PAGE_SHIFT);

    if (!size) BUG();

    if (align & (align-1))
        BUG();

    offset = 0;
    if (align &&
            (bdata->node_boot_start & (align - 1UL)) != 0)
        offset = (align - (bdata->node_boot_start & (align - 1UL)));
    offset >>= PAGE_SHIFT;

    /*
     * We try to allocate bootmem pages above 'goal'
     * first, then we try to allocate lower pages.
     */
    if (goal && (goal >= bdata->node_boot_start) &&
            ((goal >> PAGE_SHIFT) < bdata->node_low_pfn))
    {
        preferred = goal - bdata->node_boot_start;
    }
    else
        preferred = 0;

    preferred = ((preferred + align - 1) & ~(align - 1)) >> PAGE_SHIFT;
    preferred += offset;
    areasize = (size+PAGE_SIZE-1)/PAGE_SIZE;
    incr = align >> PAGE_SHIFT ? : 1;

restart_scan:
    for (i = preferred; i < eidx; i += incr)
    {
        unsigned long j;
        if (test_bit(i, bdata->node_bootmem_map))
            continue;
        for (j = i + 1; j < i + areasize; ++j)
        {
            if (j >= eidx)
                goto fail_block;
            if (test_bit (j, bdata->node_bootmem_map))
                goto fail_block;
        }
        start = i;
        goto found;
fail_block:
        ;
    }
    if (preferred)
    {
        preferred = offset;
        goto restart_scan;
    }
    return NULL;
found:
    if (start >= eidx)
        BUG();

    /*
     * Is the next page of the previous allocation-end the start
     * of this allocation's buffer? If yes then we can 'merge'
     * the previous partial page with this allocation.
     */
    if (align <= PAGE_SIZE
            && bdata->last_offset && bdata->last_pos+1 == start)
    {
        offset = (bdata->last_offset+align-1) & ~(align-1);
        if (offset > PAGE_SIZE)
            BUG();
        remaining_size = PAGE_SIZE-offset;
        if (size < remaining_size)
        {
            areasize = 0;
            // last_pos unchanged
            bdata->last_offset = offset+size;
            ret = phys_to_virt(bdata->last_pos*PAGE_SIZE + offset +
                               bdata->node_boot_start);
        }
        else
        {
            remaining_size = size - remaining_size;
            areasize = (remaining_size+PAGE_SIZE-1)/PAGE_SIZE;
            ret = phys_to_virt(bdata->last_pos*PAGE_SIZE + offset +
                               bdata->node_boot_start);
            bdata->last_pos = start+areasize-1;
            bdata->last_offset = remaining_size;
        }
        bdata->last_offset &= ~PAGE_MASK;
    }
    else
    {
        bdata->last_pos = start + areasize - 1;
        bdata->last_offset = size & ~PAGE_MASK;
        ret = phys_to_virt(start * PAGE_SIZE + bdata->node_boot_start);
    }
    /*
     * Reserve the area now:
     */
    for (i = start; i < start+areasize; i++)
        if (test_and_set_bit(i, bdata->node_bootmem_map))
            BUG();
    memset(ret, 0, size);
    printk("alloc_bootmem_core %x %x\n",ret,size);
    return ret;
}

int in_free_all_bootmem_core=0;

static unsigned long __init free_all_bootmem_core(pg_data_t *pgdat)
{
    struct page *page = pgdat->node_mem_map;
    bootmem_data_t *bdata = pgdat->bdata;
    unsigned long i, count, total = 0;
    unsigned long idx;
    struct page * prev;

    in_free_all_bootmem_core=1;

    if (!bdata->node_bootmem_map) BUG();

    count = 0;
    idx = bdata->node_low_pfn - (bdata->node_boot_start >> PAGE_SHIFT);
    for (i = 0; i < idx; i++, page++)
    {
        if (!test_bit(i, bdata->node_bootmem_map))
        {
            count++;
            ClearPageReserved(page);
            set_page_count(page, 1);
            __free_page(page);
        }
    }
    total += count;

    /*
     * Now free the allocator bitmap itself, it's not
     * needed anymore:
     */
    page = virt_to_page(bdata->node_bootmem_map);
    count = 0;
    for (i = 0; i < ((bdata->node_low_pfn-(bdata->node_boot_start >> PAGE_SHIFT))/8 + PAGE_SIZE-1)/PAGE_SIZE; i++,page++)
    {
        count++;
        ClearPageReserved(page);
        set_page_count(page, 1);
        __free_page(page);
    }
    total += count;
    bdata->node_bootmem_map = NULL;

    prev = 0;
#ifndef OLDINT
    int previ = 0;
#endif
    page = pgdat->node_mem_map;
    page++;
    for (i = 1; i < idx; i++, page++)   // skip physical 0
    {
        if (!PageReserved(page))
        {
            sch$gl_freecnt++;
            if (prev)
            {
#ifdef OLDINT
                prev->pfn$l_flink=page;
#else
                prev->pfn$l_flink=i; // was: page;
#endif
            }
            else
            {
#ifdef OLDINT
                pfn$al_head[PFN$C_FREPAGLST]=page;
#else
                pfn$al_head[PFN$C_FREPAGLST]=i; // was: page;
#endif
            }
            {
#ifdef OLDINT
                unsigned long * prev2=&page->pfn$l_flink;
#else
                unsigned int * prev2=&page->pfn$l_flink;
#endif
                prev2++;
#ifdef OLDINT
                *prev2=prev;
#else
                *prev2=previ; // was: prev;
#endif
                // sick compiler
                // it optimized away page->pfn$l_blink=prev
                // or interpreted as flink
            }
#ifdef OLDINT
            prev=page;
#else
            prev=page;
            previ=i;
#endif
            page->pfn$v_loc=PFN$C_FREPAGLST;
            page->pfn$v_pagtyp=PFN$C_SYSTEM;
        }
    }
    prev->pfn$l_flink=0;
#ifdef OLDINT
    pfn$al_tail[PFN$C_FREPAGLST]=prev;
#else
    pfn$al_tail[PFN$C_FREPAGLST]=i; // was: prev;
#endif
    if (0)
    {
        int k,l,m[24];
        l=pfn$al_head[0];
        for(k=0; k<24; k++)
        {
            printk("%lx ",l);
            m[k]=mem_map[l].pfn$l_blink;
            l=mem_map[l].pfn$l_flink;
        }
        printk("\n");
        l=pfn$al_head[0];
        for(k=0; k<24; k++)
        {
            printk("%lx ",m[k]);
        }
        printk("\n");
    }

    pfn$al_head[PFN$C_MFYPAGLST]=0;
    pfn$al_tail[PFN$C_MFYPAGLST]=0;
    pfn$al_head[PFN$C_BADPAGLST]=0;
    pfn$al_tail[PFN$C_BADPAGLST]=0;

    in_free_all_bootmem_core=0;

    return total;
}

unsigned long __init init_bootmem_node (pg_data_t *pgdat, unsigned long freepfn, unsigned long startpfn, unsigned long endpfn)
{
    return(init_bootmem_core(pgdat, freepfn, startpfn, endpfn));
}

void __init reserve_bootmem_node (pg_data_t *pgdat, unsigned long physaddr, unsigned long size)
{
    reserve_bootmem_core(pgdat->bdata, physaddr, size);
}

void __init free_bootmem_node (pg_data_t *pgdat, unsigned long physaddr, unsigned long size)
{
    return(free_bootmem_core(pgdat->bdata, physaddr, size));
}

unsigned long __init free_all_bootmem_node (pg_data_t *pgdat)
{
    return(free_all_bootmem_core(pgdat));
}

unsigned long __init init_bootmem (unsigned long start, unsigned long pages)
{
    max_low_pfn = pages;
    min_low_pfn = start;
    return(init_bootmem_core(&contig_page_data, start, 0, pages));
}

void __init reserve_bootmem (unsigned long addr, unsigned long size)
{
    reserve_bootmem_core(contig_page_data.bdata, addr, size);
}

void __init free_bootmem (unsigned long addr, unsigned long size)
{
    return(free_bootmem_core(contig_page_data.bdata, addr, size));
}

unsigned long __init free_all_bootmem (void)
{
    return(free_all_bootmem_core(&contig_page_data));
}

void * __init __alloc_bootmem (unsigned long size, unsigned long align, unsigned long goal)
{
    pg_data_t *pgdat = pgdat_list;
    void *ptr;

    while (pgdat)
    {
        if ((ptr = __alloc_bootmem_core(pgdat->bdata, size,
                                        align, goal)))
            return(ptr);
        pgdat = pgdat->node_next;
    }
    /*
     * Whoops, we cannot satisfy the allocation request.
     */
    printk(KERN_ALERT "bootmem alloc of %lu bytes failed!\n", size);
    panic("Out of memory");
    return NULL;
}

void * __init __alloc_bootmem_node (pg_data_t *pgdat, unsigned long size, unsigned long align, unsigned long goal)
{
    void *ptr;

    ptr = __alloc_bootmem_core(pgdat->bdata, size, align, goal);
    if (ptr)
        return (ptr);

    /*
     * Whoops, we cannot satisfy the allocation request.
     */
    printk(KERN_ALERT "bootmem alloc of %lu bytes failed!\n", size);
    panic("Out of memory");
    return NULL;
}
