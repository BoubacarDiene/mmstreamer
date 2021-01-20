//////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                              //
//              Copyright © 2016, 2018 Boubacar DIENE                                           //
//                                                                                              //
//              This file is part of mmstreamer project.                                        //
//                                                                                              //
//              mmstreamer is free software: you can redistribute it and/or modify              //
//              it under the terms of the GNU General Public License as published by            //
//              the Free Software Foundation, either version 2 of the License, or               //
//              (at your option) any later version.                                             //
//                                                                                              //
//              mmstreamer is distributed in the hope that it will be useful,                   //
//              but WITHOUT ANY WARRANTY; without even the implied warranty of                  //
//              MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                   //
//              GNU General Public License for more details.                                    //
//                                                                                              //
//              You should have received a copy of the GNU General Public License               //
//              along with mmstreamer. If not, see <http://www.gnu.org/licenses/>               //
//              or write to the Free Software Foundation, Inc., 51 Franklin Street,             //
//              51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.                   //
//                                                                                              //
//////////////////////////////////////////////////////////////////////////////////////////////////

/*!
* \file Drawer2.c
* \brief Graphics elements drawer based on SDLv2
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// HEADERS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#include <fcntl.h>
#include <sys/types.h>

#include "SDL2/SDL.h"
#include "SDL2/SDL_events.h"
#include "SDL2/SDL_image.h"
#include "SDL2/SDL_ttf.h"

#include "utils/Log.h"

#include "graphics/Drawer.h"

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// MACROS ////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "Drawer"

#define SCREENSHOT_ADJUST_VIDEO_X 60
#define SCREENSHOT_ADJUST_VIDEO_Y 25

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// TYPES /////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

struct drawer_video_s {
    struct gfx_video_s params;

    SDL_Window         *window;
    SDL_Renderer       *renderer;
    SDL_Surface        *surface;
    SDL_Texture        *texture;
    SDL_Texture        *initialBgTexture;

    SDL_RWops          *rwops;

    uint32_t           windowID;
    uint32_t           pixelFormat;
};

struct drawer_image_s {
    SDL_Surface *surface;
    SDL_Texture *texture;
};

struct drawer_text_s {
    SDL_Surface *surface;
    SDL_Texture *texture;

    SDL_Color   color;
    TTF_Font    *font;
};

struct drawer_screen_s {
    struct gfx_screen_s params;

    SDL_Window          *window;
    SDL_Renderer        *renderer;
    SDL_Surface         *surface;
    SDL_Texture         *texture;
    SDL_Texture         *initialBgTexture;

    uint32_t            windowID;

    volatile uint8_t    update;
};

struct drawer_private_data_s {
    struct drawer_screen_s screen;
    struct drawer_video_s  video;
    struct drawer_image_s  image;
    struct drawer_text_s   text;

    SDL_Rect               rect;
    SDL_Event              event;
    SDL_mutex              *lock;
};

/* -------------------------------------------------------------------------------------------- */
/*                                 PUBLIC FUNCTIONS PROTOTYPES                                  */
/* -------------------------------------------------------------------------------------------- */

static enum drawer_error_e initScreen_f(struct drawer_s *obj, struct gfx_screen_s *screenParams);
static enum drawer_error_e uninitScreen_f(struct drawer_s *obj);

static enum drawer_error_e drawVideo_f(struct drawer_s *obj, struct gfx_rect_s *rect,
                                       struct buffer_s *buffer);
static enum drawer_error_e drawImage_f(struct drawer_s *obj, struct gfx_rect_s *rect,
                                       struct gfx_image_s *image, enum gfx_target_e target);
static enum drawer_error_e drawText_f(struct drawer_s *obj, struct gfx_rect_s *rect,
                                      struct gfx_text_s *text, enum gfx_target_e target);

static enum drawer_error_e setBgColor_f(struct drawer_s *obj, struct gfx_rect_s *rect,
                                        struct gfx_color_s *color, enum gfx_target_e target);
static enum drawer_error_e restoreBgColor_f(struct drawer_s *obj, struct gfx_rect_s *rect,
                                            struct gfx_color_s *fallbackColor,
                                            enum gfx_target_e target);

static enum drawer_error_e startDrawingInBg_f(struct drawer_s *obj);
static enum drawer_error_e stopDrawingInBg_f(struct drawer_s *obj);

static enum drawer_error_e saveBuffer_f(struct drawer_s *obj, struct buffer_s *buffer,
                                        struct gfx_image_s *inOut);
static enum drawer_error_e saveTarget_f(struct drawer_s *obj, struct gfx_image_s *inOut,
                                        struct gfx_rect_s *srcRect, enum gfx_target_e target);

static enum drawer_error_e getEvent_f(struct drawer_s *obj, struct gfx_event_s *gfxEvent);
static enum drawer_error_e stopAwaitingEvent_f(struct drawer_s *obj);

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////// PRIVATE FUNCTIONS PROTOTYPES /////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

static int sdlEventFilter_f(void *userdata, SDL_Event* event);
static enum drawer_error_e convertSdlEvent_f(struct drawer_s *obj, SDL_Event *event,
                                             struct gfx_event_s *out);

static enum drawer_error_e initSdlComponents_f(struct drawer_s *obj, int32_t imgFlags);
static enum drawer_error_e uninitSdlComponents_f(struct drawer_s *obj);

static enum drawer_error_e initWindowAndRenderer_f(struct drawer_s *obj, enum gfx_target_e target,
                                                   struct gfx_rect_s *pos, uint8_t showWindow);
static enum drawer_error_e uninitWindowAndRenderer_f(struct drawer_s *obj,
                                                     enum gfx_target_e target);

static enum drawer_error_e initVideoContext_f(struct drawer_s *obj, struct gfx_rect_s *rect,
                                              uint8_t showWindow);
static enum drawer_error_e uninitVideoContext_f(struct drawer_s *obj);

static enum drawer_error_e setInitialBackground_f(struct drawer_s *obj, enum gfx_target_e target);
static enum drawer_error_e setWindowIcon_f(struct drawer_s *obj, SDL_Window *window,
                                           struct gfx_image_s *icon);
static enum drawer_error_e waitForWindowEvent_f(struct drawer_s *obj, enum gfx_target_e target,
                                                SDL_WindowEventID eventID, SDL_Event *inOut);

static enum drawer_error_e setTextureTransparency_f(struct drawer_s *obj, SDL_Texture *texture,
                                                    uint8_t alpha);

static enum drawer_error_e renderTexture_f(struct drawer_s *obj, enum gfx_target_e target,
                                           SDL_Texture *texture, SDL_Rect *srcRect,
                                           SDL_Rect *dstRect);

static enum drawer_error_e createSurfaceFromTarget_f(struct drawer_s *obj, enum gfx_target_e target,
                                                     SDL_Surface **surface, SDL_Rect *rect);
static enum drawer_error_e writeSurfaceToFile_f(struct drawer_s *obj, SDL_Surface *surface,
                                                struct gfx_image_s *inOut);

static enum drawer_error_e adjustDrawingRect_f(struct drawer_s *obj, enum gfx_target_e target,
                                               SDL_Rect *inOut);

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// INITIALIZER //////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
enum drawer_error_e Drawer_Init(struct drawer_s **obj)
{
    ASSERT(obj && (*obj = calloc(1, sizeof(struct drawer_s))));
    
    struct drawer_private_data_s *pData;
    ASSERT((pData = calloc(1, sizeof(struct drawer_private_data_s))));
    
    (*obj)->initScreen        = initScreen_f;
    (*obj)->uninitScreen      = uninitScreen_f;
    
    (*obj)->drawVideo         = drawVideo_f;
    (*obj)->drawImage         = drawImage_f;
    (*obj)->drawText          = drawText_f;
    
    (*obj)->setBgColor        = setBgColor_f;
    (*obj)->restoreBgColor    = restoreBgColor_f;
    
    (*obj)->startDrawingInBg  = startDrawingInBg_f;
    (*obj)->stopDrawingInBg   = stopDrawingInBg_f;
    
    (*obj)->saveBuffer        = saveBuffer_f;
    (*obj)->saveTarget        = saveTarget_f;
    
    (*obj)->getEvent          = getEvent_f;
    (*obj)->stopAwaitingEvent = stopAwaitingEvent_f;
    
