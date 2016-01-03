// $Id$
// $Locker$

// Author. Linux people.
// Author. Roar Thron�s.

// Modified Linux source code 2007

/*
   Copyright (C) 2002 Richard Henderson
   Copyright (C) 2001 Rusty Russell, 2002 Rusty Russell IBM.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#define CONFIG_KALLSYMS
#ifdef __i386__
#define Elf_Ehdr Elf32_Ehdr
#define Elf_Shdr Elf32_Shdr
#define Elf_Sym Elf32_Sym
#else
#define Elf_Ehdr Elf64_Ehdr
#define Elf_Shdr Elf64_Shdr
#define Elf_Sym Elf64_Sym
#endif
#define ELF_ST_BIND(x) ((x) >> 4)
#define ELF_ST_TYPE(x) (((unsigned int) x) & 0xf)
#include <linux/module6.h>

#include <linux/moduleloader.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/seq_file.h>
#include <linux/string.h>
#include <asm/uaccess.h>

#include <crbdef.h>
#include <ddbdef.h>
#include <ddtdef.h>
#include <dptdef.h>
#include <ucbdef.h>

#define ALIGN(x,a) (((x)+(a)-1)&~((a)-1))
#define PERCPU_ENOUGH_ROOM 32768
spinlock_t modlist_lock = SPIN_LOCK_UNLOCKED;
#define __user

#define list_for_each_entry(pos, head, member)                          \
        for (pos = list_entry((head)->next, typeof(*pos), member);      \
             prefetch(pos->member.next), &pos->member != (head);        \
             pos = list_entry(pos->member.next, typeof(*pos), member))

#if 0
#define DEBUGP printk
#else
#define DEBUGP(fmt , a...)
#endif

#ifndef ARCH_SHF_SMALL
#define ARCH_SHF_SMALL 0
#endif

/* If this is set, the section belongs in the init part of the module */
#define INIT_OFFSET_MASK (1UL << (BITS_PER_LONG-1))

/* List of modules */
static LIST_HEAD(modules);

/* We require a truly strong try_module_get() */
static inline int strong_try_module_get(struct module *mod)
{
#if 0
    if (mod && mod->state == MODULE_STATE_COMING)
        return 0;
#endif
    return try_module_get(mod);
}

/* Find a module section: 0 means not found. */
static unsigned int find_sec(Elf_Ehdr *hdr,
                             Elf_Shdr *sechdrs,
                             const char *secstrings,
                             const char *name)
{
    unsigned int i;

    for (i = 1; i < hdr->e_shnum; i++)
        /* Alloc bit cleared means "ignore it." */
        if ((sechdrs[i].sh_flags & SHF_ALLOC)
                && strcmp(secstrings+sechdrs[i].sh_name, name) == 0)
            return i;
    return 0;
}

/* Provided by the linker */
extern const struct kernel_symbol __start___ksymtab[];
extern const struct kernel_symbol __stop___ksymtab[];
#if 0
extern const struct kernel_symbol __start___ksymtab_gpl[];
extern const struct kernel_symbol __stop___ksymtab_gpl[];
extern const struct kernel_symbol __start___ksymtab_gpl_future[];
extern const struct kernel_symbol __stop___ksymtab_gpl_future[];
extern const struct kernel_symbol __start___ksymtab_unused[];
extern const struct kernel_symbol __stop___ksymtab_unused[];
extern const struct kernel_symbol __start___ksymtab_unused_gpl[];
extern const struct kernel_symbol __stop___ksymtab_unused_gpl[];
extern const struct kernel_symbol __start___ksymtab_gpl_future[];
extern const struct kernel_symbol __stop___ksymtab_gpl_future[];
extern const unsigned long __start___kcrctab[];
extern const unsigned long __start___kcrctab_gpl[];
extern const unsigned long __start___kcrctab_gpl_future[];
extern const unsigned long __start___kcrctab_unused[];
extern const unsigned long __start___kcrctab_unused_gpl[];
#endif

#ifndef CONFIG_MODVERSIONS
#define symversion(base, idx) NULL
#else
#define symversion(base, idx) ((base != NULL) ? ((base) + (idx)) : NULL)
#endif

/* lookup symbol in given range of kernel_symbols */
static const struct kernel_symbol *lookup_symbol(const char *name,
        const struct kernel_symbol *start,
        const struct kernel_symbol *stop)
{
    const struct kernel_symbol *ks = start;
    for (; ks < stop; ks++)
        if (strcmp(ks->name, name) == 0)
            return ks;
    return NULL;
}

static void printk_unused_warning(const char *name)
{
    printk(KERN_WARNING "Symbol %s is marked as UNUSED, "
           "however this module is using it.\n", name);
    printk(KERN_WARNING "This symbol will go away in the future.\n");
    printk(KERN_WARNING "Please evalute if this is the right api to use, "
           "and if it really is, submit a report the linux kernel "
           "mailinglist together with submitting your code for "
           "inclusion.\n");
}

