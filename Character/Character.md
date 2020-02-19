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

// 向内核注册设备号
int register_chrdev_region(dev_t from, unsigned count, const char *name);
// 将设备行为 写入 设备描述中
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
};  // 在 include/linux/Fs.h 中声明
```

### 驱动的生存周期 - 从 insmod 到 rmmod  

## ***<u>内核打印</u>***
```C
//
printk();
```

## ***<u>内核空间申请内存</u>***
```C
(void *) kzalloc(size， GFP_KERNEL);  // 将申请到的空间中值全部清零
(void *) kzalloc(size， GFP_DMA);
(void *) kmalloc();  // 申请空间
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
