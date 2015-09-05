// $Id$
// $Locker$

// Author. Roar Thron�s.
// Modified Linux source file, 2001-2004.

/*
 *  linux/fs/ext2/dir.c
 *
 * Copyright (C) 1992, 1993, 1994, 1995
 * Remy Card (card@masi.ibp.fr)
 * Laboratoire MASI - Institut Blaise Pascal
 * Universite Pierre et Marie Curie (Paris VI)
 *
 *  from
 *
 *  linux/fs/minix/dir.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 *
 *  ext2 directory handling functions
 *
 *  Big-endian to little-endian byte-swapping/bitmaps by
 *        David S. Miller (davem@caip.rutgers.edu), 1995
 *
 * All code that works with directory layout had been switched to pagecache
 * and moved here. AV
 */

#include <linux/fs.h>
#include <linux/pagemap.h>

#include <misc_routines.h>
#include <vcbdef.h>

typedef struct ext2_dir_entry_2 ext2_dirent;

extern struct address_space_operations ext2_aops;

/*
 * ext2 uses block-sized chunks. Arguably, sector-sized ones would be
 * more robust, but we have what we have
 */
static inline unsigned ext2_chunk_size(struct _vcb *vcb)
{
    struct ext2_super_block * sb = vcb->vcb$l_cache;
    return EXT2_BLOCK_SIZE(sb);
}

static inline void ext2_put_page(struct page *page)
{
    kunmap(page);
    page_cache_release(page);
}

static inline unsigned long dir_pages(struct _fcb * inode)
{
    return (inode->fcb$l_filesize+PAGE_CACHE_SIZE-1)>>PAGE_CACHE_SHIFT;
}

static int ext2_commit_chunk(struct _fcb *dir, struct page *page, unsigned from, unsigned to)
{
    int err = 0;
#if 0
    dir->i_version = ++event;
#endif
    generic_commit_write2(dir, page, from, to, 0);
#if 0
    goto sync;
    if (IS_SYNC(dir))
    {
        int err2;
sync:
        err = writeout_one_page(page);
        err2 = 0; //waitfor_one_page(page);
        if (err == 0)
            err = err2;
    }
#endif
    return err;
}

static void ext2_check_page(struct _vcb * vcb, struct page *page, struct _fcb *inode, unsigned long pageno)
{
    struct _fcb *dir = inode;
    struct ext2_super_block *sb = vcb->vcb$l_cache;;
    unsigned chunk_size = ext2_chunk_size(dir);
    char *kaddr = page_address(page);
    u32 max_inumber = le32_to_cpu(sb->s_inodes_count);
    unsigned offs, rec_len;
    unsigned limit = PAGE_CACHE_SIZE;
    ext2_dirent *p;
    char *error;
    int i_ino = 0;

    if ((dir->fcb$l_filesize >> PAGE_CACHE_SHIFT) == pageno)
    {
        limit = dir->fcb$l_filesize & ~PAGE_CACHE_MASK;
        if (limit & (chunk_size - 1))
            goto Ebadsize;
        for (offs = limit; offs<PAGE_CACHE_SIZE; offs += chunk_size)
        {
            ext2_dirent *p = (ext2_dirent*)(kaddr + offs);
            p->rec_len = cpu_to_le16(chunk_size);
        }
        if (!limit)
            goto out;
    }
    for (offs = 0; offs <= limit - EXT2_DIR_REC_LEN(1); offs += rec_len)
    {
        p = (ext2_dirent *)(kaddr + offs);
        rec_len = le16_to_cpu(p->rec_len);

        if (rec_len < EXT2_DIR_REC_LEN(1))
            goto Eshort;
        if (rec_len & 3)
            goto Ealign;
        if (rec_len < EXT2_DIR_REC_LEN(p->name_len))
            goto Enamelen;
        if (((offs + rec_len - 1) ^ offs) & ~(chunk_size-1))
            goto Espan;
        if (le32_to_cpu(p->inode) > max_inumber)
            goto Einumber;
    }
    if (offs != limit)
        goto Eend;
out:
#if 0
    SetPageChecked(page);
#endif
    return;

    /* Too bad, we had an error */

Ebadsize:
    i_ino = FCB_FID_TO_INO(dir);
    ext2_error (vcb, "ext2_check_page",
                "size of directory #%lu is not a multiple of chunk size",
                i_ino
               );
    goto fail;
Eshort:
    error = "rec_len is smaller than minimal";
    goto bad_entry;
Ealign:
    error = "unaligned directory entry";
    goto bad_entry;
Enamelen:
    error = "rec_len is too small for name_len";
    goto bad_entry;
Espan:
    error = "directory entry across blocks";
    goto bad_entry;
Einumber:
    error = "inode out of bounds";
bad_entry:
    ext2_error (vcb, "ext2_check_page", "bad entry in directory #%lu: %s - "
                "offset=%lu, inode=%lu, rec_len=%d, name_len=%d",
                i_ino, error, (pageno<<PAGE_CACHE_SHIFT)+offs,
                (unsigned long) le32_to_cpu(p->inode),
                rec_len, p->name_len);
    goto fail;
Eend:
    p = (ext2_dirent *)(kaddr + offs);
    ext2_error (vcb, "ext2_check_page",
                "entry in directory #%lu spans the page boundary"
                "offset=%lu, inode=%lu",
                i_ino, (pageno<<PAGE_CACHE_SHIFT)+offs,
                (unsigned long) le32_to_cpu(p->inode));
fail:
    {}
#if 0
    SetPageChecked(page);
    SetPageError(page);
#endif
}

