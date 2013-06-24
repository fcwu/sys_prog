#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>

static int __init hello_init(void)
{
    printk(KERN_INFO "Hello Init\n");
    return 0;
}

static void __exit hello_exit(void)
{
    printk(KERN_INFO "Bye Init\n");
}

module_init(hello_init)
module_exit(hello_exit)
MODULE_LICENSE("GPL");
