/*  Kernel module help for x86-64
    Copyright (C) 2001 Rusty Russell.
    Copyright (C) 2002,2003 Andi Kleen, SuSE Labs.

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
#define Elf_Ehdr Elf64_Ehdr
#define Elf_Shdr Elf64_Shdr
#define Elf_Sym Elf64_Sym
#define ELF64_R_SYM(i) ((i) >> 32)
#define ELF64_R_TYPE(i) ((i) & 0xffffffff)
#include <linux/moduleloader.h>
#include <linux/elf.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/slab.h>

#include <asm/system.h>
#include <asm/page.h>
#include <asm/pgtable.h>

#define DEBUGP(fmt...)

#ifndef CONFIG_UML
void module_free(struct module *mod, void *module_region)
{
    vfree(module_region);
    /* FIXME: If module_region == mod->init_region, trim exception
           table entries. */
}

void *module_alloc(unsigned long size)
{
    struct vm_struct *area;

    if (!size)
        return NULL;
    size = PAGE_ALIGN(size);
    if (size > MODULES_LEN)
        return NULL;

#if 0
    area = __get_vm_area(size, VM_ALLOC, MODULES_VADDR, MODULES_END);
#else
    area = get_vm_area(size, VM_ALLOC);
#endif
    if (!area)
        return NULL;

#if 0
    return __vmalloc_area(area, GFP_KERNEL, PAGE_KERNEL_EXEC);
#else
    return vmalloc(area);
#endif
}
#endif

/* We don't need anything special. */
int module_frob_arch_sections(Elf_Ehdr *hdr,
                              Elf_Shdr *sechdrs,
                              char *secstrings,
                              struct module *mod)
{
    return 0;
}

int apply_relocate_add(Elf64_Shdr *sechdrs,
                       const char *strtab,
                       unsigned int symindex,
                       unsigned int relsec,
                       struct module *me)
{
    unsigned int i;
    Elf64_Rela *rel = (void *)sechdrs[relsec].sh_addr;
    Elf64_Sym *sym;
    void *loc;
    u64 val;

    DEBUGP("Applying relocate section %u to %u\n", relsec,
           sechdrs[relsec].sh_info);
    for (i = 0; i < sechdrs[relsec].sh_size / sizeof(*rel); i++)
    {
        /* This is where to make the change */
        loc = (void *)sechdrs[sechdrs[relsec].sh_info].sh_addr
              + rel[i].r_offset;

        /* This is the symbol it is referring to.  Note that all
           undefined symbols have been resolved.  */
        sym = (Elf64_Sym *)sechdrs[symindex].sh_addr
              + ELF64_R_SYM(rel[i].r_info);

        DEBUGP("type %d st_value %Lx r_addend %Lx loc %Lx\n",
               (int)ELF64_R_TYPE(rel[i].r_info),
               sym->st_value, rel[i].r_addend, (u64)loc);

        val = sym->st_value + rel[i].r_addend;

        switch (ELF64_R_TYPE(rel[i].r_info))
        {
        case R_X86_64_NONE:
            break;
        case R_X86_64_64:
            *(u64 *)loc = val;
            break;
        case R_X86_64_32:
            *(u32 *)loc = val;
            if (val != *(u32 *)loc)
                goto overflow;
            break;
        case R_X86_64_32S:
            *(s32 *)loc = val;
            if ((s64)val != *(s32 *)loc)
                goto overflow;
            break;
        case R_X86_64_PC32:
            val -= (u64)loc;
            *(u32 *)loc = val;
#if 0
            if ((s64)val != *(s32 *)loc)
                goto overflow;
#endif
            break;
        default:
            printk(KERN_ERR "module %s: Unknown rela relocation: %Lu\n",
                   me->name, ELF64_R_TYPE(rel[i].r_info));
            return -ENOEXEC;
        }
    }
    return 0;

overflow:
    printk(KERN_ERR "overflow in relocation type %d val %Lx\n",
           (int)ELF64_R_TYPE(rel[i].r_info), val);
    printk(KERN_ERR "`%s' likely not compiled with -mcmodel=kernel\n",
           me->name);
    return -ENOEXEC;
}

int apply_relocate(Elf_Shdr *sechdrs,
                   const char *strtab,
                   unsigned int symindex,
                   unsigned int relsec,
                   struct module *me)
{
    printk("non add relocation not supported\n");
    return -ENOSYS;
}

int module_finalize(const Elf_Ehdr *hdr,
                    const Elf_Shdr *sechdrs,
                    struct module *me)
{
    const Elf_Shdr *s, *text = NULL, *alt = NULL, *locks = NULL;
    char *secstrings = (void *)hdr + sechdrs[hdr->e_shstrndx].sh_offset;

    for (s = sechdrs; s < sechdrs + hdr->e_shnum; s++)
    {
        if (!strcmp(".text", secstrings + s->sh_name))
            text = s;
        if (!strcmp(".altinstructions", secstrings + s->sh_name))
            alt = s;
        if (!strcmp(".smp_locks", secstrings + s->sh_name))
            locks= s;
    }

    if (alt)
    {
        /* patch .altinstructions */
        void *aseg = (void *)alt->sh_addr;
#if 0
        apply_alternatives(aseg, aseg + alt->sh_size);
#endif
    }
    if (locks && text)
    {
        void *lseg = (void *)locks->sh_addr;
        void *tseg = (void *)text->sh_addr;
#if 0
        alternatives_smp_module_add(me, me->name,
                                    lseg, lseg + locks->sh_size,
                                    tseg, tseg + text->sh_size);
#endif
    }
    return 0;
}

void module_arch_cleanup(struct module *mod)
{
#if 0
    alternatives_smp_module_del(mod);
#endif
}