    ASSERT((pData->lock = SDL_CreateMutex()));
    
    (*obj)->pData = (void*)pData;
    
    return DRAWER_ERROR_NONE;
}

/*!
 *
 */
enum drawer_error_e Drawer_UnInit(struct drawer_s **obj)
{
    ASSERT(obj && *obj && (*obj)->pData);
    
    struct drawer_private_data_s *pData = (struct drawer_private_data_s*)((*obj)->pData);
    
    SDL_DestroyMutex(pData->lock);
    
    free(pData);
    free(*obj);
    *obj = NULL;
    
    return DRAWER_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////// PUBLIC FUNCTIONS IMPLEMENTATION ///////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static enum drawer_error_e initScreen_f(struct drawer_s *obj, struct gfx_screen_s *screenParams)
{
    ASSERT(obj && obj->pData && screenParams);
    
    struct drawer_private_data_s *pData = (struct drawer_private_data_s*)(obj->pData);
    enum drawer_error_e ret             = DRAWER_ERROR_INIT;

    pData->screen.params = *screenParams;
    pData->video.params  = screenParams->video;

    ret = initSdlComponents_f(obj, IMG_INIT_JPG|IMG_INIT_PNG);
    if (ret != DRAWER_ERROR_NONE) {
        Loge("Failed to init SDL components");
        goto initSdlComponentsExit;
    }

    ret = initWindowAndRenderer_f(obj, GFX_TARGET_SCREEN, &pData->screen.params.rect, 1);
    if (ret != DRAWER_ERROR_NONE) {
        Loge("Failed to init screen window and renderer");
        goto initScreenWindowAndRendererExit;
    }

    ret = setInitialBackground_f(obj, GFX_TARGET_SCREEN);
    if (ret != DRAWER_ERROR_NONE) {
        Loge("Failed to set screen's background");
        goto initScreenWindowAndRendererExit;
    }

    ret = initVideoContext_f(obj, &pData->video.params.rect, 1);
    if (ret != DRAWER_ERROR_NONE) {
        Loge("Failed to init video context (window, renderer, texture, ...)");
        goto initVideoContextExit;
    }

    ret = setInitialBackground_f(obj, GFX_TARGET_VIDEO);
    if (ret != DRAWER_ERROR_NONE) {
        Loge("Failed to set video area's background");
        goto initVideoContextExit;
    }

    // Filter out some events before they are posted to the SDL's internal
    // event queue
    SDL_SetEventFilter(sdlEventFilter_f, obj);

    return DRAWER_ERROR_NONE;

initVideoContextExit:
    (void)uninitVideoContext_f(obj);

initScreenWindowAndRendererExit:
    (void)uninitWindowAndRenderer_f(obj, GFX_TARGET_SCREEN);

initSdlComponentsExit:
    (void)uninitSdlComponents_f(obj);

    return ret;
}

/*!
 *
 */
static enum drawer_error_e uninitScreen_f(struct drawer_s *obj)
{
    ASSERT(obj && obj->pData);

    (void)uninitVideoContext_f(obj);
    (void)uninitWindowAndRenderer_f(obj, GFX_TARGET_SCREEN);
    (void)uninitSdlComponents_f(obj);
    
    return DRAWER_ERROR_NONE;
}

/*!
 *
 */
static enum drawer_error_e drawVideo_f(struct drawer_s *obj, struct gfx_rect_s *rect,
                                       struct buffer_s *buffer)
{
    ASSERT(obj && obj->pData);
    
    if (!rect || !buffer || !buffer->data) {
        //Loge("No valid video buffer received yet");
        return DRAWER_ERROR_PARAMS;
    }
    
    struct drawer_private_data_s *pData = (struct drawer_private_data_s*)(obj->pData);
    enum drawer_error_e ret             = DRAWER_ERROR_DRAW;
    
    if (SDL_LockMutex(pData->lock) != 0) {
        Loge("Failed to lock mutex");
        return DRAWER_ERROR_LOCK;
    }

    SDL_Rect newRect = {rect->x, rect->y, (int32_t)rect->w, (int32_t)rect->h};

    (void)adjustDrawingRect_f(obj, GFX_TARGET_VIDEO, &newRect);

    // MJPEG format
    if (pData->video.pixelFormat == SDL_PIXELFORMAT_ARGB8888) {
        pData->video.rwops   = SDL_RWFromMem(buffer->data, (int32_t)buffer->length);
        pData->video.surface = IMG_Load_RW(pData->video.rwops, 0);

        pData->video.texture = SDL_CreateTextureFromSurface(pData->video.renderer,
                                                            pData->video.surface);
        SDL_FreeSurface(pData->video.surface);
        SDL_RWclose(pData->video.rwops);

        if (!pData->video.texture) {
            // This can occur if
            // - video.pixelformat does not match video streaming format
            //   (E.g. Graphics.xml#pixelformat="0" && Videos.xml#configChoice="2"
            //   i.e ARGB8888 vs YVYU)
            // - Buffer is received from "Clients" module which sometimes could provide
            //   incomplete data (To be fixed!)

            //Loge("Failed to create video texture - %s", SDL_GetError());
            goto unlockMutexExit;
        }

        ret = renderTexture_f(obj, GFX_TARGET_VIDEO, pData->video.texture, NULL, &newRect);

        SDL_DestroyTexture(pData->video.texture);
        goto unlockMutexExit;
    }

    // YUV format
    //
    // Lots of people use SDL_UpdateTexture() to update texture with received data but this
    // method has a big drawback :
    //   "This is a fairly slow function, intended for use with static textures that do
    //    not change often" --> Cf. SDL official documentation
    //
    // Another reason to not use it is that the pitch (i.e the number of bytes per line) is
    // one of its expected argument. Here are the two common ways of calculating pitch :
    // - pitch = buffer->length / newRect.h
    // - pitch = SDL_BYTESPERPIXEL(pData->video.pixelFormat) * newRect.w
    //
    // Using these formula could lead to serious issues
    // E.g. Segmentation fault when for example trying to SDL_UpdatTexture() from a 640 x 480
    //   video buffer to a 320 x 240 streaming texture
    //
    // For YUV format, it's indeed a bit risky to calculate pitch in such ways due to internal
    // operations made by SDL when creating a streaming texture :
    //
    // if (texture->access == SDL_TEXTUREACCESS_STREAMING) {
    //    size_t size;
    //    data->pitch = texture->w * SDL_BYTESPERPIXEL(texture->format);
    //    size = texture->h * data->pitch;
    //    if (data->yuv) {
    //        /* Need to add size for the U and V planes */
    //        size += (2 * (texture->h * data->pitch) / 4);
    //    }
    //    if (data->nv12) {
    //        /* Need to add size for the U/V plane */
    //        size += ((texture->h * data->pitch) / 2);
    //    }
    //    data->pixel_data = SDL_calloc(1, size);
    //    if (!data->pixel_data) {
    //        SDL_free(data);
    //        return SDL_OutOfMemory();
    //    }
    // }
    //
    // As you can see, size is calculated differently for YUV and NV12 formats
    // => A more reliable solution must be used as for example SDL_LockTexture() which returns
    //    pitch value and pixels in write-only mode so received data could be simply copied using
    //    memcpy() function without taking care of pitch.
    void *pixels = NULL;
    int pitch    = 0;

    // --> Lock texture to retrieve pixels and pitch
    if (SDL_LockTexture(pData->video.texture, NULL, &pixels, &pitch) != 0) {
        Loge("SDL_LockTexture() failed - %s", SDL_GetError());
        goto unlockMutexExit;
    }

    // --> Copy buffer to texture's pixels
    memcpy(pixels, buffer->data, buffer->length);

    // --> Unlock texture to apply changes
    SDL_UnlockTexture(pData->video.texture);

    // --> Now, render texture to screen
    ret = renderTexture_f(obj, GFX_TARGET_VIDEO, pData->video.texture, NULL, &newRect);

unlockMutexExit:
    SDL_UnlockMutex(pData->lock);
    
    return ret;
}

/*!
 *
 */
static enum drawer_error_e drawImage_f(struct drawer_s *obj, struct gfx_rect_s *rect,
                                       struct gfx_image_s *image, enum gfx_target_e target)
{
    ASSERT(obj && obj->pData);
    
