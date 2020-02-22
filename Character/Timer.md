# Linux定时器实现

## 使用场景：  
> 延后执行某个动作  
> 定期查询硬件状态  
> ...

## 内核相关时间概念
- HZ  
    通过 CONFIG_HZ 来设置，范围为 100 ~ 1000
    > HZ 决定了系统时钟中断发生的频率， HZ 值不能设置太大或太小  

- jiffies  
    记录内核自启动以来的节拍数  
    > 全局变量 jiffies 用来记录子系统启动以来产生的节拍总数。启动时内核将该变量  
    > 初始化为0, 此后每次时钟中断处理程序都会增加该变量的值  
    > 因为 1 秒内时钟中断的次数等于 HZ ，jiffies 一秒以内增加的值也等于 HZ  

## ***定时器代码解读***

- 定时器相关函数  

```C
struct timer_list mytimer;
// 内核中每一个timer都用一个timer_list结构体来描述

struct timer_list {
// 这是 Kernel 3.0.8 中对于 timer_list 的描述
// 与 当前编译环境 5.3.28 不同
// 详细信息可以参考 kernel 4.15 kernel/timer/timer.c 等代码
    struct list_head entry; // 双向链表
    /*
     * struct list_head {
     *     struct list_head *next, *prev;
     * };
     */
    unsigned long expires;  // 超时时间
    struct tvec_base *base; 
    // 时钟管理的结构体，表示这个结构体是隶属于这个base来管理的

    void (*function)(unsigned long);    // 超时执行的动作
    unsigned long data; // 作为参数传给函数
    
    int slack;
#ifdef CONFIG_TIMER_STATS
    int start_pid;
    void *start_site;
    char start_comm[16];
#endif

#ifdef CONFIG_LOCKDEP   // 是否定义需要查看 .config 文件
    struct lockdep_map lockdep_map;
#endif
};

// setup_timer 函数
// 在 include/linux/timer.h 中声明

```
## linux定时器使用实例
- 内核注册定时器最终都会通过调用 internal_add_timer 来实现，具体工作方式：
    1. 如果定时器在接下来的 0~255 个 jiffies 中到期，则将定时器添加到 v1
    2. 如果定时器在接下来的 255*64 个 jiffies 中到期，则将定时器添加到 v2
    3. 如果定时器在接下来的 255*64*64 个 jiffies 中到期，则将定时器添加到 v3
    4. 如果定时器在接下来的 255*64*64*64 个jiffies 中到期，则将定时器添加到 v4
    5. 如果有更大的超时时间，则利用0xffffffff来计算 hash ，然后插入到 v5 (这个只会出现在64bit的系统中)