static struct page * ext2_get_page(struct _vcb * vcb, struct _fcb *dir, unsigned long n)
{
    struct page *page = alloc_pages(GFP_KERNEL, 0);
    block_read_full_page2(dir,page,n);
    // read_cache_page(mapping, n, (filler_t*)ext2_aops.readpage, NULL);
    if (!IS_ERR(page))
    {
        //wait_on_page(page);
        kmap(page);
#if 0
        if (!Page_Uptodate(page))
            goto fail;
        if (!PageChecked(page))
            ext2_check_page(vcb,page,dir,n);
        if (PageError(page))
            goto fail;
#else
        ext2_check_page(vcb,page,dir,n);
#endif
    }
    return page;

fail:
    ext2_put_page(page);
    return ERR_PTR(-EIO);
}

/*
 * NOTE! unlike strncmp, ext2_match returns 1 for success, 0 for failure.
 *
 * len <= EXT2_NAME_LEN and de != NULL are guaranteed by caller.
 */
static inline int ext2_match (int len, const char * const name,
                              struct ext2_dir_entry_2 * de)
{
    if (len != de->name_len)
        return 0;
    if (!de->inode)
        return 0;
    return !memcmp(name, de->name, len);
}

/*
 * p is at least 6 bytes before the end of page
 */
static inline ext2_dirent *ext2_next_entry(ext2_dirent *p)
{
    return (ext2_dirent *)((char*)p + le16_to_cpu(p->rec_len));
}

static inline unsigned
ext2_validate_entry(char *base, unsigned offset, unsigned mask)
{
    ext2_dirent *de = (ext2_dirent*)(base + offset);
    ext2_dirent *p = (ext2_dirent*)(base + (offset&mask));
    while ((char*)p < (char*)de)
        p = ext2_next_entry(p);
    return (char *)p - base;
}

static unsigned char ext2_filetype_table[EXT2_FT_MAX] =
{
    [EXT2_FT_UNKNOWN]   DT_UNKNOWN,
    [EXT2_FT_REG_FILE]  DT_REG,
    [EXT2_FT_DIR]       DT_DIR,
    [EXT2_FT_CHRDEV]    DT_CHR,
    [EXT2_FT_BLKDEV]    DT_BLK,
    [EXT2_FT_FIFO]      DT_FIFO,
    [EXT2_FT_SOCK]      DT_SOCK,
    [EXT2_FT_SYMLINK]   DT_LNK,
};

#define S_SHIFT 12
static unsigned char ext2_type_by_mode[S_IFMT >> S_SHIFT] =
{
    [S_IFREG >> S_SHIFT]    EXT2_FT_REG_FILE,
    [S_IFDIR >> S_SHIFT]    EXT2_FT_DIR,
    [S_IFCHR >> S_SHIFT]    EXT2_FT_CHRDEV,
    [S_IFBLK >> S_SHIFT]    EXT2_FT_BLKDEV,
    [S_IFIFO >> S_SHIFT]    EXT2_FT_FIFO,
    [S_IFSOCK >> S_SHIFT]   EXT2_FT_SOCK,
    [S_IFLNK >> S_SHIFT]    EXT2_FT_SYMLINK,
};

