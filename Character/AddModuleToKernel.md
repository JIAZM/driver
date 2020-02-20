# 将驱动代码添加到内核中

## make menuconfig 的本质

> 遍历， 解析并且显示所有目录下的 kconfig 文件  
> make menuconfig 中的内容来源于各个 Kconfig 文件中的item实例
> 配置 make menuconfig 就是配置 Kconfig 文件各个 item 的值  
> .config 文件 - 实时汇总 make menuconfig 中做的所有配置

## make实质

> 遍历所有目录下的 Makefile 文件， 并按照 Makefile 的内容编译相关代码  
> 参考 .config 文件的每一个配置选项， 确定编译生成的文件为 obj-y 或 obj-m  
> obj-y ： 编译到内核文件中 (zImage)  
> obj-m ： 编译为驱动模块

- 添加驱动代码到内核的方式
    1. 在 linux-kernel/drivers/char/Kconfig 中为驱动 (charDev) 添加一个 config条目
    2. 在 linux-kernel/drivers/char/Makefile 中添加
        ```Makefile
        obj-$(CONFIG_TEST_CHAR) += charDev.o
        ```
    3. 把 charDev 拷贝到linux-kernel/drivers/char/ 目录下
    4. make menuconfig 选中CONFIG_TEST_CHAR 条目，并设定为 y 或者 m
    5. 重新编译

## ***<u>copy_from_user() & copy_to_user() 函数</u>***

> 内核为驱动程序提供的在内核空间与用户空间传递数据的方法  
> 定义在 arch/arm/include/asm/uaccess.h 中

- copy_from_user()
    ```C
        unsigned long copy_from_user(void *to, const void *from, unsigned long n);
        /*
         * to :目标地址 - 内核空间
         * from :源地址 - 用户空间
         * n :要拷贝的字节数
         * 
         * return :成功返回 0
         *         失败返回 没有拷贝成功的数据字节数
         */
    ```

- copy_to_user()
    ```C
        unsigned long copy_to_user(void *to, const void *from, unsigned long n);
        /*
         * to :目标地址 - 用户空间
         * from :源地址 - 内核空间
         * n :要拷贝的字节数
         * 
         * return :成功返回 0
         *         失败返回 没有拷贝成功的数据字节数
         */
    ```