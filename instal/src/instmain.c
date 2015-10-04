// $Id$
// $Locker$

// Author. Roar Thron�s.

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include <descrip.h>
#include <starlet.h>
#include <cli$routines.h>
#include <lib$routines.h>

int inst$add(int mask);
int inst$exit(int mask);

struct cli_struct
{
    char * cliroutine;
    void (*fn)();
};

struct cli_struct cliroutines[] =
    {
        { "inst_exit", inst$exit, },
        { "inst_add", inst$add, },
        { 0, 0, }, };

void * get_cli_int(char * c)
{
    int i;
    for (i = 0; cliroutines[i].cliroutine; i++)
    {
        if (0 == strncmp(c, cliroutines[i].cliroutine, strlen(cliroutines[i].cliroutine)))
            return cliroutines[i].fn;
    }
    return 0;
}

int vms_mm = 1;

extern long inst_parse[];

int do_ret = 0;

main()
{
    printf("\nFreeVMS install utility\n\n");

    int retlen = 0;
    $DESCRIPTOR(p, "p1");
    char c[80];
    struct dsc$descriptor o;
    o.dsc$a_pointer = c;
    o.dsc$w_length = 80;
    memset(c, 0, 80);
    int sts = cli$present(&p);
    if (sts & 1)
    {
        sts = lib$get_foreign(&o, 0, &retlen, 0);
        o.dsc$w_length = retlen;
        char * ostr = strchr(o.dsc$a_pointer, ' ');
        int olen = ostr - (char *) o.dsc$a_pointer;
        if (0 == strncasecmp(o.dsc$a_pointer, "install", olen))
        {
            memset(o.dsc$a_pointer, 32, olen);
        }
        o.dsc$a_pointer += olen + 1;
        o.dsc$w_length -= olen + 1;
        printf("o is %s\n", (char *) o.dsc$a_pointer);
        fflush(stdout);
        int sts = cli$dcl_parse(&o, &inst_parse, 0, 0, 0);
        if (sts & 1)
            cli$dispatch(0);
        return 1;
    }

    while (!do_ret)
    {
        printf("INSTALL> ");
        fflush(stdout);

        char command_str[256];
        int len = read(0, command_str, 255);

        struct dsc$descriptor command_line;
        command_line.dsc$a_pointer = command_str;
        command_line.dsc$w_length = len;

        int sts = cli$dcl_parse(&command_line, inst_parse, 0, 0, 0);
        if (sts & 1)
            cli$dispatch(0);
    }
}