static inline void ext2_set_de_type(ext2_dirent *de, struct _fcb *inode)
{
#if 0
    mode_t mode = inode->i_mode;
    if (EXT2_HAS_INCOMPAT_FEATURE(inode->i_sb, EXT2_FEATURE_INCOMPAT_FILETYPE))
        de->file_type = ext2_type_by_mode[(mode & S_IFMT)>>S_SHIFT];
    else
        de->file_type = 0;
#endif
}

static int
ext2_readdir (struct _vcb * vcb, struct file * filp, void * dirent, filldir_t filldir)
{
    loff_t pos = filp->f_pos;
    struct _fcb *inode = filp->f_dentry->d_inode;
    struct ext2_super_block *sb = vcb->vcb$l_cache;
    unsigned offset = pos & ~PAGE_CACHE_MASK;
    unsigned long n = pos >> PAGE_CACHE_SHIFT;
    unsigned long npages = dir_pages(inode);
    unsigned chunk_mask = ~(ext2_chunk_size(inode)-1);
    unsigned char *types = NULL;
#if 0
    int need_revalidate = (filp->f_version != inode->i_version);
#else
    int need_revalidate = 0;
#endif

    if (pos > inode->fcb$l_filesize - EXT2_DIR_REC_LEN(1))
        goto done;

#if 0
    if (EXT2_HAS_INCOMPAT_FEATURE(sb, EXT2_FEATURE_INCOMPAT_FILETYPE))
        types = ext2_filetype_table;
#endif

    for ( ; n < npages; n++, offset = 0)
    {
        char *kaddr, *limit;
        ext2_dirent *de;
        struct page *page = ext2_get_page(vcb, inode, n);

        if (IS_ERR(page))
            continue;
        kaddr = page_address(page);
        if (need_revalidate)
        {
            offset = ext2_validate_entry(kaddr, offset, chunk_mask);
            need_revalidate = 0;
        }
        de = (ext2_dirent *)(kaddr+offset);
        limit = kaddr + PAGE_CACHE_SIZE - EXT2_DIR_REC_LEN(1);
        for ( ; (char*)de <= limit; de = ext2_next_entry(de))
            if (de->inode)
            {
                int over;
                unsigned char d_type = DT_UNKNOWN;

                if (types && de->file_type < EXT2_FT_MAX)
                    d_type = types[de->file_type];

                offset = (char *)de - kaddr;
                over = filldir(dirent, de->name, de->name_len,
                               (n<<PAGE_CACHE_SHIFT) | offset,
                               le32_to_cpu(de->inode), d_type);
                if (over)
                {
                    ext2_put_page(page);
                    goto done;
                }
            }
        ext2_put_page(page);
    }

done:
    filp->f_pos = (n << PAGE_CACHE_SHIFT) | offset;
#if 0
    filp->f_version = inode->i_version;
    UPDATE_ATIME(inode);
#endif
    return 0;
}

/*
 *  ext2_find_entry()
 *
 * finds an entry in the specified directory with the wanted name. It
 * returns the page in which the entry was found, and the entry itself
 * (as a parameter - res_dir). Page is returned mapped and unlocked.
 * Entry is guaranteed to be valid.
 */
struct ext2_dir_entry_2 * ext2_find_entry (struct _vcb * vcb, struct _fcb * dir,
        struct dentry *dentry, struct page ** res_page)
{
    const char *name = dentry->d_name.name;
    int namelen = dentry->d_name.len;
    unsigned reclen = EXT2_DIR_REC_LEN(namelen);
    unsigned long start = 0, n;
    unsigned long npages = dir_pages(dir);
    struct page *page = NULL;
    ext2_dirent * de;

    /* OFFSET_CACHE */
    *res_page = NULL;

#if 0
    start = dir->u.ext2_i.i_dir_start_lookup;
#endif
    if (start >= npages)
        start = 0;
    n = start;
    do
    {
        char *kaddr;
        page = ext2_get_page(vcb, dir, n);
        if (!IS_ERR(page))
        {
            kaddr = page_address(page);
            de = (ext2_dirent *) kaddr;
            kaddr += PAGE_CACHE_SIZE - reclen;
            while ((char *) de <= kaddr)
            {
                if (ext2_match (namelen, name, de))
                    goto found;
                de = ext2_next_entry(de);
            }
            ext2_put_page(page);
        }
        if (++n >= npages)
            n = 0;
    }
    while (n != start);
    return NULL;

found:
    *res_page = page;
#if 0
    dir->u.ext2_i.i_dir_start_lookup = n;
#endif
    return de;
}

