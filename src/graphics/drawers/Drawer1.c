//////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                              //
//              Copyright © 2016, 2017 Boubacar DIENE                                           //
//                                                                                              //
//              This file is part of mmstreamer project.                                        //
//                                                                                              //
//              mmstreamer is free software: you can redistribute it and/or modify              //
//              it under the terms of the GNU General Public License as published by            //
//              the Free Software Foundation, either version 3 of the License, or               //
//              (at your option) any later version.                                             //
//                                                                                              //
//              mmstreamer is distributed in the hope that it will be useful,                   //
//              but WITHOUT ANY WARRANTY; without even the implied warranty of                  //
//              MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                   //
//              GNU General Public License for more details.                                    //
//                                                                                              //
//              You should have received a copy of the GNU General Public License               //
//              along with mmstreamer.  If not, see <http://www.gnu.org/licenses/>.             //
//                                                                                              //
//////////////////////////////////////////////////////////////////////////////////////////////////

/*!
* \file   Drawer1.c
* \brief  Graphics elements drawer based on SDLv1
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include <fcntl.h>
#include <sys/types.h>

#include "SDL/SDL.h"
#include "SDL/SDL_events.h"
#include "SDL/SDL_image.h"
#include "SDL/SDL_ttf.h"

#include "utils/Log.h"

#include "graphics/Drawer.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                            */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "DRAWER"

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

typedef struct DRAWER_PRIVATE_DATA_S {
    GFX_SCREEN_S      screenParams;
    
    int32_t           videoFmt;
    
    SDL_Rect          rect;
    
    SDL_Surface       *screen;
    SDL_RWops         *rwops;
    union {
        SDL_Surface   *mjpeg;
        SDL_Overlay   *overlay;
    } video;
    SDL_Surface       *image;
    SDL_Surface       *text;
    
    SDL_Color         textColor;
    TTF_Font          *textFont;
    
    SDL_Event         event;
    
    SDL_mutex         *lock;
} DRAWER_PRIVATE_DATA_S;

/* -------------------------------------------------------------------------------------------- */
/*                                          VARIABLES                                           */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                         PROTOTYPES                                           */
/* -------------------------------------------------------------------------------------------- */

static DRAWER_ERROR_E initScreen_f  (DRAWER_S *obj, GFX_SCREEN_S *screenParams);
static DRAWER_ERROR_E unInitScreen_f(DRAWER_S *obj);

static DRAWER_ERROR_E drawVideo_f(DRAWER_S *obj, GFX_RECT_S *rect, BUFFER_S *buffer);
static DRAWER_ERROR_E drawImage_f(DRAWER_S *obj, GFX_RECT_S *rect, GFX_IMAGE_S *image);
static DRAWER_ERROR_E drawText_f (DRAWER_S *obj, GFX_RECT_S *rect, GFX_TEXT_S *text);

static DRAWER_ERROR_E setBgColor_f(DRAWER_S *obj, GFX_RECT_S *rect, GFX_COLOR_S *color);

static DRAWER_ERROR_E saveBuffer_f(DRAWER_S *obj, BUFFER_S *buffer, GFX_IMAGE_S *inOut);
static DRAWER_ERROR_E saveScreen_f(DRAWER_S *obj, GFX_IMAGE_S *inOut);

static DRAWER_ERROR_E getEvent_f(DRAWER_S *obj, GFX_EVENT_S *gfxEvent);

/* -------------------------------------------------------------------------------------------- */
/*                                      PUBLIC FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
DRAWER_ERROR_E Drawer_Init(DRAWER_S **obj)
{
    assert(obj && (*obj = calloc(1, sizeof(DRAWER_S))));
    
    DRAWER_PRIVATE_DATA_S *pData;
    assert((pData = calloc(1, sizeof(DRAWER_PRIVATE_DATA_S))));
    
    (*obj)->initScreen   = initScreen_f;
    (*obj)->unInitScreen = unInitScreen_f;
    
    (*obj)->drawVideo    = drawVideo_f;
    (*obj)->drawImage    = drawImage_f;
    (*obj)->drawText     = drawText_f;
    
    (*obj)->setBgColor   = setBgColor_f;
    
    (*obj)->saveBuffer   = saveBuffer_f;
    (*obj)->saveScreen   = saveScreen_f;
    
    (*obj)->getEvent     = getEvent_f;
    
    pData->videoFmt = -1;
    
    assert((pData->lock = SDL_CreateMutex()));
    
    (*obj)->pData = (void*)pData;
    
    return DRAWER_ERROR_NONE;
}

/*!
 *
 */
