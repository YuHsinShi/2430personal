#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "ite/ug.h"
#include "ug_cfg.h"

typedef enum
{
    TAG_END         = 0,
    TAG_DEVICE      = 1,
    TAG_UNFORMATTED = 2,
    TAG_RAWDATA     = 3,
    TAG_PARTITION   = 4,
    TAG_DIRECTORY   = 5,
    TAG_FILE        = 6
} tag_type;

#define FLAG_ENCRYPT            (0x1 << 0)
#define FLAG_PARTITION          (0x1 << 1)
#define FLAG_NO_PARTITION       (0x1 << 2)
#define FLAG_FORMAT_PARTITION   (0x1 << 3)

#pragma pack(1)
typedef struct
{
    uint8_t version[16];
    uint32_t blocks_offset;
    uint32_t flags;
} package_t;

static UGDevice ugDevice;

static int ugProgressPercentage = 0;

int ugUpgradePackage(ITCStream* file)
{
    package_t pkg;
    uint32_t readsize = 0;
    int ret = 0;
    bool firstTag = true;
    uint32_t tag = 0;
    int partition = 0;
    char drive[4] = { '\0' };

    LOG_INFO "Start to upgrade package\n" LOG_END

    ugProgressPercentage = 0;

    // read package header
    readsize = itcStreamRead(file, &pkg, sizeof(package_t));
    if (readsize != sizeof(package_t))
    {
        LOG_ERR "Cannot read file: %d != %d\n", readsize, sizeof(package_t) LOG_END
        goto end;
    }

    pkg.blocks_offset = itpLetoh32(pkg.blocks_offset);
    pkg.flags = itpLetoh32(pkg.flags);

    LOG_DBG "version=%c%c%c%c%c%c%c%c blocks_offset=0x%X flags=0x%X\n",
        pkg.version[0], pkg.version[1], pkg.version[2], pkg.version[3], pkg.version[4], pkg.version[5], pkg.version[6], pkg.version[7],
        pkg.blocks_offset,
        pkg.flags
    LOG_END

    for (;;)
    {
        // reag tag
        readsize = itcStreamRead(file, &tag, sizeof(uint32_t));
        if (readsize != sizeof(uint32_t))
        {
            LOG_ERR "Cannot read file: %ld != %ld\n", readsize, sizeof(uint32_t) LOG_END
            goto end;
        }

        tag = itpLetoh32(tag);

        if (firstTag && !ugDevice.restore && (tag >= TAG_RAWDATA))
        {
            bool backup = false;

            if (tag == TAG_RAWDATA)
            {
                #pragma pack(1)
                typedef struct
                {
                    uint32_t position;
                    uint32_t rawdata_size;
                } rawdata_t;
                rawdata_t rdata;
                uint32_t readsize;

                // read raw data header
                readsize = itcStreamRead(file, &rdata, sizeof(rawdata_t));
                if (readsize != sizeof(rawdata_t))
                {
                    ret = __LINE__;
                    LOG_ERR "Cannot read file: %ld != %ld\n", readsize, sizeof(rawdata_t) LOG_END
                    goto end;
                }
                itcStreamSeek(file, -(long)sizeof(rawdata_t), SEEK_CUR);

                rdata.rawdata_size  = itpLetoh32(rdata.rawdata_size);

                if (rdata.rawdata_size == 0)
                {
                    backup = true;
                }
            }

            ret = ugUpgradeStart(backup);
            if (ret)
            {
                LOG_ERR "Start to upgrade package failed: %d\n", ret LOG_END
                goto end;
            }
            firstTag = false;
        }

        switch (tag)
        {
        case TAG_END:
            LOG_DBG "end:\n" LOG_END
            ret = ugUpgradeEnd(file);
            if (ret)
                goto end;

            ret = ugUpgradeFinish();
            if (ret)
            {
                LOG_ERR "Finish upgrading package failed: %d\n", ret LOG_END
                goto end;
            }
            goto end;

        case TAG_DEVICE:
            LOG_DBG "device:\n" LOG_END
            ret = ugUpgradeDevice(file, &ugDevice);
            if (ret)
                goto end;

            ugDevice.partition = pkg.flags & FLAG_PARTITION ? true : false;
            ugDevice.nopartition = pkg.flags & FLAG_NO_PARTITION ? true : false;

            partition = 0;
            break;

        case TAG_UNFORMATTED:
            LOG_DBG "unformatted:\n" LOG_END
            ret = ugUpgradeUnformatted(file, ugDevice.disk);
            if (ret)
                goto end;
            break;

        case TAG_RAWDATA:
            LOG_DBG "rawdata:\n" LOG_END
            ret = ugUpgradeRawData(file, ugDevice.disk, ugDevice.restore);
            if (ret)
                goto end;
            break;

        case TAG_PARTITION:
            LOG_DBG "partition:\n" LOG_END
            if (partition == 0)
            {
                ret = ugUpgradeDoPartition(&ugDevice);
                if (ret)
                    goto end;
            }
            ret = ugUpgradePartition(file, ugDevice.disk, partition, drive, (pkg.flags & FLAG_FORMAT_PARTITION) ? true : false);
            if (ret)
                goto end;

            partition++;
            break;

        case TAG_DIRECTORY:
            LOG_DBG "directory:\n" LOG_END
            ret = ugUpgradeDirectory(file, drive);
            if (ret)
                goto end;
            break;

        case TAG_FILE:
            LOG_DBG "file:\n" LOG_END
            ret = ugUpgradeFile(file, drive);
            if (ret)
                goto end;
            break;

        default:
            LOG_ERR "Unknown tag %d\n", tag LOG_END
            ret = __LINE__;
            goto end;
        }
        ugProgressPercentage = (int)((uint64_t)(file->size - file->Available(file)) * 100 / file->size);
    }

end:
    itcStreamClose(file);
    return ret;
}

int ugGetProrgessPercentage(void)
{
    return ugProgressPercentage;
}

void ugRestoreStart(void)
{
    ugDevice.restore = true;
}
