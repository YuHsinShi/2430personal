#include <sys/stat.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "png.h"
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

static int ituPngCurrPos;
static png_size_t ituPngCurrSize;

static void ReadDataFromInputStream(png_structp png_ptr, png_bytep outBytes, png_size_t byteCountToRead)
{
    int size;
    uint8_t* io_ptr = (uint8_t*)png_get_io_ptr(png_ptr);
    if (io_ptr == NULL)
        return;

    size = (byteCountToRead <= ituPngCurrSize) ? byteCountToRead : ituPngCurrSize;
    if (size > 0)
    {
        memcpy(outBytes, &io_ptr[ituPngCurrPos], size);
        ituPngCurrPos += size;
        ituPngCurrSize -= size;
    }
}

ITUSurface *ituPngLoad(int width, int height, uint8_t *data, int size)
{
    ITUSurface* surf = NULL;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    png_bytepp row_pointers = NULL;
    int bit_depth, color_type;
    png_uint_32 temp_width, temp_height;
    ITUPixelFormat format;
    int w, h, x, y;

    assert(data);

    if (!png_check_sig(data, 8))
    {
        LOG_ERR "png_check_sig returned 0.\n" LOG_END
        goto end;
    }

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
    {
        LOG_ERR "png_create_read_struct returned 0.\n" LOG_END
        goto end;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        LOG_ERR "png_create_info_struct returned 0.\n" LOG_END
        goto end;
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        LOG_ERR "error from libpng.\n" LOG_END
        goto end;
    }

    ituPngCurrPos = 0;
    ituPngCurrSize = size;
    png_set_read_fn(png_ptr, data + 8, ReadDataFromInputStream);
    png_set_sig_bytes(png_ptr, 8);
    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND | PNG_TRANSFORM_BGR, NULL);
    png_get_IHDR(png_ptr, info_ptr, &temp_width, &temp_height, &bit_depth, &color_type, NULL, NULL, NULL);

    LOG_DBG "png: %lux%lu %d\n", temp_width, temp_height, color_type LOG_END

    if (bit_depth != 8)
    {
        LOG_ERR "Unsupported bit depth %d.  Must be 8.\n", bit_depth LOG_END
        goto end;
    }

    switch (color_type)
    {
    case PNG_COLOR_TYPE_RGB:
        format = ITU_RGB565;
        break;

    case PNG_COLOR_TYPE_RGB_ALPHA:
        format = ITU_ARGB8888;
        break;

    default:
        LOG_ERR "Unknown libpng color type %d.\n", bit_depth LOG_END
        goto end;
    }

    if (width == 0 || height == 0)
    {
        w = (int)temp_width;
        h = (int)temp_height;
    }
    else
    {
        w = width < (int)temp_width ? width : (int)temp_width;
        h = height < (int)temp_height ? height : (int)temp_height;
    }

    surf = ituCreateSurface(w, h, 0, format, NULL, 0);
    if (!surf)
        goto end;

    row_pointers = png_get_rows(png_ptr, info_ptr);

    if (format == ITU_RGB565)
    {
        uint16_t* dest = (uint16_t*)ituLockSurface(surf, 0, 0, w, h);
        assert(dest);

        for (y = 0; y < h; y++)
        {
            uint8_t* row = (uint8_t*)row_pointers[y];
            for (x = 0; x < w; x++)
            {
                dest[x + y * w] = ITH_RGB565(row[x * 3 + 2], row[x * 3 + 1], row[x * 3 + 0]);
            }
        }
        ituUnlockSurface(surf);
    }
    else // if (format == ITU_ARGB8888)
    {
        uint32_t* dest = (uint32_t*)ituLockSurface(surf, 0, 0, w, h);
        assert(dest);

        for (y = 0; y < h; y++)
        {
            uint32_t* row = (uint32_t*)row_pointers[y];
            for (x = 0; x < w; x++)
            {
                dest[x + y * w] = row[x];
            }
        }
        ituUnlockSurface(surf);
    }

end:
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

    return surf;
}

