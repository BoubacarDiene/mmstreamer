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
* \file Graphics.h
* \author Boubacar DIENE
*/

#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

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

enum graphics_error_e;

struct graphics_params_s;
struct graphics_s;

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////////// CALLBACKS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

typedef void (*graphics_on_gfx_event_cb)(struct gfx_event_s *gfxEvent, void *userData);

/* -------------------------------------------------------------------------------------------- */
/* ///////////////////////////////////// PUBLIC FUNCTIONS ///////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

typedef enum graphics_error_e (*graphics_create_drawer_f)(struct graphics_s *obj,
                                                          struct graphics_params_s *params);
typedef enum graphics_error_e (*graphics_destroy_drawer_f)(struct graphics_s *obj);

typedef enum graphics_error_e (*graphics_create_element_f)(struct graphics_s *obj,
                                                           struct gfx_element_s **newGfxElement);
typedef enum graphics_error_e (*graphics_push_element_f)(struct graphics_s *obj,
                                                         struct gfx_element_s *gfxElement);
typedef enum graphics_error_e (*graphics_remove_element_f)(struct graphics_s *obj,
                                                           char *gfxElementName);
typedef enum graphics_error_e (*graphics_remove_all_f)(struct graphics_s *obj);

typedef enum graphics_error_e (*graphics_set_visible_f)(struct graphics_s *obj,
                                                        char *gfxElementName, uint8_t isVisible);
typedef enum graphics_error_e (*graphics_set_focus_f)(struct graphics_s *obj,
                                                      char *gfxElementName);
typedef enum graphics_error_e (*graphics_set_clickable_f)(struct graphics_s *obj,
                                                          char *gfxElementName,
                                                          uint8_t isClickable);
typedef enum graphics_error_e (*graphics_set_nav_f)(struct graphics_s *obj, char *gfxElementName,
                                                    struct gfx_nav_s *nav);
typedef enum graphics_error_e (*graphics_set_data_f)(struct graphics_s *obj, char *gfxElementName,
                                                     void *data);

typedef enum graphics_error_e (*graphics_save_video_frame_f)(struct graphics_s *obj,
                                                             struct buffer_s *buffer,
                                                             struct gfx_image_s *inOut);
typedef enum graphics_error_e (*graphics_save_video_element_f)(struct graphics_s *obj,
                                                               char *gfxElementName,
                                                               struct gfx_image_s *inOut);
typedef enum graphics_error_e (*graphics_take_screenshot_f)(struct graphics_s *obj,
                                                            struct gfx_image_s *inOut);

typedef enum graphics_error_e (*graphics_draw_all_elements_f)(struct graphics_s *obj);

typedef enum graphics_error_e (*graphics_simulate_gfx_event_f)(struct graphics_s *obj,
                                                               struct gfx_event_s *gfxEvent);
typedef enum graphics_error_e (*graphics_handle_gfx_events_f)(struct graphics_s *obj);

typedef enum graphics_error_e (*graphics_quit_f)(struct graphics_s *obj);

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// TYPES /////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum graphics_error_e {
    GRAPHICS_ERROR_NONE,
    GRAPHICS_ERROR_INIT,
    GRAPHICS_ERROR_UNINIT,
    GRAPHICS_ERROR_LOCK,
    GRAPHICS_ERROR_LIST,
    GRAPHICS_ERROR_DRAWER,
    GRAPHICS_ERROR_PARAMS
};

struct graphics_params_s {
    struct gfx_screen_s      screenParams;
    
    struct gfx_color_s       colorOnFocus;
    struct gfx_color_s       colorOnBlur;
    struct gfx_color_s       colorOnReset;
    
    graphics_on_gfx_event_cb onGfxEventCb;
    
    void                     *userData;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// MAIN CONTEXT /////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

struct graphics_s {
    graphics_create_drawer_f      createDrawer;
    graphics_destroy_drawer_f     destroyDrawer;
    
    graphics_create_element_f     createElement;
    graphics_push_element_f       pushElement;
    graphics_remove_element_f     removeElement;
    graphics_remove_all_f         removeAll;
    
    graphics_set_visible_f        setVisible;
    graphics_set_focus_f          setFocus;
    graphics_set_clickable_f      setClickable;
    graphics_set_nav_f            setNav;
    graphics_set_data_f           setData;
    
    graphics_save_video_frame_f   saveVideoFrame;
    graphics_save_video_element_f saveVideoElement;
    graphics_take_screenshot_f    takeScreenshot;
    
    graphics_draw_all_elements_f  drawAllElements;

    graphics_simulate_gfx_event_f simulateGfxEvent;
    graphics_handle_gfx_events_f  handleGfxEvents;
    
    graphics_quit_f               quit;
    
    void *pData;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// INITIALIZER //////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum graphics_error_e Graphics_Init(struct graphics_s **obj);
enum graphics_error_e Graphics_UnInit(struct graphics_s **obj);

#ifdef __cplusplus
}
#endif

#endif //__GRAPHICS_H__