    if (!rect || !image) {
        Loge("Bad arguments");
        return DRAWER_ERROR_PARAMS;
    }
    
    if (access(image->path, F_OK) != 0) {
        Loge("File \"%s\" not found", image->path);
        return DRAWER_ERROR_DRAW;
    }
    
    struct drawer_private_data_s *pData = (struct drawer_private_data_s*)(obj->pData);
    enum drawer_error_e ret             = DRAWER_ERROR_DRAW;

    if (SDL_LockMutex(pData->lock) != 0) {
        Loge("Failed to lock mutex");
        return DRAWER_ERROR_LOCK;
    }

    // Get renderer
    SDL_Renderer *renderer = (target == GFX_TARGET_VIDEO ? pData->video.renderer
                                                         : pData->screen.renderer);
    if (!renderer) {
        Loge("Renderer not initialized yet for target %d", target);
        goto exit;
    }

    // Create image texture
    int32_t width  = -1;
    int32_t height = -1;

    if (image->hiddenColor) {
        pData->image.surface = IMG_Load(image->path);
        if (!pData->image.surface) {
            Loge("Failed to load image \"%s\"", image->path);
            goto exit;
        }

        SDL_SetColorKey(pData->image.surface,
            SDL_TRUE,
            SDL_MapRGBA(pData->image.surface->format,
                image->hiddenColor->red,
                image->hiddenColor->green,
                image->hiddenColor->blue,
                image->hiddenColor->alpha
            )
        );

        width  = pData->image.surface->w;
        height = pData->image.surface->h;

        pData->image.texture = SDL_CreateTextureFromSurface(renderer, pData->image.surface);
        SDL_FreeSurface(pData->image.surface); // Surface not needed anymore
    }
    else {
        pData->image.texture = IMG_LoadTexture(renderer, image->path);
    }

    // Set dstRect
    if (!pData->image.texture) {
        Loge("Failed to load image \"%s\"", image->path);
        goto exit;
    }

    if ((width < 0) || (height < 0)) {
        SDL_QueryTexture(pData->image.texture, NULL, NULL, &width, &height);
    }

    Logd("Real size of image \"%s\": %d x %d", image->path, width, height);

    if ((width < (int32_t)rect->w) && (height < (int32_t)rect->h)) {
        Logd("Centering image \"%s\" inside element", image->path);
        pData->rect.x = rect->x + (((int32_t)rect->w - width) / 2);
        pData->rect.y = rect->y + (((int32_t)rect->h - height) / 2);
        pData->rect.w = width;
        pData->rect.h = height;
    }
    else {
        pData->rect.x = rect->x;
        pData->rect.y = rect->y;
        pData->rect.w = (int32_t)rect->w;
        pData->rect.h = (int32_t)rect->h;
    }

    (void)adjustDrawingRect_f(obj, target, &pData->rect);

    // Render to screen
    ret = renderTexture_f(obj, target, pData->image.texture, NULL, &pData->rect);

    // Destroy texture
    SDL_DestroyTexture(pData->image.texture);

exit:
    SDL_UnlockMutex(pData->lock);
    
    return ret;
}

/*!
 *
 */
static enum drawer_error_e drawText_f(struct drawer_s *obj, struct gfx_rect_s *rect,
                                      struct gfx_text_s *text, enum gfx_target_e target)
{
    ASSERT(obj && obj->pData);
    
    if (!rect || !text) {
        Loge("Bad arguments");
        return DRAWER_ERROR_PARAMS;
    }
    
    if (access(text->ttfFont, F_OK) != 0) {
        Loge("Font file \"%s\" not found", text->ttfFont);
        return DRAWER_ERROR_PARAMS;
    }
    
    struct drawer_private_data_s *pData = (struct drawer_private_data_s*)(obj->pData);
    enum drawer_error_e ret             = DRAWER_ERROR_DRAW;
    
    if (SDL_LockMutex(pData->lock) != 0) {
        Loge("Failed to lock mutex");
        return DRAWER_ERROR_LOCK;
    }

    // Get renderer
    SDL_Renderer *renderer = (target == GFX_TARGET_VIDEO ? pData->video.renderer
                                                         : pData->screen.renderer);
    if (!renderer) {
        Loge("Renderer not initialized yet for target %d", target);
        goto exit;
    }

    // Create texture from text  
    pData->text.font = TTF_OpenFont(text->ttfFont, text->ttfFontSize);  
    if (!pData->text.font) {
        Loge("Failed to open font file \"%s\" - %s", text->ttfFont, TTF_GetError());
        goto exit;
    }
    
    pData->text.color.r = text->color.red;
    pData->text.color.g = text->color.green;
    pData->text.color.b = text->color.blue;
    pData->text.color.a = text->color.alpha;

    pData->text.surface = TTF_RenderText_Solid(pData->text.font, text->str, pData->text.color);
    if (!pData->text.surface) {
        Loge("Failed to render text - %s", TTF_GetError());
        goto exit;
    }

    int32_t width  = pData->text.surface->w;
    int32_t height = pData->text.surface->h;

    pData->text.texture = SDL_CreateTextureFromSurface(renderer, pData->text.surface);
    SDL_FreeSurface(pData->text.surface); // Surface not needed anymore

    if (!pData->text.texture) {
        Loge("Failed to create texture from text \"%s\"", text->str);
        goto exit;
    }

    Logd("Real size of text \"%s\": %u x %u", text->str, width, height);

    // Set transparency
    (void)setTextureTransparency_f(obj, pData->text.texture, pData->text.color.a);

    // Set dstRect
    if ((width < (int32_t)rect->w) && (height < (int32_t)rect->h)) {
        Logd("Centering text \"%s\" inside element", text->str);
        pData->rect.x = rect->x + (((int32_t)rect->w - width) / 2);
        pData->rect.y = rect->y + (((int32_t)rect->h - height) / 2);
        pData->rect.w = width;
        pData->rect.h = height;
    }
    else {
        pData->rect.x = rect->x;
        pData->rect.y = rect->y;
        pData->rect.w = (int32_t)rect->w;
        pData->rect.h = (int32_t)rect->h;
    }

    (void)adjustDrawingRect_f(obj, target, &pData->rect);

    // Render to screen
    ret = renderTexture_f(obj, target, pData->text.texture, NULL, &pData->rect);

    // Destroy texture
    SDL_DestroyTexture(pData->text.texture);

exit:
    if (pData->text.font) {
        TTF_CloseFont(pData->text.font);
    }
    
    SDL_UnlockMutex(pData->lock);
    
    return ret;
}

/*!
 *
 */
static enum drawer_error_e setBgColor_f(struct drawer_s *obj, struct gfx_rect_s *rect,
                                        struct gfx_color_s *color, enum gfx_target_e target)
{
    ASSERT(obj && obj->pData);
    
    if (!color) {
        Loge("Bad arguments");
        return DRAWER_ERROR_PARAMS;
    }
    
    struct drawer_private_data_s *pData = (struct drawer_private_data_s*)obj->pData;
    enum drawer_error_e ret             = DRAWER_ERROR_DRAW;
    
    if (SDL_LockMutex(pData->lock) != 0) {
        Loge("Failed to lock mutex");
        return DRAWER_ERROR_LOCK;
    }

    // Get renderer
    SDL_Renderer *renderer = (target == GFX_TARGET_VIDEO ? pData->video.renderer
                                                         : pData->screen.renderer);
    if (!renderer) {
        Loge("Renderer not initialized yet for target %d", target);
        goto exit;
    }

    // Set color
    if (rect) {
        pData->rect.x = rect->x;
        pData->rect.y = rect->y;
        pData->rect.w = (int32_t)rect->w;
        pData->rect.h = (int32_t)rect->h;
    }

    (void)adjustDrawingRect_f(obj, target, &pData->rect);

    if (SDL_SetRenderDrawColor(renderer, color->red, color->green,
                                                       color->blue, color->alpha) < 0) {
        Loge("SDL_SetRenderDrawColor() failed - %s", SDL_GetError());
        goto exit;
    }
    
    if (SDL_RenderFillRect(renderer, rect ? &pData->rect : NULL) < 0) {
        Loge("SDL_RenderFillRect() failed - %s", SDL_GetError());
        goto exit;
    }

