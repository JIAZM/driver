#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/slab.h>

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
char *buffer;
int flag = 0;

/* 
 * open file interface
 */
int testOpen(struct inode *p, struct file *f)
{
	printk(KERN_EMERG"test open function \n");

	return 0;
}

/* 
 * open write interface
 * 从 u 指向的用户空间 buffer 中拿到数据
 * 然后操作寄存器，写到硬件中去
 */
ssize_t testWrite(struct file *f, const char __user *u, size_t s, loff_t *l)
{
	printk(KERN_EMERG"test write function \n");

	return 0;
}

/*
 * 从硬件中读取数据
 * 写到 u 指向用户下空间 buffer 中去
 */
ssize_t testRead(struct file *f, char __user *u, size_t s, loff_t *l)
{
	printk(KERN_EMERG"test read function \n");

	return 0;
}

int charDevInit(void)
{
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
	gFile->owner = THIS_MODULE;

	cdev_init(gDev, gFile);
	cdev_add(gDev, devNo, 3);

	return 0;
}

void __exit charDevExit(void)
{
	cdev_del(gDev);
	unregister_chrdev_region(devNo, subDevNo);
	printk(KERN_EMERG"unregister device module %s \n", NAME);

	kfree(gDev);
	printk(KERN_EMERG"free memory space struct cdev \n");

	kfree(gFile);
	printk(KERN_EMERG"free memory space struct file_operations \n");

	return ;
}

module_init(charDevInit);
module_exit(charDevExit);
MODULE_LICENSE("GPL");
