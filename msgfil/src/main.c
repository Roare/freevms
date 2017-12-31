// $Id$
// $Locker$

// Author. Roar Thron�s.

#define GTY(x)
#define HOST_BITS_PER_WIDE_INT 64
#define POINTER_SIZE 32
#define true 1
#define false 0

#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <stdlib.h>

int vms_mm=0;

#include "tree.h"

#include "../../cdu/src/cli.h"

extern char *strndup (__const char *__string, size_t __n);

genwrite();

char * module_name;

main(int argc, char ** argv)
{
    char * name = 0;
    if (argc==2)
        name = argv[1];
    extern int yydebug;
    yydebug=1;
    extern FILE *yyin;
    init_stringpool();
    yyin=fopen(name, "r");
    if (yyin==0)
    {
        printf("could not fopen %s\n",name);
        return 1;
    }
    name = basename(name);
    int newlen = (long)strchr(name,'.')-(long)name;
    module_name = strndup(name, newlen);
    genwrite();
    yyparse();
    extern int out;
    fclose(out);
    extern tree root_tree;
    //  gencode(root_tree);
    return 0;
}

void * get_cli_int(char * c)
{
    return 0;
}
