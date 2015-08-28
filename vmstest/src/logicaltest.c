#include <stdlib.h>
#include <string.h>

#include <descrip.h>
#include <iledef.h>
#include <lnmdef.h>
#include <psldef.h>
#include <ssdef.h>

#include <starlet.h>

#include "expect.h"
#include "logicaltest.h"

#define MAX_VALUE_LENGTH 255

int call_sys$crelnm(char *table_name, char *logical_name, char *value, unsigned int *attribute, unsigned char *access_mode)
{
    struct dsc$descriptor_s table_name_desc =
        { (unsigned short) strlen(table_name), DSC$K_DTYPE_T, DSC$K_CLASS_S, table_name };
    struct dsc$descriptor_s logical_name_desc =
        { (unsigned short) strlen(logical_name), DSC$K_DTYPE_T, DSC$K_CLASS_S, logical_name };

    struct _ile3 item_list[] =
    {
        { (unsigned short) strlen(value), LNM$_STRING, value, NULL },
        { 0, 0, NULL, NULL }
    };

    return sys$crelnm(attribute, &table_name_desc, &logical_name_desc, access_mode, item_list);
}

int call_sys$crelnt(char *table_name, char *parent_table, unsigned int *attribute, unsigned int *quota,
                    unsigned short* protection_mask, unsigned char *access_mode)
{
    /* TODO: resnam */
    struct dsc$descriptor_s table_name_desc =
        { (unsigned short) strlen(table_name), DSC$K_DTYPE_T, DSC$K_CLASS_S, table_name };
    struct dsc$descriptor_s parent_table_desc =
        { (unsigned short) strlen(parent_table), DSC$K_DTYPE_T, DSC$K_CLASS_S, parent_table };

    return sys$crelnt(attribute, NULL, NULL, quota, protection_mask, &table_name_desc, &parent_table_desc, access_mode);
}

int call_sys$dellnm(char *table_name, char *logical_name, unsigned char *access_mode)
{
    struct dsc$descriptor_s table_name_desc =
        { (unsigned short) strlen(table_name), DSC$K_DTYPE_T, DSC$K_CLASS_S, table_name };
    struct dsc$descriptor_s logical_name_desc =
        { (unsigned short) strlen(logical_name), DSC$K_DTYPE_T, DSC$K_CLASS_S, logical_name };

    return sys$dellnm(&table_name_desc, &logical_name_desc, access_mode);
}

int call_sys$trnlnm(char *table_name, char *logical_name, char *value_list, unsigned int *attribute, unsigned char *access_mode)
{
    int ret_status = SS$_NORMAL;

    struct dsc$descriptor_s table_name_desc =
        { (unsigned short) strlen(table_name), DSC$K_DTYPE_T, DSC$K_CLASS_S, table_name };
    struct dsc$descriptor_s logical_name_desc =
        { (unsigned short) strlen(logical_name), DSC$K_DTYPE_T, DSC$K_CLASS_S, logical_name };

    char *value_ptr = value_list;
    unsigned int item_index = 0;
    unsigned int max_item_index = 0;
    unsigned short value_length;
    struct _ile3 item_list[] =
    {
        { sizeof(item_index), LNM$_INDEX, &item_index, NULL },
        { 0, LNM$_STRING, NULL, &value_length },
        { sizeof(max_item_index), LNM$_MAX_INDEX, &max_item_index, NULL },
        { 0, 0, NULL, NULL }
    };

    do
    {
        item_list[1].ile3$w_length = MAX_VALUE_LENGTH;
        item_list[1].ile3$ps_bufaddr = value_ptr;
        int status = sys$trnlnm(attribute, &table_name_desc, &logical_name_desc, access_mode, item_list);
        if ($VMS_STATUS_SUCCESS(status))
        {
            value_ptr[value_length] = '\0';
        }
        else
        {
            ret_status = status;
        }
        item_index++;
        value_ptr += MAX_VALUE_LENGTH + 1;
    }
    while (item_index < max_item_index);

    return ret_status;
}

void run_logical_test(void)
{
    char value_list[LNM$_MAX_INDEX * (MAX_VALUE_LENGTH + 1)];

    EXPECT_EQ(call_sys$trnlnm("LNM$PROCESS_TABLE", "SYS$SYSTEM", value_list, NULL, NULL), SS$_NORMAL);
    EXPECT_EQ(strcmp(value_list, "dqa001:[vms$common.sysexe]"), 0);
    EXPECT_EQ(call_sys$trnlnm("LNM$SYSTEM_TABLE", "SYS$SYSTEM", value_list, NULL, NULL), SS$_NORMAL);
    EXPECT_EQ(strcmp(value_list, "dqa001:[vms$common.sysexe]"), 0);
    EXPECT_EQ(call_sys$trnlnm("lnm$system", "sys$system", value_list, NULL, NULL), SS$_NORMAL);
    EXPECT_EQ(strcmp(value_list, "dqa001:[vms$common.sysexe]"), 0);

    EXPECT_EQ(call_sys$trnlnm("LNM$PROCESS_TABLE", "SOME_NON_EXISTING_LOGICALNAME", value_list, NULL, NULL), SS$_NOLOGNAM);
    EXPECT_SUCCESS(call_sys$crelnm("LNM$PROCESS_TABLE", "MY_TEST_LOGICAL", "Test Value 1", NULL, NULL));
    EXPECT_EQ(call_sys$trnlnm("LNM$PROCESS_TABLE", "MY_TEST_LOGICAL", value_list, NULL, NULL), SS$_NORMAL);
    EXPECT_EQ(strcmp(value_list, "Test Value 1"), 0);
    EXPECT_SUCCESS(call_sys$dellnm("LNM$PROCESS_TABLE", "MY_TEST_LOGICAL", NULL));
    EXPECT_EQ(call_sys$trnlnm("LNM$PROCESS_TABLE", "MY_TEST_LOGICAL", value_list, NULL, NULL), SS$_NORMAL);

    EXPECT_EQ(call_sys$crelnt("MY_TEST_TABLE", "LNM$PROCESS_DIRECTORY", NULL, NULL, NULL, NULL), SS$_NOPRIV);
    /*
     EXPECT_SUCCESS(call_sys$crelnt("MY_TEST_TABLE", "LNM$PROCESS_DIRECTORY", NULL, NULL, NULL, NULL));
     EXPECT_SUCCESS(call_sys$crelnm("MY_TEST_TABLE", "MY_TEST_LOGICAL", "Test Value 2", NULL, NULL));
     EXPECT_EQ(call_sys$trnlnm("MY_TEST_TABLE", "MY_TEST_LOGICAL", value_list, NULL, NULL), SS$_NORMAL);
     EXPECT_EQ(strcmp(value_list, "Test Value 2"), 0);
     EXPECT_SUCCESS(call_sys$dellnm("LNM$PROCESS_DIRECTORY", "MY_TEST_TABLE", NULL));
     EXPECT_EQ(call_sys$trnlnm("MY_TEST_TABLE", "MY_TEST_LOGICAL", value_list, NULL, NULL), SS$_NORMAL);
     */
}
