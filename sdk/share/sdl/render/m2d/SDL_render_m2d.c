/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2011 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "SDL_config.h"

#if SDL_VIDEO_RENDER_M2D && !SDL_RENDER_DISABLED

#include "SDL_hints.h"
#include "gfx/gfx.h"
#include "../SDL_sysrender.h"

static SDL_Renderer *M2D_CreateRenderer(SDL_Window * window, Uint32 flags);
static void M2D_WindowEvent(SDL_Renderer * renderer,
                             const SDL_WindowEvent *event);
static int M2D_CreateTexture(SDL_Renderer * renderer, SDL_Texture * texture);
static int M2D_UpdateTexture(SDL_Renderer * renderer, SDL_Texture * texture,
                              const SDL_Rect * rect, const void *pixels,
                              int pitch);
static int M2D_LockTexture(SDL_Renderer * renderer, SDL_Texture * texture,
                            const SDL_Rect * rect, void **pixels, int *pitch);
static void M2D_UnlockTexture(SDL_Renderer * renderer,
                               SDL_Texture * texture);
static int M2D_UpdateViewport(SDL_Renderer * renderer);
static int M2D_RenderClear(SDL_Renderer * renderer);
static int M2D_RenderDrawPoints(SDL_Renderer * renderer,
                                 const SDL_Point * points, int count);
static int M2D_RenderDrawLines(SDL_Renderer * renderer,
                                const SDL_Point * points, int count);
static int M2D_RenderFillRects(SDL_Renderer * renderer,
                                const SDL_Rect * rects, int count);
static int M2D_RenderCopy(SDL_Renderer * renderer, SDL_Texture * texture,
                           const SDL_Rect * srcrect,
                           const SDL_Rect * dstrect);
static int M2D_RenderCopyEx(SDL_Renderer * renderer, SDL_Texture * texture,
                         const SDL_Rect * srcrect, const SDL_Rect * dstrect,
                         const double angle, const SDL_Point *center, const SDL_RendererFlip flip);
static void M2D_RenderPresent(SDL_Renderer * renderer);
static void M2D_DestroyTexture(SDL_Renderer * renderer,
                                SDL_Texture * texture);
static void M2D_DestroyRenderer(SDL_Renderer * renderer);


SDL_RenderDriver M2D_RenderDriver = {
    M2D_CreateRenderer,
    {
     "m2d",
     (SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC),
     7,
     {
      SDL_PIXELFORMAT_RGB565,
      SDL_PIXELFORMAT_RGB888,
      SDL_PIXELFORMAT_BGR888,
      SDL_PIXELFORMAT_ARGB8888,
      SDL_PIXELFORMAT_RGBA8888,
      SDL_PIXELFORMAT_ABGR8888,
      SDL_PIXELFORMAT_BGRA8888
     },
     0,
     0}
};

typedef struct
{
    struct {
        Uint32 color;
        int blendMode;
    } current;

} M2D_RenderData;

typedef struct
{
    int texw;
    int texh;
    void *pixels;
    int pitch;
} M2D_TextureData;

static void
M2D_SetError(const char *prefix, int result)
{
    const char *error;

    switch (result) {
    default:
        error = "UNKNOWN";
        break;
    }
    SDL_SetError("%s: %s", prefix, error);
}

static int
M2D_ActivateRenderer(SDL_Renderer * renderer)
{
    M2D_RenderData *data = (M2D_RenderData *) renderer->driverdata;

    M2D_UpdateViewport(renderer);

    return 0;
}

SDL_Renderer *
M2D_CreateRenderer(SDL_Window * window, Uint32 flags)
{
    SDL_Renderer *renderer;
    M2D_RenderData *data;

    renderer = (SDL_Renderer *) SDL_calloc(1, sizeof(*renderer));
    if (!renderer) {
        SDL_OutOfMemory();
        return NULL;
    }

    data = (M2D_RenderData *) SDL_calloc(1, sizeof(*data));
    if (!data) {
        M2D_DestroyRenderer(renderer);
        SDL_OutOfMemory();
        return NULL;
    }

    renderer->WindowEvent = M2D_WindowEvent;
    renderer->CreateTexture = M2D_CreateTexture;
    renderer->UpdateTexture = M2D_UpdateTexture;
    renderer->LockTexture = M2D_LockTexture;
    renderer->UnlockTexture = M2D_UnlockTexture;
    renderer->UpdateViewport = M2D_UpdateViewport;
    renderer->RenderClear = M2D_RenderClear;
    renderer->RenderDrawPoints = M2D_RenderDrawPoints;
    renderer->RenderDrawLines = M2D_RenderDrawLines;
    renderer->RenderFillRects = M2D_RenderFillRects;
    renderer->RenderCopy = M2D_RenderCopy;
    renderer->RenderCopyEx = M2D_RenderCopyEx;
    renderer->RenderPresent = M2D_RenderPresent;
    renderer->DestroyTexture = M2D_DestroyTexture;
    renderer->DestroyRenderer = M2D_DestroyRenderer;
    renderer->info = M2D_RenderDriver.info;
    renderer->info.flags = SDL_RENDERER_ACCELERATED;
    renderer->driverdata = data;

    /* Set up parameters for rendering */
	renderer->window = window;

    return renderer;
}

