<font face=Simsun size=4>

## 设备类概念
class 是设备类，它是一个抽象的概念，没有对应的实体。提供给用户接口相似的一类设备
的集合  
- 使用命令查看当前系统中使用的设备类
    ```shell
    $ ls /sys/class #/sys/class路径下的目录表示系统当前存在的设备类
    ```

## 生成字符类设备节点
- 函数class_create创建class类文件
    ```C
    struct class    // 在 include/linux/device.h 中定义
    extern struct class * __must_check __class_create(struct module *owner, const char *name, struct lock_class_key *key);
    extern void class_destory(struct class *cls);

    #define class_create(owner, name)   \
    ({                                  \
        static struct lock_class_key __key; \
        __class_create(owner, name, &__key);    \
    })
    class_create(THIS_MODULE, "class name")
    ```
- 创建设备节点函数 device_create
    ```C
    struct device *device_create(struct class *cls, struct device *parent, dev_t devt, void *drvdata, const char *fmt, ...);
    /* 在 include/linux/device.h 中声明
     * cls 设备所属的类
     * parent 设备的父设备， NULL
     * devt 设备号
     * drvdata 设备数据， NULL
     * fmt 设备名称 */
    
    extern void device_destroy(struct class *cls, dev_t devt);
    // 销毁设备节点
    ```


</font>