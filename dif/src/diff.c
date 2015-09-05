#define MODULE_NAME ODS2
#define MODULE_IDENT    "V1.3"

/*     Ods2.c v1.3   Mainline ODS2 program   */

/*
        This is part of ODS2 written by Paul Nankervis,
        email address:  Paulnank@au1.ibm.com

        ODS2 is distributed freely for all members of the
        VMS community to use. However all derived works
        must maintain comments in their source to acknowledge
        the contibution of the original author.

        The modules in ODS2 are:-

                ACCESS.C        Routines for accessing ODS2 disks
                CACHE.C         Routines for managing memory cache
                DEVICE.C        Routines to maintain device information
                DIRECT.C        Routines for handling directories
                ODS2.C          The mainline program
                PHYVMS.C        Routine to perform physical I/O
                RMS.C           Routines to handle RMS structures
                VMSTIME.C       Routines to handle VMS times

        On non-VMS platforms PHYVMS.C should be replaced as follows:-

                OS/2            PHYOS2.C
                Windows 95/NT   PHYNT.C

        For example under OS/2 the program is compiled using the GCC
        compiler with the single command:-

                gcc -fdollars-in-identifiers ods2.c,rms.c,direct.c,
                      access.c,device.c,cache.c,phyos2.c,vmstime.c
*/

/* Modified by:
 *
 *   31-AUG-2001 01:04  Hunter Goatley <goathunter@goatley.com>
 *
 *  For VMS, added routine getcmd() to read commands with full
 *  command recall capabilities.
 *
 */

/*  This is the top level set of routines. It is fairly
    simple minded asking the user for a command, doing some
    primitive command parsing, and then calling a set of routines
    to perform whatever function is required (for example COPY).
    Some routines are implemented in different ways to test the
    underlying routines - for example TYPE is implemented without
    a NAM block meaning that it cannot support wildcards...
    (sorry! - could be easily fixed though!)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <mytypes.h>
#include <descrip.h>
#include <fabdef.h>
#include <fiddef.h>
#include <misc.h>
#include <namdef.h>
#include <rabdef.h>
#include <rmsdef.h>
#include <ssdef.h>
#include <uicdef.h>
#include <xabdef.h>
#include <xabdatdef.h>
#include <xabfhcdef.h>
#include <xabprodef.h>
#include <hm2def.h>
#include "../../rms/src/cache.h"
#include "../../f11x/src/access.h"
#include <starlet.h>
#include <cli$routines.h>
//#include "rms.h"

//extern struct _nam cc$rms_nam;
//extern struct _fab cc$rms_fab;

#if 0
struct _fabdef cc$rms_fab = {NULL,0,NULL,NULL,0,0,0,0,0,0,0,0,0,0,0,0,0,NULL};
struct _namdef cc$rms_nam = {0,0,0,0,0,0,0,0,0,0,0,NULL,0,NULL,0,NULL,0,NULL,0,NULL,0,NULL,0,NULL,0,0,0};
struct _xabdatdef cc$rms_xabdat = {XAB$C_DAT,0,
           0, 0, 0, 0,
           VMSTIME_ZERO, VMSTIME_ZERO,
           VMSTIME_ZERO, VMSTIME_ZERO,
           VMSTIME_ZERO, VMSTIME_ZERO
};
struct _xabfhcdef cc$rms_xabfhc = {XAB$C_FHC,0,0,0,0,0,0,0,0,0,0,0};
struct _xabprodef1 cc$rms_xabpro = {XAB$C_PRO,0,0,0};
struct _rabdef cc$rms_rab = {NULL,NULL,NULL,NULL,0,0,0,{0,0,0}};
#endif

#define PRINT_ATTR (FAB$M_CR | FAB$M_PRN | FAB$M_FTN)

void main()
{
    printf("main is a dummy\n");
    return 1;
}

#define MAXREC 32767

/* diff: a simple file difference routine */

unsigned diff(int userarg)
{
    setvbuf(stdout, NULL, _IONBF, 0);      // need this to see i/o at all
    int sts;
    $DESCRIPTOR(p1, "p1");
    $DESCRIPTOR(p2, "p2");
    char c1[80];
    struct dsc$descriptor o1;
    o1.dsc$a_pointer=c1;
    o1.dsc$w_length=80;
    memset (c1, 0, 80);
    char c2[80];
    struct dsc$descriptor o2;
    o2.dsc$a_pointer=c2;
    o2.dsc$w_length=80;
    memset (c2, 0, 80);
    sts = cli$present(&p1);
    if ((sts&1)==0)
        return sts;
    sts = cli$present(&p2);
    if ((sts&1)==0)
        return sts;
    int retlen;
    sts = cli$get_value(&p1, &o1, &retlen);
    sts = cli$get_value(&p2, &o2, &retlen);
    struct _fabdef fab = cc$rms_fab;
    FILE *tof;
    int records = 0;
    fab.fab$l_fna = c1;
    fab.fab$b_fns = strlen(fab.fab$l_fna);
    tof = fopen(c2,"r");
    if (tof == NULL)
    {
        printf("Could not open file %s\n",c1);
        sts = 0;
    }
    else
    {
        if ((sts = sys$open(&fab, 0, 0)) & 1)
        {
            struct _rabdef rab = cc$rms_rab;
            rab.rab$l_fab = &fab;
            if ((sts = sys$connect(&rab, 0, 0)) & 1)
            {
                char rec[MAXREC + 2],cpy[MAXREC + 1];
                rab.rab$l_ubf = rec;
                rab.rab$w_usz = MAXREC;
                while ((sts = sys$get(&rab, 0, 0)) & 1)
                {
                    strcpy(rec + rab.rab$w_rsz,"\n");
                    fgets(cpy,MAXREC,tof);
                    if (strcmp(rec,cpy) != 0)
                    {
                        printf("%%DIFF-F-DIFFERENT Files are different!\n");
                        sts = 4;
                        break;
                    }
                    else
                    {
                        records++;
                    }
                }
                sys$disconnect(&rab, 0, 0);
            }
            sys$close(&fab, 0, 0);
        }
        fclose(tof);
        if (sts == RMS$_EOF) sts = 1;
    }
    if (sts & 1)
    {
        printf("%%DIFF-I-Compared %d records\n",records);
    }
    else
    {
        printf("%%DIFF-F-Error %d in difference\n",sts);
    }
    return sts;
}