DRAWER_ERROR_E Drawer_UnInit(DRAWER_S **obj)
{
    assert(obj && *obj && (*obj)->pData);
    
    DRAWER_PRIVATE_DATA_S *pData = (DRAWER_PRIVATE_DATA_S*)((*obj)->pData);
    
    SDL_DestroyMutex(pData->lock);
    
    free(pData);
    pData = NULL;
    
    free(*obj);
    *obj = NULL;
    
    return DRAWER_ERROR_NONE;
}	

/* -------------------------------------------------------------------------------------------- */
/*                                     PRIVATE FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static DRAWER_ERROR_E initScreen_f(DRAWER_S *obj, GFX_SCREEN_S *screenParams)
{
    assert(obj && obj->pData && screenParams);
    
    DRAWER_PRIVATE_DATA_S *pData = (DRAWER_PRIVATE_DATA_S*)(obj->pData);
    
    memcpy(&pData->screenParams, screenParams, sizeof(GFX_SCREEN_S));
    
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        Loge("SDL_Init() failed : %s", SDL_GetError());
        return DRAWER_ERROR_INIT;
    }

    if (!screenParams->showCursor) {
        SDL_ShowCursor(SDL_DISABLE);
    }

    uint32_t imgFlags = IMG_INIT_JPG;
    
    if ((IMG_Init(imgFlags) & imgFlags) != imgFlags) {
        Loge("IMG_Init() failed to load required modules : %s", IMG_GetError());
        SDL_Quit();
        return DRAWER_ERROR_INIT;
    }

    if (TTF_Init() != 0) {
        Loge("TTF_Init() failed - %s", TTF_GetError());
        IMG_Quit();
        SDL_Quit();
        return DRAWER_ERROR_INIT;
    }

    uint32_t videoFlags = SDL_HWSURFACE | SDL_DOUBLEBUF;
    
    if (screenParams->isFullScreen) {
        videoFlags |= SDL_FULLSCREEN;
    }
    
    if (!screenParams->isTitleBarUsed) {
        videoFlags |= SDL_NOFRAME;
    }
    
    if (access(screenParams->icon.path, F_OK) == 0) {
        if (screenParams->icon.format != GFX_IMAGE_FORMAT_BMP) {
            Loge("Only .bmp is supported");
            goto exit;
        }
        
        Logd("Loading icon : \"%s\"", screenParams->icon.path);
        //pData->image = IMG_LOAD(screenParams->icon.path);
        pData->image = SDL_LoadBMP(screenParams->icon.path);

        if (!pData->image) {
            Loge("Failed to load image \"%s\"", screenParams->icon.path);
            goto exit;
        }
        
        if (screenParams->icon.hiddenColor) {
            SDL_SetColorKey(pData->image, SDL_SRCCOLORKEY,
                            SDL_MapRGB(pData->image->format,
                                        screenParams->icon.hiddenColor->red,
                                        screenParams->icon.hiddenColor->green,
                                        screenParams->icon.hiddenColor->blue)
                            );
        }
        
        SDL_WM_SetIcon(pData->image, NULL);
        SDL_FreeSurface(pData->image);
    }

    pData->screen = SDL_SetVideoMode(
                    videoFlags & SDL_FULLSCREEN ? 0 : screenParams->rect.w,
                    videoFlags & SDL_FULLSCREEN ? 0 : screenParams->rect.h,
                    screenParams->bitsPerPixel,
                    videoFlags
                );
    
    if (!pData->screen) {
        Loge("SDL_SetVideoMode() failed : %s", SDL_GetError());
        goto exit;
    }
    
    // Write window's title - " " to avoid "Untitled window"
    SDL_WM_SetCaption(screenParams->isTitleBarUsed ? screenParams->caption : " ", NULL);
    
    // Set screen background
    if (!screenParams->isBgImageUsed) {
        if (setBgColor_f(obj, &screenParams->rect, &screenParams->background.color) != DRAWER_ERROR_NONE) {
            Loge("Setting screen's background color has failed");
            goto exit;
        }
    }
    else {
        if (drawImage_f(obj, &screenParams->rect, &screenParams->background.image) != DRAWER_ERROR_NONE) {
            Loge("Setting screen's background image has failed");
            goto exit;
        }
    }
    
    switch (screenParams->videoFormat) {
        case GFX_VIDEO_FORMAT_MJPEG:
            Logd("Video format : GFX_VIDEO_FORMAT_MJPEG");
            break;
            
        case GFX_VIDEO_FORMAT_YVYU:
            Logd("Video format : GFX_VIDEO_FORMAT_YVYU");
            pData->videoFmt = SDL_YVYU_OVERLAY;
            break;
            
        default:
            Loge("Unknown video format");
            goto exit;
    }
    
    return DRAWER_ERROR_NONE;
    
exit:
    unInitScreen_f(obj);
    
    return DRAWER_ERROR_DRAW;
}

/*!
 *
 */