struct ext2_dir_entry_2 * ext2_dotdot (struct _vcb * vcb, struct _fcb *dir, struct page **p)
{
    struct page *page = ext2_get_page(vcb, dir, 0);
    ext2_dirent *de = NULL;

    if (!IS_ERR(page))
    {
        de = ext2_next_entry((ext2_dirent *) page_address(page));
        *p = page;
    }
    return de;
}

ino_t ext2_inode_by_name(struct _vcb * vcb, struct _fcb * dir, struct dentry *dentry)
{
    ino_t res = 0;
    struct ext2_dir_entry_2 * de;
    struct page *page;

    de = ext2_find_entry (vcb, dir, dentry, &page);
    if (de)
    {
        res = le32_to_cpu(de->inode);
        kunmap(page);
        page_cache_release(page);
    }
    return res;
}

/* Releases the page */
void ext2_set_link(struct _vcb *vcb, struct _fcb *dir, struct ext2_dir_entry_2 *de,
                   struct page *page, struct _fcb *inode)
{
    unsigned from = (char *) de - (char *) page_address(page);
    unsigned to = from + le16_to_cpu(de->rec_len);
    int err;

#if 0
    lock_page(page);
#endif
    err = block_prepare_write2(NULL, page, from, to, 0);
    if (err)
        BUG();
    int i_ino = FCB_FID_TO_INO(inode);
    de->inode = cpu_to_le32(i_ino);
    ext2_set_de_type (de, inode);
    err = ext2_commit_chunk(inode, page, from, to);
#if 0
    UnlockPage(page);
#endif
    ext2_put_page(page);
#if 0
    dir->i_mtime = dir->i_ctime = CURRENT_TIME;
#endif
    ext2_sync_inode (vcb, dir);
    //mark_inode_dirty(dir);
}

/*
 *  Parent is locked.
 */
int ext2_add_link (struct _vcb * vcb, struct dentry *dentry, struct _fcb *inode)
{
    struct _fcb *dir = dentry->d_parent->d_inode;
    const char *name = dentry->d_name.name;
    int namelen = dentry->d_name.len;
    unsigned reclen = EXT2_DIR_REC_LEN(namelen);
    unsigned short rec_len, name_len;
    struct page *page = NULL;
    ext2_dirent * de;
    unsigned long npages = dir_pages(dir);
    unsigned long n;
    char *kaddr;
    unsigned from, to;
    int err;

    /* We take care of directory expansion in the same loop */
    for (n = 0; n <= npages; n++)
    {
        page = ext2_get_page(vcb, dir, n);
        err = PTR_ERR(page);
        if (IS_ERR(page))
            goto out;
        kaddr = page_address(page);
        de = (ext2_dirent *)kaddr;
        kaddr += PAGE_CACHE_SIZE - reclen;
        while ((char *)de <= kaddr)
        {
            err = -EEXIST;
            if (ext2_match (namelen, name, de))
                goto out_page;
            name_len = EXT2_DIR_REC_LEN(de->name_len);
            rec_len = le16_to_cpu(de->rec_len);
            if (!de->inode && rec_len >= reclen)
                goto got_it;
            if (rec_len >= name_len + reclen)
                goto got_it;
            de = (ext2_dirent *) ((char *) de + rec_len);
        }
        ext2_put_page(page);
    }
    BUG();
    return -EINVAL;

got_it:
    from = (char*)de - (char*)page_address(page);
    to = from + rec_len;
#if 0
    lock_page(page);
#endif
    err = block_prepare_write2(dir, page, from, to, 0);
    if (err)
        goto out_unlock;
    if (de->inode)
    {
        ext2_dirent *de1 = (ext2_dirent *) ((char *) de + name_len);
        de1->rec_len = cpu_to_le16(rec_len - name_len);
        de->rec_len = cpu_to_le16(name_len);
        de = de1;
    }
    de->name_len = namelen;
    memcpy (de->name, name, namelen);
    int i_ino = FCB_FID_TO_INO(inode);
    de->inode = cpu_to_le32(i_ino);
    ext2_set_de_type (de, inode);
    err = ext2_commit_chunk(dir, page, from, to);
#if 0
    dir->i_mtime = dir->i_ctime = CURRENT_TIME;
#endif
    ext2_sync_inode (vcb, dir);
    //mark_inode_dirty(dir);
    /* OFFSET_CACHE */
out_unlock:
#if 0
    UnlockPage(page);
#endif
out_page:
    ext2_put_page(page);
out:
    return err;
}

