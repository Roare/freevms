#ifndef DEVICETEST_H
#define DEVICETEST_H

#include <iosbdef.h>

int call_sys$assign(char *device_name, unsigned short *channel, unsigned int access_mode, char *mailbox_name, unsigned int flags);

int call_sys$dassgn(unsigned short channel);

int call_sys$getdviw1(unsigned short code, void *value, unsigned short *length);

int call_sys$getdviw2(unsigned short code1, void *value1, unsigned short *length1, unsigned short code2, void *value2,
        unsigned short *length2);

int call_sys$qiow(unsigned short channel, unsigned int function, struct _iosb *io_status_block, unsigned char *buffer,
        unsigned int length);

void run_device_test(void);

#endif /* DEVICETEST_H */