static DRAWER_ERROR_E unInitScreen_f(DRAWER_S *obj)
{
    assert(obj && obj->pData);
    
    DRAWER_PRIVATE_DATA_S *pData = (DRAWER_PRIVATE_DATA_S*)(obj->pData);
    
    if (pData->video.overlay) {
        SDL_FreeYUVOverlay(pData->video.overlay);
    }
    
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    
    return DRAWER_ERROR_NONE;
}

/*!
 *
 */
static DRAWER_ERROR_E drawVideo_f(DRAWER_S *obj, GFX_RECT_S *rect, BUFFER_S *buffer)
{
    assert(obj && obj->pData);
    
    if (!rect || !buffer || !buffer->data) {
        //Loge("Bad arguments --> Probably initializing screen");
        return DRAWER_ERROR_PARAMS;
    }
    
    DRAWER_PRIVATE_DATA_S *pData = (DRAWER_PRIVATE_DATA_S*)(obj->pData);
    
    if (SDL_LockMutex(pData->lock) != 0) {
        Loge("Failed to lock mutex");
        return DRAWER_ERROR_LOCK;
    }

    pData->rect.x = rect->x;
    pData->rect.y = rect->y;
    pData->rect.w = rect->w;
    pData->rect.h = rect->h;
    
    if ((pData->videoFmt != -1)
        && !pData->video.overlay
        && !(pData->video.overlay = SDL_CreateYUVOverlay(pData->rect.w, pData->rect.h, pData->videoFmt, pData->screen))) {
        Loge("Failed to create yuv overlay");
        SDL_UnlockMutex(pData->lock);
        return DRAWER_ERROR_DRAW;
    }
    
    if (pData->video.overlay) {
        memcpy(pData->video.overlay->pixels[0], buffer->data, buffer->length);
        SDL_DisplayYUVOverlay(pData->video.overlay, &pData->rect);
    }
    else {
        pData->rwops       = SDL_RWFromMem(buffer->data, buffer->length);
        pData->video.mjpeg = IMG_Load_RW(pData->rwops, 0);

        SDL_BlitSurface(pData->video.mjpeg, NULL, pData->screen, &pData->rect);
        
        SDL_FreeSurface(pData->video.mjpeg);
        SDL_RWclose(pData->rwops);

        pData->video.mjpeg = NULL;
    }
    
    SDL_Flip(pData->screen);

    SDL_UnlockMutex(pData->lock);
    
    return DRAWER_ERROR_NONE;
}

/*!
 *
 */