/*
 * ext2_delete_entry deletes a directory entry by merging it with the
 * previous entry. Page is up-to-date. Releases the page.
 */
int ext2_delete_entry2 (struct _vcb * vcb, struct ext2_dir_entry_2 * dir, struct page * page, struct _fcb * inode )
{
    char *kaddr = page_address(page);
    unsigned from = ((char*)dir - kaddr) & ~(ext2_chunk_size(inode)-1);
    unsigned to = ((char*)dir - kaddr) + le16_to_cpu(dir->rec_len);
    ext2_dirent * pde = NULL;
    ext2_dirent * de = (ext2_dirent *) (kaddr + from);
    int err;

    while ((char*)de < (char*)dir)
    {
        pde = de;
        de = ext2_next_entry(de);
    }
    if (pde)
        from = (char*)pde - (char*)page_address(page);
#if 0
    lock_page(page);
#endif
    err = block_prepare_write2(inode, page, from, to, 0);
    if (err)
        BUG();
    if (pde)
        pde->rec_len = cpu_to_le16(to-from);
    dir->inode = 0;
    err = ext2_commit_chunk(inode, page, from, to);
#if 0
    UnlockPage(page);
#endif
    ext2_put_page(page);
#if 0
    inode->i_ctime = inode->i_mtime = CURRENT_TIME;
#endif
    ext2_sync_inode (vcb, inode);
    //mark_inode_dirty(inode);
    return err;
}

/*
 * Set the first fragment of directory.
 */
int ext2_make_empty(struct _fcb *inode, struct _fcb *parent)
{
    struct page *page = alloc_pages(GFP_KERNEL, 0);// was grab_cache_page
    unsigned chunk_size = ext2_chunk_size(inode);
    struct ext2_dir_entry_2 * de;
    char *base;
    int err;

    if (!page)
        return -ENOMEM;
    err = block_prepare_write2(inode, page, 0, chunk_size, 0);
    if (err)
        goto fail;

    base = page_address(page);

    de = (struct ext2_dir_entry_2 *) base;
    de->name_len = 1;
    de->rec_len = cpu_to_le16(EXT2_DIR_REC_LEN(1));
    memcpy (de->name, ".\0\0", 4);
    int i_ino = FCB_FID_TO_INO(inode);
    de->inode = cpu_to_le32(i_ino);
    ext2_set_de_type (de, inode);

    de = (struct ext2_dir_entry_2 *) (base + EXT2_DIR_REC_LEN(1));
    de->name_len = 2;
    de->rec_len = cpu_to_le16(chunk_size - EXT2_DIR_REC_LEN(1));
    i_ino = FCB_FID_TO_INO(parent);
    de->inode = cpu_to_le32(i_ino);
    memcpy (de->name, "..\0", 4);
    ext2_set_de_type (de, inode);

    err = ext2_commit_chunk(inode, page, 0, chunk_size);
fail:
#if 0
    UnlockPage(page);
#endif
    page_cache_release(page);
    return err;
}

/*
 * routine to check that the specified directory is empty (for rmdir)
 */
int ext2_empty_dir (struct _vcb * vcb, struct _fcb * inode)
{
    struct page *page = NULL;
    unsigned long i, npages = dir_pages(inode);

    for (i = 0; i < npages; i++)
    {
        char *kaddr;
        ext2_dirent * de;
        page = ext2_get_page(vcb, inode, i);

        if (IS_ERR(page))
            continue;

        kaddr = page_address(page);
        de = (ext2_dirent *)kaddr;
        kaddr += PAGE_CACHE_SIZE-EXT2_DIR_REC_LEN(1);

        while ((char *)de <= kaddr)
        {
            if (de->inode != 0)
            {
                /* check for . and .. */
                if (de->name[0] != '.')
                    goto not_empty;
                if (de->name_len > 2)
                    goto not_empty;
                if (de->name_len < 2)
                {
                    if (de->inode !=
                            cpu_to_le32(FCB_FID_TO_INO(inode)))
                        goto not_empty;
                }
                else if (de->name[1] != '.')
                    goto not_empty;
            }
            de = ext2_next_entry(de);
        }
        ext2_put_page(page);
    }
    return 1;

not_empty:
    ext2_put_page(page);
    return 0;
}

#if 0
struct file_operations ext2_dir_operations =
{
read:
    generic_read_dir,
readdir:
    ext2_readdir,
ioctl:
    ext2_ioctl,
fsync:
    ext2_sync_file,
};
#endif
