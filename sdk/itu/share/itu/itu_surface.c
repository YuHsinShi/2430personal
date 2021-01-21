#include <sys/endian.h>
#include <sys/ioctl.h>
#include <assert.h>
#include <unistd.h>
#include "ucl/ucl.h"
#include "ite/itp.h"
#include "ite/itu.h"
#include "itu_cfg.h"
#include "gfx/gfx.h"
#include "itu_private.h"

#if CFG_CHIP_FAMILY != 970
#include "speedy_comp.h"
#endif

ITUSurface *ituSurfaceDecompress(ITUSurface *surf)
{
    ITUSurface   *retSurf = NULL;
    unsigned int flags;
    assert(surf->flags & ITU_COMPRESSED);

    if ((surf->flags & ITU_STATIC) && (surf->lockSize > 0)) // as reference count
    {
        assert(surf->lockAddr);                             // as cached decompressed surface
        surf->lockSize++;
        return (ITUSurface *) surf->lockAddr;
    }

    if (surf->flags & ITU_JPEG)
    {
        flags   = surf->flags & ~(ITU_COMPRESSED | ITU_STATIC | ITU_JPEG);

        if (surf->format == ITU_ARGB8888)
        {
            int ret, jpegSize, alphaSize, alphaCompressSize;
            uint8_t *jpegData, *alphaData, *buf;
            uint32_t vmem;

            memcpy(&jpegSize, (uint8_t*)surf->addr, 4);
            jpegData = (uint8_t*)surf->addr + 4;
            memcpy(&alphaSize, jpegData + jpegSize, 4);
            alphaSize = be32toh(alphaSize);
            memcpy(&alphaCompressSize, jpegData + jpegSize + 4, 4);
            alphaCompressSize = be32_to_cpu(alphaCompressSize);
            alphaData = (uint8_t*)jpegData + jpegSize;

            vmem = itpVmemAlloc(alphaSize);
            if (!vmem)
            {
                LOG_ERR "out of memory: %d\n", alphaSize LOG_END
                return NULL;
            }
            buf = ithMapVram(vmem, alphaSize, ITH_VRAM_WRITE);

        #if defined(CFG_DCPS_ENABLE) && !defined(CFG_ITU_UCL_ENABLE)
            // hardware decompress
            ioctl(ITP_DEVICE_DECOMPRESS, ITP_IOCTL_INIT, NULL);

        #if CFG_CHIP_FAMILY != 970
            if (surf->flags & ITU_SPEEDY)
            {
              	uint8_t dcpsMode = ITP_DCPS_SPEEDY_MODE;      //ITP_DCPS_SPEEDY_MODE=2
                ioctl(ITP_DEVICE_DECOMPRESS, ITP_IOCTL_SET_MODE, &dcpsMode);
            }
        #endif // CFG_CHIP_FAMILY != 970

            ret = write(ITP_DEVICE_DECOMPRESS, alphaData, alphaCompressSize);
            if (ret != alphaCompressSize)
            {
                LOG_ERR "decompress write error: %d != %d\n", ret, alphaCompressSize LOG_END
                ithUnmapVram(buf, alphaSize);
                itpVmemFree(vmem);
                return NULL;
            }

            ret = read(ITP_DEVICE_DECOMPRESS, buf, alphaSize);
            if (ret != alphaSize)
            {
                LOG_ERR "decompress read error: %d != %d\n", ret, alphaSize LOG_END
                ithUnmapVram(buf, alphaSize);
                itpVmemFree(vmem);
                return NULL;
            }
            ioctl(ITP_DEVICE_DECOMPRESS, ITP_IOCTL_EXIT, NULL);

        #else
            // software decompress
        #if CFG_CHIP_FAMILY != 970
            if (surf->flags & ITU_SPEEDY)
            {
                myLZParaType para = { 2, 11, 4096, 6 };
                ret = myLZ_depack(alphaData + 8 + 14, buf, alphaSize, 4096, &para);
                if (ret != alphaSize)
                {
                    LOG_ERR "internal error - decompression speedy failed\n" LOG_END
                    ithUnmapVram(buf, alphaSize);
                    itpVmemFree(vmem);
                    return NULL;
                }
            }
            else
        #endif // CFG_CHIP_FAMILY != 970
            {
                if (ucl_init() != UCL_E_OK)
                {
                    LOG_ERR "internal error - ucl_init() failed !!!\n" LOG_END
                    ithUnmapVram(buf, alphaSize);
                    itpVmemFree(vmem);
                    return NULL;
                }

                ret = alphaSize;
                if (ucl_nrv2e_decompress_8((const ucl_bytep)alphaData + 8, alphaCompressSize, buf, &ret, NULL) != UCL_E_OK || ret != alphaSize)
                {
                    LOG_ERR "internal error - decompression failed\n" LOG_END
                    ithUnmapVram(buf, alphaSize);
                    itpVmemFree(vmem);
                    return NULL;
                }
            }
        #endif     // defined(CFG_DCPS_ENABLE) && !defined(CFG_ITU_UCL_ENABLE)

            ithFlushDCacheRange(buf, alphaSize);
            retSurf = ituJpegAlphaLoad(surf->width, surf->height, buf, jpegData, jpegSize);
            ithUnmapVram(buf, alphaSize);
            itpVmemFree(vmem);
        }
        else
        {
            retSurf = ituJpegLoad(surf->width, surf->height, (uint8_t *)surf->addr, surf->size, 0);
        }
    }
    else
    {
        int ret, bufSize;
        uint32_t compressedSize, vmem;
        uint8_t* buf;

        if (surf->format == ITU_RGB565A8)
            bufSize = surf->pitch * surf->height + surf->width * surf->height;
        else
            bufSize = surf->pitch * surf->height;

        vmem = itpVmemAlloc(bufSize);
        if (!vmem)
        {
            LOG_WARN "out of memory: %d, try to reset font cache...\n", surf->pitch * surf->height LOG_END
            ituFtResetCache();
            vmem = itpVmemAlloc(bufSize);
            if (!vmem)
            {
                LOG_ERR "out of memory: %d\n", surf->pitch * surf->height LOG_END
                return NULL;
            }
        }
        buf = ithMapVram(vmem, bufSize, ITH_VRAM_WRITE);

        compressedSize = surf->size;
        LOG_DBG "decompress surface size %d to %d\n", compressedSize, bufSize LOG_END

#if defined(CFG_DCPS_ENABLE) && !defined(CFG_ITU_UCL_ENABLE)
        // hardware decompress
        ioctl(ITP_DEVICE_DECOMPRESS, ITP_IOCTL_INIT, NULL);

    #if CFG_CHIP_FAMILY != 970
        if (surf->flags & ITU_SPEEDY)
        {
          	uint8_t dcpsMode = ITP_DCPS_SPEEDY_MODE;      //ITP_DCPS_SPEEDY_MODE=2
            ioctl(ITP_DEVICE_DECOMPRESS, ITP_IOCTL_SET_MODE, &dcpsMode);
        }
    #endif // CFG_CHIP_FAMILY != 970

        surf->lockSize = ithBswap32(bufSize);
        surf->parent   = (ITUSurface *)ithBswap32(compressedSize);
        ret = write(ITP_DEVICE_DECOMPRESS, (uint8_t*)surf->addr, compressedSize);
        if (ret != compressedSize)
        {
            LOG_ERR "decompress write error: %d != %d\n", ret, compressedSize LOG_END
            ithUnmapVram(buf, bufSize);
            itpVmemFree(vmem);
            return NULL;
        }

        ret = read(ITP_DEVICE_DECOMPRESS, buf, bufSize);
        if (ret != bufSize)
        {
            LOG_ERR "decompress read error: %d != %d\n", ret, bufSize LOG_END
            ithUnmapVram(buf, bufSize);
            itpVmemFree(vmem);
            return NULL;
        }
        ioctl(ITP_DEVICE_DECOMPRESS, ITP_IOCTL_EXIT, NULL);
        surf->lockSize = 0;

#else
        // software decompress
    #if CFG_CHIP_FAMILY != 970
        if (surf->flags & ITU_SPEEDY)
        {
            if (compressedSize == bufSize)
            {
                memcpy((void*)buf, (const void*)surf->addr, bufSize);
            }
            else
            {
                myLZParaType para = { 2, 11, 4096, 6 };
                ret = myLZ_depack((const void*)(surf->addr + 14), (void*)buf, bufSize, 4096, &para);
                if (ret != bufSize)
                {
                    LOG_ERR "internal error - decompression speedy failed\n" LOG_END
                    ithUnmapVram(buf, bufSize);
                    itpVmemFree(vmem);
                    return NULL;
                }
            }
        }
        else
    #endif // CFG_CHIP_FAMILY != 970
        {
            if (ucl_init() != UCL_E_OK)
            {
                LOG_ERR "internal error - ucl_init() failed !!!\n" LOG_END
                ithUnmapVram(buf, bufSize);
                itpVmemFree(vmem);
                return NULL;
            }
            else
            {
                uint32_t i = 0;
                uint32_t o = 0;
                uint32_t srcSize;
                uint32_t dstSize;
                uint32_t MaxUclBlockSize = 0x400000;
                uint8_t *ptr = (uint8_t*)surf->addr;
                uint8_t *dstBuf = (uint8_t*)buf;

                do
                {
                    dstSize = ((ptr[i] << 24) | (ptr[i + 1] << 16) | (ptr[i + 2] << 8) | (ptr[i + 3]));
                    i += 4;
                    srcSize = ((ptr[i] << 24) | (ptr[i + 1] << 16) | (ptr[i + 2] << 8) | (ptr[i + 3]));
                    i += 4;
                    ret = dstSize;

                    if (!srcSize || !dstSize || (dstSize > MaxUclBlockSize))
                    {
                        if (dstSize > MaxUclBlockSize)
                        {
                            LOG_ERR "internal error - decompression failed, dst size > 4MB\n" LOG_END
                        }
                        break;
                    }

                    if (ucl_nrv2e_decompress_8((const ucl_bytep)&ptr[i], srcSize, dstBuf, &ret, NULL) != UCL_E_OK || ret != dstSize)
                    {
                        LOG_ERR "internal error - decompression failed\n" LOG_END
                        ithUnmapVram(buf, bufSize);
                        itpVmemFree(vmem);
                        return NULL;
                    }

                    i += srcSize;
                    o += dstSize;
                    dstBuf = (uint8_t*)&buf[o];
                } while (i < compressedSize);
            }
        }
#endif     // defined(CFG_DCPS_ENABLE) && !defined(CFG_ITU_UCL_ENABLE)

        ithFlushDCacheRange(buf, bufSize);
        ithUnmapVram(buf, bufSize);

        flags = surf->flags & ~(ITU_COMPRESSED | ITU_STATIC | ITU_SPEEDY);
        retSurf = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (uint8_t*)vmem, flags);
    }

    if (retSurf && (surf->flags & ITU_STATIC) && (surf->lockSize == 0))
    {
        surf->lockSize++;
        surf->lockAddr  = (uint8_t *)retSurf;
        retSurf->parent = (struct ITUSurfaceTag *)surf;
    }
    return retSurf;
}

void ituSurfaceRelease(ITUSurface *surf)
{
    ITUSurface *parentSurf = (ITUSurface *)surf->parent;

    if (parentSurf && (parentSurf->flags & ITU_STATIC))
    {
        assert(parentSurf->flags & ITU_COMPRESSED);
        assert(parentSurf->lockSize);

        if (--parentSurf->lockSize == 0)
        {
            ituDestroySurface(surf);
        }
    }
    else
        ituDestroySurface(surf);
}