#include <linux/module.h>
#include <linux/time.h>
#include <linux/jiffies.h>

struct timer_list mytimer;	// 定义一个定时器

static void myfunc(struct timer_list *timer)	// linux-5.3.0.28 Kernel 版本
												// 对于struct timer_list 的描述与 3.0.8 不同
{
	printk("time(secs) = %ld \n", jiffies / HZ);
	mod_timer(&mytimer, jiffies + 3 * HZ);
}

static int __init mytimer_init(void)	// 初始化函数 - 注册timer
{
	printk(KERN_INFO "init timer ... \n");
	timer_setup(&mytimer, myfunc, 0);	// 初始化 timer_list (unsigned int)"my timer !"
	// 函数在 include/linux/timer.h 中声明
	mytimer.expires = jiffies + 1 * HZ;	// jiffies 表示内核总计节拍数 jiffies + 1*HZ 表示超时时间为1秒
	add_timer(&mytimer);	// 将 timer_list 注册到系统中

	return 0;
}

static void __exit mytimer_exit(void)
{
	printk(KERN_INFO "delete my timer !\n");
	del_timer(&mytimer);

	return;
}

module_init(mytimer_init);
module_exit(mytimer_exit);

MODULE_LICENSE("GPL");
