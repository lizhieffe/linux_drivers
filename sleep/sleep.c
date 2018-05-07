#include <linux/delay.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/workqueue.h>

static DECLARE_WAIT_QUEUE_HEAD(my_wq);
static int condition = 0;

// Declare a work queue.
static struct work_struct wrk;

static void work_handler(struct work_struct *work) {
  pr_info("Waitqueue module handler %s running in PID: %d\n", __FUNCTION__, task_pid_nr(current));
  msleep(5000);
  pr_info("Wake up the sleeping module\n");
  condition = 1;
  wake_up_interruptible(&my_wq);
}

static int __init my_init(void) {
  pr_info("Wait queue example\n");

  INIT_WORK(&wrk, work_handler);
  // Put work task in global waitqueue.
  schedule_work(&wrk);

  pr_info("Going to sleep %s\n", __FUNCTION__);
  // Block the current task in the wait queue |my_wq|. Note that this runs in a
  // different wait queue than the schedule_work().
  wait_event_interruptible(my_wq, condition != 0);

  pr_info("Woken up by the work job running in PID: %d\n", task_pid_nr(current));
  return 0;
}

static void __exit my_exit(void) {
  pr_info("Waitqueue example cleanup\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("lizhi");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Sample module to test sleep and wake up");
