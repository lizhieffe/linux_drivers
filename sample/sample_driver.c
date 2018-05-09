#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>

static int my_int = 1;

static int __init my_init(void) {
  pr_info("Hello world, %d!\n", my_int);
  return 0;
}

static void __exit my_exit(void) {
  pr_info("End of Hello world!\n");
}

module_init(my_init);
module_exit(my_exit);

module_param(my_int, int, S_IRUGO /* read only */);
MODULE_PARM_DESC(my_int, "this is the one of the module params");

MODULE_AUTHOR("lizhi");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Sample module with bare skeleton");
