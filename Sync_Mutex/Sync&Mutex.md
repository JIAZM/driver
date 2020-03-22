# Linux内核同步与互斥机制

## *<u>本文中进程指驱动中的进程上下文代码</u>*

> 对于某个字符设备驱动，要求同一时刻只能被一个进程打开  
> 两个进程同时对共享资源进行操作， 产生竞争行为  
> 产生结果 ---> 进程对共享资源的访问都不是完整的  

```C
int xxx_count = 0;
int testOpen(struct inode *p, struct file *f)
{
    if(xxx_count)   // xxx_count 为共享资源， 访问共享资源的代码块为临界区
    > 临界区 需要以某种互斥机制加以保护 ： 自旋锁， 信号量 等  
    // 对于驱动的 open 函数而言， 返回值位0表示成功， 返回负值表示失败
}
```
***由于以上代码不是原子操作，在CPU调度时不一定会在哪个位置产生调度***  
***如果在函数内部产生调度到其他进程则无法起到作用***

## ***<u>对共享资源加锁， 避免竞争行为</u>***

- 做到互斥访问
    > 访问共享资源的代码区 ---> “ 临界区 ”  
    > 临界区 需要以某种互斥机制加以保护 ： 自旋锁， 信号量 等  

- ## ***信号量***
    > Linux 中的信号量是一种睡眠锁。假如进程 A 先持有信号量 F， 然后进程 B 试图  
    > 获得已经被进程 A 持有的信号量 F 时 (假设信号量 F 资源值为1)，信号量会将  
    > 进程 B 推入等待队列，然后让其睡眠。当持有信号量的进程 A 将信号量 F 释放后  
    > 进程 B 才会被唤醒，从而获得这个信号量，继续执行进程 B 的代码  

    ## <u>***由于信号量的睡眠特性，使得信号量适用于锁会被长时间特有的情况，信号量只能在进程中使用，不能在中断中使用***</u>

    ```C
    /* 信号量基本使用形式 */
    struct semaphore sem;   // 定义信号量
    void sema_init(struct semaphore *sem, int val); // 初始化信号量
    static inline void init_MUTEX (struct semaphore *sem)// 初始化互斥信号量 - 2.6.25及以后的linux内核版本废除了init_MUTEX函数
    {
        sema_init(sem, 0);
    }
    static inline void init_MUTEX_LOCKED (struct semaphore *sem)// 释放互斥信号量 - 2.6.25及以后的linux内核版本废除了init_MUTEX函数
    {
        sema_init(sem, 0);
    }
    // 互斥信号量 - 只有一个资源的信号量，等同于 :
    sema_init(struct semaphore *sem, 1);

    /* 获得信号量 */
    void down(struct semaphore *sem);
    // 获得信号量sem，会导致睡眠，不能用于中断上下文
    int down_interruptible(struct semaphore *sem);
    /* 
     * 与 down 不同之处在于 调用down进入睡眠状态的进程不能被信号打断，调用
     * down_interruptible进入睡眠状态的进程可以被信号打断，信号也会导致
     * 该函数返回，此时函数返回值非0
     */
    int down_trylock(struct semaphore *sem);
    // 尝试获得信号量 sem，如果能够立即获得，就获得该信号量返回0
    // 否则返回非0值，不会导致调用者睡眠，可以在中断上下文使用

    /* 释放信号量 */
    void up(struct semaphore *sem);
    // 该函数释放信号量 sem，唤醒等待进程
    ```

    - ### ***<u>信号量使用注意</u>***
        1. 可以长期加锁
        2. 只能用于进程上下文，不能用于中断上下文
        3. 在持有自旋锁的同时，不能持有信号量


- ## ***自旋锁***
    ***<u>得到自旋锁后不能引起阻塞，否则会引起死锁</u>***  
    - ### Linux 内核中可能引起互斥访问的情形
      1. 进程和进程之间 - 信号量
      2. 进程和其他内核代码 - 自旋锁
      3. 进程和中断代码 - 自旋锁
      > 自旋锁最多只能被一个内核任务持有。若锁未被持有，请求她的任务便立即得到她并且执行  
      > 若一个内核任务试图请求一个已经被别的内核任务持有的自旋锁  
      > CPU会一直进行：忙循环——旋转——等待锁重新可用

    - ### ***自旋锁的使用方法***
        ```C
        spinlock_t my_lock = SPIN_LOCK_UNLOCKED;    // 静态初始化自旋锁
        void spin_lock_init(spinlock_t *lock);
        // 动态初始化自旋锁
        void spin_lock(spinlock_t *lock);
        // 得到自旋锁
        void spin_unlock(spinlock_t *lock);
        // 释放自旋锁
        ```
    - ### ***衍生函数***
        > 尽管自旋锁可以保证临界区不受别的执行单元抢占打扰  
        > 得到锁的代码在执行临界区时仍然可能受到本地中断的影响
        > 为了防止这种影响，需要用到自旋锁的衍生函数
        ```C
        void spin_lock_irqsave(spinlock_t *lock, unsigned long flags);
        // 获得自旋锁之前禁止本地 CPU 中断，之前的中断状态保存在flags中，可以避免进程上下文(共享资源的访问)被本地硬件中断打断
        // 相应的释放函数:
        void spin_unlock_irqrestore(spinlock_t *lock, unsigned long flags);

        void spin_lock_irq(spinlock_t *lock);
        // 获得自旋锁之前禁止本地 CPU 中断，但不保存中断状态
        // 相应的释放函数:
        void spin_unlock_irq(spinlock_t *lock);

        void spin_lock_bh(spinlock_t *lock);
        // 获得自旋锁之前禁止软中断，允许硬件中断
        // 相应的释放函数:
        void spin_unlock_bh(spinlock_t *lock);
        ```
    - ## ***自旋锁的使用案例***
        > drivers/net/8139cp.c 文件 cp_close() 进程上下文，cp_interrupt() 中断  
    - ## ***自旋锁使用注意***
        1. 会有系统开销(CPU 忙等)，不可以滥用
        2. 不可以长期加锁
        3. 在持有自旋锁的同时，不能持有信号量(不能调用会引起阻塞的函数，容易引起死锁)
        4. 在持有自旋锁的同时，不能在二次持有她(引起死锁)

- ## ***其他同步方法***
    - 原子操作
        - 原子整数操作  
            在 atomic.h 文件中定义
        - 原子位操作  
            在 bitops.h 文件中定义