/* Find a symbol, return value, crc and module which owns it */
static unsigned long __find_symbol(const char *name,
                                   struct module **owner,
                                   const unsigned long **crc,
                                   int gplok)
{
    struct module *mod;
    const struct kernel_symbol *ks;

    /* Core kernel first. */
    *owner = NULL;
    ks = lookup_symbol(name, __start___ksymtab, __stop___ksymtab);
    if (ks)
    {
#if 0
        *crc = symversion(__start___kcrctab, (ks - __start___ksymtab));
#endif
        return ks->value;
    }
#if 0
    if (gplok)
    {
        ks = lookup_symbol(name, __start___ksymtab_gpl,
                           __stop___ksymtab_gpl);
        if (ks)
        {
            *crc = symversion(__start___kcrctab_gpl,
                              (ks - __start___ksymtab_gpl));
            return ks->value;
        }
    }
    ks = lookup_symbol(name, __start___ksymtab_gpl_future,
                       __stop___ksymtab_gpl_future);
    if (ks)
    {
        if (!gplok)
        {
            printk(KERN_WARNING "Symbol %s is being used "
                   "by a non-GPL module, which will not "
                   "be allowed in the future\n", name);
            printk(KERN_WARNING "Please see the file "
                   "Documentation/feature-removal-schedule.txt "
                   "in the kernel source tree for more "
                   "details.\n");
        }
        *crc = symversion(__start___kcrctab_gpl_future,
                          (ks - __start___ksymtab_gpl_future));
        return ks->value;
    }

    ks = lookup_symbol(name, __start___ksymtab_unused,
                       __stop___ksymtab_unused);
    if (ks)
    {
        printk_unused_warning(name);
        *crc = symversion(__start___kcrctab_unused,
                          (ks - __start___ksymtab_unused));
        return ks->value;
    }

    if (gplok)
        ks = lookup_symbol(name, __start___ksymtab_unused_gpl,
                           __stop___ksymtab_unused_gpl);
    if (ks)
    {
        printk_unused_warning(name);
        *crc = symversion(__start___kcrctab_unused_gpl,
                          (ks - __start___ksymtab_unused_gpl));
        return ks->value;
    }
#endif

    /* Now try modules. */
#if 0
    list_for_each(mod, &modules)
    {
#else
    list_for_each_entry(mod, &modules, list)
    {
#endif
        *owner = mod;
        ks = lookup_symbol(name, mod->syms, mod->syms + mod->num_syms);
        if (ks)
        {
#if 0
            *crc = symversion(mod->crcs, (ks - mod->syms));
#else
            *crc = 0;
#endif
            return ks->value;
        }

#if 0
        if (gplok)
        {
            ks = lookup_symbol(name, mod->gpl_syms,
                               mod->gpl_syms + mod->num_gpl_syms);
            if (ks)
            {
#if 0
                *crc = symversion(mod->gpl_crcs,
                                  (ks - mod->gpl_syms));
#else
                *crc = 0;
#endif
                return ks->value;
            }
        }
#endif
#if 0
        ks = lookup_symbol(name, mod->unused_syms, mod->unused_syms + mod->num_unused_syms);
        if (ks)
        {
            printk_unused_warning(name);
#if 0
            *crc = symversion(mod->unused_crcs, (ks - mod->unused_syms));
#else
            *crc = 0;
#endif
            return ks->value;
        }
#endif

#if 0
        if (gplok)
        {
            ks = lookup_symbol(name, mod->unused_gpl_syms,
                               mod->unused_gpl_syms + mod->num_unused_gpl_syms);
            if (ks)
            {
                printk_unused_warning(name);
#if 0
                *crc = symversion(mod->unused_gpl_crcs,
                                  (ks - mod->unused_gpl_syms));
#else
                *crc = 0;
#endif
                return ks->value;
            }
        }
        ks = lookup_symbol(name, mod->gpl_future_syms,
                           (mod->gpl_future_syms +
                            mod->num_gpl_future_syms));
#endif
        if (ks)
        {
            if (!gplok)
            {
                printk(KERN_WARNING "Symbol %s is being used "
                       "by a non-GPL module, which will not "
                       "be allowed in the future\n", name);
                printk(KERN_WARNING "Please see the file "
                       "Documentation/feature-removal-schedule.txt "
                       "in the kernel source tree for more "
                       "details.\n");
            }
#if 0
            *crc = symversion(mod->gpl_future_crcs,
                              (ks - mod->gpl_future_syms));
#else
            *crc = 0;
#endif
            return ks->value;
        }
    }
    DEBUGP("Failed to find symbol %s\n", name);
    return 0;
}

/* Search for module by name: must hold module_mutex. */
static struct module *find_module(const char *name)
{
    struct module *mod;

#if 0
    list_for_each(mod, &modules)
    {
#else
    list_for_each_entry(mod, &modules, list)
    {
#endif
        if (strcmp(mod->name, name) == 0)
            return mod;
    }
    return NULL;
}

#ifdef CONFIG_SMP
/* Number of blocks used and allocated. */
static unsigned int pcpu_num_used, pcpu_num_allocated;
/* Size of each block.  -ve means used. */
static int *pcpu_size;

static int split_block(unsigned int i, unsigned short size)
{
    /* Reallocation required? */
    if (pcpu_num_used + 1 > pcpu_num_allocated)
    {
        int *new = kmalloc(sizeof(new[0]) * pcpu_num_allocated*2,
                           GFP_KERNEL);
        if (!new)
            return 0;

        memcpy(new, pcpu_size, sizeof(new[0])*pcpu_num_allocated);
        pcpu_num_allocated *= 2;
        kfree(pcpu_size);
        pcpu_size = new;
    }

    /* Insert a new subblock */
    memmove(&pcpu_size[i+1], &pcpu_size[i],
            sizeof(pcpu_size[0]) * (pcpu_num_used - i));
    pcpu_num_used++;

    pcpu_size[i+1] -= size;
    pcpu_size[i] = size;
    return 1;
}

static inline unsigned int block_size(int val)
{
    if (val < 0)
        return -val;
    return val;
}

/* Created by linker magic */
extern char __per_cpu_start[], __per_cpu_end[];

static void *percpu_modalloc(unsigned long size, unsigned long align,
                             const char *name)
{
#if 0
    unsigned long extra;
    unsigned int i;
    void *ptr;

    if (align > SMP_CACHE_BYTES)
    {
        printk(KERN_WARNING "%s: per-cpu alignment %li > %i\n",
               name, align, SMP_CACHE_BYTES);
        align = SMP_CACHE_BYTES;
    }

    ptr = __per_cpu_start;
    for (i = 0; i < pcpu_num_used; ptr += block_size(pcpu_size[i]), i++)
    {
        /* Extra for alignment requirement. */
        extra = ALIGN((unsigned long)ptr, align) - (unsigned long)ptr;
        BUG_ON(i == 0 && extra != 0);

        if (pcpu_size[i] < 0 || pcpu_size[i] < extra + size)
            continue;

        /* Transfer extra to previous block. */
        if (pcpu_size[i-1] < 0)
            pcpu_size[i-1] -= extra;
        else
            pcpu_size[i-1] += extra;
        pcpu_size[i] -= extra;
        ptr += extra;

        /* Split block if warranted */
        if (pcpu_size[i] - size > sizeof(unsigned long))
            if (!split_block(i, size))
                return NULL;

        /* Mark allocated */
        pcpu_size[i] = -pcpu_size[i];
        return ptr;
    }

    printk(KERN_WARNING "Could not allocate %lu bytes percpu data\n",
           size);
#endif
    return NULL;
}

static void percpu_modfree(void *freeme)
{
#if 0
    unsigned int i;
    void *ptr = __per_cpu_start + block_size(pcpu_size[0]);

    /* First entry is core kernel percpu data. */
    for (i = 1; i < pcpu_num_used; ptr += block_size(pcpu_size[i]), i++)
    {
        if (ptr == freeme)
        {
            pcpu_size[i] = -pcpu_size[i];
            goto free;
        }
    }
    BUG();

free:
    /* Merge with previous? */
    if (pcpu_size[i-1] >= 0)
    {
        pcpu_size[i-1] += pcpu_size[i];
        pcpu_num_used--;
        memmove(&pcpu_size[i], &pcpu_size[i+1],
                (pcpu_num_used - i) * sizeof(pcpu_size[0]));
        i--;
    }
    /* Merge with next? */
    if (i+1 < pcpu_num_used && pcpu_size[i+1] >= 0)
    {
        pcpu_size[i] += pcpu_size[i+1];
        pcpu_num_used--;
        memmove(&pcpu_size[i+1], &pcpu_size[i+2],
                (pcpu_num_used - (i+1)) * sizeof(pcpu_size[0]));
    }
#endif
}

static unsigned int find_pcpusec(Elf_Ehdr *hdr,
                                 Elf_Shdr *sechdrs,
                                 const char *secstrings)
{
    return find_sec(hdr, sechdrs, secstrings, ".data.percpu");
}

static int percpu_modinit(void)
{
#if 0
    pcpu_num_used = 2;
    pcpu_num_allocated = 2;
    pcpu_size = kmalloc(sizeof(pcpu_size[0]) * pcpu_num_allocated,
                        GFP_KERNEL);
    /* Static in-kernel percpu data (used). */
    pcpu_size[0] = -ALIGN(__per_cpu_end-__per_cpu_start, SMP_CACHE_BYTES);
    /* Free room. */
    pcpu_size[1] = PERCPU_ENOUGH_ROOM + pcpu_size[0];
    if (pcpu_size[1] < 0)
    {
        printk(KERN_ERR "No per-cpu room for modules.\n");
        pcpu_num_used = 1;
    }
#endif

    return 0;
}
__initcall(percpu_modinit);
#else /* ... !CONFIG_SMP */
static inline void *percpu_modalloc(unsigned long size, unsigned long align,
                                    const char *name)
{
    return NULL;
}
static inline void percpu_modfree(void *pcpuptr)
{
    BUG();
}
static inline unsigned int find_pcpusec(Elf_Ehdr *hdr,
                                        Elf_Shdr *sechdrs,
                                        const char *secstrings)
{
    return 0;
}
static inline void percpu_modcopy(void *pcpudst, const void *src,
                                  unsigned long size)
{
    /* pcpusec should be 0, and size of that section should be 0. */
    BUG_ON(size != 0);
}
#endif /* CONFIG_SMP */

#define MODINFO_ATTR(field) \
static void setup_modinfo_##field(struct module *mod, const char *s)  \
{                                                                     \
    mod->field = kstrdup(s, GFP_KERNEL);                          \
}                                                                     \
static ssize_t show_modinfo_##field(struct module_attribute *mattr,   \
                    struct module *mod, char *buffer)             \
{                                                                     \
    return sprintf(buffer, "%s\n", mod->field);                   \
}                                                                     \
static int modinfo_##field##_exists(struct module *mod)               \
{                                                                     \
    return mod->field != NULL;                                    \
}                                                                     \
static void free_modinfo_##field(struct module *mod)                  \
{                                                                     \
        kfree(mod->field);                                            \
        mod->field = NULL;                                            \
}                                                                     \
static struct module_attribute modinfo_##field = {                    \
    .attr = { .name = __stringify(field), .mode = 0444,           \
          .owner = THIS_MODULE },                             \
    .show = show_modinfo_##field,                                 \
    .setup = setup_modinfo_##field,                               \
    .test = modinfo_##field##_exists,                             \
    .free = free_modinfo_##field,                                 \
};

#if 0
MODINFO_ATTR(version);
MODINFO_ATTR(srcversion);
#endif

static void print_unload_info(struct seq_file *m, struct module *mod)
{
    /* We don't know the usage count, or what modules are using. */
    seq_printf(m, " - -");
}

static inline void module_unload_free(struct module *mod)
{
}

static inline int use_module(struct module *a, struct module *b)
{
    return strong_try_module_get(b);
}

static inline void module_unload_init(struct module *mod)
{
}

#if 0
static struct module_attribute *modinfo_attrs[] =
{
    &modinfo_version,
    &modinfo_srcversion,
    NULL,
};
#endif

#if 0
static const char vermagic[] = VERMAGIC_STRING;
#endif

#ifdef CONFIG_MODVERSIONS
static int check_version(Elf_Shdr *sechdrs,
                         unsigned int versindex,
                         const char *symname,
                         struct module *mod,
                         const unsigned long *crc)
{
    unsigned int i, num_versions;
    struct modversion_info *versions;

    /* Exporting module didn't supply crcs?  OK, we're already tainted. */
    if (!crc)
        return 1;

#if 1
    return 1;
#else
    versions = (void *) sechdrs[versindex].sh_addr;
    num_versions = sechdrs[versindex].sh_size
                   / sizeof(struct modversion_info);

    for (i = 0; i < num_versions; i++)
    {
        if (strcmp(versions[i].name, symname) != 0)
            continue;

        if (versions[i].crc == *crc)
            return 1;
        printk("%s: disagrees about version of symbol %s\n",
               mod->name, symname);
        DEBUGP("Found checksum %lX vs module %lX\n",
               *crc, versions[i].crc);
        return 0;
    }
    /* Not in module's version table.  OK, but that taints the kernel. */
    if (!(tainted & TAINT_FORCED_MODULE))
    {
        printk("%s: no version for \"%s\" found: kernel tainted.\n",
               mod->name, symname);
        add_taint(TAINT_FORCED_MODULE);
    }
    return 1;
#endif
}

