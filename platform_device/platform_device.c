#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>

#define JZ4740_UDC_BASE_ADDR 0x1234578
#define JZ4740_UDC_BASE_ADDR2 0x1234678
#define JZ4740_IRQ_UDC 3

struct lizhi_gpios {
  int reset_gpio;
  int led_gpio;
};

static struct lizhi_gpios needed_gpios = {
  .reset_gpio = 47,
  .led_gpio = 41,
};

static struct resource needed_resources[] = {
  [0] = {
    .start  = JZ4740_UDC_BASE_ADDR,
    .end    = JZ4740_UDC_BASE_ADDR + 0x10000 - 1,
    .flags  = IORESOURCE_MEM,
    .name   = "mem1",
  },
  [1] = {
    .start  = JZ4740_UDC_BASE_ADDR2,
    .end    = JZ4740_UDC_BASE_ADDR2 + 0x10000 - 1,
    .flags  = IORESOURCE_MEM,
    .name   = "mem2",
  },
  [2] = {
    .start  = JZ4740_IRQ_UDC,
    .end    = JZ4740_IRQ_UDC,
    .flags  = IORESOURCE_IRQ,
    .name   = "mc",
  },
};

static struct platform_device lizhi_device = {
  .name = "lizhi-device",
  .id = 0,
  .dev = {
    .platform_data = &needed_gpios,
  },
  .resource = needed_resources,
  .num_resources = ARRAY_SIZE(needed_resources),
};

static int __init my_init(void) {
  pr_info("Hello world");
  platform_device_register(&lizhi_device);
  return 0;
}

static void __exit my_exit(void) {
  pr_info("End of Hello world!\n");
  platform_device_unregister(&lizhi_device);
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("lizhi");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Sample module with bare skeleton");
