#include <stdio.h>

#include "logicaltest.h"
#include "processtest.h"
#include "systemtest.h"

unsigned int passed = 0;
unsigned int failed = 0;

int main(void)
{
    printf("Running system services tests ...\n");
    run_logical_test();
    run_process_test();
    run_system_test();
    printf("#Executed: %d\t#Passed: %d\t#Failed: %d\n", passed + failed, passed, failed);
    return 0;
}
