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
* \file   GfxCommon.h
* \author Boubacar DIENE
*/

#ifndef __GFX_COMMON_H__
#define __GFX_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include "utils/Common.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                            */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

typedef enum   GFX_ELEMENT_TYPE_E  GFX_ELEMENT_TYPE_E;
typedef enum   GFX_VIDEO_FORMAT_E  GFX_VIDEO_FORMAT_E;
typedef enum   GFX_EVENT_TYPE_E    GFX_EVENT_TYPE_E;
typedef enum   GFX_IMAGE_FORMAT_E  GFX_IMAGE_FORMAT_E;

typedef struct GFX_COLOR_S         GFX_COLOR_S;
typedef struct GFX_TEXT_S          GFX_TEXT_S;
typedef struct GFX_IMAGE_S         GFX_IMAGE_S;
typedef struct GFX_RECT_S          GFX_RECT_S;
typedef struct GFX_SCREEN_S        GFX_SCREEN_S;
typedef struct GFX_ELEMENT_S       GFX_ELEMENT_S;
typedef struct GFX_EVENT_S         GFX_EVENT_S;

enum GFX_ELEMENT_TYPE_E {
    GFX_ELEMENT_TYPE_VIDEO,
    GFX_ELEMENT_TYPE_IMAGE,
    GFX_ELEMENT_TYPE_TEXT
};

enum GFX_VIDEO_FORMAT_E {
    GFX_VIDEO_FORMAT_MJPEG,
    GFX_VIDEO_FORMAT_YVYU
};

enum GFX_EVENT_TYPE_E {
    GFX_EVENT_TYPE_QUIT,
    GFX_EVENT_TYPE_ESC,
    GFX_EVENT_TYPE_FOCUS,
    GFX_EVENT_TYPE_CLICK,
    GFX_EVENT_TYPE_ENTER,
    GFX_EVENT_TYPE_MOVE_LEFT,
    GFX_EVENT_TYPE_MOVE_UP,
    GFX_EVENT_TYPE_MOVE_RIGHT,
    GFX_EVENT_TYPE_MOVE_DOWN,
    GFX_EVENT_TYPE_COUNT
};

enum GFX_IMAGE_FORMAT_E {
    GFX_IMAGE_FORMAT_BMP,
    GFX_IMAGE_FORMAT_PNG,
    GFX_IMAGE_FORMAT_JPG
};

struct GFX_COLOR_S {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

struct GFX_TEXT_S {
    char        str[MAX_STR_SIZE];
    char        ttfFont[MAX_PATH_SIZE];
    uint8_t     ttfFontSize;
    GFX_COLOR_S color;
    
};

struct GFX_IMAGE_S {
    char               path[MAX_PATH_SIZE];
    GFX_IMAGE_FORMAT_E format;
    GFX_COLOR_S        *hiddenColor; // NULL if nothing in image is transparent
};

struct GFX_RECT_S {
    uint16_t x;
    uint16_t y;
    uint16_t w;
    uint16_t h;
};

struct GFX_SCREEN_S {
    GFX_RECT_S         rect;
    
    uint8_t            bitsPerPixel;
    
    uint8_t            isFullScreen;
    
    uint8_t            isTitleBarUsed;        // 1 => Show / 0 => Hide (so caption and icon not used)
    char               caption[MAX_STR_SIZE];
    GFX_IMAGE_S        icon;
    
    uint8_t            isBgImageUsed;         // 0 => color used / 1 => image used
    union {
        GFX_COLOR_S    color;
        GFX_IMAGE_S    image;
    } background;
    
    GFX_VIDEO_FORMAT_E videoFormat;
};

struct GFX_ELEMENT_S {
    char               name[MAX_NAME_SIZE];
    char               groupName[MAX_NAME_SIZE];
    
    uint8_t            redrawGroup;
    
    GFX_ELEMENT_TYPE_E type;
    
    union {
        BUFFER_S       buffer;
        GFX_IMAGE_S    image;
        GFX_TEXT_S     text;
    } data;
    
    GFX_RECT_S         rect;
    
    uint8_t            isVisible;
    uint8_t            isClickable;
    uint8_t            isFocusable;
    uint8_t            hasFocus;
    
    char               navLeft[MAX_NAME_SIZE];
    char               navUp[MAX_NAME_SIZE];
    char               navRight[MAX_NAME_SIZE];
    char               navDown[MAX_NAME_SIZE];

    uint8_t            surfaceUpdated; //reserved
    
    void               *pData;
};

struct GFX_EVENT_S {
    GFX_EVENT_TYPE_E type;
    GFX_RECT_S       rect;
    char             *gfxElementName;
    void             *gfxElementPData;
};

#ifdef __cplusplus
}
#endif

#endif //__GFX_COMMON_H__
