#include <linux/acpi.h>
#include <linux/module.h>
#include <asm/uaccess.h>
#include <linux/suspend.h>
#include <linux/seq_file.h>
#include <linux/pm_runtime.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Toggle the discrete graphics card");
MODULE_AUTHOR("Peter Wu <lekensteyn@gmail.com>");
MODULE_VERSION("0.1");

static int acpi_call_osi(const char * arg1) {
    struct acpi_buffer output = { ACPI_ALLOCATE_BUFFER, NULL };
    struct acpi_object_list input;
    union acpi_object params[1];
    union acpi_object *obj;
    int err;

    input.count = 1;
    input.pointer = params;
    params[0].type = ACPI_TYPE_STRING;
    params[0].buffer.length = strlen(arg1);
    params[0].buffer.pointer = (char *)arg1;

    err = acpi_evaluate_object(NULL, "\\_OSI", &input, &output);
    //err = acpi_evaluate_object(NULL, "\\_REV", 0, &output);
    if (err) {
        printk("ACPI_CALL_OSI: Error _OSI: %d\n", err);
        return 0;
    }

    obj = (union acpi_object *)output.pointer;

    if (obj->type == ACPI_TYPE_INTEGER) {
        printk("ACPI_CALL_OSI: %s = 0x%08X 0x%08X\n", arg1, 
                (u32)(obj->integer.value & 0xFFFFFFFF00000000 >> 32),
                (u32)(obj->integer.value & 0x0FFFFFFFF));
    } else {
        printk("ACPI_CALL_OSI: Error no int\n");
    }

    return 0;
}

static void __exit acpi_osi_eval_exit(void)
{
}


static int __init acpi_osi_eval_init(void) {
    acpi_call_osi("Linux");
    acpi_call_osi("Windows ");
    acpi_call_osi("Windows");
    acpi_call_osi("Windows 2000");
    acpi_call_osi("Windows 2001 SP1");
    acpi_call_osi("Windows 2001.1");
    acpi_call_osi("Windows 2001 SP2");
    acpi_call_osi("Windows 2001.1 SP1");
    acpi_call_osi("Windows 2006");
    acpi_call_osi("Windows 2006.1");
    acpi_call_osi("Windows 2006 SP1");
    acpi_call_osi("Windows 2006 SP2");
    acpi_call_osi("Windows 2009");
    acpi_call_osi("Windows 2010");
    acpi_call_osi("Windows 2011");
    acpi_call_osi("Windows 2012");
    acpi_call_osi("Windows 2013");

    return 0;

}

module_init(acpi_osi_eval_init);
module_exit(acpi_osi_eval_exit);

/* vim: set sw=4 ts=4 et: */
