// $Id$
// $Locker$

// Author. Paul Nankervis.

/*

       VMSTIME.C  v1.1

       Author: Paul Nankervis

       Please send bug reports or requests for enhancement
       or improvement via email to:     PaulNank@au1.ibm.com


       This module contains versions of the VMS time routines
       sys$numtim(), sys$asctim() and friends... They are
       intended to be compatible with the routines of the same
       name on a VMS system (so descriptors feature regularly!)

       This code relies on being able to manipluate day numbers
       and times using 32 bit arithmetic to crack a VMS quadword
       byte by byte. If your C compiler doesn't have 32 bit int
       fields give up now! On a 64 bit systems this code could
       be modified to do 64 bit operations directly....

       One advantage of doing arihmetic byte by byte is that
       the code does not depend on what 'endian' the target
       machine is - it will always treat bytes in the same order!
       (Hopefully VMS time bytes will always be in the same order!)

       A couple of stupid questions to go on with:-
           o OK, I give up! What is the difference between a zero
             date and a zero delta time?
           o Anyone notice that the use of 16 bit words in
             sys$numtim restricts delta times to 65535 days?

                                       Paul Nankervis

History:
	02/12/1998 - Kevin Handy
		This funtion returns a time based upon GMT, and
		is thus off if you are in the wrong area.
		Added use of localtime()/timezone to adjust date
		(Probibly did it wrong, but ...).
*/

/* Some mods by Roar Thron�s */

#include <system_data_cells.h>	/* SYS$ header file */
#include <sys$routines.h>	/* SYS$ header file */
#include <ssdef.h>	/* SYS$ header file */
#include <linux/time.h>		/* C header for $GETTIM to find time */
#include<linux/linkage.h>
#include <linux/string.h>

/*
 * Special function that is not really part of SYS$ library
 */
unsigned long sys$__combine_date_time(int days, void *timadr,int day_time);

/*
 * This variable is set by the Unix localtime() function
 * and contains the offset from UTC for the locale.
 */
extern long timezone;

/* sys_gettim() implemented here by getting UNIX time in seconds since
   1-Jan-1970 using time() and munging into a quadword... Some run time
   systems don't seem to do this properly!!! Note that time() has a
   resolution of only one second. */

#define SECINDAY (24L * 60L * 60L)
#define FUDGEFACTOR 40587

asmlinkage int exe$gettim(unsigned long long *timadr)
{
    long * tmp=(long *)&exe$gq_systime, *tmp2=(long *)timadr;
    if (!timadr) return SS$_ACCVIO;
    if (timadr<4096)
    {
#if 0
        printk("gettim got accvio addr\n");
#endif
        return SS$_ACCVIO;
    }
    memcpy(timadr,&exe$gq_systime,8);
#if 0
    printk("setime %x %x\n",tmp[0],tmp[1]);
    printk("setime %x %x\n",tmp2[0],tmp2[1]);
#endif
    return SS$_NORMAL;
}
