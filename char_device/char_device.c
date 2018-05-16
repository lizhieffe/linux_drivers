/**
 * Char device driver. To use, sudo open one of the /dev/eep-mem[0-7] device.
 */
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/uaccess.h>  // Required for the copy_to_user() fn.

#define EEP_NBANK 8
#define EEP_DEVICE_NAME "eep-mem"
#define EEP_CLASS "eep-class"

static char msg[256] = "abcd\n";

// Handler for file ops.
static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);

static struct file_operations eep_fops = {
  .open = dev_open,
  .release = dev_release,
  .read = dev_read,
  .write = dev_write,
};

struct class *eep_class;
struct cdev eep_cdev[EEP_NBANK];
dev_t dev_num;  // uint32. major/minor number


static int number_opens = 0;
static int dev_open(struct inode *inodep, struct file *filep){
  number_opens++;
  printk(KERN_ALERT "EBBChar: Device has been opened %d time(s)\n", number_opens);
  return 0;
}

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
  int i;
  int err;
  dev_t curr_dev;
  struct device *curr_device;

  // Request kernel for device number
  alloc_chrdev_region(&dev_num, 0, EEP_NBANK, EEP_DEVICE_NAME);
  printk("===lizhi assigned dev_num = <%lu,%u>\n",
         (unsigned long)MAJOR(dev_num), MINOR(dev_num));

  eep_class = class_create(THIS_MODULE, EEP_CLASS);
  if (IS_ERR(eep_class)) {
    unregister_chrdev(MAJOR(dev_num), EEP_DEVICE_NAME);
    printk(KERN_ALERT "Failed to register device class\n");
    return PTR_ERR(eep_class);  // correct way to return an error on a pointer.
  }
  printk(KERN_INFO "===lizhi device class registered correctly\n");

  for (i = 0; i < EEP_NBANK; i++) {
    // Tie file_operations to the cdev
    cdev_init(&eep_cdev[i], &eep_fops);
    printk("===lizhi open function address: %p", eep_fops.open);
    eep_cdev[i].owner = THIS_MODULE;

    // Device number to be use to add cdev to the core.
    curr_dev = MKDEV(MAJOR(dev_num), MINOR(dev_num) + i);

    // Add cdev to the core.
    // TODO: what is 
    err = cdev_add(&eep_cdev[i],
        curr_dev, /* the major and minor of the device */
        1 /* the number of consecutive minor numbers corresponding to this device */);
    if (err != 0) {
      printk(KERN_ALERT "===lizhi Failed to add cdev.\n");
      return err;
    }
    printk(KERN_INFO "===lizhi add cdev correctly\n");

    // Create a device node each device /dev/eep-mem[0-7]. With our class used
    // here, devices can also be viewed under /sys/class/eep-class.
    curr_device = device_create(eep_class,
        NULL, /* no parent device */
        curr_dev,
        NULL, /* no additional data */
        EEP_DEVICE_NAME "%d", i); /* eep-mem[0-7] */
    if (IS_ERR(curr_device)) {
      class_destroy(eep_class);
      unregister_chrdev(MAJOR(dev_num), EEP_DEVICE_NAME);
      printk(KERN_ALERT "Failed to create the device\n");
      return PTR_ERR(curr_device);
    }
    printk(KERN_INFO "===lizhi device <%lu,%u> registered correctly\n",
        (unsigned long)MAJOR(dev_num), MINOR(dev_num) + i);
  }

  return 0;
}

static void __exit my_exit(void) {
  int i;
  for (i = 0; i < EEP_NBANK; i++) {
    device_destroy(eep_class, MKDEV(MAJOR(dev_num), MINOR(dev_num) + i));     // remove the device
  }
  class_unregister(eep_class);                          // unregister the device class
  class_destroy(eep_class);                             // remove the device class
  unregister_chrdev(MAJOR(dev_num), EEP_DEVICE_NAME);             // unregister the major number
  printk(KERN_INFO "EBBChar: Goodbye from the LKM!\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("lizhi");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Sample module with char device.");
