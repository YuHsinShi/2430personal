#include <sys/ioctl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "ite/ug.h"
#include "nor/mmp_nor.h"
#include "ug_cfg.h"
#ifndef _WIN32
#include "ssp/mmp_axispi.h"
#endif

#define MAX_PARTITION_COUNT 4

#pragma pack(1)
typedef struct
{
    uint64_t partition_size;
} partition_t;

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

static const char* disk_table[] =
{
    "SD0",
    "SD1",
    "CF",
    "MS",
    "XD",
    "NAND",
    "NOR",
    "MSC00",
    "MSC01",
    "MSC02",
    "MSC03",
    "MSC04",
    "MSC05",
    "MSC06",
    "MSC07",
    "MSC10",
    "MSC11",
    "MSC12",
    "MSC13",
    "MSC14",
    "MSC15",
    "MSC16",
    "MSC17",
    "RAM",
};

int ugUpgradePartition(ITCStream *f, ITPDisk disk, int partition, char* drive, bool format)
{
    ITPDriveStatus* driveStatusTable;
    int ret, i, j;
    partition_t par;
    uint32_t readsize;

    // read partition header
    readsize = itcStreamRead(f, &par, sizeof(partition_t));
    if (readsize != sizeof(partition_t))
    {
        ret = __LINE__;
        LOG_ERR "Cannot read file: %ld != %ld\n", readsize, sizeof(partition_t) LOG_END
        return ret;
    }

    par.partition_size  = itpLetoh64(par.partition_size);

    LOG_DBG "partition_size=%ld\n", par.partition_size LOG_END

#ifdef _WIN32
    goto end;
#endif

    // try to find the drive
    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_GET_TABLE, &driveStatusTable);

    j = 0;
    for (i = 0; i < ITP_MAX_DRIVE; i++)
    {
        ITPDriveStatus* driveStatus = &driveStatusTable[i];

        if (driveStatus->avail)
        {
            LOG_DBG
                "drive %c: disk=%s name=%s\n", 'A' + i, disk_table[driveStatus->disk], driveStatus->name
            LOG_END
        }

        if (driveStatus->disk == disk && driveStatus->avail)
        {
            if (j++ == partition)
            {
                if (format && par.partition_size > 0)
                {
                    ret = ioctl(ITP_DEVICE_FAT, ITP_IOCTL_FORMAT, (void*) i);
                LOG_DBG
                    "format drive %c: disk=%s name=%s ret=%d\n", 'A' + i, disk_table[driveStatus->disk], driveStatus->name, ret
                LOG_END
                }
                strcpy(drive, driveStatus->name);
                ret = 0;
                goto end;
            }
        }
    }

    LOG_ERR "Cannot find the partition %d in %s disk\n", partition, disk_table[disk] LOG_END
    ret = __LINE__;

end:
#ifdef _WIN32
    ret = 0;
    strcpy(drive, "A:/");
#endif
    return ret;
}

