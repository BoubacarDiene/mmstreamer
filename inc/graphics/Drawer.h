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
* \file Drawer.h
* \author Boubacar DIENE
*/

#ifndef __DRAWER_H__
#define __DRAWER_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// HEADERS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#include "utils/Common.h"
#include "graphics/GfxCommon.h"

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////// TYPES DECLARATION ///////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum drawer_error_e;

struct drawer_s;

/* -------------------------------------------------------------------------------------------- */
/* ///////////////////////////////////// PUBLIC FUNCTIONS ///////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

typedef enum drawer_error_e (*drawer_init_screen_f)(struct drawer_s *obj,
                                                    struct gfx_screen_s *screenParams);
typedef enum drawer_error_e (*drawer_uninit_screen_f)(struct drawer_s *obj);

typedef enum drawer_error_e (*drawer_draw_video_f)(struct drawer_s *obj, struct gfx_rect_s *rect,
                                                   struct buffer_s *buffer);
typedef enum drawer_error_e (*drawer_draw_image_f)(struct drawer_s *obj, struct gfx_rect_s *rect,
                                                   struct gfx_image_s *image);
typedef enum drawer_error_e (*drawer_draw_text_f)(struct drawer_s *obj, struct gfx_rect_s *rect,
                                                  struct gfx_text_s *text);

typedef enum drawer_error_e (*drawer_set_bgcolor_f)(struct drawer_s *obj, struct gfx_rect_s *rect,
                                                    struct gfx_color_s *color);

typedef enum drawer_error_e (*drawer_save_buffer_f)(struct drawer_s *obj, struct buffer_s *buffer,
                                                    struct gfx_image_s *inOut);
typedef enum drawer_error_e (*drawer_save_screen_f)(struct drawer_s *obj,
                                                    struct gfx_image_s *inOut);

typedef enum drawer_error_e (*drawer_get_event_f)(struct drawer_s *obj,
                                                  struct gfx_event_s *gfxEvent);
typedef enum drawer_error_e (*drawer_stop_awaiting_event_f)(struct drawer_s *obj);

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// TYPES /////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum drawer_error_e {
    DRAWER_ERROR_NONE,
    DRAWER_ERROR_INIT,
    DRAWER_ERROR_UNINIT,
    DRAWER_ERROR_PARAMS,
    DRAWER_ERROR_EVENT,
    DRAWER_ERROR_DRAW,
    DRAWER_ERROR_LOCK,
    DRAWER_ERROR_SAVE
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// MAIN CONTEXT /////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

struct drawer_s {
    drawer_init_screen_f         initScreen;
    drawer_uninit_screen_f       uninitScreen;
    
    drawer_draw_video_f          drawVideo;
    drawer_draw_image_f          drawImage;
    drawer_draw_text_f           drawText;
    
    drawer_set_bgcolor_f         setBgColor;
    
    drawer_save_buffer_f         saveBuffer;
    drawer_save_screen_f         saveScreen;
    
    drawer_get_event_f           getEvent;
    drawer_stop_awaiting_event_f stopAwaitingEvent;
    
    void *pData;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// INITIALIZER //////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum drawer_error_e Drawer_Init(struct drawer_s **obj);
enum drawer_error_e Drawer_UnInit(struct drawer_s **obj);

#ifdef __cplusplus
}
#endif

#endif //__DRAWER_H__
