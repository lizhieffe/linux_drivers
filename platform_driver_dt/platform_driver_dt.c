// This platform driver is used to drive the device registered in DT. Please
// check my rpi_linux repository which has a device called "hellokeys" in DT.

#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/platform_device.h>

static int my_dev_open(struct inode *inode, struct file *file) {
  pr_info("%s is called\n", __func__);
  return 0;
}

static int my_dev_close(struct inode *inode, struct file *file) {
  pr_info("%s is called\n", __func__);
  return 0;
}

static long my_dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
  pr_info("%s is called. cmd = %d, arg = %ld\n", __func__, cmd, arg);
  return 0;
}

static const struct file_operations my_dev_fops = {
  .owner = THIS_MODULE,
  .open = my_dev_open,
  .release = my_dev_close,
  .unlocked_ioctl = my_dev_ioctl,
};

// Sometimes people need to write “small” device drivers, to support custom
// hacks—either hardware or software ones. To this end, as well as to host some
// real drivers, the Linux kernel exports an interface to allow modules to
// register their own small drivers. The misc driver was designed for this
// purpose. The code introduced here is meant to run with version 2.0 of the
// Linux kernel.
//
// In UNIX, Linux and similar operating systems, every device is identified by
// two numbers: a “major” number and a “minor” number. These numbers can be seen
// by invoking ls -l /dev. Every device driver registers its major number with
// the kernel and is completely responsible for managing its minor numbers. Use
// of any device with that major number will fall on the same device driver,
// regardless of the minor number. As a result, every driver needs to register a
// major number, even if it only deals with a single device, like a pointing
// tool.
//
// Since the kernel keeps a static table of device drivers, frivolous allocation
// of major numbers is rather wasteful of RAM. The Linux kernel, therefore,
// offers a simplified interface for simple drivers—those that will register a
// single entry point. Note that, in general, allocating the whole name space of
// a major number to every device is beneficial. This allows the handling of
// multiple terminals, multiple serial ports and several disk partitions without
// any overhead in the kernel proper: a single driver takes care of all of them,
// and uses the minor number to differentiate.
//
// Major number 10 is officially assigned to the misc driver. Modules can
// register individual minor numbers with the misc driver and take care of a
// small device, needing only a single entry point.
static struct miscdevice helloworld_miscdevice = {
  .minor = MISC_DYNAMIC_MINOR,
  .name = "mydev",  // the registered device can be found at /dev/mydev
  .fops = &my_dev_fops,
};

static const struct of_device_id my_of_ids[] = {
  {.compatible = "arrow,hellokeys"},
  {},
};
MODULE_DEVICE_TABLE(of, my_of_ids);

// TODO: is __init needed?
static int __init my_probe(struct platform_device *pdev) {
  int retval;
  pr_info("my_probe() is called.\n");
  retval = misc_register(&helloworld_miscdevice);
  // TODO: is this right? misc_register returns 0 on success, negative val on
  // failure.
  if (retval) {
    return retval;
  }
  pr_info("mydev: got minor %i\n", helloworld_miscdevice.minor);
  return 0;
}

// TODO: is __exit needed?
static int __exit my_remove(struct platform_device *pdev) {
  pr_info("my_remove() is called.\n");
  misc_deregister(&helloworld_miscdevice);
  return 0;
}

static struct platform_driver my_platform_driver = {
  .probe = my_probe,
  .remove = my_remove,
  .driver = {
    .name = "hellokeys",
    .of_match_table = my_of_ids,
    .owner = THIS_MODULE,
  }
};
module_platform_driver(my_platform_driver);

MODULE_AUTHOR("lizhi");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Platform driver for hellokeys device registered in DT.");