static inline int check_modstruct_version(Elf_Shdr *sechdrs,
        unsigned int versindex,
        struct module *mod)
{
    const unsigned long *crc;
    struct module *owner;

    if (!__find_symbol("struct_module", &owner, &crc, 1))
        BUG();
    return check_version(sechdrs, versindex, "struct_module", mod,
                         crc);
}

#else
static inline int check_version(Elf_Shdr *sechdrs,
                                unsigned int versindex,
                                const char *symname,
                                struct module *mod,
                                const unsigned long *crc)
{
    return 1;
}

static inline int check_modstruct_version(Elf_Shdr *sechdrs,
        unsigned int versindex,
        struct module *mod)
{
    return 1;
}

static inline int same_magic(const char *amagic, const char *bmagic)
{
    return strcmp(amagic, bmagic) == 0;
}
#endif /* CONFIG_MODVERSIONS */

/* Resolve a symbol for this module.  I.e. if we find one, record usage.
   Must be holding module_mutex. */
static unsigned long resolve_symbol(Elf_Shdr *sechdrs,
                                    unsigned int versindex,
                                    const char *name,
                                    struct module *mod)
{
    struct module *owner;
    unsigned long ret;
    const unsigned long *crc;

#if 1
    ret = __find_symbol(name, &owner, &crc, 0);
#else
    ret = __find_symbol(name, &owner, &crc, mod->license_gplok);
#endif
    if (ret)
    {
        /* use_module can fail due to OOM, or module unloading */
        if (!check_version(sechdrs, versindex, name, mod, crc) ||
                !use_module(mod, owner))
            ret = 0;
    }
    return ret;
}


/*
 * /sys/module/foo/sections stuff
 * J. Corbet <corbet@lwn.net>
 */
#undef CONFIG_KALLSYMS
#ifdef CONFIG_KALLSYMS
static ssize_t module_sect_show(struct module_attribute *mattr,
                                struct module *mod, char *buf)
{
    struct module_sect_attr *sattr =
        container_of(mattr, struct module_sect_attr, mattr);
    return sprintf(buf, "0x%lx\n", sattr->address);
}

static void add_sect_attrs(struct module *mod, unsigned int nsect,
                           char *secstrings, Elf_Shdr *sechdrs)
{
    unsigned int nloaded = 0, i, size[2];
    struct module_sect_attrs *sect_attrs;
    struct module_sect_attr *sattr;
    struct attribute **gattr;

    /* Count loaded sections and allocate structures */
    for (i = 0; i < nsect; i++)
        if (sechdrs[i].sh_flags & SHF_ALLOC)
            nloaded++;
    size[0] = ALIGN(sizeof(*sect_attrs)
                    + nloaded * sizeof(sect_attrs->attrs[0]),
                    sizeof(sect_attrs->grp.attrs[0]));
    size[1] = (nloaded + 1) * sizeof(sect_attrs->grp.attrs[0]);
    if (! (sect_attrs = kmalloc(size[0] + size[1], GFP_KERNEL)))
        return;

    /* Setup section attributes. */
    sect_attrs->grp.name = "sections";
    sect_attrs->grp.attrs = (void *)sect_attrs + size[0];

    sattr = &sect_attrs->attrs[0];
    gattr = &sect_attrs->grp.attrs[0];
    for (i = 0; i < nsect; i++)
    {
        if (! (sechdrs[i].sh_flags & SHF_ALLOC))
            continue;
        sattr->address = sechdrs[i].sh_addr;
#if 0
        // check
        strlcpy(sattr->name, secstrings + sechdrs[i].sh_name,
                MODULE_SECT_NAME_LEN);
#endif
        sattr->mattr.show = module_sect_show;
        sattr->mattr.store = NULL;
        sattr->mattr.attr.name = sattr->name;
        sattr->mattr.attr.owner = mod;
        sattr->mattr.attr.mode = S_IRUGO;
        *(gattr++) = &(sattr++)->mattr.attr;
    }
    *gattr = NULL;

    if (sysfs_create_group(&mod->mkobj.kobj, &sect_attrs->grp))
        goto out;

    mod->sect_attrs = sect_attrs;
    return;
out:
    kfree(sect_attrs);
}

static void remove_sect_attrs(struct module *mod)
{
    if (mod->sect_attrs)
    {
        sysfs_remove_group(&mod->mkobj.kobj,
                           &mod->sect_attrs->grp);
        /* We are positive that no one is using any sect attrs
         * at this point.  Deallocate immediately. */
        kfree(mod->sect_attrs);
        mod->sect_attrs = NULL;
    }
}


#else
static inline void add_sect_attrs(struct module *mod, unsigned int nsect,
                                  char *sectstrings, Elf_Shdr *sechdrs)
{
}

static inline void remove_sect_attrs(struct module *mod)
{
}
#endif /* CONFIG_KALLSYMS */

static int module_add_modinfo_attrs(struct module *mod)
{
    struct module_attribute *attr;
    struct module_attribute *temp_attr;
    int error = 0;
    int i;

#if 0
    mod->modinfo_attrs = kzalloc((sizeof(struct module_attribute) *
                                  (ARRAY_SIZE(modinfo_attrs) + 1)),
                                 GFP_KERNEL);
    if (!mod->modinfo_attrs)
        return -ENOMEM;

    temp_attr = mod->modinfo_attrs;
    for (i = 0; (attr = modinfo_attrs[i]) && !error; i++)
    {
        if (!attr->test ||
                (attr->test && attr->test(mod)))
        {
            memcpy(temp_attr, attr, sizeof(*temp_attr));
            temp_attr->attr.owner = mod;
            error = sysfs_create_file(&mod->mkobj.kobj,&temp_attr->attr);
            ++temp_attr;
        }
    }
#endif
    return error;
}

static void module_remove_modinfo_attrs(struct module *mod)
{
    struct module_attribute *attr;
    int i;

#if 0
    for (i = 0; (attr = &mod->modinfo_attrs[i]); i++)
    {
        /* pick a field to test for end of list */
        if (!attr->attr.name)
            break;
        sysfs_remove_file(&mod->mkobj.kobj,&attr->attr);
        if (attr->free)
            attr->free(mod);
    }
    kfree(mod->modinfo_attrs);
#endif
}

#if 0
static int mod_sysfs_setup(struct module *mod,
                           struct kernel_param *kparam,
                           unsigned int num_params)
{
    int err;

    if (!module_subsys.kset.subsys)
    {
        printk(KERN_ERR "%s: module_subsys not initialized\n",
               mod->name);
        err = -EINVAL;
        goto out;
    }
    memset(&mod->mkobj.kobj, 0, sizeof(mod->mkobj.kobj));
    err = kobject_set_name(&mod->mkobj.kobj, "%s", mod->name);
    if (err)
        goto out;
    kobj_set_kset_s(&mod->mkobj, module_subsys);
    mod->mkobj.mod = mod;
    err = kobject_register(&mod->mkobj.kobj);
    if (err)
        goto out;

    err = module_param_sysfs_setup(mod, kparam, num_params);
    if (err)
        goto out_unreg;

    err = module_add_modinfo_attrs(mod);
    if (err)
        goto out_unreg;

    return 0;

out_unreg:
    kobject_unregister(&mod->mkobj.kobj);
out:
    return err;
}

static void mod_kobject_remove(struct module *mod)
{
    module_remove_modinfo_attrs(mod);
    module_param_sysfs_remove(mod);

    kobject_unregister(&mod->mkobj.kobj);
}

/*
 * unlink the module with the whole machine is stopped with interrupts off
 * - this defends against kallsyms not taking locks
 */
static int __unlink_module(void *_mod)
{
    struct module *mod = _mod;
    list_del(&mod->list);
    return 0;
}

/* Free a module, remove from lists, etc (must hold module mutex). */
static void free_module(struct module *mod)
{
    /* Delete from various lists */
    stop_machine_run(__unlink_module, mod, NR_CPUS);
    remove_sect_attrs(mod);
    mod_kobject_remove(mod);

    unwind_remove_table(mod->unwind_info, 0);

    /* Arch-specific cleanup. */
    module_arch_cleanup(mod);

    /* Module unload stuff */
    module_unload_free(mod);

    /* This may be NULL, but that's OK */
    module_free(mod, mod->module_init);
    kfree(mod->args);
    if (mod->percpu)
        percpu_modfree(mod->percpu);

    /* Free lock-classes: */
    lockdep_free_key_range(mod->module_core, mod->core_size);

    /* Finally, free the core (containing the module structure) */
    module_free(mod, mod->module_core);
}
#endif

