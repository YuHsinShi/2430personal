#ifndef _DEVICE_H_
#define _DEVICE_H_

#include <linux/os.h>

#ifdef __cplusplus
extern "C"
{
#endif

struct device;
struct device_driver;
struct module;

struct bus_type {
	const char		*name;
	int (*match)(struct device *dev, struct device_driver *drv);
	int (*probe)(struct device *dev);
	int (*remove)(struct device *dev);
};

struct device_driver {
	const char		*name;
	struct module		*owner;
};

struct device_type {
	const char *name;
};

struct device {
	struct device		*parent;
	const char *name;
	struct bus_type	*bus;
	struct device_driver *driver;
	void *driver_data;
	void (*release)(struct device *dev);
	unsigned registerd : 1;
};

static inline int dev_set_name(struct device *dev, const char *name)
{
	dev->name = name;
	return 0;
}

static inline const char *dev_name(const struct device *dev)
{
	return dev->name;
}

static inline void *dev_get_drvdata(const struct device *dev)
{
	return dev->driver_data;
}

static inline void dev_set_drvdata(struct device *dev, void *data)
{
	dev->driver_data = data;
}

static inline void device_initialize(struct device *dev)
{
}

static inline int device_add(struct device *dev)
{
    if (dev->bus && dev->bus->probe)
        dev->bus->probe(dev);

	dev->registerd = 1;

	return 0;
}

static inline int device_register(struct device *dev)
{
	device_initialize(dev);
	return device_add(dev);
}

static inline void device_unregister(struct device *dev)
{
	if (dev->release)
		dev->release(dev);

	dev->registerd = 0;
}


static inline int device_is_registered(struct device *dev)
{
    return dev->registerd;
}

static inline int dev_to_node(struct device *dev)
{
	return -1;
}
static inline void set_dev_node(struct device *dev, int node)
{
}

static inline void device_lock(struct device *dev)
{
}

static inline int device_trylock(struct device *dev)
{
}

static inline void device_unlock(struct device *dev)
{
}

static inline void device_del(struct device *dev)
{
    if (dev->bus && dev->bus->remove)
        dev->bus->remove(dev);

    dev_set_drvdata(dev, NULL);

    if (dev->release)
        dev->release(dev);

    dev->registerd = 0;
}

#define put_device(x)   do {} while(0)
#ifndef get_device
#define get_device(x)	(x)
#endif
#define device_enable_async_suspend(x)  do {} while(0)




#ifdef __cplusplus
}
#endif

#endif // _DEVICE_H_