    // In fullscreen, unlike SDL 2.0.5, SDL 2.0.8 does not refresh screen if
    // RenderPresent() is called outside the main thread i.e thread that set
    // the video mode
    pData->screen.update = pData->screen.params.isFullScreen;

    SDL_RenderPresent(renderer);
    ret = DRAWER_ERROR_NONE;

exit:
    SDL_UnlockMutex(pData->lock);

    return ret;
}

/*!
 *
 */
static enum drawer_error_e restoreBgColor_f(struct drawer_s *obj, struct gfx_rect_s *rect,
                                            struct gfx_color_s *fallbackColor,
                                            enum gfx_target_e target)
{
    ASSERT(obj && obj->pData);
    
    if (!rect || !fallbackColor) {
        Loge("Invalid arguments");
        return DRAWER_ERROR_PARAMS;
    }

    struct drawer_private_data_s *pData = (struct drawer_private_data_s*)obj->pData;
    
    if (SDL_LockMutex(pData->lock) != 0) {
        Loge("Failed to lock mutex");
        return DRAWER_ERROR_LOCK;
    }

    enum drawer_error_e ret       = DRAWER_ERROR_DRAW;
    SDL_Renderer *renderer        = NULL;
    SDL_Texture *initialBgTexture = NULL;
    SDL_Rect srcRect              = {rect->x, rect->y, (int32_t)rect->w, (int32_t)rect->h};
    SDL_Rect dstRect              = {rect->x, rect->y, (int32_t)rect->w, (int32_t)rect->h};

    // Prepare parameters
    switch (target) {
        case GFX_TARGET_SCREEN:
            renderer         = pData->screen.renderer;
            initialBgTexture = pData->screen.initialBgTexture;
            break;

        case GFX_TARGET_VIDEO:
            renderer         = pData->video.renderer;
            initialBgTexture = pData->video.initialBgTexture;
            break;

        default:
            ;
    }

    // Only positions on screen and video area have to be adjusted
    // initialBgTexture is just a texture in which initial content is saved
    // there's no adjustDrwaingRect to call when extracting rect from it
    (void)adjustDrawingRect_f(obj, target, &dstRect);

    // Restore background color/image on given rect
    if (initialBgTexture) {
        Logd("Restore from saved texture");
        ret = renderTexture_f(obj, target, initialBgTexture, &srcRect, &dstRect);
    }

    if (ret != DRAWER_ERROR_NONE) {
        Logd("Restore from provided fallbackColor");
        if (SDL_SetRenderDrawColor(renderer, fallbackColor->red, fallbackColor->green,
                                   fallbackColor->blue, fallbackColor->alpha) < 0) {
            Loge("SDL_SetRenderDrawColor() failed - %s", SDL_GetError());
            goto exit;
        }
        
        if (SDL_RenderFillRect(renderer, &dstRect) < 0) {
            Loge("SDL_RenderFillRect() failed - %s", SDL_GetError());
            goto exit;
        }

        // In fullscreen, unlike SDL 2.0.5, SDL 2.0.8 does not refresh screen if
        // RenderPresent() is called outside the main thread i.e thread that set
        // the video mode
        pData->screen.update = pData->screen.params.isFullScreen;

        SDL_RenderPresent(renderer);
        ret = DRAWER_ERROR_NONE;
    }

exit:
    SDL_UnlockMutex(pData->lock);

    return ret;
}

/*!
 *
 */
static enum drawer_error_e startDrawingInBg_f(struct drawer_s *obj)
{
    ASSERT(obj && obj->pData);

    struct drawer_private_data_s *pData = (struct drawer_private_data_s*)obj->pData;
    enum drawer_error_e ret             = DRAWER_ERROR_DRAW;

    if (SDL_LockMutex(pData->lock) != 0) {
        Loge("Failed to lock mutex");
        return DRAWER_ERROR_LOCK;
    }

    // Create screen surface with the right pixel format
    SDL_Rect screenRect = {0, 0, (int32_t)pData->screen.params.rect.w,
                                 (int32_t)pData->screen.params.rect.h};

    ret = createSurfaceFromTarget_f(obj, GFX_TARGET_SCREEN, &pData->screen.surface, &screenRect);
    if (ret != DRAWER_ERROR_NONE) {
        goto unlockExit;
    }

    // Create screen texture to render to and update it with screen surface
    uint32_t windowFormat = SDL_GetWindowPixelFormat(pData->screen.window);
    pData->screen.texture = SDL_CreateTexture(pData->screen.renderer, windowFormat,
                                              SDL_TEXTUREACCESS_TARGET,
                                              screenRect.w, screenRect.h);
    if (!pData->screen.texture) {
        Loge("SDL_CreateTextureFromSurface() failed - %s", SDL_GetError());
        goto freeSurfaceExit;
    }

    SDL_SetTextureBlendMode(pData->screen.texture, SDL_BLENDMODE_BLEND);

    // Unfortunately, SDL_LockTexture() can't be used here because it expects a streaming
    // texture
    if (SDL_UpdateTexture(pData->screen.texture, NULL, pData->screen.surface->pixels,
                                                       pData->screen.surface->pitch) < 0) {
        Loge("SDL_UpdateTexture() failed - %s", SDL_GetError());
        SDL_DestroyTexture(pData->screen.texture);
        goto freeSurfaceExit;
    }

    // Ask SDL to start rendering to screen texture
    if (SDL_SetRenderTarget(pData->screen.renderer, pData->screen.texture) != 0) {
        Loge("SDL_SetRenderTarget() failed - %s", SDL_GetError());
        SDL_DestroyTexture(pData->screen.texture);
        goto freeSurfaceExit;
    }

    ret = DRAWER_ERROR_NONE;

freeSurfaceExit:
    SDL_FreeSurface(pData->screen.surface);

unlockExit:
    SDL_UnlockMutex(pData->lock);

    return ret;
}

/*!
 *
 */
static enum drawer_error_e stopDrawingInBg_f(struct drawer_s *obj)
{
    ASSERT(obj && obj->pData);

    struct drawer_private_data_s *pData = (struct drawer_private_data_s*)obj->pData;
    enum drawer_error_e ret             = DRAWER_ERROR_DRAW;

    if (SDL_LockMutex(pData->lock) != 0) {
        Loge("Failed to lock mutex");
        return DRAWER_ERROR_LOCK;
    }

    if (!pData->screen.texture) {
        Loge("Screen texture not initialized");
        goto unlockExit;
    }

    // Ask SDL to stop rendering to screen texture
    if (SDL_SetRenderTarget(pData->screen.renderer, NULL) != 0) {
        Loge("SDL_SetRenderTarget() failed - %s", SDL_GetError());
    }

    // Refresh screen
    ret = renderTexture_f(obj, GFX_TARGET_SCREEN, pData->screen.texture, NULL, NULL);

    // Destroy screen texture
    SDL_DestroyTexture(pData->screen.texture);

unlockExit:
    SDL_UnlockMutex(pData->lock);

    return ret;
}

/*!
 *
 */
static enum drawer_error_e saveBuffer_f(struct drawer_s *obj, struct buffer_s *buffer,
                                        struct gfx_image_s *inOut)
{
    ASSERT(obj && obj->pData && buffer && inOut);

    if (!buffer->data) {
        Loge("No data provided");
        return DRAWER_ERROR_PARAMS;
    }

    struct drawer_private_data_s *pData = (struct drawer_private_data_s*)(obj->pData);
    enum drawer_error_e ret             = DRAWER_ERROR_NONE;
    
    if (SDL_LockMutex(pData->lock) != 0) {
        Loge("Failed to lock mutex");
        return DRAWER_ERROR_LOCK;
    }

    if (pData->video.pixelFormat == SDL_PIXELFORMAT_ARGB8888) { // MJPEG format
        pData->video.rwops   = SDL_RWFromMem(buffer->data, (int32_t)buffer->length);
        pData->video.surface = IMG_Load_RW(pData->video.rwops, 0);

        Logd("Save video buffer to \"%s\"", inOut->path);
        ret = writeSurfaceToFile_f(obj, pData->video.surface, inOut);

        SDL_FreeSurface(pData->video.surface);
        SDL_RWclose(pData->video.rwops);
    }
    else {
        // TODO: Implementation for non-MJPEG format
        Logw("Non-MJPEG format not supported yet");
    }