void *__symbol_get(const char *symbol)
{
    struct module *owner;
    unsigned long value, flags;
    const unsigned long *crc;

    spin_lock_irqsave(&modlist_lock, flags);
    value = __find_symbol(symbol, &owner, &crc, 1);
    if (value && !strong_try_module_get(owner))
        value = 0;
    spin_unlock_irqrestore(&modlist_lock, flags);

    return (void *)value;
}
#if 0
EXPORT_SYMBOL_GPL(__symbol_get);
#endif

/*
 * Ensure that an exported symbol [global namespace] does not already exist
 * in the Kernel or in some other modules exported symbol table.
 */
static int verify_export_symbols(struct module *mod)
{
    const char *name = NULL;
    unsigned long i, ret = 0;
    struct module *owner;
    const unsigned long *crc;

    for (i = 0; i < mod->num_syms; i++)
        if (__find_symbol(mod->syms[i].name, &owner, &crc, 1))
        {
            name = mod->syms[i].name;
            ret = -ENOEXEC;
            goto dup;
        }

#if 0
    for (i = 0; i < mod->num_gpl_syms; i++)
        if (__find_symbol(mod->gpl_syms[i].name, &owner, &crc, 1))
        {
            name = mod->gpl_syms[i].name;
            ret = -ENOEXEC;
            goto dup;
        }
#endif

dup:
    if (ret)
        printk(KERN_ERR "%s: exports duplicate symbol %s (owned by %s)\n",
#if 0
               mod->name, name, module_name(owner));
#else
               mod->name, name, "module_name(owner)");
#endif

    return ret;
}

/* Change all symbols so that sh_value encodes the pointer directly. */
static int simplify_symbols(Elf_Shdr *sechdrs,
                            unsigned int symindex,
                            const char *strtab,
                            unsigned int versindex,
                            unsigned int pcpuindex,
                            struct module *mod)
{
    Elf_Sym *sym = (void *)sechdrs[symindex].sh_addr;
    unsigned long secbase;
    unsigned int i, n = sechdrs[symindex].sh_size / sizeof(Elf_Sym);
    int ret = 0;

    for (i = 1; i < n; i++)
    {
        switch (sym[i].st_shndx)
        {
        case SHN_COMMON:
            /* We compiled with -fno-common.  These are not
               supposed to happen.  */
            DEBUGP("Common symbol: %s\n", strtab + sym[i].st_name);
            printk("%s: please compile with -fno-common\n",
                   mod->name);
            ret = -ENOEXEC;
            break;

        case SHN_ABS:
            /* Don't need to do anything */
            DEBUGP("Absolute symbol: 0x%08lx\n",
                   (long)sym[i].st_value);
            break;

        case SHN_UNDEF:
            sym[i].st_value
            = resolve_symbol(sechdrs, versindex,
                             strtab + sym[i].st_name, mod);

            /* Ok if resolved.  */
            if (sym[i].st_value != 0)
                break;
            /* Ok if weak.  */
            if (ELF_ST_BIND(sym[i].st_info) == STB_WEAK)
                break;

            printk(KERN_WARNING "%s: Unknown symbol %s\n",
                   mod->name, strtab + sym[i].st_name);
            ret = -ENOENT;
            break;

        default:
            /* Divert to percpu allocation if a percpu var. */
#if 0
            if (sym[i].st_shndx == pcpuindex)
                secbase = (unsigned long)mod->percpu;
            else
#endif
                secbase = sechdrs[sym[i].st_shndx].sh_addr;
            sym[i].st_value += secbase;
            break;
        }
    }

    return ret;
}

/* Update size with this section: return offset. */
static long get_offset(unsigned long *size, Elf_Shdr *sechdr)
{
    long ret;

    ret = ALIGN(*size, sechdr->sh_addralign ?: 1);
    *size = ret + sechdr->sh_size;
    return ret;
}

/* Lay out the SHF_ALLOC sections in a way not dissimilar to how ld
   might -- code, read-only data, read-write data, small data.  Tally
   sizes, and place the offsets into sh_entsize fields: high bit means it
   belongs in init. */
static void layout_sections(struct module *mod,
                            const Elf_Ehdr *hdr,
                            Elf_Shdr *sechdrs,
                            const char *secstrings)
{
    static unsigned long const masks[][2] =
    {
        /* NOTE: all executable code must be the first section
         * in this array; otherwise modify the text_size
         * finder in the two loops below */
        { SHF_EXECINSTR | SHF_ALLOC, ARCH_SHF_SMALL },
        { SHF_ALLOC, SHF_WRITE | ARCH_SHF_SMALL },
        { SHF_WRITE | SHF_ALLOC, ARCH_SHF_SMALL },
        { ARCH_SHF_SMALL | SHF_ALLOC, 0 }
    };
    unsigned int m, i;

    for (i = 0; i < hdr->e_shnum; i++)
        sechdrs[i].sh_entsize = ~0UL;

    DEBUGP("Core section allocation order:\n");
    for (m = 0; m < ARRAY_SIZE(masks); ++m)
    {
        for (i = 0; i < hdr->e_shnum; ++i)
        {
            Elf_Shdr *s = &sechdrs[i];

            if ((s->sh_flags & masks[m][0]) != masks[m][0]
                    || (s->sh_flags & masks[m][1])
                    || s->sh_entsize != ~0UL
                    || strncmp(secstrings + s->sh_name,
                               ".init", 5) == 0)
                continue;
            s->sh_entsize = get_offset(&mod->core_size, s);
            DEBUGP("\t%s\n", secstrings + s->sh_name);
        }
        if (m == 0)
            mod->core_text_size = mod->core_size;
    }

    DEBUGP("Init section allocation order:\n");
    for (m = 0; m < ARRAY_SIZE(masks); ++m)
    {
        for (i = 0; i < hdr->e_shnum; ++i)
        {
            Elf_Shdr *s = &sechdrs[i];

            if ((s->sh_flags & masks[m][0]) != masks[m][0]
                    || (s->sh_flags & masks[m][1])
                    || s->sh_entsize != ~0UL
                    || strncmp(secstrings + s->sh_name,
                               ".init", 5) != 0)
                continue;
            s->sh_entsize = (get_offset(&mod->init_size, s)
                             | INIT_OFFSET_MASK);
            DEBUGP("\t%s\n", secstrings + s->sh_name);
        }
        if (m == 0)
            mod->init_text_size = mod->init_size;
    }
}

static void set_license(struct module *mod, const char *license)
{
#if 0
    if (!license)
        license = "unspecified";

    mod->license_gplok = license_is_gpl_compatible(license);
    if (!mod->license_gplok && !(tainted & TAINT_PROPRIETARY_MODULE))
    {
        printk(KERN_WARNING "%s: module license '%s' taints kernel.\n",
               mod->name, license);
        add_taint(TAINT_PROPRIETARY_MODULE);
    }
#endif
}

/* Parse tag=value strings from .modinfo section */
static char *next_string(char *string, unsigned long *secsize)
{
    /* Skip non-zero chars */
    while (string[0])
    {
        string++;
        if ((*secsize)-- <= 1)
            return NULL;
    }

    /* Skip any zero padding. */
    while (!string[0])
    {
        string++;
        if ((*secsize)-- <= 1)
            return NULL;
    }
    return string;
}

static char *get_modinfo(Elf_Shdr *sechdrs,
                         unsigned int info,
                         const char *tag)
{
    char *p;
    unsigned int taglen = strlen(tag);
    unsigned long size = sechdrs[info].sh_size;

    for (p = (char *)sechdrs[info].sh_addr; p; p = next_string(p, &size))
    {
        if (strncmp(p, tag, taglen) == 0 && p[taglen] == '=')
            return p + taglen + 1;
    }
    return NULL;
}

#if 0
static void setup_modinfo(struct module *mod, Elf_Shdr *sechdrs,
                          unsigned int infoindex)
{
    struct module_attribute *attr;
    int i;

    for (i = 0; (attr = modinfo_attrs[i]); i++)
    {
        if (attr->setup)
            attr->setup(mod,
                        get_modinfo(sechdrs,
                                    infoindex,
                                    attr->attr.name));
    }
}
#endif

#define CONFIG_KALLSYMS
#ifdef CONFIG_KALLSYMS
int is_exported(const char *name, const struct module *mod)
{
    if (!mod && lookup_symbol(name, __start___ksymtab, __stop___ksymtab))
        return 1;
    else if (mod && lookup_symbol(name, mod->syms, mod->syms + mod->num_syms))
        return 1;
    else
        return 0;
}

