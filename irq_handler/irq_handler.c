#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

static const int irq_number = 8;  // real time clock

struct my_data {
  struct input_dev *idev;
  struct i2c_client *client;
  char name[64];
  char phys[32];
};

static irqreturn_t my_irq_handler(int irq, void *dev_id) {
  struct my_data *md = dev_id;
  // TODO: doesn't see the log in journalctl. Fix it.
  printk("===lizhi irq handler handing ...");
  return IRQ_HANDLED;
}

int ret;
struct my_data *md;
char DRV_NAME[] = "===lizhi";

static int __init my_init(void) {
  md = kmalloc(sizeof(*md), GFP_KERNEL);
  ret = request_irq(irq_number, my_irq_handler, IRQF_SHARED, DRV_NAME, md);
  return 0;
}

static void __exit my_exit(void) {
  free_irq(irq_number, md);
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("lizhi");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Sample module for irq handler");
