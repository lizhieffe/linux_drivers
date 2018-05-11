#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>

#define EEP_NBANK 8
#define EEP_DEVICE_NAME "eep-mem"
#define EEP_CLASS "eep-class"

// Handler for file ops.
static int dev_open(struct inode *inodep, struct file *filep);
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset);
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset);
static int dev_release(struct inode *inodep, struct file *filep);

struct class *eep_class;
struct cdev eep_cdev[EEP_NBANK];
dev_t dev_num;  // uint32. major/minor number
struct file_operations eep_fops = {
  .open = dev_open,
  .write = dev_write,
  .read = dev_read,
  .release = dev_release,
};

static int number_opens = 0;
static int dev_open(struct inode *inodep, struct file *filep){
  number_opens++;
  printk("EBBChar: Device has been opened %d time(s)\n", number_opens);
  return 0;
}

static int number_writes = 0;
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset) {
  number_writes++;
  printk("EBBChar: Device has been write %d time(s)\n", number_writes);
  return 0;
}

static int number_reads = 0;
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset) {
  number_reads++;
  printk("EBBChar: Device has been read %d time(s)\n", number_reads);
  return 0;
}

static int dev_release(struct inode *inodep, struct file *filep){
     printk(KERN_INFO "EBBChar: Device successfully closed\n");
        return 0;
}

static int __init my_init(void) {
  int i;
  dev_t curr_dev;

  // Request kernel for device number
  alloc_chrdev_region(&dev_num, 0, EEP_NBANK, EEP_DEVICE_NAME);
  printk("===lizhi assigned dev_num = <%lu,%lu>\n",
         (unsigned long)MAJOR(dev_num), MINOR(dev_num));

  eep_class = class_create(THIS_MODULE, EEP_CLASS);

  for (i = 0; i < EEP_NBANK; i++) {
    // Tie file_operations to the cdev
    cdev_init(&eep_cdev[i], &eep_fops);
    eep_cdev[i].owner = THIS_MODULE;

    // Device number to be use to add cdev to the core.
    curr_dev = MKDEV(MAJOR(dev_num), MINOR(dev_num) + i);

    // Add cdev to the core.
    // TODO: what is 
    cdev_add(&eep_cdev[i], curr_dev,
             1 /* the number of consecutive minor numbers corresponding to this device */);

    // Create a device node each device /dev/eep-mem[0-7]. With our class used
    // here, devices can also be viewed under /sys/class/eep-class.
    device_create(eep_class,
        NULL, /* no parent device */
        curr_dev,
        NULL, /* no additional data */
        EEP_DEVICE_NAME "%d", i); /* eep-mem[0-7] */
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
