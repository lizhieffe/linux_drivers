/**
 * Char device driver. To use, sudo open the /dev/sram-dev device.
 */
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/mutex.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/uaccess.h>  // Required for the copy_to_user() fn.
#include <linux/wait.h>

#define EEP_DEVICE_NAME "sram-dev"
#define EEP_CLASS "sram-class"

#define SRAM_SIZE 10

static DEFINE_MUTEX(device_list_lock);

static DECLARE_WAIT_QUEUE_HEAD(write_queue);
static DECLARE_WAIT_QUEUE_HEAD(read_queue);
static bool ready_to_read = false;

struct pcf2127 {
  struct cdev cdev;
  unsigned char *sram_data;
  struct i2c_client *client;
  int sram_size;
  int users;
};

static unsigned int sram_major = 0;
static struct class *sram_class = NULL;

static struct pcf2127 my_dev = {
  .sram_data = NULL,
  .client = NULL,
  .users = 0,
};

static int sram_open(struct inode *inode, struct file *filp) {
  unsigned int maj = imajor(inode);
  unsigned int min = iminor(inode);

  struct pcf2127 *pcf = NULL;
  pcf = container_of(inode->i_cdev, struct pcf2127, cdev);

  mutex_lock(&device_list_lock);
  pcf->sram_size = SRAM_SIZE;

  if (maj != sram_major || min < 0) {
    pr_err("===lizhi device not found\n");
    mutex_unlock(&device_list_lock);
    return -ENODEV;  // No such device
  }

  if (pcf->sram_data == NULL) {
    pcf->sram_data = kzalloc(pcf->sram_size, GFP_KERNEL);
    if (pcf->sram_data == NULL) {
      pr_err("===lizhi Open: memory allocation failed\n");
      mutex_unlock(&device_list_lock);
      return -ENOMEM;
    }
  }

  pcf->users++;
  pr_info("===lizhi opening file <%d,%d>, total users: %d\n",
      maj, min, pcf->users);
  // Set the private data on open so that the other file operation handler can
  // use it later.
  filp->private_data = pcf;
  mutex_unlock(&device_list_lock);

  return 0;
}

static int sram_release(struct inode *inode, struct file *filp) {
  unsigned int maj = imajor(inode);
  unsigned int min = iminor(inode);

  struct pcf2127 *pcf = NULL;
  pcf = container_of(inode->i_cdev, struct pcf2127, cdev);

  mutex_lock(&device_list_lock);
  filp->private_data = NULL;

  /* last close? */
  pcf->users--;
  if (!pcf->users) {
    // TODO: do necessary clean up for initialized resources, after we add logic
    // to init resource.
  }
  pr_info("===lizhi releasing file <%d,%d>, total users: %d\n",
      maj, min, pcf->users);
  mutex_unlock(&device_list_lock);

  return 0;
}

static int number_writes = 0;
// |filp| is the file to write, |buf| contains the bytes in user space that user
// wants to write, |count| is the size of |buf|, f_pos is the position in the
// kernel space destination to start writing.
ssize_t eeprom_write(struct file *filp, const char __user *buf, size_t count,
    loff_t *f_pos) {
  struct pcf2127 *pcf = NULL;

  mutex_lock(&device_list_lock);
  number_writes++;
  printk(KERN_ALERT "EBBChar: Device has been write %d time(s)\n", number_writes);

  printk("===lizhi eeprom_write is called: buf = %s, count = %ld, f_pos = %lld",
      buf, count, *f_pos);

  pcf = filp->private_data;  // private_data is a void *

  // Writing beyond the end of data allocated mem is not allowed.
  if (*f_pos >= pcf->sram_size) {
    mutex_unlock(&device_list_lock);
    return -EINVAL;
  }

  if (*f_pos + count > pcf->sram_size) {
    count = pcf->sram_size - *f_pos;
  }

  if (copy_from_user(pcf->sram_data, buf, count) != 0) {
    mutex_unlock(&device_list_lock);
    return -EFAULT;
  }

  printk("===lizhi wrote %ld bytes", count);
  *f_pos += count;
  
  ready_to_read = true;
  wake_up_interruptible(&read_queue);

  mutex_unlock(&device_list_lock);
  return count;
}

static int number_reads = 0;
ssize_t eeprom_read(struct file *filp, char __user *buf, size_t count,
    loff_t *f_pos) {
  struct pcf2127 *pcf = NULL;
  
  mutex_lock(&device_list_lock);
  number_reads++;
  printk(KERN_ALERT "===lizhi: Device has been read %d time(s)\n", number_reads);

  printk("===lizhi eeprom_read is called: count = %ld, f_pos = %lld",
      count, *f_pos);

  pcf = filp->private_data;

  if (*f_pos >= pcf->sram_size) {
    mutex_unlock(&device_list_lock);
    return 0;
  }

  if (*f_pos + count > pcf->sram_size) {
    count = pcf->sram_size - *f_pos;
  }

  if (copy_to_user(buf, pcf->sram_data, count) != 0) {
    mutex_unlock(&device_list_lock);
    return -EIO;
  }

  printk("===lizhi read %ld bytes", count);
  *f_pos += count;
  mutex_unlock(&device_list_lock);
  return count;
}


static unsigned int eep_poll(struct file *file, poll_table *wait) {
  unsigned int reval_mask;
  reval_mask = 0;

  pr_info("===lizhi eep_poll waiting for ready");

  poll_wait(file, &write_queue, wait);
  poll_wait(file, &read_queue, wait);

  mutex_lock(&device_list_lock);
  if (true) {
    reval_mask |= (POLLOUT | POLLWRNORM);
    pr_info("===lizhi eep_poll ready to write");
  }
  if (ready_to_read) {
    reval_mask |= (POLLIN | POLLRDNORM);
    pr_info("===lizhi eep_poll ready to read");
    ready_to_read = false;  // TODO: there can still be some race condition.
  }
  mutex_unlock(&device_list_lock);

  return reval_mask;
}

static struct file_operations eep_fops = {
  .open = sram_open,
  .release = sram_release,
  .read = eeprom_read,
  .write = eeprom_write,
  .poll = eep_poll,
};

dev_t dev_num;  // uint32. major/minor number

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
