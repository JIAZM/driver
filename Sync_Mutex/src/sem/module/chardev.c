#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/slab.h>

#define BUFFER_MAX	10
#define OK			0
#define ERROR		-1

struct cdev *gDev;
struct file_operations *gFile;
dev_t devNum;
unsigned int subDevNum = 1;
int reg_major = 232;
int reg_minor = 0;
struct semaphore sema;
int xxx_count = 0;

int testOpen(struct inode *p, struct file *f)
{
	down(&sema);
	if(xxx_count){
		up(&sema);
		return -EBUSY;
	}
	
	++xxx_count;

	up(&sema);
	printk(KERN_EMERG"Device open OK ! \n");

	return 0;
}

int testRelease(struct inode *inode, struct file *filep)
{
	down(&sema);
	--xxx_count;
	up(&sema);

	printk(KERN_EMERG"Device close OK !\n");

	return 0;
}

ssize_t testWrite(struct file *f, const char __user *u, size_t s, loff_t *l)
{
	printk(KERN_EMERG"test Write() !\n");

	return 0;
}

ssize_t testRead(struct file *f, char __user *u, size_t s, loff_t *l)
{
	printk(KERN_EMERG"test read() !\n");

	return 0;
}

int charDrvInit(void)
{
	devNum = MKDEV(reg_major, reg_minor);

	printk(KERN_EMERG"devNum is %d \n", devNum);
	if(OK == register_chrdev_region(devNum, subDevNum, "restchar"))
		printk(KERN_EMERG"register character device region OK \n");
	else{
		printk(KERN_EMERG"register character device region error \n");
		return 0;
	}

	printk(KERN_EMERG"Device Number is %d \n", devNum);

	gDev = (struct cdev *)kzalloc(sizeof(struct cdev), GFP_KERNEL);
	gFile = (struct file_operations *)kzalloc(sizeof(struct file_operations), GFP_KERNEL);

	gFile->open = testOpen;
	gFile->read = testRead;
	gFile->write = testWrite;
	gFile->release = testRelease;
	gFile->owner = THIS_MODULE;

	cdev_init(gDev, gFile);
	cdev_add(gDev, devNum, 3);

	sema_init(&sema, 1);

	return 0;
}

void __exit charDrvExit(void)
{
	cdev_del(gDev);
	unregister_chrdev_region(devNum, subDevNum);

	kfree(gDev);
	printk(KERN_EMERG"free memory space struct cdev \n");

	kfree(gFile);
	printk(KERN_EMERG"free memory space struct file_operations \n");
}
