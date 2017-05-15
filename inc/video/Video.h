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
* \file   Video.h
* \author Boubacar DIENE
*/

#ifndef __VIDEO_H__
#define __VIDEO_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include "utils/Common.h"
#include "video/V4l2.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                             */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

typedef enum   VIDEO_ERROR_E      VIDEO_ERROR_E;
typedef enum   VIDEO_AWAIT_MODE_E VIDEO_AWAIT_MODE_E;

typedef struct VIDEO_LISTENER_S   VIDEO_LISTENER_S;
typedef struct VIDEO_AREA_S       VIDEO_AREA_S;
typedef struct VIDEO_BUFFER_S     VIDEO_BUFFER_S;
typedef struct VIDEO_PARAMS_S     VIDEO_PARAMS_S;
typedef struct VIDEO_S            VIDEO_S;

typedef void (*ON_VIDEO_BUFFER_AVAILABLE_CB)(VIDEO_BUFFER_S *videoBuffer, void *userData);

typedef VIDEO_ERROR_E (*VIDEO_REGISTER_LISTENER_F  )(VIDEO_S *obj, VIDEO_PARAMS_S *params, VIDEO_LISTENER_S *listener);
typedef VIDEO_ERROR_E (*VIDEO_UNREGISTER_LISTENER_F)(VIDEO_S *obj, VIDEO_PARAMS_S *params, VIDEO_LISTENER_S *listener);

typedef VIDEO_ERROR_E (*VIDEO_GET_FINAL_VIDEO_AREA_F)(VIDEO_S *obj, VIDEO_PARAMS_S *params, VIDEO_AREA_S *videoArea);
typedef VIDEO_ERROR_E (*VIDEO_GET_MAX_BUFFER_SIZE_F )(VIDEO_S *obj, VIDEO_PARAMS_S *params, size_t *size);

typedef VIDEO_ERROR_E (*VIDEO_START_DEVICE_CAPTURE_F)(VIDEO_S *obj, VIDEO_PARAMS_S *params);
typedef VIDEO_ERROR_E (*VIDEO_STOP_DEVICE_CAPTURE_F )(VIDEO_S *obj, VIDEO_PARAMS_S *params);

enum VIDEO_ERROR_E {
    VIDEO_ERROR_NONE,
    VIDEO_ERROR_INIT,
    VIDEO_ERROR_UNINIT,
    VIDEO_ERROR_LOCK,
    VIDEO_ERROR_LIST,
    VIDEO_ERROR_START,
    VIDEO_ERROR_STOP,
    VIDEO_ERROR_PARAMS
};

enum VIDEO_AWAIT_MODE_E {
    VIDEO_AWAIT_MODE_BLOCKING,
    VIDEO_AWAIT_MODE_NON_BLOCKING
};

struct VIDEO_LISTENER_S {
    char                         name[MAX_NAME_SIZE];
    ON_VIDEO_BUFFER_AVAILABLE_CB onVideoBufferAvailableCb;
    
    void                         *userData;
};

struct VIDEO_AREA_S {
    uint32_t left;
    uint32_t top;
    uint32_t width;
    uint32_t height;
};

struct VIDEO_BUFFER_S {
    uint32_t index;
    uint32_t offset;
    void     *data;
    size_t   length;
};

struct VIDEO_PARAMS_S {
    char                 name[MAX_NAME_SIZE];

    /* Open device */
    char                 path[MAX_PATH_SIZE];
    uint32_t             caps;
    
    /* Configure device */
    enum v4l2_buf_type   type;
    uint32_t             pixelformat;
    enum v4l2_colorspace colorspace;
    
    PRIORITY_E           priority;
    uint32_t             desiredFps;
    
    VIDEO_AREA_S         captureArea;
    VIDEO_AREA_S         croppingArea;
    VIDEO_AREA_S         composingArea;
    
    /* Request buffers */
    uint32_t             count;
    enum v4l2_memory     memory;
    
    /* Await data */
    VIDEO_AWAIT_MODE_E   awaitMode;
};

struct VIDEO_S {
    VIDEO_REGISTER_LISTENER_F    registerListener;
    VIDEO_UNREGISTER_LISTENER_F  unregisterListener;
    
    VIDEO_GET_FINAL_VIDEO_AREA_F getFinalVideoArea;
    VIDEO_GET_MAX_BUFFER_SIZE_F  getMaxBufferSize;
    
    VIDEO_START_DEVICE_CAPTURE_F startDeviceCapture;
    VIDEO_STOP_DEVICE_CAPTURE_F  stopDeviceCapture;
    
    VIDEO_BUFFER_S               buffer;
    
    void                         *pData;
};

/* -------------------------------------------------------------------------------------------- */
/*                                      PUBLIC FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

VIDEO_ERROR_E Video_Init  (VIDEO_S **obj);
VIDEO_ERROR_E Video_UnInit(VIDEO_S **obj);

#ifdef __cplusplus
}
#endif

#endif //__VIDEO_H__
