//////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                              //
//              Copyright © 2016, 2018 Boubacar DIENE                                           //
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
* \file V4l2.h
* \author Boubacar DIENE
*/

#ifndef __V4L2_H__
#define __V4L2_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// HEADERS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#include <linux/videodev2.h>

#include "utils/Common.h"

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////// TYPES DECLARATION ///////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum v4l2_error_e;
enum v4l2_pipe_e;

struct v4l2_open_device_params_s;
struct v4l2_configure_device_params_s;
struct v4l2_selection_params_s;
struct v4l2_request_buffers_params_s;
struct v4l2_mapping_buffer_s;
struct v4l2_s;

/* -------------------------------------------------------------------------------------------- */
/* ///////////////////////////////////// PUBLIC FUNCTIONS ///////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

typedef enum v4l2_error_e (*v4l2_open_device_f)(struct v4l2_s *obj,
                                                struct v4l2_open_device_params_s *params);
typedef enum v4l2_error_e (*v4l2_close_device_f)(struct v4l2_s *obj);

typedef enum v4l2_error_e (*v4l2_configure_device_f)(struct v4l2_s *obj,
                                                     struct v4l2_configure_device_params_s *params);
typedef enum v4l2_error_e (*v4l2_set_cropping_area_f)(struct v4l2_s *obj,
                                                      struct v4l2_selection_params_s *cropRectInOut);
typedef enum v4l2_error_e (*v4l2_set_composing_area_f)(struct v4l2_s *obj,
                                                       struct v4l2_selection_params_s *composeRectInOut);

typedef enum v4l2_error_e (*v4l2_request_buffers_f)(struct v4l2_s *obj,
                                                    struct v4l2_request_buffers_params_s *params);
typedef enum v4l2_error_e (*v4l2_release_buffers_f)(struct v4l2_s *obj);

typedef enum v4l2_error_e (*v4l2_start_capture_f)(struct v4l2_s *obj);
typedef enum v4l2_error_e (*v4l2_stop_capture_f)(struct v4l2_s *obj);

typedef enum v4l2_error_e (*v4l2_await_data_f)(struct v4l2_s *obj, int32_t timeout_ms);
typedef enum v4l2_error_e (*v4l2_stop_awaiting_data_f)(struct v4l2_s *obj);

typedef enum v4l2_error_e (*v4l2_queue_buffer_f)(struct v4l2_s *obj, uint32_t index);
typedef enum v4l2_error_e (*v4l2_dequeue_buffer_f)(struct v4l2_s *obj);

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// TYPES /////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum v4l2_error_e {
    V4L2_ERROR_NONE,
    V4L2_ERROR_IO,
    V4L2_ERROR_MEMORY,
    V4L2_ERROR_CAPTURE,
    V4L2_ERROR_TIMEOUT,
    V4L2_ERROR_UNKNOWN_DEVICE,
    V4L2_ERROR_BAD_CAPS
};

enum v4l2_pipe_e {
    V4L2_PIPE_READ,
    V4L2_PIPE_WRITE,
    V4L2_PIPE_COUNT
};

struct v4l2_open_device_params_s {
    char     path[MAX_PATH_SIZE];
    uint32_t caps;
};

struct v4l2_configure_device_params_s {
    enum v4l2_buf_type   type;
    uint32_t             pixelformat;
    enum v4l2_colorspace colorspace;
    int32_t              width;
    int32_t              height;
    uint32_t             desiredFps;
};

struct v4l2_selection_params_s {
    int32_t left;
    int32_t top;
    int32_t width;
    int32_t height;
};

struct v4l2_request_buffers_params_s {
    uint32_t         count;
    enum v4l2_memory memory;
};

struct v4l2_mapping_buffer_s {
    uint32_t index;
    size_t   length;
    uint32_t offset;
    void     *start;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// MAIN CONTEXT /////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

struct v4l2_s {
    v4l2_open_device_f        openDevice;
    v4l2_close_device_f       closeDevice;

    v4l2_configure_device_f   configureDevice;
    v4l2_set_cropping_area_f  setCroppingArea;
    v4l2_set_composing_area_f setComposingArea;

    v4l2_request_buffers_f    requestBuffers;
    v4l2_release_buffers_f    releaseBuffers;

    v4l2_start_capture_f      startCapture;
    v4l2_stop_capture_f       stopCapture;

    v4l2_await_data_f         awaitData;
    v4l2_stop_awaiting_data_f stopAwaitingData;

    v4l2_queue_buffer_f       queueBuffer;
    v4l2_dequeue_buffer_f     dequeueBuffer;

    char                         path[MAX_PATH_SIZE];
    int32_t                      deviceFd;
    int32_t                      quitFd[V4L2_PIPE_COUNT];

    struct v4l2_capability       caps;
    struct v4l2_format           format;
    enum   v4l2_memory           memory;
    
    uint32_t                     nbBuffers;
    size_t                       maxBufferSize;
    
    struct v4l2_mapping_buffer_s *map;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// INITIALIZER //////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum v4l2_error_e V4l2_Init(struct v4l2_s **obj);
enum v4l2_error_e V4l2_UnInit(struct v4l2_s **obj);

#ifdef __cplusplus
}
#endif

#endif //__V4L2_H__
