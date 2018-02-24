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
* \file Video.h
* \author Boubacar DIENE
*/

#ifndef __VIDEO_H__
#define __VIDEO_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// HEADERS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#include "utils/Common.h"
#include "video/V4l2.h"

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////// TYPES DECLARATION ///////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum video_error_e;
enum video_await_mode_e;

struct video_buffer_s;
struct video_listener_s;
struct video_area_s;
struct video_params_s;
struct video_s;

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////////// CALLBACKS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

typedef void (*video_on_buffer_available_cb)(struct video_buffer_s *videoBuffer, void *userData);

/* -------------------------------------------------------------------------------------------- */
/* ///////////////////////////////////// PUBLIC FUNCTIONS ///////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

typedef enum video_error_e (*video_register_listener_f)(struct video_s *obj,
                                                        struct video_params_s *params,
                                                        struct video_listener_s *listener);
typedef enum video_error_e (*video_unregister_listener_f)(struct video_s *obj,
                                                          struct video_params_s *params,
                                                          struct video_listener_s *listener);

typedef enum video_error_e (*video_get_final_area_f)(struct video_s *obj,
                                                     struct video_params_s *params,
                                                     struct video_area_s *videoArea);
typedef enum video_error_e (*video_get_max_buffer_size_f)(struct video_s *obj,
                                                          struct video_params_s *params,
                                                          size_t *size);

typedef enum video_error_e (*video_start_device_capture_f)(struct video_s *obj,
                                                           struct video_params_s *params);
typedef enum video_error_e (*video_stop_device_capture_f)(struct video_s *obj,
                                                          struct video_params_s *params);

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// TYPES /////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum video_error_e {
    VIDEO_ERROR_NONE,
    VIDEO_ERROR_INIT,
    VIDEO_ERROR_UNINIT,
    VIDEO_ERROR_LOCK,
    VIDEO_ERROR_LIST,
    VIDEO_ERROR_START,
    VIDEO_ERROR_STOP,
    VIDEO_ERROR_PARAMS
};

enum video_await_mode_e {
    VIDEO_AWAIT_MODE_BLOCKING,
    VIDEO_AWAIT_MODE_NON_BLOCKING
};

struct video_buffer_s {
    uint32_t index;
    uint32_t offset;
    void     *data;
    size_t   length;
};

struct video_listener_s {
    char                         name[MAX_NAME_SIZE];

    video_on_buffer_available_cb onVideoBufferAvailableCb;
    void                         *userData;
};

struct video_area_s {
    uint32_t left;
    uint32_t top;
    uint32_t width;
    uint32_t height;
};

struct video_params_s {
    char                    name[MAX_NAME_SIZE];

    /* Open device */
    char                    path[MAX_PATH_SIZE];
    uint32_t                caps;
    
    /* Configure device */
    enum v4l2_buf_type      type;
    uint32_t                pixelformat;
    enum v4l2_colorspace    colorspace;
    
    enum priority_e         priority;
    uint32_t                desiredFps;
    
    struct video_area_s     captureArea;
    struct video_area_s     croppingArea;
    struct video_area_s     composingArea;
    
    /* Request buffers */
    uint32_t                count;
    enum v4l2_memory        memory;
    
    /* Await data */
    enum video_await_mode_e awaitMode;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// MAIN CONTEXT /////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

struct video_s {
    video_register_listener_f    registerListener;
    video_unregister_listener_f  unregisterListener;

    video_get_final_area_f       getFinalVideoArea;
    video_get_max_buffer_size_f  getMaxBufferSize;

    video_start_device_capture_f startDeviceCapture;
    video_stop_device_capture_f  stopDeviceCapture;

    struct video_buffer_s buffer;
    void                  *pData;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// INITIALIZER //////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum video_error_e Video_Init(struct video_s **obj);
enum video_error_e Video_UnInit(struct video_s **obj);

#ifdef __cplusplus
}
#endif

#endif //__VIDEO_H__
