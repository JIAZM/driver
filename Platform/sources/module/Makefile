ifneq ($(KERNELRELEASE),)
    obj-m := platform_chardev.o
else
    PWD := $(shell pwd)
    KDIR := /lib/modules/$(shell uname -r)/build

all:
	make -C $(KDIR) M=$(PWD)    
	#在内核中编译 M 指定的路径
	# -C $(KDIR) change到内核源码目录中编译 - 驱动依赖内核存在
	# M 制定编译的路径

clean:
	rm -rf *.o *.ko *.mod.c *.symvers *.order *.mod

endif
