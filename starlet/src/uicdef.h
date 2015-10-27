#ifndef UICDEF_H
#define UICDEF_H

/* Access.h v1.3    Definitions for file access routines */

/*
        This is part of ODS2 written by Paul Nankervis,
        email address:  Paulnank@au1.ibm.com

        ODS2 is distributed freely for all members of the
        VMS community to use. However all derived works
        must maintain comments in their source to acknowledge
        the contibution of the original author.

    Originally part of access.h
*/

#include <vms_types.h>

struct _uicdef
{
    union
    {
        UINT32 uic$l_owner; /* File owner's UIC */
        struct
        {
            UINT16 uic$w_mem; /* UIC member field */
            UINT16 uic$w_grp; /* UIC group field */
        };
    };
};

#endif