/* As per nm */
static char elf_type(const Elf_Sym *sym,
                     Elf_Shdr *sechdrs,
                     const char *secstrings,
                     struct module *mod)
{
    if (ELF_ST_BIND(sym->st_info) == STB_WEAK)
    {
        if (ELF_ST_TYPE(sym->st_info) == STT_OBJECT)
            return 'v';
        else
            return 'w';
    }
    if (sym->st_shndx == SHN_UNDEF)
        return 'U';
    if (sym->st_shndx == SHN_ABS)
        return 'a';
    if (sym->st_shndx >= SHN_LORESERVE)
        return '?';
    if (sechdrs[sym->st_shndx].sh_flags & SHF_EXECINSTR)
        return 't';
    if (sechdrs[sym->st_shndx].sh_flags & SHF_ALLOC
            && sechdrs[sym->st_shndx].sh_type != SHT_NOBITS)
    {
        if (!(sechdrs[sym->st_shndx].sh_flags & SHF_WRITE))
            return 'r';
        else if (sechdrs[sym->st_shndx].sh_flags & ARCH_SHF_SMALL)
            return 'g';
        else
            return 'd';
    }
    if (sechdrs[sym->st_shndx].sh_type == SHT_NOBITS)
    {
        if (sechdrs[sym->st_shndx].sh_flags & ARCH_SHF_SMALL)
            return 's';
        else
            return 'b';
    }
    if (strncmp(secstrings + sechdrs[sym->st_shndx].sh_name,
                ".debug", strlen(".debug")) == 0)
        return 'n';
    return '?';
}

static void add_kallsyms(struct module *mod,
                         Elf_Shdr *sechdrs,
                         unsigned int symindex,
                         unsigned int strindex,
                         const char *secstrings)
{
    unsigned int i;

    mod->symtab = (void *)sechdrs[symindex].sh_addr;
    mod->num_symtab = sechdrs[symindex].sh_size / sizeof(Elf_Sym);
    mod->strtab = (void *)sechdrs[strindex].sh_addr;

    /* Set types up while we still have access to sections. */
    for (i = 0; i < mod->num_symtab; i++)
        mod->symtab[i].st_info
        = elf_type(&mod->symtab[i], sechdrs, secstrings, mod);
}
#else
static inline void add_kallsyms(struct module *mod,
                                Elf_Shdr *sechdrs,
                                unsigned int symindex,
                                unsigned int strindex,
                                const char *secstrings)
{
}
#endif /* CONFIG_KALLSYMS */

static unsigned long mod_find_symname(struct module *mod, const char *name);

/* Allocate and load the module: note that size of section 0 is always
   zero, and we rely on this for optional sections. */
