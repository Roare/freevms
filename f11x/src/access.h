/* Access.h v1.3    Definitions for file access routines */

/*
 This is part of ODS2 written by Paul Nankervis,
 email address:  Paulnank@au1.ibm.com

 ODS2 is distributed freely for all members of the
 VMS community to use. However all derived works
 must maintain comments in their source to acknowledge
 the contibution of the original author.
 */

#ifndef ACCESS_H
#define ACCESS_H

#include <fcbdef.h>
#include <fh2def.h>
#include <fiddef.h>
#include <iosbdef.h>
#include <irpdef.h>
#include <vcbdef.h>

void fid_copy(struct _fiddef *dst, struct _fiddef *src, unsigned rvn);

unsigned dismount(struct _vcb *vcb);

unsigned accesserase(struct _vcb *vcb, struct _irp * irp);
unsigned deaccessfile(struct _fcb *fcb);
unsigned f11b_access(struct _vcb * vcb, struct _irp * irp);

unsigned accesschunk(struct _fcb *fcb, unsigned vbn, char **retbuff,
                     unsigned *retblocks, unsigned wrtblks, struct _irp * i);
unsigned access_extend(struct _fcb *fcb, unsigned blocks, unsigned contig);
unsigned update_freecount(struct _vcb *vcbdev, unsigned *retcount);
unsigned update_create(struct _vcb *vcb, struct _irp * i);
unsigned update_extend(struct _fcb *fcb, unsigned blocks, unsigned contig);
unsigned short checksum(UINT16 *block);

void * f11b_read_block(struct _vcb * vcb, unsigned long lbn,
                       unsigned long count, struct _iosb * iosb);
void * f11b_write_block(struct _vcb * vcb, unsigned char * buf,
                        unsigned long lbn, unsigned long count, struct _iosb * iosb);
struct _fcb * getidxfcb(struct _vcb * vcb);
unsigned writehead(struct _fcb * fcb, struct _fh2 *headbuff);
void * f11b_read_header(struct _vcb *vcb, struct _fiddef *fid,
                        struct _fcb * fcb, unsigned long * retsts);

#endif /* ACCESS_H */
