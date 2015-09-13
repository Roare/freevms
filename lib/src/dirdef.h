#ifndef DIRDEF_H
#define DIRDEF_H

/* Direct.h v1.3    Definitions for directory access routines */

/*
        This is part of ODS2 written by Paul Nankervis,
        email address:  Paulnank@au1.ibm.com

        ODS2 is distributed freely for all members of the
        VMS community to use. However all derived works
        must maintain comments in their source to acknowledge
        the contibution of the original author.
*/



struct _dir
{
    _uword dir$w_size;
    _uword dir$w_verlimit;
    _ubyte dir$b_flags;
    _ubyte dir$b_namecount;
    char dir$t_name[1];
};

struct _dir1
{
    _uword dir$w_version;
    struct _fiddef dir$fid;
};

#endif