static struct module *load_module(void __user *umod,
                                  unsigned long len,
                                  const char __user *uargs)
{
    Elf_Ehdr *hdr;
    Elf_Shdr *sechdrs;
    char *secstrings, *args, *modmagic, *strtab = NULL;
    unsigned int i;
    unsigned int symindex = 0;
    unsigned int strindex = 0;
    unsigned int setupindex;
    unsigned int exindex;
    unsigned int exportindex;
    unsigned int modindex;
    unsigned int obsparmindex;
    unsigned int infoindex;
    unsigned int gplindex;
    unsigned int crcindex;
    unsigned int gplcrcindex;
    unsigned int versindex;
    unsigned int pcpuindex;
    unsigned int gplfutureindex;
    unsigned int gplfuturecrcindex;
    unsigned int unwindex = 0;
    unsigned int unusedindex;
    unsigned int unusedcrcindex;
    unsigned int unusedgplindex;
    unsigned int unusedgplcrcindex;
    struct module *mod;
    long err = 0;
    void *percpu = NULL, *ptr = NULL; /* Stops spurious gcc warning */
    struct exception_table_entry *extable;
    mm_segment_t old_fs;

    DEBUGP("load_module: umod=%p, len=%lu, uargs=%p\n",
           umod, len, uargs);
    if (len < sizeof(*hdr))
        return ERR_PTR(-ENOEXEC);

    /* Suck in entire file: we'll want most of it. */
    /* vmalloc barfs on "unusual" numbers.  Check here */
#if 0
    if (len > 64 * 1024 * 1024 || (hdr = vmalloc(len)) == NULL)
#else
    if (len > 64 * 1024 * 1024 || (hdr = kmalloc(len, GFP_KERNEL)) == NULL)
#endif
        return ERR_PTR(-ENOMEM);
    if (copy_from_user(hdr, umod, len) != 0)
    {
        err = -EFAULT;
        goto free_hdr;
    }

    /* Sanity checks against insmoding binaries or wrong arch,
           weird elf version */
    if (memcmp(hdr->e_ident, ELFMAG, 4) != 0
            || hdr->e_type != ET_REL
            || !elf_check_arch(hdr)
            || hdr->e_shentsize != sizeof(*sechdrs))
    {
        err = -ENOEXEC;
        goto free_hdr;
    }

    if (len < hdr->e_shoff + hdr->e_shnum * sizeof(Elf_Shdr))
        goto truncated;

    /* Convenience variables */
    sechdrs = (void *)hdr + hdr->e_shoff;
    secstrings = (void *)hdr + sechdrs[hdr->e_shstrndx].sh_offset;
    sechdrs[0].sh_addr = 0;

    for (i = 1; i < hdr->e_shnum; i++)
    {
        if (sechdrs[i].sh_type != SHT_NOBITS
                && len < sechdrs[i].sh_offset + sechdrs[i].sh_size)
            goto truncated;

        /* Mark all sections sh_addr with their address in the
           temporary image. */
        sechdrs[i].sh_addr = (size_t)hdr + sechdrs[i].sh_offset;

        /* Internal symbols and strings. */
        if (sechdrs[i].sh_type == SHT_SYMTAB)
        {
            symindex = i;
            strindex = sechdrs[i].sh_link;
            strtab = (char *)hdr + sechdrs[strindex].sh_offset;
        }
        /* Don't load .exit sections */
        if (strncmp(secstrings+sechdrs[i].sh_name, ".exit", 5) == 0)
            sechdrs[i].sh_flags &= ~(unsigned long)SHF_ALLOC;
    }

#if 0
    modindex = find_sec(hdr, sechdrs, secstrings,
                        ".gnu.linkonce.this_module");
    if (!modindex)
    {
        printk(KERN_WARNING "No module found in object\n");
        err = -ENOEXEC;
        goto free_hdr;
    }
    mod = (void *)sechdrs[modindex].sh_addr;
#else
    mod = kmalloc(sizeof(struct module), GFP_KERNEL);
    memset(mod, 0, sizeof(struct module));
#endif

    if (symindex == 0)
    {
        printk(KERN_WARNING "%s: module has no symbols (stripped?)\n",
               mod->name);
        err = -ENOEXEC;
        goto free_hdr;
    }

    /* Optional sections */
    exportindex = find_sec(hdr, sechdrs, secstrings, "__ksymtab");
    gplindex = find_sec(hdr, sechdrs, secstrings, "__ksymtab_gpl");
    gplfutureindex = find_sec(hdr, sechdrs, secstrings, "__ksymtab_gpl_future");
    unusedindex = find_sec(hdr, sechdrs, secstrings, "__ksymtab_unused");
    unusedgplindex = find_sec(hdr, sechdrs, secstrings, "__ksymtab_unused_gpl");
    crcindex = find_sec(hdr, sechdrs, secstrings, "__kcrctab");
    gplcrcindex = find_sec(hdr, sechdrs, secstrings, "__kcrctab_gpl");
    gplfuturecrcindex = find_sec(hdr, sechdrs, secstrings, "__kcrctab_gpl_future");
    unusedcrcindex = find_sec(hdr, sechdrs, secstrings, "__kcrctab_unused");
    unusedgplcrcindex = find_sec(hdr, sechdrs, secstrings, "__kcrctab_unused_gpl");
    setupindex = find_sec(hdr, sechdrs, secstrings, "__param");
    exindex = find_sec(hdr, sechdrs, secstrings, "__ex_table");
    obsparmindex = find_sec(hdr, sechdrs, secstrings, "__obsparm");
    versindex = find_sec(hdr, sechdrs, secstrings, "__versions");
    infoindex = find_sec(hdr, sechdrs, secstrings, ".modinfo");
    pcpuindex = find_pcpusec(hdr, sechdrs, secstrings);

    /* Don't keep modinfo section */
    sechdrs[infoindex].sh_flags &= ~(unsigned long)SHF_ALLOC;
#ifdef CONFIG_KALLSYMS
    /* Keep symbol and string tables for decoding later. */
    sechdrs[symindex].sh_flags |= SHF_ALLOC;
    sechdrs[strindex].sh_flags |= SHF_ALLOC;
#endif
    if (unwindex)
        sechdrs[unwindex].sh_flags |= SHF_ALLOC;

    /* Check module struct version now, before we try to use module. */
    if (!check_modstruct_version(sechdrs, versindex, mod))
    {
        err = -ENOEXEC;
        goto free_hdr;
    }

    modmagic = get_modinfo(sechdrs, infoindex, "vermagic");
    /* This is allowed: modprobe --force will invalidate it. */
#if 0
    if (!modmagic)
    {
        add_taint(TAINT_FORCED_MODULE);
        printk(KERN_WARNING "%s: no version magic, tainting kernel.\n",
               mod->name);
    }
    else if (!same_magic(modmagic, vermagic))
    {
        printk(KERN_ERR "%s: version magic '%s' should be '%s'\n",
               mod->name, modmagic, vermagic);
        err = -ENOEXEC;
        goto free_hdr;
    }
#endif

    /* Now copy in args */
#if 0
    args = strndup_user(uargs, ~0UL >> 1);
#else
    args = uargs;
#endif
    if (IS_ERR(args))
    {
        err = PTR_ERR(args);
        goto free_hdr;
    }

#if 0
    if (find_module(mod->name))
    {
        err = -EEXIST;
        goto free_mod;
    }
#else
    memcpy (mod->name, "tempmodname", 11);
#endif

    mod->state = MODULE_STATE_COMING;

    /* Allow arches to frob section contents and sizes.  */
    err = module_frob_arch_sections(hdr, sechdrs, secstrings, mod);
    if (err < 0)
        goto free_mod;

    if (pcpuindex)
    {
        /* We have a special allocation for this section. */
        percpu = percpu_modalloc(sechdrs[pcpuindex].sh_size,
                                 sechdrs[pcpuindex].sh_addralign,
                                 mod->name);
        if (!percpu)
        {
            err = -ENOMEM;
            goto free_mod;
        }
        sechdrs[pcpuindex].sh_flags &= ~(unsigned long)SHF_ALLOC;
        mod->percpu = percpu;
    }

    /* Determine total sizes, and put offsets in sh_entsize.  For now
       this is done generically; there doesn't appear to be any
       special cases for the architectures. */
    layout_sections(mod, hdr, sechdrs, secstrings);

    /* Do the allocs. */
    ptr = module_alloc(mod->core_size);
    if (!ptr)
    {
        err = -ENOMEM;
        goto free_percpu;
    }
    memset(ptr, 0, mod->core_size);
    mod->module_core = ptr;

    ptr = module_alloc(mod->init_size);
    if (!ptr && mod->init_size)
    {
        err = -ENOMEM;
        goto free_core;
    }
    memset(ptr, 0, mod->init_size);
    mod->module_init = ptr;

    /* Transfer each section which specifies SHF_ALLOC */
    DEBUGP("final section addresses:\n");
    for (i = 0; i < hdr->e_shnum; i++)
    {
        void *dest;

        if (!(sechdrs[i].sh_flags & SHF_ALLOC))
            continue;

        if (sechdrs[i].sh_entsize & INIT_OFFSET_MASK)
            dest = mod->module_init
                   + (sechdrs[i].sh_entsize & ~INIT_OFFSET_MASK);
        else
            dest = mod->module_core + sechdrs[i].sh_entsize;

        if (sechdrs[i].sh_type != SHT_NOBITS)
            memcpy(dest, (void *)sechdrs[i].sh_addr,
                   sechdrs[i].sh_size);
        /* Update sh_addr to point to copy in image. */
        sechdrs[i].sh_addr = (unsigned long)dest;
        DEBUGP("\t0x%lx %s\n", sechdrs[i].sh_addr, secstrings + sechdrs[i].sh_name);
    }
    /* Module has been moved. */
#if 0
    mod = (void *)sechdrs[modindex].sh_addr;
#endif

    /* Now we've moved module, initialize linked lists, etc. */
    module_unload_init(mod);

    /* Set up license info based on the info section */
#if 0
    set_license(mod, get_modinfo(sechdrs, infoindex, "license"));

    if (strcmp(mod->name, "ndiswrapper") == 0)
        add_taint(TAINT_PROPRIETARY_MODULE);
    if (strcmp(mod->name, "driverloader") == 0)
        add_taint(TAINT_PROPRIETARY_MODULE);
#endif

#if 0
    /* Set up MODINFO_ATTR fields */
    setup_modinfo(mod, sechdrs, infoindex);
#endif

    /* Fix up syms, so that st_value is a pointer to location. */
    err = simplify_symbols(sechdrs, symindex, strtab, versindex, pcpuindex,
                           mod);
    if (err < 0)
        goto cleanup;

    /* Set up EXPORTed & EXPORT_GPLed symbols (section 0 is 0 length) */
    mod->num_syms = sechdrs[exportindex].sh_size / sizeof(*mod->syms);
    mod->syms = (void *)sechdrs[exportindex].sh_addr;
#if 0
    if (crcindex)
        mod->crcs = (void *)sechdrs[crcindex].sh_addr;
    mod->num_gpl_syms = sechdrs[gplindex].sh_size / sizeof(*mod->gpl_syms);
    mod->gpl_syms = (void *)sechdrs[gplindex].sh_addr;
    if (gplcrcindex)
        mod->gpl_crcs = (void *)sechdrs[gplcrcindex].sh_addr;
    mod->num_gpl_future_syms = sechdrs[gplfutureindex].sh_size /
                               sizeof(*mod->gpl_future_syms);
#endif
#if 0
    mod->num_unused_syms = sechdrs[unusedindex].sh_size /
                           sizeof(*mod->unused_syms);
#endif
#if 0
    mod->num_unused_gpl_syms = sechdrs[unusedgplindex].sh_size /
                               sizeof(*mod->unused_gpl_syms);
    mod->gpl_future_syms = (void *)sechdrs[gplfutureindex].sh_addr;
    if (gplfuturecrcindex)
        mod->gpl_future_crcs = (void *)sechdrs[gplfuturecrcindex].sh_addr;
#endif

#if 0
    mod->unused_syms = (void *)sechdrs[unusedindex].sh_addr;
#endif
#if 0
    if (unusedcrcindex)
        mod->unused_crcs = (void *)sechdrs[unusedcrcindex].sh_addr;
    mod->unused_gpl_syms = (void *)sechdrs[unusedgplindex].sh_addr;
    if (unusedgplcrcindex)
        mod->unused_crcs = (void *)sechdrs[unusedgplcrcindex].sh_addr;
#endif

#ifdef CONFIG_MODVERSIONS
    if ((mod->num_syms && !crcindex) ||
#if 0
            (mod->num_gpl_syms && !gplcrcindex) ||
            (mod->num_gpl_future_syms && !gplfuturecrcindex) ||
            (mod->num_unused_syms && !unusedcrcindex) ||
            (mod->num_unused_gpl_syms && !unusedgplcrcindex))
    {
#else
            0)
    {
#endif
#if 0
        printk(KERN_WARNING "%s: No versions for exported symbols."
               " Tainting kernel.\n", mod->name);
        add_taint(TAINT_FORCED_MODULE);
#endif
    }
#endif

    /* Now do relocations. */
    for (i = 1;
            i < hdr->e_shnum;
            i++)
    {
        const char *strtab = (char *)sechdrs[strindex].sh_addr;
        unsigned int info = sechdrs[i].sh_info;

        /* Not a valid relocation section? */
        if (info >= hdr->e_shnum)
            continue;

        /* Don't bother with non-allocated sections */
        if (!(sechdrs[info].sh_flags & SHF_ALLOC))
            continue;

        if (sechdrs[i].sh_type == SHT_REL)
            err = apply_relocate(sechdrs, strtab, symindex, i,mod);
        else if (sechdrs[i].sh_type == SHT_RELA)
            err = apply_relocate_add(sechdrs, strtab, symindex, i,
                                     mod);
        if (err < 0)
            goto cleanup;
    }

    /* Find duplicate symbols */
    err = verify_export_symbols(mod);

    if (err < 0)
        goto cleanup;

    /* Set up and sort exception table */
    mod->num_exentries = sechdrs[exindex].sh_size / sizeof(*mod->extable);
    mod->extable = extable = (void *)sechdrs[exindex].sh_addr;
#if 0
    sort_extable(extable, extable + mod->num_exentries);
#endif

#if 0
    /* Finally, copy percpu area over. */
    percpu_modcopy(mod->percpu, (void *)sechdrs[pcpuindex].sh_addr,
                   sechdrs[pcpuindex].sh_size);
#endif

    add_kallsyms(mod, sechdrs, symindex, strindex, secstrings);

#if 1
    mod->init = mod_find_symname (mod, "driver$init_tables");
#endif

#if 1
    if (find_module(mod->name))
    {
        err = -EEXIST;
        goto free_mod;
    }
#endif
    err = module_finalize(hdr, sechdrs, mod);
    if (err < 0)
        goto cleanup;

    /* flush the icache in correct context */
    old_fs = get_fs();
    set_fs(KERNEL_DS);

    /*
     * Flush the instruction cache, since we've played with text.
     * Do it before processing of module parameters, so the module
     * can provide parameter accessor functions of its own.
     */
    if (mod->module_init)
        flush_icache_range((unsigned long)mod->module_init,
                           (unsigned long)mod->module_init
                           + mod->init_size);
    flush_icache_range((unsigned long)mod->module_core,
                       (unsigned long)mod->module_core + mod->core_size);

    set_fs(old_fs);

    mod->args = args;
    if (obsparmindex)
        printk(KERN_WARNING "%s: Ignoring obsolete parameters\n",
               mod->name);

#if 0
    /* Size of section 0 is 0, so this works well if no params */
    err = parse_args(mod->name, mod->args,
                     (struct kernel_param *)
                     sechdrs[setupindex].sh_addr,
                     sechdrs[setupindex].sh_size
                     / sizeof(struct kernel_param),
                     NULL);
    if (err < 0)
        goto arch_cleanup;
#endif

#if 0
    err = mod_sysfs_setup(mod,
                          (struct kernel_param *)
                          sechdrs[setupindex].sh_addr,
                          sechdrs[setupindex].sh_size
                          / sizeof(struct kernel_param));
    if (err < 0)
        goto arch_cleanup;
    add_sect_attrs(mod, hdr->e_shnum, secstrings, sechdrs);

    /* Size of section 0 is 0, so this works well if no unwind info. */
    mod->unwind_info = unwind_add_table(mod,
                                        (void *)sechdrs[unwindex].sh_addr,
                                        sechdrs[unwindex].sh_size);
#endif

    /* Get rid of temporary copy */
#if 0
    vfree(hdr);
#else
    kfree(hdr);
#endif

    /* Done! */
    return mod;

arch_cleanup:
    module_arch_cleanup(mod);
cleanup:
    module_unload_free(mod);
    module_free(mod, mod->module_init);
free_core:
    module_free(mod, mod->module_core);
free_percpu:
    if (percpu)
        percpu_modfree(percpu);
free_mod:
#if 0
    kfree(args);
#endif
free_hdr:
#if 0
    vfree(hdr);
#else
    kfree(hdr);
#endif
    return ERR_PTR(err);

truncated:
    printk(KERN_ERR "Module len %lu truncated\n", len);
    err = -ENOEXEC;
    goto free_hdr;
}