static void
M2D_WindowEvent(SDL_Renderer * renderer, const SDL_WindowEvent *event)
{
    if (event->event == SDL_WINDOWEVENT_SIZE_CHANGED) {
        /* Rebind the context to the window area and update matrices */
    }
}

static int
M2D_CreateTexture(SDL_Renderer * renderer, SDL_Texture * texture)
{
    M2D_TextureData *data;

    M2D_ActivateRenderer(renderer);

    switch (texture->format) {
    case SDL_PIXELFORMAT_RGB565:
        break;
    case SDL_PIXELFORMAT_RGB888:
        break;
    case SDL_PIXELFORMAT_BGR888:
        break;
    case SDL_PIXELFORMAT_ARGB8888:
        break;
    case SDL_PIXELFORMAT_RGBA8888:
        break;
    case SDL_PIXELFORMAT_ABGR8888:
        break;
    case SDL_PIXELFORMAT_BGRA8888:
        break;
    default:
        SDL_SetError("Texture format not supported");
        return -1;
    }

    data = (M2D_TextureData *) SDL_calloc(1, sizeof(*data));
    if (!data) {
        SDL_OutOfMemory();
        return -1;
    }

    if (texture->access == SDL_TEXTUREACCESS_STREAMING) {
        data->pitch = texture->w * SDL_BYTESPERPIXEL(texture->format);
        data->pixels = SDL_calloc(1, texture->h * data->pitch);
        if (!data->pixels) {
            SDL_OutOfMemory();
            SDL_free(data);
            return -1;
        }
    }

    texture->driverdata = data;

    data->texw = texture->w;
    data->texh = texture->h;

    return 0;
}

static int
M2D_UpdateTexture(SDL_Renderer * renderer, SDL_Texture * texture,
                   const SDL_Rect * rect, const void *pixels, int pitch)
{
    M2D_TextureData *data = (M2D_TextureData *) texture->driverdata;
    Uint8 *blob = NULL;
    Uint8 *src;
    int srcPitch;
    int y;

    M2D_ActivateRenderer(renderer);

    /* Bail out if we're supposed to update an empty rectangle */
    if (rect->w <= 0 || rect->h <= 0)
        return 0;

    /* Reformat the texture data into a tightly packed array */
    srcPitch = rect->w * SDL_BYTESPERPIXEL(texture->format);
    src = (Uint8 *)pixels;
    if (pitch != srcPitch)
    {
        blob = (Uint8 *)SDL_malloc(srcPitch * rect->h);
        if (!blob)
        {
            SDL_OutOfMemory();
            return -1;
        }
        src = blob;
        for (y = 0; y < rect->h; ++y)
        {
            SDL_memcpy(src, pixels, srcPitch);
            src += srcPitch;
            pixels = (Uint8 *)pixels + pitch;
        }
        src = blob;
    }

    /* Create a texture subimage with the supplied data */
    if (blob) {
        SDL_free(blob);
    }
    return 0;
}

static int
M2D_LockTexture(SDL_Renderer * renderer, SDL_Texture * texture,
                 const SDL_Rect * rect, void **pixels, int *pitch)
{
    M2D_TextureData *data = (M2D_TextureData *) texture->driverdata;

    *pixels =
        (void *) ((Uint8 *) data->pixels + rect->y * data->pitch +
                  rect->x * SDL_BYTESPERPIXEL(texture->format));
    *pitch = data->pitch;
    return 0;
}

static void
M2D_UnlockTexture(SDL_Renderer * renderer, SDL_Texture * texture)
{
    M2D_TextureData *data = (M2D_TextureData *) texture->driverdata;
    SDL_Rect rect;

    /* We do whole texture updates, at least for now */
    rect.x = 0;
    rect.y = 0;
    rect.w = texture->w;
    rect.h = texture->h;
    M2D_UpdateTexture(renderer, texture, &rect, data->pixels, data->pitch);
}

