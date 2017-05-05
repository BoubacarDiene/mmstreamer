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
* \file   V4l2.h
* \author Boubacar DIENE
*/

#ifndef __V4L2_H__
#define __V4L2_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include <linux/videodev2.h>

#include "utils/Common.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                            */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

typedef enum   V4L2_ERROR_E                   V4L2_ERROR_E;

typedef struct V4L2_OPEN_DEVICE_PARAMS_S      V4L2_OPEN_DEVICE_PARAMS_S;
typedef struct V4L2_CONFIGURE_DEVICE_PARAMS_S V4L2_CONFIGURE_DEVICE_PARAMS_S;
typedef struct V4L2_SELECTION_PARAMS_S        V4L2_SELECTION_PARAMS_S;
typedef struct V4L2_REQUEST_BUFFERS_PARAMS_S  V4L2_REQUEST_BUFFERS_PARAMS_S;

typedef struct V4L2_MAPPING_BUFFER_S          V4L2_MAPPING_BUFFER_S;
typedef struct V4L2_S                         V4L2_S;

typedef V4L2_ERROR_E (*V4L2_OPEN_DEVICE_F )(V4L2_S *obj, V4L2_OPEN_DEVICE_PARAMS_S *params);
typedef V4L2_ERROR_E (*V4L2_CLOSE_DEVICE_F)(V4L2_S *obj);

typedef V4L2_ERROR_E (*V4L2_CONFIGURE_DEVICE_F  )(V4L2_S *obj, V4L2_CONFIGURE_DEVICE_PARAMS_S *params);
typedef V4L2_ERROR_E (*V4L2_SET_CROPPING_AREA_F )(V4L2_S *obj, V4L2_SELECTION_PARAMS_S *cropRectInOut);
typedef V4L2_ERROR_E (*V4L2_SET_COMPOSING_AREA_F)(V4L2_S *obj, V4L2_SELECTION_PARAMS_S *composeRectInOut);

typedef V4L2_ERROR_E (*V4L2_REQUEST_BUFFERS_F)(V4L2_S *obj, V4L2_REQUEST_BUFFERS_PARAMS_S *params);
typedef V4L2_ERROR_E (*V4L2_RELEASE_BUFFERS_F)(V4L2_S *obj);

typedef V4L2_ERROR_E (*V4L2_START_CAPTURE_F)(V4L2_S *obj);
typedef V4L2_ERROR_E (*V4L2_STOP_CAPTURE_F )(V4L2_S *obj);

typedef V4L2_ERROR_E (*V4L2_AWAIT_DATA_F        )(V4L2_S *obj, int32_t timeout_ms);
typedef V4L2_ERROR_E (*V4L2_STOP_AWAITING_DATA_F)(V4L2_S *obj);

typedef V4L2_ERROR_E (*V4L2_QUEUE_BUFFER_F  )(V4L2_S *obj, uint32_t index);
typedef V4L2_ERROR_E (*V4L2_DEQUEUE_BUFFER_F)(V4L2_S *obj);

enum V4L2_ERROR_E {
    V4L2_ERROR_NONE,
    V4L2_ERROR_IO,
    V4L2_ERROR_MEMORY,
    V4L2_ERROR_CAPTURE,
    V4L2_ERROR_TIMEOUT,
    V4L2_ERROR_UNKNOWN_DEVICE,
    V4L2_ERROR_BAD_CAPS
};

struct V4L2_OPEN_DEVICE_PARAMS_S {
    char     path[MAX_PATH_SIZE];
    uint32_t caps;
};

struct V4L2_CONFIGURE_DEVICE_PARAMS_S {
    enum v4l2_buf_type   type;
    uint32_t             pixelformat;
    enum v4l2_colorspace colorspace;
    uint32_t             width;
    uint32_t             height;
    uint32_t             desiredFps;
};

struct V4L2_SELECTION_PARAMS_S {
    int32_t  left;
    int32_t  top;
    uint32_t width;
    uint32_t height;
};

struct V4L2_REQUEST_BUFFERS_PARAMS_S {
    uint32_t         count;
    enum v4l2_memory memory;
};

struct V4L2_MAPPING_BUFFER_S {
    uint32_t index;
    size_t   length;
    uint32_t offset;
    void     *start;
};

struct V4L2_S {
    char                      path[MAX_PATH_SIZE];
    int32_t                   deviceFd;
    int32_t                   quitFd[PIPE_COUNT];

    struct v4l2_capability    caps;
    struct v4l2_format        format;
    enum   v4l2_memory        memory;
    
    uint32_t                  nbBuffers;
    size_t                    maxBufferSize;
    
    V4L2_MAPPING_BUFFER_S     *map;

    V4L2_OPEN_DEVICE_F        openDevice;
    V4L2_CLOSE_DEVICE_F       closeDevice;
    
    V4L2_CONFIGURE_DEVICE_F   configureDevice;
    V4L2_SET_CROPPING_AREA_F  setCroppingArea;
    V4L2_SET_COMPOSING_AREA_F setComposingArea;
    
    V4L2_REQUEST_BUFFERS_F    requestBuffers;
    V4L2_RELEASE_BUFFERS_F    releaseBuffers;
    
    V4L2_START_CAPTURE_F      startCapture;
    V4L2_STOP_CAPTURE_F       stopCapture;
    
    V4L2_AWAIT_DATA_F         awaitData;
    V4L2_STOP_AWAITING_DATA_F stopAwaitingData;
    
    V4L2_QUEUE_BUFFER_F       queueBuffer;
    V4L2_DEQUEUE_BUFFER_F     dequeueBuffer;
};

/* -------------------------------------------------------------------------------------------- */
/*                                      PUBLIC FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

V4L2_ERROR_E V4l2_Init  (V4L2_S **obj);
V4L2_ERROR_E V4l2_UnInit(V4L2_S **obj);

#ifdef __cplusplus
}
#endif

#endif //__V4L2_H__