int ugUpgradeDoPartition(UGDevice* device)
{
    int i, ret = 0;
    uint64_t inc;
    ITPPartition par = {0};
    ITPDriveStatus* driveStatusTable;
    uint8_t fat_exbr[ITP_FAT_EXBR_SIZE] = {0};
    FILE *f = NULL;

    if (device->device_type == DEVICE_NAND)
    {
        par.disk = ITP_DISK_NAND;
    }
    else if (device->device_type == DEVICE_NOR)
    {
        par.disk = ITP_DISK_NOR;
    }
    else if (device->device_type == DEVICE_SD0)
    {
        par.disk = ITP_DISK_SD0;
    }
    else if (device->device_type == DEVICE_SD1)
    {
        par.disk = ITP_DISK_SD1;
    }
    else
    {
        ret = __LINE__;
        LOG_ERR "Unknown device type: %d\n", device->device_type LOG_END
        goto end;
    }

    if (device->partition_count == 0)
        goto end;

    if (device->partition)
    {
        LOG_INFO "Force to partition\n" LOG_END
        goto repartition;
    }

#ifdef _WIN32
    goto end;
#endif

    if (!device->nopartition)
    {
        ret = ioctl(ITP_DEVICE_FAT, ITP_IOCTL_GET_PARTITION, &par);
        if (ret)
        {
            LOG_INFO "Origial partition not exist\n" LOG_END
            goto repartition;
        }

        if (par.count != device->partition_count)
        {
            LOG_INFO "Origial partition count %d not equal %d\n", par.count, device->partition_count LOG_END
            goto repartition;
        }

        for (i = 0; i < (int)device->partition_count; i++)
        {
            LOG_INFO "Origial partition %d size is %lld bytes\n", i, par.size[i] LOG_END
            if (device->size[i] && par.size[i] != device->size[i])
                goto repartition;
        }
    }
    LOG_INFO "Mount %s disk(s)...\n", device_table[device->device_type] LOG_END
    ret = ioctl(ITP_DEVICE_FAT, ITP_IOCTL_MOUNT, (void*)par.disk);
    if (ret)
    {
        LOG_ERR "Mount %s disk(s) fail: 0x%X\n", device_table[device->device_type], ret LOG_END
        goto end;
    }
    goto end;

repartition:
    LOG_INFO "Partitioning...\n" LOG_END

    par.count = device->partition_count;
    inc = device->unformatted_size;
    for (i = 0; i < (int)device->partition_count; i++)
    {
        par.start[i] = inc;
        par.size[i] = device->size[i];
        inc += device->size[i];

		LOG_INFO "(%d) inc 0x%x.size=0x%x \n",i,inc,par.size[i]  LOG_END

		
    }

    LOG_INFO "Unmount %s disk(s)...\n", device_table[device->device_type] LOG_END
    ret = ioctl(ITP_DEVICE_FAT, ITP_IOCTL_UNMOUNT, (void*)par.disk);
    if (ret)
    {
        LOG_ERR "Unmount %s disk(s) fail: 0x%X\n", device_table[device->device_type], ret LOG_END
    }

    LOG_INFO "Create partition(s)...\n" LOG_END
    ret = ioctl(ITP_DEVICE_FAT, ITP_IOCTL_CREATE_PARTITION, &par);
    if (ret)
    {
        LOG_ERR "Create partition(s) fail: 0x%X\n", ret LOG_END
        goto end;
    }

    LOG_INFO "Mount %s disk(s)...\n", device_table[device->device_type] LOG_END
    ret = ioctl(ITP_DEVICE_FAT, ITP_IOCTL_FORCE_MOUNT, (void*)par.disk);
    if (ret)
    {
        LOG_ERR "Mount %s disk(s) fail: 0x%X\n", device_table[device->device_type], ret LOG_END
        goto end;
    }

    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_GET_TABLE, &driveStatusTable);

    for (i = 0; i < ITP_MAX_DRIVE; i++)
    {
        ITPDriveStatus* driveStatus = &driveStatusTable[i];

        LOG_DBG "drive[%d] disk=%d avail=%d\n", i, driveStatus->disk, driveStatus->avail LOG_END

        if (driveStatus->disk == par.disk && driveStatus->avail)
        {
            LOG_INFO "Format drive %c: ...\n", 'A' + i LOG_END

            ret = ioctl(ITP_DEVICE_FAT, ITP_IOCTL_FORMAT, (void*) i);
            if (ret)
            {
                LOG_ERR "Format fail: 0x%X\n", ret LOG_END
                goto end;
            }
        }
    }

#if !defined(_WIN32) && defined(CFG_NOR_ENABLE)
    if (par.disk == ITP_DISK_NOR && par.count > 3)
    {
    #if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
        ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
    #endif

        if (NorRead(SPI_0, SPI_CSN_0, (uint32_t)(par.exbr_start), fat_exbr + 8, ITP_FAT_EXBR_SIZE - 8))
        {
            LOG_ERR "NorRead fail.\n" LOG_END
        }
        else
        {
            memcpy(fat_exbr, &par.exbr_start, 8);

            /*for (i = 0; i < ITP_FAT_EXBR_SIZE; i++)
                LOG_INFO "%x ", fat_exbr[i] LOG_END
            LOG_INFO "\n" LOG_END*/

            f = fopen("B:/" ITP_FAT_EXBR_FILE, "wb");
            if (!f)
            {
                LOG_ERR "fopen fail\n" LOG_END
            }
            else
            {
                if (fwrite(fat_exbr, 1, ITP_FAT_EXBR_SIZE, f) != ITP_FAT_EXBR_SIZE)
                    LOG_ERR "fwrite fail\n" LOG_END
                fclose(f);
            }
        }
    }
#endif

    LOG_INFO "Partition finished.\n" LOG_END

end:
#ifdef _WIN32
    ret = 0;
#endif
    return ret;
}
