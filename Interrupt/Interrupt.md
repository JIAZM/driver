# **Linux 驱动中断剖析**

## ***注册中断相关函数***
```C
// 注册中断
#include <linux/interrupt.h>

static inline int __must_check
request_irq(unsigned int irq, irq_handler_t handler, unsigned long flags, const char *name, void *dev)
{
    return request_threaded_irq(irq, handler, NULL, flags, name, dev);
}
/*
 * irq是要申请的硬件中断号
 *
 * handler是向系统注册的中断处理函数，是一个回调函数，中断发生时，系统调用这个
 * 函数，dev_id参数将被传递给它
 *
 * flags是中断处理的属性，若设置了IRQF_DISABLED （老版本中的SA_INTERRUPT，
 * 本版已经不支持了），则表示中断处理程序是快速处理程序，快速处理程序被调用时屏蔽
 * 所有中断，慢速处理程序不屏蔽；若设置了IRQF_SHARED （老版本中的SA_SHIRQ），
 * 则表示多个设备共享中断，若设置了IRQF_SAMPLE_RANDOM
 * （老版本中的SA_SAMPLE_RANDOM），表示对系统熵有贡献，对系统获取随机数有好处
 * （这几个flag是可以通过或的方式同时使用的）
 *
 * devname设置中断名称，通常是设备驱动程序的名称在
 * cat /proc/interrupts中可以看到此名称。
 *
 * dev_id在中断共享时会用到，一般设置为这个设备的设备结构体或者NULL
 *
 * 返回0表示成功，返回-INVAL表示中断号无效或处理函数指针为NULL
 *              返回-EBUSY表示中断已经被占用且不能共享。
 */

int __must_check
request_threaded_irq(unsigned int irq, irq_handler_t handler,
                     irq_handler_t thread_fn,
                     unsigned long flags, const char *name, void *dev);
```

## ***共享中断***
> 需要硬件的支持  
> 详细实例可以参考 /linux-kernel/drivers/tty/serial/8250.c 代码

## ISR 编写注意

> ISR 中断处理程序  
1. 处于中断上下文，不能调用任何可能引起睡眠的代码
2. 不能调用任何可能把自己调度出去的函数 - 没有现场保护，PC指针会不来
3. 不能调用disable_irq / enable_irq 之类的代码
4. 代码应该尽量精炼