    SDL_UnlockMutex(pData->lock);
    
    return ret;
}

/*!
 *
 */
static enum drawer_error_e saveTarget_f(struct drawer_s *obj, struct gfx_image_s *inOut,
                                        struct gfx_rect_s *srcRect, enum gfx_target_e target)
{
    ASSERT(obj && obj->pData && inOut);

    struct drawer_private_data_s *pData = (struct drawer_private_data_s*)(obj->pData);
    enum drawer_error_e ret             = DRAWER_ERROR_NONE;
    
    if (SDL_LockMutex(pData->lock) != 0) {
        Loge("Failed to lock mutex");
        return DRAWER_ERROR_LOCK;
    }

    Logd("Create surface from video content");
    // {0,0} to use the whole content when not in fullscreen
    SDL_Rect videoRect = {0, 0, (int32_t)pData->video.params.rect.w,
                                (int32_t)pData->video.params.rect.h};
    if (srcRect) {
        videoRect.x = srcRect->x;
        videoRect.y = srcRect->y;
        videoRect.w = (int32_t)srcRect->w;
        videoRect.h = (int32_t)srcRect->h;
    }

    (void)adjustDrawingRect_f(obj, target, &videoRect);

    ret = createSurfaceFromTarget_f(obj, GFX_TARGET_VIDEO,
                                    &pData->video.surface, &videoRect);
    if (ret != DRAWER_ERROR_NONE) {
        goto unlockExit;
    }

    if (target == GFX_TARGET_SCREEN) {
        Logd("Create surface from screen content");
        SDL_Rect screenRect = {pData->screen.params.rect.x,
                               pData->screen.params.rect.y,
                               (int32_t)pData->screen.params.rect.w,
                               (int32_t)pData->screen.params.rect.h};
        ret = createSurfaceFromTarget_f(obj, GFX_TARGET_SCREEN,
                                        &pData->screen.surface, &screenRect);
        if (ret != DRAWER_ERROR_NONE) {
            goto freeVideoSurfaceExit;
        }

        // In fullscreen mode, screen contains all gfx elements so no need to append
        // video content
        if (!pData->screen.params.isFullScreen) {
            Logd("Add video content to screen content");
            videoRect.x = pData->video.params.rect.x - SCREENSHOT_ADJUST_VIDEO_X;
            videoRect.y = pData->video.params.rect.y - SCREENSHOT_ADJUST_VIDEO_Y;
            SDL_BlitSurface(pData->video.surface, NULL, pData->screen.surface, &videoRect);
        }

        Logd("Save screen target to \"%s\"", inOut->path);
        ret = writeSurfaceToFile_f(obj, pData->screen.surface, inOut);

        SDL_FreeSurface(pData->screen.surface);
    }
    else {
        Logd("Save video target to \"%s\"", inOut->path);
        ret = writeSurfaceToFile_f(obj, pData->video.surface, inOut);
    }

freeVideoSurfaceExit:
    SDL_FreeSurface(pData->video.surface);

unlockExit:
    SDL_UnlockMutex(pData->lock);

    return ret;
}

/*!
 *
 */
static enum drawer_error_e getEvent_f(struct drawer_s *obj, struct gfx_event_s *gfxEvent)
{
    ASSERT(obj && obj->pData && gfxEvent);
    
    struct drawer_private_data_s *pData = (struct drawer_private_data_s*)(obj->pData);
    enum drawer_error_e ret             = DRAWER_ERROR_EVENT;
    int newEventReceived                = 0;
    
    gfxEvent->type = GFX_EVENT_TYPE_COUNT;

    // In fullscreen, unlike SDL 2.0.5, SDL 2.0.8 does not refresh screen if
    // RenderPresent() is called outside the main thread i.e thread that set
    // the video mode
    // As a consequence, video area is redrawn only after an event is received
    // and convertSdlEvent_f() below called because this latter ends with a call
    // to RenderPresent()
    //
    // getEvent_f() is called in the main thread so in fullscreen mode, RenderPresent()
    // could be called here. To do so, waiting for events should not be a blocking call
    // as SDL_WaitEvent() does
    //
    // => SDL_PollEvent() used in fullscreen mode otherwise SDL_WaitEvent() is used
    //
    // The main drawback of using PollEvent() and refreshing screen here might be an
    // excessive CPU consumption
    if (pData->screen.params.isFullScreen) {
        newEventReceived = SDL_PollEvent(&pData->event);
    }
    else {
        newEventReceived = SDL_WaitEvent(&pData->event);
    }

    if (newEventReceived) {
        Logd("Event received - type : %d", pData->event.type);
        ret =  convertSdlEvent_f(obj, &pData->event, gfxEvent);
    }

    if (pData->screen.update) {
        pData->screen.update = 0;
        SDL_RenderPresent(pData->screen.renderer);
    }

    return ret;
}

/*!
 *
 */
static enum drawer_error_e stopAwaitingEvent_f(struct drawer_s *obj)
{
    ASSERT(obj && obj->pData);

    SDL_Event userEvent;
    userEvent.type = SDL_USEREVENT;
    SDL_PushEvent(&userEvent);

    return DRAWER_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/* ///////////////////////////// PRIVATE FUNCTIONS IMPLEMENTATION ///////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

static int sdlEventFilter_f(void *userdata, SDL_Event* event)
{
    uint8_t addToEventqueue = 0;

    if (!event || !userdata) {
        Loge("Invalid parameters");
        return addToEventqueue;
    }

    struct drawer_s *obj = (struct drawer_s*)userdata;

    if (convertSdlEvent_f(obj, event, NULL) == DRAWER_ERROR_NONE) {
        addToEventqueue = 1;
    }

    Logd("Event type : %d added to internal event queue ? %d", event->type, addToEventqueue);

    return addToEventqueue;
}

/*!
 *
 */
static enum drawer_error_e convertSdlEvent_f(struct drawer_s *obj, SDL_Event *event,
                                             struct gfx_event_s *out)
{
    (void)obj;

    enum drawer_error_e ret = DRAWER_ERROR_NONE;
    const uint8_t *keyState = NULL;

    switch (event->type) {
	    case SDL_QUIT:
	        if (out) {
 	            out->type = GFX_EVENT_TYPE_QUIT;
 	        }
	        break;
	        
	    case SDL_MOUSEBUTTONUP:
	        if (event->button.button == SDL_BUTTON_LEFT) {
	            if (out) {
	                out->type   = GFX_EVENT_TYPE_CLICK;
	                out->rect.x = event->button.x;
	                out->rect.y = event->button.y;
	            }
	            break;
	        }
	        ret = DRAWER_ERROR_EVENT;
	        break;
	        
	    case SDL_KEYUP:
	        // This is required on SDL2.0.5 to avoid following issue:
	        //
	        // Tests showed that on long press (keyboard's button held), SDL continuously sends
	        // KeyDown/KeyUp/KeyDown/kKeyUp/... with repeat flag always equal to 0 instead of
	        // sending multiple successive KeyDown and one KeyUp
	        //
	        // As a consequence, when keeping a button pressed for a long moment then click on
	        // ESCAPE for example to stop application (while button is held or just after it is
	        // released), this latter will stay active until the internal event queue is empty
	        //
	        // Doing the following helps avoid this kind of behaviour
	        keyState = SDL_GetKeyboardState(NULL);
	        if (keyState && keyState[event->key.keysym.scancode]) {
	            Loge("Key \"%s\" still active", SDL_GetKeyName(event->key.keysym.sym));
	            ret = DRAWER_ERROR_EVENT;
	            break;
	        }

	        switch(event->key.keysym.sym) {
	            case SDLK_ESCAPE:
	                if (out) {
	                    out->type = GFX_EVENT_TYPE_ESC;
	                }
	                break;
	                
	            case SDLK_RETURN:
	                if (out) {
	                    out->type = GFX_EVENT_TYPE_ENTER;
	                }
	                break;
	                
	            case SDLK_LEFT:
	                if (out) {
	                    out->type = GFX_EVENT_TYPE_MOVE_LEFT;
	                }
	                break;
	                
	            case SDLK_UP:
	                if (out) {
	                    out->type = GFX_EVENT_TYPE_MOVE_UP;
	                }
	                break;
	                
	            case SDLK_RIGHT:
	                if (out) {
	                    out->type = GFX_EVENT_TYPE_MOVE_RIGHT;
	                }
	                break;
	                
	            case SDLK_DOWN:
	                if (out) {
	                    out->type = GFX_EVENT_TYPE_MOVE_DOWN;
	                }
	                break;
	                
	            default:
	                ret = DRAWER_ERROR_EVENT;
	        }
	        break;

        case SDL_USEREVENT:
            break;

	    default:
	        ret = DRAWER_ERROR_EVENT;
    }

