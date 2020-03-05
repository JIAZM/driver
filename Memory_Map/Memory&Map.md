# **驱动的mmap机制**

- ## ***Linux 内存映射机制***
    - ### ***早期***
        > MMU 未开启或者没有 MMU  
        > CPU 通过物理地址访问外设 (包括物理内存)
    - ### ***启动 MMU 后***
        > CPU 核心对 MMU 发出***虚拟地址***  
        > MMU 将虚拟地址转换为物理地址，最后使用***物理地址***读取实际设备
    > 使用 MMU 为进程提供独立的内存虚拟地址空间  

- ## <u>***MMU***</u>
    > 内存管理单元 Memory Manage Unit
    - <u>负责虚拟地址到物理地址的***转换***</u>
    - <u>提供硬件机制的内存访问***权限***检查</u>

- ## <u>***虚拟地址转换为物理地址的方法***</u>
    - 确定的<u>***数学公式***</u>进行转换
    - 用<u>***表格***</u>存储虚拟地址对应的物理地址
        > Tiny210 使用第二种方法，根据每次转换时查表的次数分为**一级页表**方式
        > (以段的方式转换)和**二级页表**方式两种(以页的方式转换)
        - ### **Linux二级页表映射机制**
            ```
                    线性地址
                    31_____________22 21___________12 11_____________0
                    |___DIRECTORY____|_____TABLE_____|__OFFSET_(4k)__|
                            |                |              |
                            |                |              |    ____页___
                            |                |              ↓   |_________|
                            |                |              +-->|_________|
                            |                |    ________  ↑   |_________|
                            |                ↓   |________| |   |_________|
                            |                +-->|__&page_|---->|___head__|
                            |    _________   ↑   |________|
                            ↓   |_________|  |   |________|
                            +-->|__&table_|----->|__head__|
                            ↑   |_________|
                ___cr3___   |   |_________|
               |_________|----->|___head__|
            ```
            > 将物理内存划分为 4k 的块 (就是物理页面) (一般是 4k)  
            > 使用内核结构体struct page描述每个4k的块 (都是物理块)
            > 在 struct page 中记录每一个块的起始物理地址与结束地址  
            > 页目录的地址在 CR3 寄存器中保存

            #### <u>***用户空间各个进程的页目录表和页表不同，所以不同进程可以将同一个虚拟地址映射到不同的物理地址， 通过这种机制可以做到所有进程共享0~3G虚拟地址空间， 同时每个进程都可以保存私有数据***</u>

- ## <u>***进程的虚拟地址管理机制***</u>
    <u>*每个进程的虚拟地址空间都是 0~3G，由于页目录与页表不同，虚拟地址最终映射到的物理地址页不同*</u>

    结构体 struct task_struct 在 include/linux/sched.h 文件中定义，描述进程信息（表示进程）  
    task_struct 结构体中定义了***进程用来管理虚拟地址空间的结构体，描述一个进程的虚拟地址空间*** struct mm_struct *mm, *active_mm;  
    ```C
    /* struct mm_struct 结构体在 include/linux/mm_types.h 中定义 */
    struct mm_struct {
        struct vm_area_struct *mmap;            /* 指向虚拟区域（VMA）链表 */
        struct rb_root mm_rb;                   /* 指向 red_black 树 */
        // 红黑树 管理 VMA 链表
        pgd_t * pgd;                            /* 指向进程的页目录表 */
        atomic_t mm_users;                      /* 用户空间中有多少用户 */
        atomic_t mm_count;                      /* 对 struct mm_struct 有多少引用 */
        int map_count;                          /* number of VMAs */
                                                /* 虚拟区间的个数 */
        struct list_head mmlist;                /* 所有活动 (active) mm的链表 */
        /* List of maybe swapped mm's.  These are globally strung
         * together off init_mm.mmlist, and are protected
         * by mmlist_lock
         */
        unsigned long start_code, end_code, start_data, end_data;
        unsigned long start_brk, brk, start_stack;
        unsigned long arg_start, arg_end, env_start, env_end;
    };
    struct vm_area_struct;
    /* 用来描述一个虚拟内存区域（VMA）
     * 内核将每一个内存区域作为一个单独的内存对象管理，内一个内存区域都有一致的属性，如权限
     * 所以程序的 代码段、数据段、BSS段 在内核里都分别由一个 struct vm_area_struct 结构体描述
     */
    struct vm_area_struct {
        struct mm_struct *vm_mm;        /* 虚拟内存区域所在的地址空间. */
        /* mm_struct 通过vm_area_struct 管理内存
         * 所以对于具体的 vm_area_struct 需要指出隶属与哪一个 mm_struct
         */

        unsigned long vm_start;         /* Our start address within vm_mm. */
        unsigned long vm_end;
        /* The first byte after our end address within vm_mm. */

        struct vm_area_struct *vm_next, *vm_prev;
        /* 每一个 mm_struct 中维护了一个 vm_area_struct 链表 */

        pgprot_t vm_page_prot;          /* Access permissions of this VMA. */
        unsigned long vm_flags;         /* Flags, see mm.h. */
        /* 虚拟内存区域的标志
         *
         * VM_READ      此虚拟内存区域可读
         * VM_WRITE     此虚拟内存区域可写
         * VM_EXEC      此虚拟内存区域可执行
         * VM_SHARED    此虚拟内存区域可共享
         * VM_IO        此虚拟内存区域映射设备IO空间
         */

         struct rb_node vm_rb;
        
        const struct vm_operations_struct *vm_ops;
        /* Function pointers to deal with this struct. */

        struct file * vm_file;          /* File we map to (can be NULL). */
        /* 可以将文件映射到 vm_start 处 */
    };
    ```
    <u>***使用命令 cat /proc/\<pid\>maps 查看进程的虚拟内存区域***</u>
    7ff3ecccd000