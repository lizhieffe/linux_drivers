#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>

static int pdrv_probe(struct platform_device *pdev) {
  struct resource *res1, *res2;
  // void *reg1, *reg2;
  int irqnum;

  pr_info("===lizhi platform device probed");

  res1 = platform_get_resource(pdev, IORESOURCE_MEM, 0);
  if (!res1) {
    pr_err("First resource not available");
    return -1;
  }
  res2 = platform_get_resource(pdev, IORESOURCE_MEM, 1);
  if (!res2) {
    pr_err("Second resource not available");
    return -1;
  }

  irqnum = platform_get_irq(pdev, 0);
  pr_info("\nIRQ number of device: %d\n", irqnum);

  return 0;
}

static int pdrv_remove(struct platform_device *pdev) {
  pr_info("===lizhi platform device removed");
  return 0;
}

static struct platform_driver pdrv = {
  // Is called every time when a given device matches with the driver.
  .probe = pdrv_probe,
  .remove = pdrv_remove,
  .driver = {
    .name = "lizhi-device",
    .owner = THIS_MODULE,
  },
};

static int __init my_init(void) {
  pr_info("Hello world\n");
  platform_driver_register(&pdrv);
  return 0;
}

static void __exit my_exit(void) {
  pr_info("End of Hello world!\n");
  platform_driver_unregister(&pdrv);
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("lizhi");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Sample module for platform driver.");