    return ret;
}

/*!
 *
 */
static enum drawer_error_e initSdlComponents_f(struct drawer_s *obj, int32_t imgFlags)
{
    ASSERT(obj && obj->pData);
    
    struct drawer_private_data_s *pData = (struct drawer_private_data_s*)(obj->pData);

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        Loge("SDL_Init() failed : %s", SDL_GetError());
        return DRAWER_ERROR_INIT;
    }

    if ((IMG_Init(imgFlags) & imgFlags) != imgFlags) {
        Loge("IMG_Init() failed to load required modules : %s", IMG_GetError());
        goto IMGInitExit;
    }

    if (TTF_Init() != 0) {
        Loge("TTF_Init() failed - %s", TTF_GetError());
        goto TTFInitExit;
    }

    if (!pData->screen.params.showCursor) {
        SDL_ShowCursor(SDL_DISABLE);
    }

    return DRAWER_ERROR_NONE;

TTFInitExit:
    IMG_Quit();

IMGInitExit:
    SDL_Quit();
    return DRAWER_ERROR_INIT;
}

/*!
 *
 */
static enum drawer_error_e uninitSdlComponents_f(struct drawer_s *obj)
{
    ASSERT(obj && obj->pData);

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    
    return DRAWER_ERROR_NONE;
}

/*!
 *
 */
static enum drawer_error_e initWindowAndRenderer_f(struct drawer_s *obj, enum gfx_target_e target,
                                                   struct gfx_rect_s *pos, uint8_t showWindow)
{
    ASSERT(obj && obj->pData && pos);
    
    struct drawer_private_data_s *pData = (struct drawer_private_data_s*)(obj->pData);
    SDL_Window **window                 = NULL;
    SDL_Renderer **renderer             = NULL;
    uint32_t windowFlags                = showWindow ? SDL_WINDOW_SHOWN : SDL_WINDOW_HIDDEN;
    const char *windowCaption           = "";
    SDL_Rect windowPos                  = {0};
    uint8_t windowHasInputFocus         = 0;
    uint32_t *windowID                  = NULL;
    struct gfx_rect_s *windowRect       = NULL;

    // Prepare parameters
    switch (target) {
        case GFX_TARGET_SCREEN:
            window   = &pData->screen.window;
            renderer = &pData->screen.renderer;

            if (!pData->screen.params.isTitleBarUsed) {
                windowFlags  |= SDL_WINDOW_BORDERLESS;
                windowCaption = pData->screen.params.caption;
            }

            if (pData->screen.params.isFullScreen) {
                windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
                windowPos.x  = SDL_WINDOWPOS_UNDEFINED;
                windowPos.y  = SDL_WINDOWPOS_UNDEFINED;
            }
            else {
                windowPos.x = pos->x;;
                windowPos.y = pos->y;;
            }
            windowPos.w = (int32_t)pos->w;
            windowPos.h = (int32_t)pos->h;

            windowHasInputFocus = 1;
            windowID            = &pData->screen.windowID;
            windowRect          = &pData->screen.params.rect;
            break;

        case GFX_TARGET_VIDEO:
            window       = &pData->video.window;
            renderer     = &pData->video.renderer;

            // In fullscreen mode, video is rendered to the same window as screen
            // A dedicated window for video would be hidden behind screen window
            if (pData->screen.params.isFullScreen) {
                Logd("Using the same window/renderer as screen for video");
                *window   = pData->screen.window;
                *renderer = pData->screen.renderer;

                // A workaround on SDL2.0.5 to draw video window at the specified area
                pData->video.params.rect.x -= SCREENSHOT_ADJUST_VIDEO_X;
                pData->video.params.rect.y -= SCREENSHOT_ADJUST_VIDEO_Y;
                goto exit;
            }

            windowFlags |= SDL_WINDOW_BORDERLESS;

            windowPos.x = pos->x;
            windowPos.y = pos->y;
            windowPos.w = (int32_t)pos->w;
            windowPos.h = (int32_t)pos->h;
            
            windowID   = &pData->video.windowID;
            windowRect = &pData->video.params.rect;
            break;

        default:
            ;
    }

    // Create window
    *window = SDL_CreateWindow(windowCaption, windowPos.x, windowPos.y,
                               windowPos.w, windowPos.h, windowFlags);
    if (!(*window)) {
        Loge("SDL_CreateWindow() failed : %s", SDL_GetError());
        return DRAWER_ERROR_INIT;
    }

    if (windowHasInputFocus) {
        // This is useful to keep screen window displayed
        // and make it the main receiver of input events
        SDL_RaiseWindow(*window);
    }

    // Create renderer
    // FIXME : Make renderer work with HW acceleration (SDL_RENDERER_ACCELERATED)
    //         The issue is that the UI becomes weird once one starts playing with
    //         some buttons.
    //
    //         Once fixed, replace
    //             uint32_t rendererFlags  = SDL_RENDERER_TARGETTEXTURE|SDL_RENDERER_SOFTWARE;
    //         with
    //             uint32_t rendererFlags  = SDL_RENDERER_TARGETTEXTURE;
    SDL_RendererInfo renderDriverInfo;
    uint32_t rendererFlags  = SDL_RENDERER_TARGETTEXTURE|SDL_RENDERER_SOFTWARE;
    int32_t nbRenderDrivers = SDL_GetNumRenderDrivers(), index = 0;
    while (index < nbRenderDrivers) {
        if (SDL_GetRenderDriverInfo(index, &renderDriverInfo) == 0) {
            if (((renderDriverInfo.flags & rendererFlags) == rendererFlags)
                && ((renderDriverInfo.flags & SDL_RENDERER_ACCELERATED) == SDL_RENDERER_ACCELERATED)) {
                Logd("Using render driver with HW acceleration: %s", renderDriverInfo.name);
                rendererFlags |= SDL_RENDERER_ACCELERATED;
                break;
            }
        }
        ++index;
    }

    if (index == nbRenderDrivers) {
        Logd("Asking SDL to use the first render driver supporting software fallback");
        rendererFlags |= SDL_RENDERER_SOFTWARE;
        index = -1;
    }

    *renderer = SDL_CreateRenderer(*window, index, rendererFlags);
    if (!(*renderer)) {
        Loge("SDL_CreateRenderer() failed : %s", SDL_GetError());
        return DRAWER_ERROR_INIT;
    }

    // Allow alpha blending on renderer
    SDL_SetRenderDrawBlendMode(*renderer, SDL_BLENDMODE_BLEND);

    // Set window ID
    *windowID = SDL_GetWindowID(*window);

    // Reconfigure window if necessary
    // This is required in fullscreen mode before drawing on it otherwise the result
    // might be weird (Some gfx elements zoomed out while other aren't, ...)
    if (windowFlags & SDL_WINDOW_FULLSCREEN_DESKTOP) {
        SDL_Event event;
        if (waitForWindowEvent_f(obj, target, SDL_WINDOWEVENT_SIZE_CHANGED,
                                 &event) != DRAWER_ERROR_NONE) {
            Loge("Failed to enable fullscreen mode");
            return DRAWER_ERROR_INIT;
        }

        // Set new window size
        windowRect->w = (uint32_t)event.window.data1;
        windowRect->h = (uint32_t)event.window.data2;

        SDL_Rect viewport = {windowRect->x, windowRect->y, (int32_t)windowRect->w, (int32_t)windowRect->h};
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
        SDL_RenderSetLogicalSize(*renderer, (int32_t)windowRect->w, (int32_t)windowRect->h);
        SDL_RenderSetViewport(*renderer, &viewport);

        Logd("Window's size reconfigured to %d x %d", windowRect->w, windowRect->h);
    }

exit:
    return DRAWER_ERROR_NONE;
}

/*!
 *
 */
static enum drawer_error_e uninitWindowAndRenderer_f(struct drawer_s *obj,
                                                     enum gfx_target_e target)
{
    ASSERT(obj && obj->pData);
    
