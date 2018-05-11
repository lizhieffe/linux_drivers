#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kmod.h>
#include <linux/module.h>
#include <linux/workqueue.h>

static void delayed_ls(struct work_struct *unused);

static DECLARE_DELAYED_WORK(work, delayed_ls);

static void delayed_ls(struct work_struct *unused) {
  char *argv[] = { "/bin/bash", "-c", "/bin/ls >> /tmp/list", NULL};
  static char *envp[] = {"HOME=/",
                         "TERM=linux",
                         "PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL };
     
  int result = call_usermodehelper( argv[0], argv, envp, UMH_WAIT_PROC );
  printk("===lizhi call_usermodehelper result is %d", result);
}

static int __init my_init(void) {
  schedule_delayed_work(&work, msecs_to_jiffies(200));
  return 0;
}

static void __exit my_exit(void) {
  return;
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("lizhi");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Sample module that triggers a delayed system shutdown");
