#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/platform_device.h>

#define BUFFER_MAX	10
#define OK	0
#define ERROR	-1
#define NAME	"testChar"

struct cdev *gDev;
struct file_operations *gFile;
dev_t devNo;
unsigned int subDevNo = 1;
int reg_magor = 232;
int reg_minor = 0;

// static struct resource *wdt_mem;	// platform设备驱动资源结构体

char *buffer;
int flag = 0;
char *kbuf = NULL;
char kbuf_rd[4] = "abc";
/* 
 * open file interface
 */
int testOpen(struct inode *p, struct file *f)
{
	printk(KERN_EMERG"platform char device test open function \n");

	return 0;
}

/* 
 * open write interface
 * 从 u 指向的用户空间 buffer 中拿到数据
 * 然后操作寄存器，写到硬件中去
 */
ssize_t testWrite(struct file *f, const char __user *u, size_t s, loff_t *l)
{
	int len = 0;

	printk(KERN_EMERG"platform char device test write function \n");

	if(kbuf == NULL){
		kbuf = (char *)kzalloc(sizeof(char) * s, GFP_KERNEL);
		if(kbuf == NULL){
			printk(KERN_EMERG"Create memory block for test write failed \n");
			return -ENOMEM;
		}
		else
			printk(KERN_EMERG"kbuf address : %x \n", kbuf);
	}
	len = copy_from_user(kbuf, u, s);
	if (len != 0){
		printk(KERN_EMERG"copy from user space failed \n");
		return -ENOMEM;
	}
	printk(KERN_EMERG"Get string : %s \n", kbuf);

	return 0;
}

/*
 * 从硬件中读取数据
 * 写到 u 指向用户下空间 buffer 中去
 */
ssize_t testRead(struct file *f, char __user *u, size_t s, loff_t *l)
{
	int len = 0;

	printk(KERN_EMERG"platform char device test read function \n");

	len = copy_to_user(u, kbuf_rd, 4);
	if (len != 0){
		printk(KERN_EMERG"copy %d Bytes to user space failed \n", len);
		return -ENOMEM;
	}
	printk(KERN_EMERG"Send string : %s \n", kbuf_rd);

	return 0;
}

/*
 * ioctl() 函数实现
 */
#ifndef ARGS_LIMIT
#define ARGS_LIMIT	4
#endif

static long testIOctl(struct file *f, unsigned int cmd, unsigned long arg)
{
	printk(KERN_EMERG"Get into ioctl() \n");
	switch(cmd){
		case 0:
			printk(KERN_EMERG"Do nothing in ioctl() \n");
			break;
		case 1:
			if(arg > ARGS_LIMIT)
				return -EINVAL;
			printk(KERN_EMERG"Get command %d and arg %ld in ioctl() \n", cmd, arg);
			break;
		default:
			return -EINVAL;
	}

	return 0;
}

static int chardev_probe(struct platform_device *pdev)
{
/*
	struct resource *res;
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if(res == NULL){
		dev_err(dev, "no memory resource specified \n");
		return -ENOENT;
	}
	size = (res->end - res->start) + 1;
	printk(KERN_EMERG"start = %x, len = %x \n", res->start, size);

	wdt_mem = request_mem_region(res->start, size, pdev->name);
	if(wdt_mem == NULL){
		dev_err(dev, "failed to get memory region \n");
		ret = -ENOENT;
		goto err_req;
	}

	led_base = ioremap(res_start, size);	// 计算size是为了在ioremap中使用
*/

	devNo = MKDEV(reg_magor, reg_minor);
	printk(KERN_EMERG"device number is %d \n", devNo);

	// register_chrdev_region(dev_t from, unsigned count, const char *name);
	// from 为设备号， count 为设备数， name 为设备文件名
	if(OK == register_chrdev_region(devNo, subDevNo, NAME)){
		printk(KERN_EMERG"register char device region OK \n");
		printk(KERN_EMERG"device number is %d \n", devNo);
	}
	else{
		printk(KERN_EMERG"register char device region error \n");
		return ERROR;
	}
	printk(KERN_EMERG"major number is %d , minor number is %d \n", MAJOR(devNo), MINOR(devNo));

	gDev = (struct cdev *)kzalloc(sizeof(struct cdev), GFP_KERNEL);
	gFile = (struct file_operations *)kzalloc(sizeof(struct file_operations), GFP_KERNEL);

	gFile->open = testOpen;
	gFile->read = testRead;
	gFile->write = testWrite;
	gFile->unlocked_ioctl = testIOctl;
	gFile->owner = THIS_MODULE;

	cdev_init(gDev, gFile);
	cdev_add(gDev, devNo, 3);

	return 0;
}

static int chardev_remove(struct platform_device *pdev)
{
	cdev_del(gDev);
	unregister_chrdev_region(devNo, subDevNo);
	printk(KERN_EMERG"unregister device module %s \n", NAME);

	kfree(gDev);
	printk(KERN_EMERG"free memory space struct cdev \n");

	kfree(gFile);
	printk(KERN_EMERG"free memory space struct file_operations \n");

	kfree(kbuf);
	printk(KERN_EMERG"free memory space kbuf \n");
	
	return 0;
}

/* platform 设备资源结构体
static struct resource platformresource[] = {
	[0] = {
		.start = LEDBASE,
		.end = LEDBASE + LEDLEN - 1,
		.flags = IORESOURCE_MEM,
	}
};
*/

struct platform_device platformdevice = {
	.name = "chardevice",
	/*
	 * .id = -1,
	 * .num_resources = ARRAY_SIZE(platformresource),	// 计算platform资源结构体数组长度
	 * .resource = platformresource,
	 */
};

static struct platform_driver platformdriver = {
	.probe = chardev_probe,
	.remove = chardev_remove,
	.driver = {
	   .owner = THIS_MODULE,
	   .name = "chardevice",
	},
};

int charDrvInit(void)
{
	platform_device_register(&platformdevice);
	return platform_driver_register(&platformdriver);
}

void __exit charDrvExit(void)
{
	platform_device_unregister(&platformdevice);
	platform_driver_unregister(&platformdriver);

	return ;
}

module_init(charDrvInit);
module_exit(charDrvExit);
MODULE_LICENSE("GPL");
