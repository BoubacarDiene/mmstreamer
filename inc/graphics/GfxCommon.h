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
* \file GfxCommon.h
* \author Boubacar DIENE
*/

#ifndef __GFX_COMMON_H__
#define __GFX_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// HEADERS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#include "utils/Common.h"

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// TYPES /////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum gfx_target_e {
    GFX_TARGET_SCREEN,
    GFX_TARGET_VIDEO
};

enum gfx_element_type_e {
    GFX_ELEMENT_TYPE_VIDEO,
    GFX_ELEMENT_TYPE_IMAGE,
    GFX_ELEMENT_TYPE_TEXT
};

enum gfx_pixel_format_e {
    GFX_PIXEL_FORMAT_MJPEG,
    GFX_PIXEL_FORMAT_YVYU
};

enum gfx_image_format_e {
    GFX_IMAGE_FORMAT_BMP,
    GFX_IMAGE_FORMAT_PNG,
    GFX_IMAGE_FORMAT_JPG
};

enum gfx_event_type_e {
    GFX_EVENT_TYPE_MOVE_LEFT,
    GFX_EVENT_TYPE_MOVE_UP,
    GFX_EVENT_TYPE_MOVE_RIGHT,
    GFX_EVENT_TYPE_MOVE_DOWN,
    GFX_EVENT_TYPE_CLICK,
    GFX_EVENT_TYPE_ENTER,
    GFX_EVENT_TYPE_FOCUS,
    GFX_EVENT_TYPE_ESC,
    GFX_EVENT_TYPE_QUIT,

    GFX_EVENT_TYPE_COUNT
};

struct gfx_color_s {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t alpha;
};

struct gfx_rect_s {
    int32_t  x;
    int32_t  y;
    uint32_t w;
    uint32_t h;
};

struct gfx_nav_s {
    char left[MAX_NAME_SIZE];
    char up[MAX_NAME_SIZE];
    char right[MAX_NAME_SIZE];
    char down[MAX_NAME_SIZE];
};

struct gfx_event_s {
    enum gfx_event_type_e type;
    struct gfx_rect_s     rect;
    char                  *gfxElementName;
    void                  *gfxElementPData;
};

struct gfx_text_s {
    char               str[MAX_STR_SIZE];
    char               ttfFont[MAX_PATH_SIZE];
    uint8_t            ttfFontSize;
    struct gfx_color_s color;
    
};

struct gfx_image_s {
    char                    path[MAX_PATH_SIZE];
    enum gfx_image_format_e format;
    struct gfx_color_s      *hiddenColor; // NULL if nothing in image is transparent
};

struct gfx_video_s {
    char                    name[MAX_NAME_SIZE];

    struct gfx_rect_s       rect;
    enum gfx_pixel_format_e pixelFormat;

    uint8_t                 isBgImageUsed; // 0 => color used / 1 => image used
    union {
        struct gfx_color_s  color;
        struct gfx_image_s  image;
    } background;
};

struct gfx_element_reserved_s;
struct gfx_element_s {
    char                          name[MAX_NAME_SIZE];
    char                          groupName[MAX_NAME_SIZE];
    
    uint8_t                       redrawGroup;
    
    enum gfx_element_type_e       type;
    
    union {
        struct buffer_s           buffer;
        struct gfx_image_s        image;
        struct gfx_text_s         text;
    } data;
    
    struct gfx_rect_s             rect;
    
    uint8_t                       isVisible;
    uint8_t                       isClickable;
    uint8_t                       isFocusable;
    uint8_t                       hasFocus;
    
    struct gfx_nav_s              nav;

    struct gfx_element_reserved_s *reserved;
    
    void                          *pData;
};

struct gfx_screen_s {
    char                    name[MAX_NAME_SIZE];

    struct gfx_rect_s       rect;
    struct gfx_video_s      video;

    char                    fbDeviceName[MAX_NAME_SIZE]; // Framebuffer device name
    uint8_t                 bitsPerPixel;
    
    uint8_t                 isFullScreen;
    uint8_t                 showCursor;
    
    uint8_t                 isTitleBarUsed; // 1 => Show / 0 => Hide (i.e caption and icon not used)
    char                    caption[MAX_STR_SIZE];
    struct gfx_image_s      icon;
    
    uint8_t                 isBgImageUsed; // 0 => color used / 1 => image used
    union {
        struct gfx_color_s  color;
        struct gfx_image_s  image;
    } background;
};

#ifdef __cplusplus
}
#endif

#endif //__GFX_COMMON_H__