static int
M2D_UpdateViewport(SDL_Renderer * renderer)
{
    M2D_RenderData *data = (M2D_RenderData *) renderer->driverdata;

    return 0;
}

static void
M2D_SetColor(M2D_RenderData * data, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    Uint32 color = ((r << 24) | (g << 16) | (b << 8) | a);

    if (color != data->current.color) {
        data->current.color = color;
    }
}

static void
M2D_SetBlendMode(M2D_RenderData * data, int blendMode)
{
    if (blendMode != data->current.blendMode) {
        switch (blendMode) {
        case SDL_BLENDMODE_NONE:
            break;
        case SDL_BLENDMODE_BLEND:
            break;
        case SDL_BLENDMODE_ADD:
            break;
        case SDL_BLENDMODE_MOD:
            break;
        }
        data->current.blendMode = blendMode;
    }
}

static void
M2D_SetDrawingState(SDL_Renderer * renderer)
{
    M2D_RenderData *data = (M2D_RenderData *) renderer->driverdata;

    M2D_ActivateRenderer(renderer);

    M2D_SetColor(data, renderer->r,
                       renderer->g,
                       renderer->b,
                       renderer->a);

    M2D_SetBlendMode(data, renderer->blendMode);
}

static int
M2D_RenderClear(SDL_Renderer * renderer)
{
    M2D_ActivateRenderer(renderer);

	//todo: transferblock 0xF0

    return 0;
}

static int
M2D_RenderDrawPoints(SDL_Renderer * renderer, const SDL_Point * points,
                      int count)
{
	M2D_RenderData *data = (M2D_RenderData *) renderer->driverdata;

    M2D_SetDrawingState(renderer);

    return 0;
}

static int
M2D_RenderDrawLines(SDL_Renderer * renderer, const SDL_Point * points,
                     int count)
{
	M2D_RenderData *data = (M2D_RenderData *) renderer->driverdata;

    M2D_SetDrawingState(renderer);

    return 0;
}

static int
M2D_RenderFillRects(SDL_Renderer * renderer, const SDL_Rect * rects,
                     int count)
{
    M2D_RenderData *data = (M2D_RenderData *) renderer->driverdata;

	M2D_SetDrawingState(renderer);

    return 0;
}

static int
M2D_RenderCopy(SDL_Renderer * renderer, SDL_Texture * texture,
                const SDL_Rect * srcrect, const SDL_Rect * dstrect)
{
	M2D_RenderData *data = (M2D_RenderData *) renderer->driverdata;
    M2D_TextureData *texturedata = (M2D_TextureData *) texture->driverdata;
    int w, h;

    SDL_GetWindowSize(renderer->window, &w, &h);

    return 0;
}

static int
M2D_RenderCopyEx(SDL_Renderer * renderer, SDL_Texture * texture,
                const SDL_Rect * srcrect, const SDL_Rect * dstrect,
                const double angle, const SDL_Point *center, const SDL_RendererFlip flip)
{
	M2D_RenderData *data = (M2D_RenderData *) renderer->driverdata;
    M2D_TextureData *texturedata = (M2D_TextureData *) texture->driverdata;
    int w, h;

    SDL_GetWindowSize(renderer->window, &w, &h);

    M2D_ActivateRenderer(renderer);

    if (texture->modMode) {
        M2D_SetColor(data, texture->r, texture->g, texture->b, texture->a);
    } else {
        M2D_SetColor(data, 255, 255, 255, 255);
    }

    M2D_SetBlendMode(data, texture->blendMode);

    return 0;
}

static void
M2D_RenderPresent(SDL_Renderer * renderer)
{
    M2D_ActivateRenderer(renderer);

    SDL_GL_SwapWindow(renderer->window);
}

static void
M2D_DestroyTexture(SDL_Renderer * renderer, SDL_Texture * texture)
{
    M2D_TextureData *data = (M2D_TextureData *) texture->driverdata;

    M2D_ActivateRenderer(renderer);

    if (!data) {
        return;
    }
    if (data->pixels) {
        SDL_free(data->pixels);
    }
    SDL_free(data);
    texture->driverdata = NULL;
}

static void
M2D_DestroyRenderer(SDL_Renderer * renderer)
{
    M2D_RenderData *data = (M2D_RenderData *) renderer->driverdata;

    if (data) {
        SDL_free(data);
    }
    SDL_free(renderer);
}

#endif /* SDL_VIDEO_RENDER_M2D && !SDL_RENDER_DISABLED */

/* vi: set ts=4 sw=4 expandtab: */
