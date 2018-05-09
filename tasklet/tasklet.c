#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>

char tasklet_data[] = "We use a string; but it could be pointer to data.";

void tasklet_work(unsigned long data) {
  printk("%s\n", (char *)data);
}

DECLARE_TASKLET(my_tasklet, tasklet_work, (unsigned long)tasklet_data);

static int __init my_init(void) {
  tasklet_schedule(&my_tasklet);
  return 0;
}

static void __exit my_exit(void) {
  tasklet_kill(&my_tasklet);
}

module_init(my_init);
module_exit(my_exit);


MODULE_AUTHOR("lizhi");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Sample module for tasklet");
