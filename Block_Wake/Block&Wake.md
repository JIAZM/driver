# **驱动程序中的阻塞与唤醒**

## **应用程序的阻塞与唤醒**  

- 应用层 opne()函数的打开标志
    > open() 函数 man 文档  
    ```C
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <fcntl.h>

    int open(const char *pathname, int flags);
    int open(const char *pathname, int flags, mode_t mode);

    // flags 参数 O_NONBLOCK 选择表示以非阻塞模式打开，否则以阻塞模式打开
    ```

    > open()时不指定 O_NONBLOCK 表示阻塞操作  
    > 那么后续对此设备的所有操作，包括对此设备调用read()，write(),ioctl()等都是阻塞操作  

    ### <u>***应用层的 flags(位图) 参数传进内核中，在(struct file \*)filep->f_flags 中体现***</u>  
    ### <u>***应用层的文件描述符传进内核，传到 「struct file」 结构体中***</u>  
    
    - 对于阻塞操作
        1. 若条件得到满足，则立即返回，return 正确的返回值
        2. 若条件暂时得不到满足，则一直等待，知道条件得到满足以后，才会 return 返回
    - 对于非阻塞操作
        1. 若条件不满足，直接 return 错误代码
    - 对驱动的影响 - 需要驱动支持

## ***Linux中关于阻塞(睡眠)的实现机制***

> 阻塞的『等待』在驱动中 ---› 睡眠  

```C
/* Linux 中一个等待队列由一个wait_queue_head_t类型的结构来描述 */
struct __wait_queue_head{
    spinlock_t lock;
    struct list_head task_list; // 等待队列 - 双向链表
};  // 在 include/linux/wait.h 中定义
typedef struct __wait_queue_head wait_queue_head_t;

/* 等待队列定义 */
static wait_queue_head_t testqueue;
/*
 * 在 linux-4.15 中定义为
 * struct wait_queue_head {
 *     spinlock_t              lock;
 *     struct list_head        head;
 * };
 * typedef struct wait_queue_head wait_queue_head_t;
 */
// 初始化调用方法
init_waitqueue_head(&testqueue);
/*
 * extern void __init_waitqueue_head(struct wait_queue_head *wq_head, const char *name, struct lock_class_key *);
 *
 * #define init_waitqueue_head(wq_head)                         \
 *     do {
 *         static struct lock_class_key __key;                  \
 *         __init_waitqueue_head((wq_head), #wq_head, &__key);  \
 *     }while(0)
 *
 * void __init_waitqueue_head(struct wait_queue_head *wq_head, const char *name, struct lock_class_key *key)
 * {
 *         spin_lock_init(&wq_head->lock);
 *         lockdep_set_class_and_name(&wq_head->lock, key, name);
 *         INIT_LIST_HEAD(&wq_head->head);
 * }
 */


// Linux 内核提供的睡眠函数 在 include/linux/wait.h 中声明
wait_event(queue, contidion);
// 以 wait_event 为例，具体的调用代码见 include/linux/wait.h
/*
 * 调用 wait_event()
 * 对结构 struct wait_queue_entry 初始化
 * struct wait_queue_entry {
 *     unsigned int        flags;
 *     void                *private;    // 指向当前进程结构体 - 进程控制块
 *     wait_queue_func_t   func;
 *     struct list_head    entry;
 * };
 *
 * 调用 prepare_to_wait_event(&wq_head, &__wq_entry, state)；
 *      // 其中 state = TASK_UNINTERRUPTIBLE    进程标记，表示进程不可打断
 *      // 函数在 kernel/sched/wait.c 中定义
 *
 * __builtin_constant_p()   // GCC 内建函数， 用于判断一个值是否为编译时常数，若值为常数则返回 1， 否则返回 0
 */
wait_event_interruptible(queue, condition);
wait_event_timeout(queue, condition, timeout);
wait_event_interruptible_timeout(queue, consition, timeout);
/*
 * 调用以上函数的进程会把它自己添加到queue对列上
 * 然后睡眠直到condition为1
 * queue: wait_queue_head_t 类型的变量，表示要等待的队列头
 * condition: 条件判断
 * timeout: 超时时限
 */

/* 返回值：
 *      对于wait_event_interruptible, 返回0表示请求的条件得到满足
 * (condition变为1), 返回非0值表示被信号打断
 *      对于wait_event_interruptible_timeout， 返回值比较复杂:
 * return < 0 - 表示被信号打断
 * return > 0 - 表示条件得到满足 (condition -> 1) 并且时间还有富余，返回值表示剩余的时剪片
 * return = 0 - 表示超时，若返回值为0，应该监测condition值是否为1
 */

// Linux 提供的唤醒函数
void wake_up(wait_queue_head_t *queue);
// 唤醒所有在给定等待队列的进程
void wake_up_interruptible(wait_queue_head_t *queue);
// 唤醒所有在指定等待队列上的可中断的进程
/*
 * queue: wait_queue_head_t 类型的指针，表示队列头
 */

```