ITUSurface *ituPngLoadFile(int width, int height, char* filepath)
{
    ITUSurface* surf = NULL;
    FILE* fp = NULL;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    png_bytepp row_pointers = NULL;
    int bit_depth, color_type;
    png_uint_32 temp_width, temp_height;
    ITUPixelFormat format;
    int w, h, x, y;

    assert(filepath);

    fp = fopen(filepath, "rb");
    if (!fp)
        goto end;

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
    {
        LOG_ERR "%s: png_create_read_struct returned 0.\n", filepath LOG_END
        goto end;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        LOG_ERR "%s: png_create_info_struct returned 0.\n", filepath LOG_END
        goto end;
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        LOG_ERR "%s: error from libpng.\n", filepath LOG_END
        goto end;
    }

    png_init_io(png_ptr, fp);
    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND | PNG_TRANSFORM_BGR, NULL);
    png_get_IHDR(png_ptr, info_ptr, &temp_width, &temp_height, &bit_depth, &color_type, NULL, NULL, NULL);

    LOG_DBG "%s: %lux%lu %d\n", filepath, temp_width, temp_height, color_type LOG_END

    if (bit_depth != 8)
    {
        LOG_ERR "%s: Unsupported bit depth %d.  Must be 8.\n", filepath, bit_depth LOG_END
        goto end;
    }

    switch (color_type)
    {
    case PNG_COLOR_TYPE_RGB:
        format = ITU_RGB565;
        break;

    case PNG_COLOR_TYPE_RGB_ALPHA:
        format = ITU_ARGB8888;
        break;

    default:
        LOG_ERR "%s: Unknown libpng color type %d.\n", filepath, bit_depth LOG_END
        goto end;
    }

    if (width == 0 || height == 0)
    {
        w = (int)temp_width;
        h = (int)temp_height;
    }
    else
    {
        w = width < (int)temp_width ? width : (int)temp_width;
        h = height < (int)temp_height ? height : (int)temp_height;
    }

    surf = ituCreateSurface(w, h, 0, format, NULL, 0);
    if (!surf)
        goto end;

    row_pointers = png_get_rows(png_ptr, info_ptr);

    if (format == ITU_RGB565)
    {
        uint16_t* dest = (uint16_t*)ituLockSurface(surf, 0, 0, w, h);
        assert(dest);

        for (y = 0; y < h; y++)
        {
            uint8_t* row = (uint8_t*)row_pointers[y];
            for (x = 0; x < w; x++)
            {
                dest[x + y * w] = ITH_RGB565(row[x * 3 + 2], row[x * 3 + 1], row[x * 3 + 0]);
            }
        }
        ituUnlockSurface(surf);
    }
    else // if (format == ITU_ARGB8888)
    {
        uint32_t* dest = (uint32_t*)ituLockSurface(surf, 0, 0, w, h);
        assert(dest);

        for (y = 0; y < h; y++)
        {
            uint32_t* row = (uint32_t*)row_pointers[y];
            for (x = 0; x < w; x++)
            {
                dest[x + y * w] = row[x];
            }
        }
        ituUnlockSurface(surf);
    }

end:
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

    if (fp)
        fclose(fp);

    return surf;
}

void ituPngSaveFile(ITUSurface* surf, char* filepath)
{
    FILE* fp = NULL;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    png_bytepp row_pointers = NULL;
    int h, y;
    uint8_t* src = NULL;

    assert(filepath);

    fp = fopen(filepath, "wb");
    if (!fp)
        goto end;

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
    {
        LOG_ERR "%s: png_create_write_struct returned 0.\n", filepath LOG_END
            goto end;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        LOG_ERR "%s: png_create_info_struct returned 0.\n", filepath LOG_END
        goto end;
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        LOG_ERR "%s: error from libpng.\n", filepath LOG_END
        goto end;
    }

    row_pointers = (png_bytep*)calloc(1, sizeof(png_bytep)* surf->height);
    if (!row_pointers)
    {
        goto end;
    }

    png_init_io(png_ptr, fp);
    png_set_IHDR(png_ptr, info_ptr, surf->width, surf->height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    for (y = 0; y < surf->height; y++)
    {
        row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png_ptr, info_ptr));
        if (!row_pointers[y])
        {
            goto end;
        }
    }

    LOG_DBG "%s: %lux%lu\n", filepath, surf->width, surf->height LOG_END

    png_write_info(png_ptr, info_ptr);

    src = ituLockSurface(surf, 0, 0, surf->width, surf->height);
    assert(src);

    if (surf == ituGetDisplaySurface())
    {
        uint32_t addr;

        switch (ithLcdGetFlip())
        {
        case 0:
            addr = ithLcdGetBaseAddrA();
            break;

        case 1:
            addr = ithLcdGetBaseAddrB();
            break;

        default:
            addr = ithLcdGetBaseAddrC();
            break;
        }
        src = ithMapVram(addr, surf->lockSize, ITH_VRAM_READ);
    }

    if (surf->format == ITU_ARGB8888)
    {
        for (h = 0; h < surf->height; h++)
        {
            int i, j;
            uint8_t* ptr = src + surf->width * 4 * h;

            // color trasform from ARGB8888 to RGB888
            for (i = (surf->width - 1) * 4, j = (surf->width - 1) * 3; i >= 0 && j >= 0; i -= 4, j -= 3)
            {
                row_pointers[h][j + 0] = ptr[i + 2];
                row_pointers[h][j + 1] = ptr[i + 1];
                row_pointers[h][j + 2] = ptr[i + 0];
            }
        }
    }
    else if (surf->format == ITU_RGB565)
    {
        for (h = 0; h < surf->height; h++)
        {
            int i, j;
            uint8_t* ptr = src + surf->width * 2 * h;

            // color trasform from RGB565 to RGB888
            for (i = (surf->width - 1) * 2, j = (surf->width - 1) * 3; i >= 0 && j >= 0; i -= 2, j -= 3)
            {
                row_pointers[h][j + 0] = ((ptr[i + 1]) & 0xf8) + ((ptr[i + 1] >> 5) & 0x07);
                row_pointers[h][j + 1] = ((ptr[i + 0] >> 3) & 0x1c) + ((ptr[i + 1] << 5) & 0xe0) + ((ptr[i + 1] >> 1) & 0x3);
                row_pointers[h][j + 2] = ((ptr[i + 0] << 3) & 0xf8) + ((ptr[i + 0] >> 2) & 0x07);
            }
        }
    }
    png_write_image(png_ptr, row_pointers);
    png_write_end(png_ptr, NULL);
    ituUnlockSurface(surf);

end:
    png_destroy_write_struct(&png_ptr, &info_ptr);

    for (y = 0; y < surf->height; y++)
        free(row_pointers[y]);

    free(row_pointers);

    if (fp)
        fclose(fp);
}