/*
 * link the module with the whole machine is stopped with interrupts off
 * - this defends against kallsyms not taking locks
 */
static int __link_module(void *_mod)
{
    struct module *mod = _mod;
    list_add(&mod->list, &modules);
    return 0;
}

/* This is where the real work happens */
asmlinkage long sys_init_module(void __user *umod, unsigned long len, const char __user *uargs)
{
    struct module *mod;
    int ret = 0;

    /* Must have permission */
    if (!capable(CAP_SYS_MODULE))
        return -EPERM;

    /* Only one module load at a time, please */
#if 0
    if (mutex_lock_interruptible(&module_mutex) != 0)
        return -EINTR;
#endif

    /* Do all the hard work */
    mod = load_module(umod, len, uargs);
    if (IS_ERR(mod))
    {
#if 0
        mutex_unlock(&module_mutex);
#endif
        return PTR_ERR(mod);
    }

    /* Now sew it into the lists.  They won't access us, since
           strong_try_module_get() will fail. */
#if 0
    stop_machine_run(__link_module, mod, NR_CPUS);
#endif

    /* Drop lock so they can recurse */
#if 0
    mutex_unlock(&module_mutex);
#endif

#if 0
    blocking_notifier_call_chain(&module_notify_list,
                                 MODULE_STATE_COMING, mod);
#endif

    /* Start the module */
#if 0
    if (mod->init != NULL)
        ret = mod->init();
#else
    printk("did mod->init at %lx %lx\n", (unsigned long)mod, (unsigned long)mod->init);
    if (mod->init != NULL)
    {
#if 0
        ret = mod->init();
        printk("did mod->init at %lx %lx\n", (unsigned long)mod, (unsigned long)mod->init);
#endif
        struct _ddt * ddt;
        struct _dpt * dpt;
        struct _dpt * fdt;
        ddt = mod_find_symname (mod, "driver$ddt");
        dpt = mod_find_symname (mod, "driver$dpt");
        fdt = mod_find_symname (mod, "driver$fdt");
        printk("ddt dpt %lx %lx\n", (unsigned long)ddt, (unsigned long)dpt);
        int load_driver_inner(int (*init_tables)(), struct _ddt * ddt, struct _dpt * dpt, struct _fdt * fdt);
        load_driver_inner(mod->init, ddt, dpt, fdt);
    }
#endif
    if (ret < 0)
    {
        /* Init routine failed: abort.  Try to protect us from
                   buggy refcounters. */
        mod->state = MODULE_STATE_GOING;
#if 0
        synchronize_sched();
#endif
        if (mod->unsafe)
            printk(KERN_ERR "%s: module is now stuck!\n",
                   mod->name);
        else
        {
            module_put(mod);
#if 0
            mutex_lock(&module_mutex);
#endif
#if 0
            free_module(mod);
#endif
#if 0
            mutex_unlock(&module_mutex);
#endif
        }
        return ret;
    }

    /* Now it's a first class citizen! */
#if 0
    mutex_lock(&module_mutex);
#endif
    mod->state = MODULE_STATE_LIVE;
    /* Drop initial reference. */
    module_put(mod);
#if 0
    unwind_remove_table(mod->unwind_info, 1);
#endif
    module_free(mod, mod->module_init);
    mod->module_init = NULL;
    mod->init_size = 0;
    mod->init_text_size = 0;
#if 0
    mutex_unlock(&module_mutex);
#endif

    return 0;
}

static inline int within(unsigned long addr, void *start, unsigned long size)
{
    return ((void *)addr >= start && (void *)addr < start + size);
}

#ifdef CONFIG_KALLSYMS
static const char *get_ksymbol(struct module *mod,
                               unsigned long addr,
                               unsigned long *size,
                               unsigned long *offset)
{
    unsigned int i, best = 0;
    unsigned long nextval;

    /* At worse, next value is at end of module */
    if (within(addr, mod->module_init, mod->init_size))
        nextval = (unsigned long)mod->module_init+mod->init_text_size;
    else
        nextval = (unsigned long)mod->module_core+mod->core_text_size;

    /* Scan for closest preceeding symbol, and next symbol. (ELF
           starts real symbols at 1). */
    for (i = 1; i < mod->num_symtab; i++)
    {
        if (mod->symtab[i].st_shndx == SHN_UNDEF)
            continue;

        /* We ignore unnamed symbols: they're uninformative
         * and inserted at a whim. */
        if (mod->symtab[i].st_value <= addr
                && mod->symtab[i].st_value > mod->symtab[best].st_value
                && *(mod->strtab + mod->symtab[i].st_name) != '\0')
            best = i;
        if (mod->symtab[i].st_value > addr
                && mod->symtab[i].st_value < nextval
                && *(mod->strtab + mod->symtab[i].st_name) != '\0')
            nextval = mod->symtab[i].st_value;
    }

    if (!best)
        return NULL;

    *size = nextval - mod->symtab[best].st_value;
    *offset = addr - mod->symtab[best].st_value;
    return mod->strtab + mod->symtab[best].st_name;
}

/* For kallsyms to ask for address resolution.  NULL means not found.
   We don't lock, as this is used for oops resolution and races are a
   lesser concern. */
