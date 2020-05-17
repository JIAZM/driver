# USB鼠标驱动

> hid：Human Interface Device
> 代码位于 drivers/hid/usbhid/usbmouse.c  

```C
// 使用`module_usb_driver(usb_mouse_driver);`指定入口函数

#define module_usb_driver(__usb_driver) \
	module_driver(__usb_driver, usb_register, \shunainai
	usb_deregister)

#define usb_register(driver)	usb_register_driver(driver, THIS_MODULE, KBUILD_MODNAME)

/*
	usb_register_driver()函数在 /drivers/usb/core/driver.c中定义
	int usb_register_driver(struct usb_driver *new_driver, struct module *owner, const char *mod_name);

	static struct usb_driver usb_mouse_driver = {
		.name       = "usbmouse",
		.probe      = usb_mouse_probe,
		.disconnect = usb_mouse_disconnect,		// 设备拔出的时候会执行函数
		.id_table   = usb_mouse_id_table,
	};
*/

// struct usb_driver结构说明
struct usb_driver {
    const char *name;
    
    int (*probe) (struct usb_interface *intf, const struct usb_device_id *id);
    
    void (*disconnect) (struct usb_interface *intf);
    
    int (*unlocked_ioctl) (struct usb_interface *intf, unsigned int code, void *buf);
    
    int (*suspend) (struct usb_interface *intf, pm_message_t message);
    int (*resume) (struct usb_interface *intf); 
    int (*reset_resume)(struct usb_interface *intf);
    
    int (*pre_reset)(struct usb_interface *intf); 
    int (*post_reset)(struct usb_interface *intf);
    
    const struct usb_device_id *id_table;
	// 结构在 /include/linux/mod_devicetable.h中定义
	/*
		struct usb_device_id {
			// which fields to match against?
			__u16       match_flags;
			
			// Used for product specific matches; range is inclusive
			__u16       idVendor;		// 产品ID
			__u16       idProduct;		// 设备ID
			__u16       bcdDevice_lo;
			__u16       bcdDevice_hi;
			
			// Used for device class matches
			__u8        bDeviceClass;	// 设备类别
			__u8        bDeviceSubClass;
			__u8        bDeviceProtocol;

			// Used for interface class matches
			__u8        bInterfaceClass;
			__u8        bInterfaceSubClass;
			__u8        bInterfaceProtocol;

			// Used for vendor-specific interface matches
			__u8        bInterfaceNumber;

			// not matched against
			kernel_ulong_t  driver_info
				__attribute__((aligned(sizeof(kernel_ulong_t))));
		};
	*/
    
    struct usb_dynids dynids;
    struct usbdrv_wrap drvwrap;
    unsigned int no_dynamic_id:1;
    unsigned int supports_autosuspend:1;
    unsigned int disable_hub_initiated_lpm:1; 
    unsigned int soft_unbind:1;
};
```