static DRAWER_ERROR_E drawImage_f(DRAWER_S *obj, GFX_RECT_S *rect, GFX_IMAGE_S *image)
{
    assert(obj && obj->pData);
    
    if (!rect || !image) {
        Loge("Bad arguments");
        return DRAWER_ERROR_PARAMS;
    }
    
    if (access(image->path, F_OK) != 0) {
        Loge("File \"%s\" not found", image->path);
        return DRAWER_ERROR_DRAW;
    }
    
    if (image->format != GFX_IMAGE_FORMAT_BMP) {
        Loge("Only .bmp files are supported");
        return DRAWER_ERROR_DRAW;
    }
    
    DRAWER_ERROR_E           ret = DRAWER_ERROR_NONE;
    DRAWER_PRIVATE_DATA_S *pData = (DRAWER_PRIVATE_DATA_S*)(obj->pData);
    
    if (SDL_LockMutex(pData->lock) != 0) {
        Loge("Failed to lock mutex");
        return DRAWER_ERROR_LOCK;
    }
    
    //pData->image = IMG_LOAD(image->path);
    pData->image = SDL_LoadBMP(image->path);
    
    if (!pData->image) {
        Loge("Failed to load image \"%s\"", image->path);
        ret = DRAWER_ERROR_DRAW;
        goto exit;
    }
    
    Logd("Real size of image \"%s\": %u x %u", image->path, pData->image->w, pData->image->h);
    
    if (image->hiddenColor) {
        SDL_SetColorKey(pData->image,
                        SDL_SRCCOLORKEY,
                        SDL_MapRGB(pData->image->format,
                                    image->hiddenColor->red,
                                    image->hiddenColor->green,
                                    image->hiddenColor->blue)
                       );
    }
    
    if ((pData->image->w < (int8_t)rect->w) && (pData->image->h < (int8_t)rect->h)) {
        Logd("Centering image \"%s\" inside element", image->path);
        pData->rect.x = rect->x + ((rect->w - pData->image->w) / 2);
        pData->rect.y = rect->y + ((rect->h - pData->image->h) / 2);
        pData->rect.w = pData->image->w;
        pData->rect.h = pData->image->h;
    }
    else {
        pData->rect.x = rect->x;
        pData->rect.y = rect->y;
        pData->rect.w = rect->w;
        pData->rect.h = rect->h;
    }
    
    //SDL_SetAlpha(pData->image, SDL_SRCALPHA, 50);
    SDL_BlitSurface(pData->image, NULL, pData->screen, &pData->rect);
    
    SDL_FreeSurface(pData->image);
    
    SDL_Flip(pData->screen);
    
exit:
    SDL_UnlockMutex(pData->lock);
    
    return ret;
}

/*!
 *
 */
static DRAWER_ERROR_E drawText_f(DRAWER_S *obj, GFX_RECT_S *rect, GFX_TEXT_S *text)
{
    assert(obj && obj->pData);
    
    if (!rect || !text) {
        Loge("Bad arguments");
        return DRAWER_ERROR_PARAMS;
    }
    
    DRAWER_ERROR_E           ret = DRAWER_ERROR_NONE;
    DRAWER_PRIVATE_DATA_S *pData = (DRAWER_PRIVATE_DATA_S*)(obj->pData);
    
    if (access(text->ttfFont, F_OK) != 0) {
        Loge("Font file \"%s\" not found", text->ttfFont);
        return DRAWER_ERROR_PARAMS;
    }
    
    if (SDL_LockMutex(pData->lock) != 0) {
        Loge("Failed to lock mutex");
        return DRAWER_ERROR_LOCK;
    }
    
    if (!(pData->textFont = TTF_OpenFont(text->ttfFont, text->ttfFontSize))) {
        Loge("Failed to open font file \"%s\" - %s", text->ttfFont, TTF_GetError());
        ret = DRAWER_ERROR_DRAW;
        goto exit;
    }
    
    pData->textColor.r = text->color.red;
    pData->textColor.g = text->color.green;
    pData->textColor.b = text->color.blue;
    
    if (!(pData->text = TTF_RenderText_Solid(pData->textFont, text->str, pData->textColor))) {
        Loge("Failed to render text - %s", TTF_GetError());
        ret = DRAWER_ERROR_DRAW;
        goto exit;
    }

    Logd("Real size of text \"%s\": %u x %u", text->str, pData->text->w, pData->text->h);

    if ((pData->text->w < (int8_t)rect->w) && (pData->text->h < (int8_t)rect->h)) {
        Logd("Centering text \"%s\" inside element", text->str);
        pData->rect.x = rect->x + ((rect->w - pData->text->w) / 2);
        pData->rect.y = rect->y + ((rect->h - pData->text->h) / 2);
        pData->rect.w = pData->text->w;
        pData->rect.h = pData->text->h;
    }
    else {
        pData->rect.x = rect->x;
        pData->rect.y = rect->y;
        pData->rect.w = rect->w;
        pData->rect.h = rect->h;
    }

    SDL_BlitSurface(pData->text, NULL, pData->screen, &pData->rect);

    SDL_FreeSurface(pData->text);

    SDL_Flip(pData->screen);
    
exit:
    if (pData->textFont) {
        TTF_CloseFont(pData->textFont);
        pData->textFont = NULL;
    }
    
    SDL_UnlockMutex(pData->lock);
    
    return ret;
}