const char *module_address_lookup(unsigned long addr,
                                  unsigned long *size,
                                  unsigned long *offset,
                                  char **modname)
{
    struct module *mod;

#if 0
    list_for_each(mod, &modules)
    {
#else
    list_for_each_entry(mod, &modules, list)
    {
#endif
        if (within(addr, mod->module_init, mod->init_size)
                || within(addr, mod->module_core, mod->core_size))
        {
            *modname = mod->name;
            return get_ksymbol(mod, addr, size, offset);
        }
    }
    return NULL;
}

struct module *module_get_kallsym(unsigned int symnum, unsigned long *value,
                                  char *type, char *name, size_t namelen)
{
    struct module *mod;

#if 0
    mutex_lock(&module_mutex);
#endif
#if 0
    list_for_each(mod, &modules)
    {
#else
    list_for_each_entry(mod, &modules, list)
    {
#endif
        if (symnum < mod->num_symtab)
        {
            *value = mod->symtab[symnum].st_value;
            *type = mod->symtab[symnum].st_info;
#if 0
            // check
            strlcpy(name, mod->strtab + mod->symtab[symnum].st_name,
                    namelen);
#endif
#if 0
            mutex_unlock(&module_mutex);
#endif
            return mod;
        }
        symnum -= mod->num_symtab;
    }
#if 0
    mutex_unlock(&module_mutex);
#endif
    return NULL;
}

static unsigned long mod_find_symname(struct module *mod, const char *name)
{
    unsigned int i;

    for (i = 0; i < mod->num_symtab; i++)
        if (strcmp(name, mod->strtab+mod->symtab[i].st_name) == 0 &&
                mod->symtab[i].st_info != 'U')
            return mod->symtab[i].st_value;
    return 0;
}

/* Look for this name: can be of form module:name. */
unsigned long module_kallsyms_lookup_name(const char *name)
{
    struct module *mod;
    char *colon;
    unsigned long ret = 0;

    /* Don't lock: we're in enough trouble already. */
    if ((colon = strchr(name, ':')) != NULL)
    {
        *colon = '\0';
        if ((mod = find_module(name)) != NULL)
            ret = mod_find_symname(mod, colon+1);
        *colon = ':';
    }
    else
    {
#if 0
        list_for_each(mod, &modules)
#else
        list_for_each_entry(mod, &modules, list)
#endif
        if ((ret = mod_find_symname(mod, name)) != 0)
            break;
    }
    return ret;
}
#endif /* CONFIG_KALLSYMS */

/* Called by the /proc file system to return a list of modules. */
static void *m_start(struct seq_file *m, loff_t *pos)
{
    struct list_head *i;
    loff_t n = 0;

#if 0
    mutex_lock(&module_mutex);
#endif
    list_for_each(i, &modules)
    {
        if (n++ == *pos)
            break;
    }
    if (i == &modules)
        return NULL;
    return i;
}

static void *m_next(struct seq_file *m, void *p, loff_t *pos)
{
    struct list_head *i = p;
    (*pos)++;
    if (i->next == &modules)
        return NULL;
    return i->next;
}

static void m_stop(struct seq_file *m, void *p)
{
#if 0
    mutex_unlock(&module_mutex);
#endif
}

static int m_show(struct seq_file *m, void *p)
{
    struct module *mod = list_entry(p, struct module, list);
    seq_printf(m, "%s %lu",
               mod->name, mod->init_size + mod->core_size);
    print_unload_info(m, mod);

    /* Informative for users. */
    seq_printf(m, " %s",
               mod->state == MODULE_STATE_GOING ? "Unloading":
               mod->state == MODULE_STATE_COMING ? "Loading":
               "Live");
    /* Used by oprofile and other similar tools. */
    seq_printf(m, " 0x%p", mod->module_core);

    seq_printf(m, "\n");
    return 0;
}

/* Format: modulename size refcount deps address

   Where refcount is a number or -, and deps is a comma-separated list
   of depends or -.
*/
struct seq_operations modules_op =
{
    .start  = m_start,
    .next   = m_next,
    .stop   = m_stop,
    .show   = m_show
};

/* Given an address, look for it in the module exception tables. */
const struct exception_table_entry *search_module_extables(unsigned long addr)
{
    unsigned long flags;
    const struct exception_table_entry *e = NULL;
    struct module *mod;

    spin_lock_irqsave(&modlist_lock, flags);
#if 0
    list_for_each(mod, &modules)
    {
#else
    list_for_each_entry(mod, &modules, list)
    {
#endif
        if (mod->num_exentries == 0)
            continue;

#if 0
        e = search_extable(mod->extable,
                           mod->extable + mod->num_exentries - 1,
                           addr);
        if (e)
            break;
#endif
    }
    spin_unlock_irqrestore(&modlist_lock, flags);

    /* Now, if we found one, we are running inside it now, hence
           we cannot unload the module, hence no refcnt needed. */
    return e;
}

/*
 * Is this a valid module address?
 */
int is_module_address(unsigned long addr)
{
    unsigned long flags;
    struct module *mod;

    spin_lock_irqsave(&modlist_lock, flags);

#if 0
    list_for_each(mod, &modules)
    {
#else
    list_for_each_entry(mod, &modules, list)
    {
#endif
        if (within(addr, mod->module_core, mod->core_size))
        {
            spin_unlock_irqrestore(&modlist_lock, flags);
            return 1;
        }
    }

    spin_unlock_irqrestore(&modlist_lock, flags);

    return 0;
}


/* Is this a valid kernel address?  We don't grab the lock: we are oopsing. */
struct module *__module_text_address(unsigned long addr)
{
    struct module *mod;

#if 0
    list_for_each(mod, &modules)
#else
    list_for_each_entry(mod, &modules, list)
#endif
    if (within(addr, mod->module_init, mod->init_text_size)
            || within(addr, mod->module_core, mod->core_text_size))
        return mod;
    return NULL;
}

struct module *module_text_address(unsigned long addr)
{
    struct module *mod;
    unsigned long flags;

    spin_lock_irqsave(&modlist_lock, flags);
    mod = __module_text_address(addr);
    spin_unlock_irqrestore(&modlist_lock, flags);

    return mod;
}

/* Don't grab lock, we're oopsing. */
void print_modules(void)
{
    struct module *mod;

    printk("Modules linked in:");
#if 0
    list_for_each(mod, &modules)
#else
    list_for_each_entry(mod, &modules, list)
#endif
    printk(" %s", mod->name);
    printk("\n");
}

#if 0
void module_add_driver(struct module *mod, struct device_driver *drv)
{
    if (!mod || !drv)
        return;

    /* Don't check return code; this call is idempotent */
    sysfs_create_link(&drv->kobj, &mod->mkobj.kobj, "module");
}
EXPORT_SYMBOL(module_add_driver);

void module_remove_driver(struct device_driver *drv)
{
    if (!drv)
        return;
    sysfs_remove_link(&drv->kobj, "module");
}
EXPORT_SYMBOL(module_remove_driver);
#endif

#ifdef CONFIG_MODVERSIONS
/* Generate the signature for struct module here, too, for modversions. */
void struct_module(struct module *mod)
{
    return;
}
EXPORT_SYMBOL(struct_module);
#endif

asmlinkage unsigned long sys_create_module(const char *name_user, size_t size)
{
    return -ENOSYS;
}

#if 0
asmlinkage long sys_init_module(const char *name_user, struct module *mod_user)
{
    return -ENOSYS;
}
#endif

asmlinkage long sys_delete_module(const char *name_user)
{
    return -ENOSYS;
}

asmlinkage long sys_query_module(const char *name_user, int which, char *buf, size_t bufsize,
                 size_t *ret)
{
    /* Let the program know about the new interface.  Not that
       it'll do them much good.  */
    if (which == 0)
        return 0;

    return -ENOSYS;
}

asmlinkage long sys_get_kernel_syms(struct kernel_sym *table)
{
    return -ENOSYS;
}

int try_inc_mod_count(struct module *mod)
{
    return 1;
}

void inter_module_register(const char *im_name, struct module *owner, const void *userdata)
{
}

void inter_module_unregister(const char *im_name)
{
}

void inter_module_put(const char *im_name)
{
}

const void *inter_module_get(const char *im_name)
{
}

const void *inter_module_get_request(const char *im_name, const char *modname)
{
}

// move to iogen dir

int iogen$load(void)
{
}

int iogen$connect(void)
{
}

int iogen$init_db(struct _ddb * ddb, struct _ddt * ddt, struct _dpt * dpt, struct _fdt * fdt)
{
    ddb->ddb$ps_dpt = dpt;
    ddt->ddt$l_fdt = fdt;
}

int iogen$init_mapping(void)
{
}

int iogen$init_controller(struct _crb *crb, struct _ddb *ddb)
{
    int ioc$ctrl_init();
    ioc$ctrl_init(crb, ddb);
}

int iogen$init_unit(struct _ucb * ucb)
{
    int ioc$unit_init();
    ioc$unit_init(ucb);
}

int load_driver_inner(int (*init_tables)(), struct _ddt * ddt, struct _dpt * dpt, struct _fdt * fdt)
{
    int init_crb();
    int init_ddb();
    int init_ucb();

    init_tables();

    char devname[9]; // check. use as parameter some time
    devname[0] = dpt->dpt$t_name[1];
    devname[1] = dpt->dpt$t_name[2];
    devname[2] = 'a';
    devname[3] = 0;
    struct _ddb * ddb=kmalloc(sizeof(struct _ddb),GFP_KERNEL);
    struct _crb * crb=kmalloc(sizeof(struct _crb),GFP_KERNEL);
    unsigned long idb=0,orb=0;

    memset(ddb,0,sizeof(struct _ddb));
    memset(crb,0,sizeof(struct _crb));

    init_ddb(ddb,ddt, 0/*ucb*/, devname);
    init_crb(crb);

    int iogen$init_db();
    int iogen$init_controller();
    int iogen$init_unit();
    iogen$init_db(ddb, ddt, dpt, fdt);
    iogen$init_controller(crb, ddb);
    int count = crb->crb$l_refc;
    //      count = idb->idb$w_maxunits;
    count = dpt->dpt$w_defunits;
    int i;
    ddb->ddb$ps_ucb = 0; // check. remove later
    for (i = 0; i < count; i++)
    {
        int size = sizeof(struct _ucb);
        if (dpt->dpt$w_ucbsize)
            size = dpt->dpt$w_ucbsize;
        struct _ucb * ucb=kmalloc(size, GFP_KERNEL);
        memset(ucb, 0, size);
        init_ucb(ucb, ddb, ddt, crb);
        ucb->ucb$w_size = size;
        if (i > 0)   // check. temp fix.
        {
            ucb->ucb$l_sts &= ~UCB$M_TEMPLATE;
            ucb->ucb$w_unit = 0;
        }
        iogen$init_unit(ucb);
        if (ddb->ddb$ps_ucb == 0)
            ddb->ddb$ps_ucb = ucb;
        else
        {
            struct _ucb ** ucbhead = &ucb->ucb$l_link;
            for (; *ucbhead; ucbhead = &(*ucbhead)->ucb$l_link) ;
            *ucbhead = ucb;
        }
    }
    int insertdevlist();
    insertdevlist(ddb);
    return ddb;
}

asmlinkage int exe$load_driver(int code, void * dev_dsc, void * name_dsc, void * list, void * iosb, int efn, int unk1, int unk2)
{
    iogen$load();
    iogen$connect();
    long ddb = 0, ucb = 0;
    iogen$init_db(ddb, 0, 0, 0);
    iogen$init_mapping();
    iogen$init_controller(0, ddb);
    iogen$init_unit(ucb);
}
