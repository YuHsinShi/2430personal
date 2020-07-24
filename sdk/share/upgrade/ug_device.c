#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "ite/ug.h"
#include "ug_cfg.h"

#pragma pack(1)
typedef struct
{
    uint32_t device_type;
    uint32_t unformatted_size;
    uint32_t partition_count;
} device_t;

typedef enum
{
    DEVICE_NAND = 0,
    DEVICE_NOR  = 1,
    DEVICE_SD0  = 2,
    DEVICE_SD1  = 3,

    DEVICE_COUNT
} DeviceType;

static const char* device_table[] =
{
    "NAND",
    "NOR",
    "SD0",
    "SD1"
};

int ugUpgradeDevice(ITCStream *f, UGDevice* device)
{
    int i, ret = 0;
    device_t dev;
    uint32_t readsize;

    // read device header
    readsize = itcStreamRead(f, &dev, sizeof(device_t));
    if (readsize != sizeof(device_t))
    {
        ret = __LINE__;
        LOG_ERR "Cannot read file: %ld != %ld\n", readsize, sizeof(device_t) LOG_END
        goto end;
    }

    device->device_type         = itpLetoh32(dev.device_type);
    device->unformatted_size    = itpLetoh32(dev.unformatted_size);
    device->partition_count     = itpLetoh32(dev.partition_count);

    LOG_INFO "Upgrade %s. Non-partition size is %ld bytes, and having %ld partition(s)\n",
        device_table[device->device_type],
        device->unformatted_size,
        device->partition_count
    LOG_END

    readsize = itcStreamRead(f, &device->size, sizeof(uint64_t) * device->partition_count);
    if (readsize != sizeof(uint64_t) * device->partition_count)
    {
        ret = __LINE__;
        LOG_ERR "Cannot read file: %ld != %ld\n", readsize, sizeof(uint64_t) * device->partition_count LOG_END
        goto end;
    }

    for (i = 0; i < (int)device->partition_count; i++)
    {
        device->size[i] = itpLetoh64(device->size[i]);

        if (device->size[i] > 0)
            LOG_INFO "Partition[%d] size is %lld\n", i, device->size[i] LOG_END
        else
            LOG_INFO "Partition[%d] size is maximum availiable\n", i LOG_END
    }

    if (device->device_type == DEVICE_NAND)
    {
        device->disk = ITP_DISK_NAND;
    }
    else if (device->device_type == DEVICE_NOR)
    {
        device->disk = ITP_DISK_NOR;
    }
    else if (device->device_type == DEVICE_SD0)
    {
        device->disk = ITP_DISK_SD0;
    }
    else if (device->device_type == DEVICE_SD1)
    {
        device->disk = ITP_DISK_SD1;
    }
    else
    {
        ret = __LINE__;
        LOG_ERR "Unknown device type: %d\n", device->device_type LOG_END
        goto end;
    }

end:
#ifdef _WIN32
    ret = 0;
#endif
    return ret;
}
