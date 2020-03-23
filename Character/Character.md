# 字符设备

> 内核中通过类型 dev_t 来描述设备号， 其实质是 unsigned int 32位整形数  
> 其中高12位为主设备号，低20位为次设备号  
> 同一类型的不同设备拥有相同的主设备号和不同的次设备号  

## ***<u>设备的注册</u>***
```C
#define MINORBITS   20
#define MINORMASK   ((1U << MINORBITS) - 1)

// 查看主设备号  
#define MAJOR(dev)   ((unsigned int)((dev) >> MINORBITS))

// 查看次设备号
#define MINOR(dev)  ((unsigned int)((dev) & MINORMASK))

// 生成设备号
#define MKDEV(ma, mi)   (((ma) << MINORBITS) | (mi))
/* dev_t dev = MKDEV(ma, mi); */

/* 向内核注册设备号 - 静态
 * from 申请注册设备的起始设备号
 * count 需要申请的连续设备个数
 * name 设备名
 * 成功返回0    否则返回错误代码 */
int register_chrdev_region(dev_t from, unsigned count, const char *name)
{
    struct char_device_struct *cd;
    dev_t to = from + count;
    dev_t n, next;

    for (n = from; n < to; n = next) {
        next = MKDEV(MAJOR(n)+1, 0);
        if (next > to)
            next = to;
        // 当count < 2^20 时注册的主设备号不变
        cd = __register_chrdev_region(MAJOR(n), MINOR(n),
                   next - n, name);
        if (IS_ERR(cd))
            goto fail;
    }
    return 0;
fail:
    to = n;
    for (n = from; n < to; n = next) {
        next = MKDEV(MAJOR(n)+1, 0);
        kfree(__unregister_chrdev_region(MAJOR(n), MINOR(n), next - n));
    }
    return PTR_ERR(cd);
}

/* 使用内核动态分配的设备号进行注册
 * dev 做为动态分配的设备号 输出
 * baseminor 起始子设备号
 * count 申请的子设备数
 * name 设备名称 执行 cat /proc/devices 显示的名称 */
 int alloc_chrdev_region(dev_t *dev, unsigned baseminor,
                        unsigned count, const char *name)
{
    struct char_device_struct *cd;
    /* 调用 __register_chrdev_region(unsigned int major,    \
                                unsigned int baseminor,     \
                                int minorct, const char *name)
     * major == 0 时函数动态分配主设备号并返回
     * major > 0 时函数尝试保留通过的子设备号范围并在成功时返回0 */
    cd = __register_chrdev_region(0, baseminor, count, name);
    if (IS_ERR(cd))
        return PTR_ERR(cd);
    *dev = MKDEV(cd->major, cd->baseminor);
    return 0; 
}
// 将 设备行为 写入 设备描述 中
void cdev_init(struct cdev *dev, const struct file_opreations *fops);
// 向内核注册设备所有信息
int cdev_add(struct cdev *p, dev_t dev, unsigned count);
```

## ***<u>字符设备描述信息机制</u>***
```C
struct cdev{
    struct kobject kobj;    // 一种设备管理机制，有内核中设备管理模型操作
    struct module *owner;
    const struct file_operations *ops;  // 文件操作函数集
    struct list_head list;
    dev_t dev;              // 设备号
    unsigned int count;     // 支持的设备数量
};

struct file_operations{
    struct module *owner;
    ...
    ssize_t (*read)(struct file *, char __user *, sizeof_t, loff_t *);
    ssize_t (*read)(struct file *, const char __user *, sizeof_t, loff_t *);
    int (*open)(struct inode *, struct file *);
    int (*release)(struct inode *, struct file *);
    ...
};  // 在 include/linux/fs.h 中声明
```

### 驱动的生存周期 - 从 insmod 到 rmmod  

## ***<u>内核打印</u>***
```C
int printk(const char *fmt, ...);
// 使用方法 : printk(KERN_XXXXX"YYYYY");

/* Kernel日志级别 - 内核中一共有8种级别
 * #define	KERN_EMERG	"<0>"	    // system is unusable
 * #define	KERN_ALERT	"<1>"	    // action must be taken immediately
 * #define	KERN_CRIT	"<2>"	    // critical conditions
 * #define	KERN_ERR	"<3>"	    // error conditions
 * #define	KERN_WARNING	"<4>"	// warning conditions
 * #define	KERN_NOTICE	"<5>"	    // normal but significant condition
 * #define	KERN_INFO	"<6>"	    // informational
 * #define	KERN_DEBUG	"<7>"	    // debug-level messages
 */
```

## ***<u>内核空间申请内存</u>***
```C
static inline void *kzalloc(size_t size, gfp_t flags);  // 将申请到的空间中值全部清零
void *kmalloc(size_t size, gfp_t flags);  // 申请空间，不清零
// 释放内存空间
void kfree(const void *objp);
/*
* flags:
* |-进程上下文，可以睡眠     GFP_KERNEL
* |-进程上下文，不可以睡眠    GFP_ATOMIC
* | |-中断处理程序          GFP_ATOMIC
* | |-软中断               GFP_ATOMIC
* | |-Tasklet             GFP_ATOMIC
* |-用于DMA的内存，可以睡眠         GFP_DMA | GFP_KERNEL
* |-用于DMA的内存，不可以睡眠       GFP_DMA | GFP_ATOMIC
*/

void *vmalloc(unsigned long size);  // 创建一块虚拟地址连续的内存空间, 不能保证物理地址连续     分配内存是可能产生阻塞
void vfree(const void *addr);
```

## ***<u>驱动模块编译</u>***
```Makefile
ifneq ($(KERNELRELEASE),)
    obj-m := charDev.o
else
    PWD := $(shell pwd)
    KDIR := /lib/modules/`uname -r`/build

all:
    make -C $(KDIR) M=$(PWD)    #在内核中编译 M 指定的路径
    # -C $(KDIR) change到内核源码目录中编译 - 驱动依赖内核存在
    # M 制定编译的路径

clean:
    rm -rf *.o *.ko *.mod.c *.symvers *.c~ *~

endif
```