    struct drawer_private_data_s *pData = (struct drawer_private_data_s*)(obj->pData);
    SDL_Window *window                  = NULL;
    SDL_Renderer *renderer              = NULL;
    SDL_Texture *initialBgTexture       = NULL;

    // Prepare parameters
    switch (target) {
        case GFX_TARGET_SCREEN:
            window           = pData->screen.window;
            renderer         = pData->screen.renderer;
            initialBgTexture = pData->screen.initialBgTexture;
            break;

        case GFX_TARGET_VIDEO:
            window           = pData->video.window;
            renderer         = pData->video.renderer;
            initialBgTexture = pData->video.initialBgTexture;
            break;

        default:
            ;
    }

    // Release resources
    if (initialBgTexture) {
        SDL_DestroyTexture(initialBgTexture);    
    }

    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }

    if (window) {
        SDL_DestroyWindow(window);
    }

    return DRAWER_ERROR_NONE;
}

/*!
 *
 */
static enum drawer_error_e initVideoContext_f(struct drawer_s *obj, struct gfx_rect_s *rect,
                                              uint8_t showWindow)
{
    ASSERT(obj && obj->pData && rect);

    (void)showWindow;

    struct drawer_private_data_s *pData = (struct drawer_private_data_s*)(obj->pData);
    enum drawer_error_e ret             = DRAWER_ERROR_NONE;

    if (initWindowAndRenderer_f(obj, GFX_TARGET_VIDEO, rect, 1) != DRAWER_ERROR_NONE) {
        Loge("Failed to init video window and renderer");
        return DRAWER_ERROR_INIT;
    }

    // Set pixel format according to video format
    switch (pData->video.params.pixelFormat) {
        case GFX_PIXEL_FORMAT_MJPEG:
            Logd("Video format : GFX_PIXEL_FORMAT_MJPEG");
            pData->video.pixelFormat = SDL_PIXELFORMAT_ARGB8888;
            break;

        case GFX_PIXEL_FORMAT_YVYU:
            Logd("Video format : GFX_PIXEL_FORMAT_YVYU");
            pData->video.pixelFormat = SDL_PIXELFORMAT_YVYU;

            // Init video texture
            pData->video.texture = SDL_CreateTexture(pData->video.renderer, pData->video.pixelFormat,
                                                     SDL_TEXTUREACCESS_STREAMING,
                                                     (int32_t)rect->w, (int32_t)rect->h);
            if (!pData->video.texture) {
                Loge("Failed to create video texture - %s", SDL_GetError());
                return DRAWER_ERROR_INIT;
            }

            // Allow alpha blending
            SDL_SetTextureBlendMode(pData->video.texture, SDL_BLENDMODE_BLEND);
            break;

        default:
            Loge("Unknown video format");
            ret = DRAWER_ERROR_INIT;
    }

    return ret;
}

/*!
 *
 */
static enum drawer_error_e uninitVideoContext_f(struct drawer_s *obj)
{
    ASSERT(obj && obj->pData);
    
    struct drawer_private_data_s *pData = (struct drawer_private_data_s*)(obj->pData);

    if (pData->video.texture) {
        SDL_DestroyTexture(pData->video.texture);
    }

    (void)uninitWindowAndRenderer_f(obj, GFX_TARGET_VIDEO);

    return DRAWER_ERROR_NONE;
}

/*!
 *
 */
static enum drawer_error_e setInitialBackground_f(struct drawer_s *obj, enum gfx_target_e target)
{
    ASSERT(obj && obj->pData);
    
    struct drawer_private_data_s *pData = (struct drawer_private_data_s*)(obj->pData);
    struct drawer_screen_s *screen      = &pData->screen;
    struct drawer_video_s *video        = &pData->video;
    uint8_t windowIconNeeded            = 0;
    uint8_t isBgImageUsed               = 0;
    struct gfx_image_s *image           = NULL;
    struct gfx_color_s *color           = NULL;
    struct gfx_image_s *icon            = NULL;
    struct gfx_rect_s *rect             = NULL;
    SDL_Window *window                  = NULL;
    SDL_Renderer *renderer              = NULL;
    SDL_Texture **initialBgTexture      = NULL;

    // Prepare parameters
    switch (target) {
        case GFX_TARGET_SCREEN:
            windowIconNeeded = 1;
            isBgImageUsed    = screen->params.isBgImageUsed;
            image            = &screen->params.background.image;
            color            = &screen->params.background.color;
            icon             = &screen->params.icon;
            rect             = &screen->params.rect;
            window           = screen->window;
            renderer         = screen->renderer;
            initialBgTexture = &screen->initialBgTexture;
            break;

        case GFX_TARGET_VIDEO:
            isBgImageUsed    = video->params.isBgImageUsed;
            image            = &video->params.background.image;
            color            = &video->params.background.color;
            rect             = &video->params.rect;
            window           = video->window;
            renderer         = video->renderer;
            initialBgTexture = &video->initialBgTexture;
            break;

        default:
            return DRAWER_ERROR_PARAMS;;
    }

    // Fill in background
    if (!isBgImageUsed) {
        if (setBgColor_f(obj, rect, color, target) != DRAWER_ERROR_NONE) {
            Loge("Setting background color has failed");
            return DRAWER_ERROR_DRAW;
        }
    }
    else {
        if (drawImage_f(obj, rect, image, target) != DRAWER_ERROR_NONE) {
            Loge("Setting background image has failed");
            return DRAWER_ERROR_DRAW;
        }
    }

    // Save initial background state - useful when restoring color/image of an area
    // If we fail creating texture, colorOnReset (See Graphics.xml) will be used
    SDL_Surface *surface = NULL;
    SDL_Rect sdlRect     = {0, 0, (int32_t)rect->w, (int32_t)rect->h};

    (void)adjustDrawingRect_f(obj, target, &sdlRect);

    if (createSurfaceFromTarget_f(obj, target, &surface, &sdlRect) == DRAWER_ERROR_NONE) {
        *initialBgTexture = SDL_CreateTextureFromSurface(renderer, surface);
        if (!(*initialBgTexture)) {
            Loge("Failed to create initial background texture - %s", SDL_GetError());
        }
        SDL_FreeSurface(surface);
    }

    // Set window icon
    if (windowIconNeeded) {
        (void)setWindowIcon_f(obj, window, icon);
    }

    return DRAWER_ERROR_NONE;
}

/*!
 *
 */
static enum drawer_error_e setWindowIcon_f(struct drawer_s *obj, SDL_Window *window,
                                           struct gfx_image_s *icon)
{
    ASSERT(obj && obj->pData);
    
    if (!window || !icon) {
        Loge("Invalid parameters");
        return DRAWER_ERROR_PARAMS;
    }

    if (access(icon->path, F_OK) == 0) {
        Logd("Loading icon : \"%s\"", icon->path);

        SDL_Surface *surface = IMG_Load(icon->path);
        if (!surface) {
            Loge("Failed to load image \"%s\"", icon->path);
            return DRAWER_ERROR_DRAW;
        }
        
        if (icon->hiddenColor) {
            SDL_SetColorKey(surface,
                SDL_TRUE,
                SDL_MapRGBA(surface->format,
                    icon->hiddenColor->red,
                    icon->hiddenColor->green,
                    icon->hiddenColor->blue,
                    icon->hiddenColor->alpha
                )
            );
        }
        
        SDL_SetWindowIcon(window, surface);
        SDL_FreeSurface(surface);
    }

    return DRAWER_ERROR_NONE;
}

/*!
 *
 */