/*!
 *
 */
static DRAWER_ERROR_E setBgColor_f(DRAWER_S *obj, GFX_RECT_S *rect, GFX_COLOR_S *color)
{
    assert(obj && obj->pData);
    
    if (!rect || !color) {
        Loge("Bad arguments");
        return DRAWER_ERROR_PARAMS;
    }
    
    DRAWER_PRIVATE_DATA_S *pData = (DRAWER_PRIVATE_DATA_S*)(obj->pData);
    
    if (SDL_LockMutex(pData->lock) != 0) {
        Loge("Failed to lock mutex");
        return DRAWER_ERROR_LOCK;
    }
    
    pData->rect.x = rect->x;
    pData->rect.y = rect->y;
    pData->rect.w = rect->w;
    pData->rect.h = rect->h;
    
    SDL_FillRect(pData->screen, &pData->rect, SDL_MapRGB(pData->screen->format, color->red, color->green, color->blue));
    
    SDL_Flip(pData->screen);
    
    SDL_UnlockMutex(pData->lock);
    
    return DRAWER_ERROR_NONE;
}

/*!
 *
 */
static DRAWER_ERROR_E saveBuffer_f(DRAWER_S *obj, BUFFER_S *buffer, GFX_IMAGE_S *inOut)
{
    assert(obj && obj->pData && buffer && inOut);

    if (!buffer->data) {
        Loge("No data provided");
        return DRAWER_ERROR_PARAMS;
    }

    DRAWER_PRIVATE_DATA_S *pData = (DRAWER_PRIVATE_DATA_S*)(obj->pData);
    DRAWER_ERROR_E ret           = DRAWER_ERROR_SAVE;
    int32_t jpeg                 = -1;
    
    if (SDL_LockMutex(pData->lock) != 0) {
        Loge("Failed to lock mutex");
        return DRAWER_ERROR_LOCK;
    }
    
    if (inOut->format == GFX_IMAGE_FORMAT_BMP) {
        if (!(pData->rwops = SDL_RWFromMem(buffer->data, buffer->length))) {
            Loge("SDL_RWFromMem() failed - %s", IMG_GetError());
            goto exit;
        }
        if (!(pData->image = IMG_Load_RW(pData->rwops, 0))) {
            Loge("IMG_Load_RW() failed - %s", IMG_GetError());
            goto exit;
        }
    }

    Logd("Saving buffer to \"%s\"", inOut->path);

    switch (inOut->format) {
        case GFX_IMAGE_FORMAT_PNG:
            Logw("PNG not supported yet");
            break;
            
        case GFX_IMAGE_FORMAT_BMP:
            SDL_SaveBMP(pData->image, inOut->path);
            break;
            
        case GFX_IMAGE_FORMAT_JPG:
            if((jpeg = open(inOut->path, O_WRONLY | O_CREAT, 0660)) < 0){
                Loge("Failed to create %s", inOut->path);
            }
            else {
                if (write(jpeg, buffer->data, buffer->length) < 0) {
                    Loge("Failed create jpeg file");
                }
                close(jpeg);
            }
            break;
            
        default:
            ;
    }
    
    ret = DRAWER_ERROR_NONE;

exit:    
    if (inOut->format == GFX_IMAGE_FORMAT_BMP) {
        if (pData->image) {
            SDL_FreeSurface(pData->image);
        }
        if (pData->rwops) {
            SDL_RWclose(pData->rwops);
        }
    }
    
    SDL_UnlockMutex(pData->lock);
    
    return ret;
}

