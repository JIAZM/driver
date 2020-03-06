#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/mm.h>
#include <linux/kernel.h>

#define MAX_MMAP_BUFFER	(8192)
#define OK	(0)
#define ERROR	(-1)

struct cdev *gdev;
struct file_operations *gfile;

dev_t devnum;
int reg_major = 232;
int reg_minor = 0;
unsigned int subdevnum = 1;

char *data = NULL;

int testOpen(struct inode *p, struct file *f)
{
	int i;
	struct page *mypage;

	printk(KERN_EMERG"test open ! \n");

	data = kzalloc(MAX_MMAP_BUFFER, GFP_KERNEL);
	for(i = 0;i < MAX_MMAP_BUFFER;i++)
		data[i] = i % 255;

	for(i = 0;i < (MAX_MMAP_BUFFER/4096);i++){
		mypage = virt_to_page((void *)(data + (i * PAGE_SIZE)));
		/* 1.get Physical address from virtual address
		 * 2.get memory block's number from physical address
		 * 3.get (struct page) from memory block's number
		 */

		SetPageReserved(mypage);
		// make sure page cannod be swap
	}

	return 0;
}

int testRelease(struct inode *p, struct file *f)
{
	int i;
	struct page *mypage;

	printk(KERN_EMERG"test close ! \n");

	for(i = 0;i < (MAX_MMAP_BUFFER/4096);i++){
		mypage = virt_to_page((void *)(data + (i * PAGE_SIZE)));
		ClearPageReserved(mypage);
	}

	kfree(data);

	return 0;
}

static int testMmap(struct file *f, struct vm_area_struct *vma)
{
	unsigned long phys, len;
	unsigned long pfn;

	phys = virt_to_phys((void *)data);
	len = vma->vm_end - vma->vm_start;
	printk(KERN_INFO"## vm_start is %lx \n", vma->vm_start);
	printk(KERN_INFO"## vm_end is %lx \n", vma->vm_end);

	pfn = phys >> PAGE_SHIFT;

	if(remap_pfn_range(vma, vma->vm_start, pfn, len, vma->vm_page_prot)){
		printk("remap error \n");
		return -EAGAIN;
	}

	return 0;
}

int chardevInit(void)
{
	devnum = MKDEV(reg_major, reg_minor);

	printk(KERN_EMERG"char device number is %d \n", devnum);
	if(OK == register_chrdev_region(devnum, subdevnum, "testchar"))
		printk(KERN_EMERG"register char device region ok \n");
	else{
		printk(KERN_EMERG"register char device region error \n");
		return ERROR;
	}
	printk(KERN_EMERG"register devnum is %d \n", devnum);

	gdev = kzalloc(sizeof(struct cdev), GFP_KERNEL);
	gfile = kzalloc(sizeof(struct file_operations), GFP_KERNEL);

	gfile->open = testOpen;
	gfile->mmap = testMmap;
	gfile->release = testRelease;
	gfile->owner = THIS_MODULE;

	cdev_init(gdev, gfile);
	cdev_add(gdev, devnum, subdevnum);

	return 0;
}

void __exit chardevExit(void)
{
	cdev_del(gdev);

	unregister_chrdev_region(devnum, subdevnum);

	kfree(gdev);
	printk(KERN_EMERG"free memory space struct cdev \n");
	kfree(gfile);
	printk(KERN_EMERG"free memory space struct file_operations \n");

	return;
}

module_init(chardevInit);
module_exit(chardevExit);

MODULE_LICENSE("GPL");


