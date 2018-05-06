#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

static void print_log(void);

static int __init hello_world_init(void) {
  print_log();
  return 0;
}

static void __exit hello_world_exit(void) {
  pr_info("End of Hello world!\n");
}

static void print_log(void) {
  pr_info("Hello worldi!\n");
}

module_init(hello_world_init);
module_exit(hello_world_exit);
MODULE_AUTHOR("lizhi");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Sample module with bare skeleton");