/*!
 *
 */
static DRAWER_ERROR_E saveScreen_f(DRAWER_S *obj, GFX_IMAGE_S *inOut)
{
    assert(obj && obj->pData && inOut);

    DRAWER_PRIVATE_DATA_S *pData = (DRAWER_PRIVATE_DATA_S*)(obj->pData);

    if (SDL_LockMutex(pData->lock) != 0) {
        Loge("Failed to lock mutex");
        return DRAWER_ERROR_LOCK;
    }

    Logd("Saving screenshot to \"%s\"", inOut->path);

    switch (inOut->format) {
        case GFX_IMAGE_FORMAT_PNG:
            Logw("PNG not supported yet");
            break;

        case GFX_IMAGE_FORMAT_BMP:
            SDL_SaveBMP(pData->screen, inOut->path);
            break;

        case GFX_IMAGE_FORMAT_JPG:
            Logw("JPEG not supported yet");
            break;

        default:
            ;
    }

    SDL_UnlockMutex(pData->lock);

    return DRAWER_ERROR_NONE;
}

/*!
 *
 */
static DRAWER_ERROR_E getEvent_f(DRAWER_S *obj, GFX_EVENT_S *gfxEvent)
{
    assert(obj && obj->pData && gfxEvent);
    
    DRAWER_PRIVATE_DATA_S *pData = (DRAWER_PRIVATE_DATA_S*)(obj->pData);
    
    gfxEvent->type = GFX_EVENT_TYPE_COUNT;
    
	SDL_WaitEvent(&pData->event);
	switch (pData->event.type) {
	    case SDL_QUIT:
 	        gfxEvent->type = GFX_EVENT_TYPE_QUIT;
	        break;
	        
	    case SDL_MOUSEBUTTONUP:
	        if (pData->event.button.button == SDL_BUTTON_LEFT) {
	            gfxEvent->type   = GFX_EVENT_TYPE_CLICK;
	            gfxEvent->rect.x = pData->event.button.x;
	            gfxEvent->rect.y = pData->event.button.y;
	        }
	        break;
	        
	    case SDL_KEYUP:
	        switch(pData->event.key.keysym.sym) {
	            case SDLK_ESCAPE:
	                gfxEvent->type = GFX_EVENT_TYPE_ESC;
	                break;
	                
	            case SDLK_RETURN:
	                gfxEvent->type = GFX_EVENT_TYPE_ENTER;
	                break;
	                
	            case SDLK_LEFT:
	                gfxEvent->type = GFX_EVENT_TYPE_MOVE_LEFT;
	                break;
	                
	            case SDLK_UP:
	                gfxEvent->type = GFX_EVENT_TYPE_MOVE_UP;
	                break;
	                
	            case SDLK_RIGHT:
	                gfxEvent->type = GFX_EVENT_TYPE_MOVE_RIGHT;
	                break;
	                
	            case SDLK_DOWN:
	                gfxEvent->type = GFX_EVENT_TYPE_MOVE_DOWN;
	                break;
	                
	            default:
	                ;
	        }
	        break;
	        
	    default:
	        ;
    }
    
    return ((gfxEvent->type != GFX_EVENT_TYPE_COUNT) ? DRAWER_ERROR_NONE : DRAWER_ERROR_EVENT);
}
