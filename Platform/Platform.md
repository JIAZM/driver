# **Platform驱动架构**

## ***Platform框架初探***
> 一种虚拟总线，没有对应的硬件结构。  
> 主要作用：  
> 1.统一管理系统的外设资源(IO内存、中断信号线资源)  
> 2.设备和驱动分离 - 提高驱动的可移植性  
> 管理的资源不包括 PCI设备、USB设备，内核存在 PCI BUS 和 USB BUS  

- Platform总线驱动架构：<u>***设备驱动分离***</u>
    1. ## ***设备***： struct platform_device    <font color=yellow size=3>描述外设设备资源</font>
    ```C
    // 在 include/linux/platform_device.h 中定义
    struct platform_device {
        const char      *name;  // 设备名
        int             id;     // 设备ID，表示设备是第几个
        bool            id_auto;
        struct device   dev;    // 内核中用来描述设备的基类
        u32             num_resources;  // 资源数 resources数
        struct resource *resource;  // 描述资源使用情况

    /* struct resource {
     *     resource_size_t start;
     *     resource_size_t end;
     *     const char *name;
     *     unsigned long flags; // 表示资源类型 (IO资源 、中断资源)
     *     unsigned long desc;
     *     struct resource *parent, *sibling, *child;
     *     // 系统用来管理resource的方式
     * };
     */

        const struct platform_device_id *id_entry;
        // 匹配时选择的依据
        char *driver_override; /* Driver name to force a match */

        /* MFD cell pointer */
        struct mfd_cell *mfd_cell;  // 多功能设备的实现

        /* arch specific additions */
        struct pdev_archdata    archdata;
    };

    ```
    2. ## ***驱动***： struct platform_driver    <font color=yellow size=3>实现驱动设备的代码</font>
    ```C
    // 在 include/linux/platform_device.h 中定义
    // 通过实现 platform_driver 结构体中定义的函数完成相应的操作
    struct platform_driver {
        int (*probe)(struct platform_device *);
        // 实现设备初始化 - 原本在 chrdevinit 中实现的内容

        int (*remove)(struct platform_device *);
        // 实现设备卸载 - 原本在 chrdevexit 中实现的内容

        // 设备的关闭、暂停与恢复 - 电源管理的动作
        void (*shutdown)(struct platform_device *);
        int (*suspend)(struct platform_device *, pm_message_t state);
        int (*resume)(struct platform_device *);

        // 驱动的基类
        struct device_driver driver;
        // 设备与驱动匹配操作时使用
        const struct platform_device_id *id_table;
        bool prevent_deferred_probe;
    };
    ```
    4. ## <u>***总线***</u>： platform_bus  <font color=yellow size=3>系统启动时注册 platform 总线</font>
        > ### ***管理设备(struct platform_device)与驱动(struct platform_driver)***  
        > ### ***根据某种规则匹配设备与驱动***

    <u>系统启动是会调用 `start_kernel()` 函数，在函数中执行一系列初始化动作，在函数体最后调用`rest_init()`</u>  
    在rest_init()中
    ```C
    // 摘自 kerlel 4.15
    pid = kernel_thread(kernel_init, NULL, CLONE_FS);
    /*
     * 创建内核线程调用 kernel_init()
    */

    static int __ref kernel_init(void *unused){
        ...
        kernel_init_freeable(); // 调用 kernel_init_freeable()
        ...
    }

    static noinline void __init kernel_init_freeable(void){
        ...
        do_basic_setup();   // 调用 do_basic_setup()
        ...
    }

    static void __init do_basic_setup(void){
        ...
        driver_init();
        // 在 drivers/base/init.c 中定义
        ...
    }

    void __init driver_init(void){
        ...
        platform_bus_init();    // 初始化 platform 总线
    // 在 /drivers/base/platform.c 中定义
    // platform_bus_init 函数成功返回后就可以利用 platform虚拟总线管理设备，可以使用 platform驱动架构
        ...
    }

    int __init platform_bus_init(void){
        int error;

        early_platform_cleanup();

        // 注册一个设备(platform_bus)，将总线看作一个设备
        error = device_register(&platform_bus);
    /* struct device platform_bus = {
     *     .init_name = "platform",
     * };
     * struct device 是描述设备的最基本单元
    */

        if (error)
            return error;
        error =  bus_register(&platform_bus_type);
        // 注册一个总线
    /* struct bus_type platform_bus_type = {
    *        .name           = "platform",
    *        .dev_groups     = platform_dev_groups,
    *        .match          = platform_match,
    *        // match 函数完成设备与驱动的匹配
    *        .uevent         = platform_uevent,
    *        // uevent 函数实现热插拔机制
    *        .pm             = &platform_dev_pm_ops,
    *        .force_dma      = true,
    *};
    */
        if (error)
            device_unregister(&platform_bus);
        of_platform_register_reconfig_notifier();
        return error;
    }

    ```

- Platform设备和驱动的注册  
    <font color=lightslateblue size=4>platform 设备注册函数</font>
    ```C
    int platform_device_register(struct platform_device *pdev)
    // 将已初始化的 platform_device 结构体注册到内核中
    {
    // 首先需要初始化一个 platform_device 结构体，作为传入参数
        device_initialize(&pdev->dev);
        arch_setup_pdev_archdata(pdev);
        return platform_device_add(pdev);
    }

    int platform_device_add(struct platform_device *pdev){
        // 在 drivers/base/platform.c 中定义
        ...
        /* 经过以下两步将platform设备挂载到platform总线上 */
        if (!pdev->dev.parent)  pdev->dev.parent = &platform_bus;
        // 以 platform_bus为父设备
        
        pdev->dev.bus = &platform_bus_type;
        // 设备总线指向 platform虚拟总线
        ...
        ret = device_add(&pdev->dev);
        // 把设备注册到内核中
    }
    ```
    <font color=lightslateblue size=4>platform 驱动注册函数</font>
    ```C
    int __platform_driver_register(struct platform_driver *drv,
    struct module *owner)
    {
        // 设置驱动基类
        drv->driver.owner = owner;
        drv->driver.bus = &platform_bus_type;
        drv->driver.probe = platform_drv_probe;
        drv->driver.remove = platform_drv_remove;
        drv->driver.shutdown = platform_drv_shutdown;

        return driver_register(&drv->driver);
        // 将驱动注册槽内核中
    }

    ```

- Platform设备和驱动的匹配