static enum drawer_error_e waitForWindowEvent_f(struct drawer_s *obj, enum gfx_target_e target,
                                                SDL_WindowEventID eventID, SDL_Event *inOut)
{
    ASSERT(obj && obj->pData);
    
    if (!inOut) {
        Loge("Invalid parameters");
        return DRAWER_ERROR_PARAMS;
    }

    struct drawer_private_data_s *pData = (struct drawer_private_data_s*)(obj->pData);
    uint32_t windowID                   = 0;
    uint8_t eventReceived               = 0;

    // Get windowID
    if (target == GFX_TARGET_VIDEO) {
        Logd("Waiting for event %d on video window", eventID);
        windowID = pData->video.windowID;
    }
    else {
        Logd("Waiting for event %d on screen window", eventID);
        windowID = pData->screen.windowID;
    }

    // Start waiting
    do {
        if (!SDL_WaitEventTimeout(inOut, WAIT_TIME_5S)) {
            Loge("SDL_WaitEventTimeout() failed - %s", SDL_GetError());
            return DRAWER_ERROR_TIMEOUT;
        }

        if ((inOut->type == SDL_WINDOWEVENT) && (inOut->window.windowID == windowID)
            && (inOut->window.event == eventID)) {
            Logd("Requested window event received");
            eventReceived = 1;
        }
    }
    while (!eventReceived);

    return DRAWER_ERROR_NONE;
}

/*!
 *
 */
static enum drawer_error_e setTextureTransparency_f(struct drawer_s *obj, SDL_Texture *texture,
                                                    uint8_t alpha)
{
    ASSERT(obj && obj->pData);

    if (!texture) {
        Loge("A valid texture is expected");
        return DRAWER_ERROR_PARAMS;
    }

    SDL_BlendMode currentBlendMode;
    if (SDL_GetTextureBlendMode(texture, &currentBlendMode) != 0) {
        Loge("SDL_GetTextureBlendMode() failed - %s", SDL_GetError());
        return DRAWER_ERROR_BLEND;
    }

    if (currentBlendMode != SDL_BLENDMODE_BLEND) {
        if (SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND) != 0) {
            Loge("SDL_SetTextureBlendMode() failed - %s", SDL_GetError());
            return DRAWER_ERROR_BLEND;
        }
    }

    uint8_t currentAlplha;
    if (SDL_GetTextureAlphaMod(texture, &currentAlplha) != 0) {
        Loge("SDL_GetTextureAlphaMod() failed - %s", SDL_GetError());
        return DRAWER_ERROR_BLEND;
    }

    if (currentAlplha != alpha) {
        if (SDL_SetTextureAlphaMod(texture, alpha) != 0) {
            Loge("SDL_SetTextureAlphaMod() failed - %s", SDL_GetError());
            return DRAWER_ERROR_BLEND;
        }
    }

    return DRAWER_ERROR_NONE;
}

/*!
 *
 */
static enum drawer_error_e renderTexture_f(struct drawer_s *obj, enum gfx_target_e target,
                                            SDL_Texture *texture, SDL_Rect *srcRect,
                                            SDL_Rect *dstRect)
{
    ASSERT(obj && obj->pData);
    
    struct drawer_private_data_s *pData = (struct drawer_private_data_s*)(obj->pData);

    if (!texture) {
        Loge("A valid texture is expected");
        return DRAWER_ERROR_PARAMS;
    }

    SDL_Renderer *renderer = (target == GFX_TARGET_VIDEO ? pData->video.renderer
                                                         : pData->screen.renderer);
    if (!renderer) {
        Loge("Renderer not initialized for target : %d", target);
        return DRAWER_ERROR_DRAW;
    }

    if (SDL_RenderCopy(renderer, texture, srcRect, dstRect) < 0) {
        Loge("SDL_RenderCopy() failed - %s", SDL_GetError());
        return DRAWER_ERROR_DRAW;
    }

    // In fullscreen, unlike SDL 2.0.5, SDL 2.0.8 does not refresh screen if
    // RenderPresent() is called outside the main thread i.e thread that set
    // the video mode
    pData->screen.update = pData->screen.params.isFullScreen;

    SDL_RenderPresent(renderer);

    return DRAWER_ERROR_NONE;
}

/*!
 *
 */
static enum drawer_error_e createSurfaceFromTarget_f(struct drawer_s *obj, enum gfx_target_e target,
                                                     SDL_Surface **surface, SDL_Rect *rect)
{
    ASSERT(obj && obj->pData);
    
    struct drawer_private_data_s *pData = (struct drawer_private_data_s*)(obj->pData);

    if (!surface || !rect) {
        Loge("Invalid parameters");
        return DRAWER_ERROR_PARAMS;
    }

    enum drawer_error_e ret = DRAWER_ERROR_NONE;
    SDL_Window *window      = NULL;
    SDL_Renderer *renderer  = NULL;

    // Prepare parameters
    switch (target) {
        case GFX_TARGET_SCREEN:
            window   = pData->screen.window;
            renderer = pData->screen.renderer;
            break;

        case GFX_TARGET_VIDEO:
            window   = pData->video.window;
            renderer = pData->video.renderer;
            break;

        default:
            ;
    }

    // Create surface with the right pixel format
    uint32_t windowFormat = SDL_GetWindowPixelFormat(window);

    if (windowFormat == SDL_PIXELFORMAT_UNKNOWN) {
        Loge("SDL_GetWindowPixelFormat() failed - %s", SDL_GetError());
        ret = DRAWER_ERROR_SAVE;
        goto exit;
    }

    *surface = SDL_CreateRGBSurfaceWithFormat(0, rect->w, rect->h,
                                              SDL_BITSPERPIXEL(windowFormat), windowFormat);
    if (!(*surface)) {
        Loge("SDL_CreateRGBSurfaceWithFormat() failed - %s", SDL_GetError());
        ret = DRAWER_ERROR_SAVE;
        goto exit;
    }

    // Fill in surface with the currently displayed content
    int readPixelsRetval = SDL_RenderReadPixels(renderer, rect, windowFormat,
                                                (*surface)->pixels, (*surface)->pitch);
    if (readPixelsRetval != 0) {
        Loge("SDL_RenderReadPixels() failed - %s", SDL_GetError());
        SDL_FreeSurface(*surface);
        ret = DRAWER_ERROR_SAVE;
    }

exit:
    return ret;
}

/*!
 *
 */
static enum drawer_error_e writeSurfaceToFile_f(struct drawer_s *obj, SDL_Surface *surface,
                                                struct gfx_image_s *inOut)
{
    ASSERT(obj && obj->pData);

    if (!surface || !inOut) {
        Loge("Invalid parameters");
        return DRAWER_ERROR_PARAMS;
    }

    switch (inOut->format) {
        case GFX_IMAGE_FORMAT_BMP:
            SDL_SaveBMP(surface, inOut->path);
            break;

        case GFX_IMAGE_FORMAT_PNG:
            IMG_SavePNG(surface, inOut->path);
            break;

        case GFX_IMAGE_FORMAT_JPG:
            IMG_SaveJPG(surface, inOut->path, 100);
            break;

        default:
            ;
    }

    return DRAWER_ERROR_NONE;
}

static enum drawer_error_e adjustDrawingRect_f(struct drawer_s *obj, enum gfx_target_e target,
                                               SDL_Rect *inOut)
{
    ASSERT(obj && obj->pData);
    
    struct drawer_private_data_s *pData = (struct drawer_private_data_s*)(obj->pData);

    if (!inOut) {
        Loge("Invalid parameters");
        return DRAWER_ERROR_PARAMS;
    }

    // In Graphics.xml, gfx elements sharing the same group as video have
    // video window {x; y} coordinates as reference point.
    //
    // In fullscreen mode, video is rendered to the same window as screen
    // Thus, those gfx elements' top-left pixel have to be re-positioned
    // with the top-left corner of the screen as their new reference point
    if (target != GFX_TARGET_VIDEO) {
        Logd("AdjustDrawingRect only needed for gfx elements inside video area");
        return DRAWER_ERROR_NONE;
    }

    uint8_t isGfxVideoArea = ((inOut->x == pData->video.params.rect.x)
                             && (inOut->y == pData->video.params.rect.y)
                             && (inOut->w == (int32_t)pData->video.params.rect.w)
                             && (inOut->h == (int32_t)pData->video.params.rect.h));

    if (pData->screen.params.isFullScreen) {
        if (isGfxVideoArea) {
            Logd("Video is already at the right position in fullscreen");
            return DRAWER_ERROR_NONE;
        }
        inOut->x += pData->video.params.rect.x;
        inOut->y += pData->video.params.rect.y;
    }
    else if (isGfxVideoArea) {
        // Not in fullscreen so a dedicated window is created for video streaming
        // Reference point is at {0; 0} of GfxVideoArea
        inOut->x = 0;
        inOut->y = 0;
    }

    return DRAWER_ERROR_NONE;
}
