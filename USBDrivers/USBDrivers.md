# USB 设备驱动  
`Universal Serial Bus (通用串行总线)`  

---
1. USB概念概述
    > ARM 平台下的USB总线接口  

    - USB接口定义  
        `Universal Serial Bus (通用串行总线)`  

    - USB硬件连接
        ![USB主控制器与USB设备连接](./USB主控制器与设备连接.png)  
        ***<u>Linux内核中的USB设备驱动包括USB主控制器驱动与USB设备驱动两部分</u >***  

    - USB主控制器类型
        > USB主控制器内嵌一个叫根集线器的硬件  

        1. UHCI `Intel提出，基于Intel的PC机一般使用这种主控制器，支持usb1.1标准`
        2. OHCI `康柏和微软提出，兼容OHCI的控制器，硬件智能程度比UHCI高，支持usb1.1标准`
        3. EHCI `支持USB2.0标准，同时兼容UHCI和OHCI`
        4. XHCI `支持最新的USB3.0标准，同时兼容UHCI、OHCI与EHCI`
        
    - USB标准
        1. USB1.0   低速(Low Speed)     1.5Mbps
        2. USB1.0   全速(Full Speed)    12Mbps
        3. USB2.0   高速(High Speed)    480Mbps
        4. USB3.0   超高速(Super Speed) 5Gbps
        
    - USB总线结构
        > USB总线采用master/slave式的总线拓扑结构，所有的事务都是由主控制器发起的  

        ![简单的USB总线拓扑结构](./USB总线结构.png)
        为每个设备编一个地址，主控制器即可通过地址访问设备  
    - USB OTG技术简介
        > 既能作为USB设备被计算机读取数据，又能作为主设备读取插入的USb设备  
        > ***OTG(On-The-Go), 即可以做主也可以做从，主要是为嵌入式设备准备的，由于USB是一种主从系统，不能支持点对点的平等的传输数据，OTG正是在这种需求下产生的，OTG不仅支持控制器的主从切换，在一定程度上也支持相同设备之间的数据交换***
    - USB 热插拔的硬件实现  
        <font face=simsun color=red>
        　　USB 主机是如何检测到设备的插入的？  
        　　首先，在USB集线器的每一个下游端口的D+和D-上，分别接了一个15KΩ的下拉电阻到地。这样，在集线器的端口悬空时，就被这两个下拉电阻拉到了低电平。而在USB设备端，在D+或者D-上接了1.5KΩ上拉电阻，对于全速和高速设备，上拉电阻是接在D+上，而低速设备则是上拉电阻接在D-上。这样当设备而插入到集线器时，由1.5K上拉电阻和15K下拉电阻分压，结果就将差分数据线中的一条拉高。集线器检测到这个状态后报告给USB主控制器(或者通过踏上一层的集线器报告给USb主控制器)，这样就实现了检测设备的插入。  
        　　USB高速设备先是被识别为全速设备，然后通过Host和Device两者通信，确认后再切换到高速模式的。
        </font>

2. USB主控制器驱动
    - 主控制器驱动功能
        1. 解析和维护URB
            > URB: USB Repuest Block, 数据通信的格式

        2. 负责不同USB传输类型的调度工作
        3. 负责USB数据的实际传输工作
            > USB设备为主从式

        4. 实现虚拟根HUB功能

    - 硬件存在 - 集成在SOC中
        > 硬件厂商会提供USB主控制器驱动

    - 驱动架构
        ![USB驱动架构](./USB驱动架构.png)
        > ***<u>USB主控制器驱动文件位于　drivers/usb/host</u>***  

        驱动文件命名规则：
        *HCI-*.c  遵循*HCI标准的主控制器
        > 例：ehci-exynos.c　三星Exynos系列处理器USB Host EHCI 控制器驱动　　
        ***<u>/drivers/usb/host/ehci-exynos.c</u>***
        ```C
        // 可以看到是一个平台驱动的架构
        static struct platform_driver ohci_hcd_s3c2410_driver = {
            .probe      = ohci_hcd_s3c2410_probe,
            .remove     = ohci_hcd_s3c2410_remove,
            .shutdown   = usb_hcd_platform_shutdown,    
            .driver     = {
                .name   = "s3c2410-ohci",
                .pm = &ohci_hcd_s3c2410_pm_ops,
                .of_match_table = ohci_hcd_s3c2410_dt_ids,
            },
        };
        ```
        ## ***<u> 平台设备的注册一般都是在/arch/arm目录下进行　！！！ </u>***
        > s3c2410-ohci 平台设备注册可以在 /arch/arm/plat-samsung/devs.c
        ```C
        struct platform_device s3c_device_ohci = {
            .name           = "s3c2410-ohci",
            .id             = -1,
            .num_resources  = ARRAY_SIZE(s3c_usb_resource),
            .resource       = s3c_usb_resource,
            /*
            static struct resource s3c_usb_resource[] = {
                [0] = DEFINE_RES_MEM(S3C_PA_USBHOST, SZ_256),
                [1] = DEFINE_RES_IRQ(IRQ_USBH),
            };
            */
            .dev            = {
                .dma_mask           = &samsung_device_dma_mask,
                .coherent_dma_mask  = DMA_BIT_MASK(32),
            }
        };
        ```



3. USB设备驱动