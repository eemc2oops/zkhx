#include <linux/notifier.h>

/**
 * struct subsys_private - structure to hold the private to the driver core portions of the bus_type/class structure.
 *
 * @subsys - the struct kset that defines this subsystem
 * @devices_kset - the subsystem's 'devices' directory
 * @interfaces - list of subsystem interfaces associated
 * @mutex - protect the devices, and interfaces lists.
 *
 * @drivers_kset - the list of drivers associated
 * @klist_devices - the klist to iterate over the @devices_kset
 * @klist_drivers - the klist to iterate over the @drivers_kset
 * @bus_notifier - the bus notifier list for anything that cares about things
 *                 on this bus.
 * @bus - pointer back to the struct bus_type that this structure is associated
 *        with.
 *
 * @glue_dirs - "glue" directory to put in-between the parent device to
 *              avoid namespace conflicts
 * @class - pointer back to the struct class that this structure is associated
 *          with.
 *
 * This structure is the one that is the actual kobject allowing struct
 * bus_type/class to be statically allocated safely.  Nothing outside of the
 * driver core should ever touch these fields.
 */
// 该结构体被多个结构体引用
// bus_type.p
// class.p    ( device.h )
struct subsys_private {
	struct kset subsys;
	struct kset *devices_kset;
	struct list_head interfaces;    // class.p :  class_interface.node 挂在这里        class_interface_register 里挂树
	                                // bus_type.p :  bus_register 里初始化
	                                //              subsys_interface.node 挂在这个队列里
	                                //              subsys_interface_register 里挂队列
	                                //              bus_probe_device  里遍历
	                                // class.p :  __class_register 里初始化
	struct mutex mutex;

	struct kset *drivers_kset;   // bus_type.p : bus_add_driver
	struct klist klist_devices;    // bus_type.p : device.device_private.knode_bus 持到这个链表里，通过这个链表，可以遍历到所有属于该总线的设备
	                               // bus_type.p : bus_add_device  往队列里添加设备
	                              //  bus_type.p : bus_for_each_dev
	                              // class.p : class_dev_iter_init 里遍历设备
	                              // class.p : device.knode_class 挂到这个队列里           device_add  完成挂树
	struct klist klist_drivers;    // bus_type.p : bus_add_driver
	struct blocking_notifier_head bus_notifier;
	unsigned int drivers_autoprobe:1;  // bus_type.p : store_drivers_autoprobe
	                                   // bus_type.p : bus_probe_device
	                                   // bus_type.p : bus_register 值赋1
	struct bus_type *bus;    // bus_type.p : bus_register

	struct kset glue_dirs;
	struct class *class;
};
#define to_subsys_private(obj) container_of(obj, struct subsys_private, subsys.kobj)
// device_driver.p
struct driver_private {
	struct kobject kobj;    // bus_add_driver     加到 driver_ktype 里
	struct klist klist_devices; // driver_bound
	struct klist_node knode_bus;   // bus_add_driver
	struct module_kobject *mkobj;
	struct device_driver *driver;
};
#define to_driver(obj) container_of(obj, struct driver_private, kobj)

/**
 * struct device_private - structure to hold the private to the driver core portions of the device structure.
 *
 * @klist_children - klist containing all children of this device
 * @knode_parent - node in sibling list
 * @knode_driver - node in driver list
 * @knode_bus - node in bus list
 * @deferred_probe - entry in deferred_probe_list which is used to retry the
 *	binding of drivers which were unable to get all the resources needed by
 *	the device; typically because it depends on another driver getting
 *	probed first.
 * @device - pointer back to the struct device that this structure is
 * associated with.
 *
 * Nothing outside of the driver core should ever touch these fields.
 */
// device.p
// device_private_init
struct device_private {
	struct klist klist_children;       // 子设备的knode_parent挂到父设备的klist_children列表里     
	                                    // device_add
	struct klist_node knode_parent;   // 子设备的knode_parent挂到父设备的klist_children列表里
	                                //  device_add
	struct klist_node knode_driver; // driver_bound
	struct klist_node knode_bus;    // 挂到设备所属的总线列表里               bus_type.subsys_private.klist_devices
	                                // bus_add_device
	struct list_head deferred_probe;
	struct device *device;    // device_private_init
};
#define to_device_private_parent(obj)	\
	container_of(obj, struct device_private, knode_parent)
#define to_device_private_driver(obj)	\
	container_of(obj, struct device_private, knode_driver)
#define to_device_private_bus(obj)	\
	container_of(obj, struct device_private, knode_bus)

extern int device_private_init(struct device *dev);

/* initialisation functions */
extern int devices_init(void);
extern int buses_init(void);
extern int classes_init(void);
extern int firmware_init(void);
#ifdef CONFIG_SYS_HYPERVISOR
extern int hypervisor_init(void);
#else
static inline int hypervisor_init(void) { return 0; }
#endif
extern int platform_bus_init(void);
extern void cpu_dev_init(void);
extern void container_dev_init(void);

struct kobject *virtual_device_parent(struct device *dev);

extern int bus_add_device(struct device *dev);
extern void bus_probe_device(struct device *dev);
extern void bus_remove_device(struct device *dev);

extern int bus_add_driver(struct device_driver *drv);
extern void bus_remove_driver(struct device_driver *drv);

extern void driver_detach(struct device_driver *drv);
extern int driver_probe_device(struct device_driver *drv, struct device *dev);
extern void driver_deferred_probe_del(struct device *dev);
// __driver_attach -> driver_match_device
static inline int driver_match_device(struct device_driver *drv,
				      struct device *dev)
{
	return drv->bus->match ? drv->bus->match(dev, drv) : 1;     //   platform_bus_type : platform_match
}
extern bool driver_allows_async_probing(struct device_driver *drv);

extern int driver_add_groups(struct device_driver *drv,
			     const struct attribute_group **groups);
extern void driver_remove_groups(struct device_driver *drv,
				 const struct attribute_group **groups);

extern int device_add_groups(struct device *dev,
			     const struct attribute_group **groups);
extern void device_remove_groups(struct device *dev,
				 const struct attribute_group **groups);

extern char *make_class_name(const char *name, struct kobject *kobj);

extern int devres_release_all(struct device *dev);
extern void device_block_probing(void);
extern void device_unblock_probing(void);

/* /sys/devices directory */
extern struct kset *devices_kset;
extern void devices_kset_move_last(struct device *dev);

#if defined(CONFIG_MODULES) && defined(CONFIG_SYSFS)
extern void module_add_driver(struct module *mod, struct device_driver *drv);
extern void module_remove_driver(struct device_driver *drv);
#else
static inline void module_add_driver(struct module *mod,
				     struct device_driver *drv) { }
static inline void module_remove_driver(struct device_driver *drv) { }
#endif

#ifdef CONFIG_DEVTMPFS
extern int devtmpfs_init(void);
#else
static inline int devtmpfs_init(void) { return 0; }
#endif
