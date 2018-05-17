/**
 * Char device driver. To use, sudo open one of the /dev/eep-mem[0-7] device.
 */
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/uaccess.h>  // Required for the copy_to_user() fn.

#define EEP_DEVICE_NAME "sram-dev"
#define EEP_CLASS "sram-class"

#define SRAM_SIZE 10

struct pcf2127 {
  struct cdev cdev;
  unsigned char *sram_data;
  struct i2c_client *client;
  int sram_size;
};

static unsigned int sram_major = 0;
static struct class *sram_class = NULL;

static struct pcf2127 my_dev = {
  .sram_data = NULL,
  .client = NULL,
};

static int sram_open(struct inode *inode, struct file *filp) {
  unsigned int maj = imajor(inode);
  unsigned int min = iminor(inode);
  pr_info("===lizhi opening file <%d,%d>\n", maj, min);

  struct pcf2127 *pcf = NULL;
  pcf = container_of(inode->i_cdev, struct pcf2127, cdev);
  pcf->sram_size = SRAM_SIZE;

  if (maj != sram_major || min < 0) {
    pr_err("device not found\n");
    return -ENODEV;  // No such device
  }

  if (pcf->sram_data == NULL) {
    pcf->sram_data = kzalloc(pcf->sram_size, GFP_KERNEL);
    if (pcf->sram_data == NULL) {
      pr_err("Open: memory allocation failed\n");
      return -ENOMEM;
    }
  }

  // Set the private data on open so that the other file operation handler can
  // use it later.
  filp->private_data = pcf;
  pr_info("===lizhi file private data is set");
  return 0;
}

static char msg[256] = "abcd\n";

// Handler for file ops.
static int dev_release(struct inode *, struct file *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);

static struct file_operations eep_fops = {
  .open = sram_open,
  .release = dev_release,
  .read = dev_read,
  .write = dev_write,
};

dev_t dev_num;  // uint32. major/minor number

static int number_writes = 0;
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset) {
  number_writes++;
  printk(KERN_ALERT "EBBChar: Device has been write %d time(s)\n", number_writes);
  return 0;
}

static int number_reads = 0;
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset) {
  int error_count;

  number_reads++;
  printk(KERN_ALERT "===lizhi: Device has been read %d time(s)\n", number_reads);

  // copy_to_user has the format (* to, *from, size) and return 0 on success
  error_count = copy_to_user(buffer, msg, strlen(msg));

  if (error_count == 0) {
    printk(KERN_INFO "===lizhi: Sent %ld characters to the user\n",
        strlen(msg));
    return 0;
  } else {
    printk(KERN_ALERT "===lizhi: Failed to send %d characters to the user\n",
        error_count);
    return -EFAULT;  // Failed -- return a bad address message (i.e. -14)
  }
}

static int dev_release(struct inode *inodep, struct file *filep){
  printk(KERN_ALERT "EBBChar: Device successfully closed\n");
  return 0;
}

static int __init my_init(void) {
  int err;
  dev_t curr_dev;
  struct device *curr_device;

  // Request kernel for device number
  err = alloc_chrdev_region(&dev_num, 0, 1, EEP_DEVICE_NAME);
  if (err != 0) {
    pr_err("===lizhi alloc_chrdev_region failed.");
    return err;
  }
  sram_major = MAJOR(dev_num);
  printk("===lizhi assigned dev_num = <%lu,%u>\n",
         (unsigned long)MAJOR(dev_num), MINOR(dev_num));

  sram_class = class_create(THIS_MODULE, EEP_CLASS);
  if (IS_ERR(sram_class)) {
    unregister_chrdev(MAJOR(dev_num), EEP_DEVICE_NAME);
    printk(KERN_ALERT "Failed to register device class\n");
    return PTR_ERR(sram_class);  // correct way to return an error on a pointer.
  }
  printk(KERN_INFO "===lizhi device class registered correctly\n");

  // Tie file_operations to the cdev
  cdev_init(&my_dev.cdev, &eep_fops);
  my_dev.cdev.owner = THIS_MODULE;

  // Device number to be use to add cdev to the core.
  curr_dev = MKDEV(MAJOR(dev_num), MINOR(dev_num));

  // Add cdev to the core.
  err = cdev_add(&my_dev.cdev,
      curr_dev, /* the major and minor of the device */
      1 /* the number of consecutive minor numbers corresponding to this device */);
  if (err != 0) {
    printk(KERN_ALERT "===lizhi Failed to add cdev.\n");
    return err;
  }
  printk(KERN_INFO "===lizhi add cdev correctly\n");

  // Create a device node each device /dev/eep-mem[0-7]. With our class used
  // here, devices can also be viewed under /sys/class/eep-class.
  curr_device = device_create(sram_class,
      NULL, /* no parent device */
      curr_dev,
      NULL, /* no additional data */
      EEP_DEVICE_NAME); /* eep-mem[0-7] */
  if (IS_ERR(curr_device)) {
    class_destroy(sram_class);
    unregister_chrdev(MAJOR(dev_num), EEP_DEVICE_NAME);
    printk(KERN_ALERT "Failed to create the device\n");
    return PTR_ERR(curr_device);
  }
  printk(KERN_INFO "===lizhi device <%lu,%u> registered correctly\n",
      (unsigned long)MAJOR(dev_num), MINOR(dev_num));

  return 0;
}

static void __exit my_exit(void) {
  device_destroy(sram_class, MKDEV(MAJOR(dev_num), MINOR(dev_num)));     // remove the device
  class_unregister(sram_class);                          // unregister the device class
  class_destroy(sram_class);                             // remove the device class
  unregister_chrdev(MAJOR(dev_num), EEP_DEVICE_NAME);             // unregister the major number
  printk(KERN_INFO "EBBChar: Goodbye from the LKM!\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("lizhi");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Sample module with char device.");
