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
* \file   Controller.h
* \author Boubacar DIENE
*
* \brief  TODO!!
*/

#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include <stdint.h>

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                             */
/* -------------------------------------------------------------------------------------------- */

#define MAX_NAME_SIZE 128

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

typedef enum   CONTROLLER_ERROR_E           CONTROLLER_ERROR_E;
typedef enum   CONTROLLER_COMMAND_E         CONTROLLER_COMMAND_E;
typedef enum   CONTROLLER_EVENT_E           CONTROLLER_EVENT_E;
typedef enum   CONTROLLER_STATE_E           CONTROLLER_STATE_E;
typedef enum   CONTROLLER_GFX_EVENT_E       CONTROLLER_GFX_EVENT_E;

typedef struct CONTROLLER_ACK_S             CONTROLLER_ACK_S;
typedef struct CONTROLLER_CLICK_S           CONTROLLER_CLICK_S;
typedef struct CONTROLLER_STATE_S           CONTROLLER_STATE_S;
typedef struct CONTROLLER_EVENT_S           CONTROLLER_EVENT_S;
typedef struct CONTROLLER_POINT_S           CONTROLLER_POINT_S;

typedef struct CONTROLLER_DATA_COMMON_S     CONTROLLER_DATA_COMMON_S;
typedef struct CONTROLLER_DATA_SCREENSHOT_S CONTROLLER_DATA_SCREENSHOT_S;
typedef struct CONTROLLER_DATA_TEXT_S       CONTROLLER_DATA_TEXT_S;
typedef struct CONTROLLER_DATA_IMAGE_S      CONTROLLER_DATA_IMAGE_S;
typedef struct CONTROLLER_DATA_NAV_S        CONTROLLER_DATA_NAV_S;
typedef struct CONTROLLER_DATA_GFX_EVENT_S  CONTROLLER_DATA_GFX_EVENT_S;

typedef struct CONTROLLER_COMMAND_S         CONTROLLER_COMMAND_S;

typedef struct CONTROLLER_FUNCTIONS_S       CONTROLLER_FUNCTIONS_S;
typedef struct CONTROLLER_S                 CONTROLLER_S;

typedef void (*CONTROLLER_REGISTER_EVENTS_F  )(void *userData, int32_t eventsMask);
typedef void (*CONTROLLER_UNREGISTER_EVENTS_F)(void *userData, int32_t eventsMask);
typedef void (*CONTROLLER_SEND_COMMAND_F     )(void *userData, CONTROLLER_COMMAND_S *command);

typedef CONTROLLER_ERROR_E (*CONTROLLER_INIT_F  )(CONTROLLER_S **obj, CONTROLLER_FUNCTIONS_S *fcts);
typedef CONTROLLER_ERROR_E (*CONTROLLER_UNINIT_F)(CONTROLLER_S **obj);
typedef CONTROLLER_ERROR_E (*CONTROLLER_NOTIFY_F)(CONTROLLER_S *obj, CONTROLLER_EVENT_S *event);

enum CONTROLLER_ERROR_E {
    CONTROLLER_ERROR_NONE,
    CONTROLLER_ERROR_INIT,
    CONTROLLER_ERROR_UNINIT,
    CONTROLLER_ERROR_PARAMS,
    CONTROLLER_ERROR_UNKNOWN
};

enum CONTROLLER_COMMAND_E {
    CONTROLLER_COMMAND_CLOSE_APPLICATION,
    CONTROLLER_COMMAND_CHANGE_LANGUAGE,

    CONTROLLER_COMMAND_SAVE_VIDEO_ELEMENT,
    CONTROLLER_COMMAND_TAKE_SCREENSHOT,

    CONTROLLER_COMMAND_HIDE_ELEMENT,
    CONTROLLER_COMMAND_SHOW_ELEMENT,
    CONTROLLER_COMMAND_SET_FOCUS,
    CONTROLLER_COMMAND_HIDE_GROUP,
    CONTROLLER_COMMAND_SHOW_GROUP,
    CONTROLLER_COMMAND_SET_CLICKABLE,
    CONTROLLER_COMMAND_SET_NOT_CLICKABLE,

    CONTROLLER_COMMAND_STOP_GRAPHICS,
    CONTROLLER_COMMAND_START_GRAPHICS,

    CONTROLLER_COMMAND_STOP_VIDEO,
    CONTROLLER_COMMAND_START_VIDEO,

    CONTROLLER_COMMAND_STOP_SERVER,
    CONTROLLER_COMMAND_START_SERVER,
    CONTROLLER_COMMAND_SUSPEND_SERVER,
    CONTROLLER_COMMAND_RESUME_SERVER,

    CONTROLLER_COMMAND_STOP_CLIENT,
    CONTROLLER_COMMAND_START_CLIENT,

    CONTROLLER_COMMAND_UPDATE_TEXT,
    CONTROLLER_COMMAND_UPDATE_IMAGE,
    CONTROLLER_COMMAND_UPDATE_NAV,

    CONTROLLER_COMMAND_SEND_GFX_EVENT
};

enum CONTROLLER_EVENT_E {
    CONTROLLER_EVENT_ACK   = 1 << 0,
    CONTROLLER_EVENT_CLICK = 1 << 1,
    CONTROLLER_EVENT_STATE = 1 << 2,

    CONTROLLER_EVENT_ALL   = 0xFF
};

enum CONTROLLER_STATE_E {
    CONTROLLER_STATE_STOPPED,
    CONTROLLER_STATE_STARTED,
    CONTROLLER_STATE_SUSPENDED
};

enum CONTROLLER_GFX_EVENT_E {
    CONTROLLER_GFX_EVENT_CLICK      = 0x03,
    CONTROLLER_GFX_EVENT_MOVE_LEFT  = 0x05,
    CONTROLLER_GFX_EVENT_MOVE_UP    = 0x06,
    CONTROLLER_GFX_EVENT_MOVE_RIGHT = 0x07,
    CONTROLLER_GFX_EVENT_MOVE_DOWN  = 0x08
};

struct CONTROLLER_ACK_S {
    CONTROLLER_COMMAND_E id;
    uint8_t              done;
};

struct CONTROLLER_CLICK_S {
    char elementName[MAX_NAME_SIZE];
};

struct CONTROLLER_STATE_S {
    char               name[MAX_NAME_SIZE];
    CONTROLLER_STATE_E state;
};

struct CONTROLLER_EVENT_S {
    CONTROLLER_EVENT_E id;

    union {
        CONTROLLER_ACK_S   ack;
        CONTROLLER_CLICK_S click;
        CONTROLLER_STATE_S module;
    } arg;
};

struct CONTROLLER_POINT_S {
    uint16_t x;
    uint16_t y;
};

struct CONTROLLER_DATA_COMMON_S {
    char name[MAX_NAME_SIZE];
};

struct CONTROLLER_DATA_SCREENSHOT_S {
    uint32_t imageFormat;
};

struct CONTROLLER_DATA_TEXT_S {
    uint32_t stringId;
    uint32_t fontId;
    uint32_t fontSize;
    uint32_t colorId;
};

struct CONTROLLER_DATA_IMAGE_S {
    uint32_t imageId;
    int32_t  hiddenColorId;
};

struct CONTROLLER_DATA_NAV_S {
    char left[MAX_NAME_SIZE];
    char up[MAX_NAME_SIZE];
    char right[MAX_NAME_SIZE];
    char down[MAX_NAME_SIZE];
};

struct CONTROLLER_DATA_GFX_EVENT_S {
    CONTROLLER_GFX_EVENT_E id;
    CONTROLLER_POINT_S     point;
};

struct CONTROLLER_COMMAND_S {
    CONTROLLER_COMMAND_E id;

    union {
        CONTROLLER_DATA_COMMON_S     common;
        CONTROLLER_DATA_SCREENSHOT_S screenshot;
        CONTROLLER_DATA_TEXT_S       text;
        CONTROLLER_DATA_IMAGE_S      image;
        CONTROLLER_DATA_NAV_S        nav;
        CONTROLLER_DATA_GFX_EVENT_S  event;
    } data;
};

struct CONTROLLER_FUNCTIONS_S {
    CONTROLLER_REGISTER_EVENTS_F   registerEvents;
    CONTROLLER_UNREGISTER_EVENTS_F unregisterEvents;
    CONTROLLER_SEND_COMMAND_F      sendCommand;

    void                           *userData;
};

struct CONTROLLER_S {
    void *pData;
};

#ifdef __cplusplus
}
#endif

#endif //__CONTROLLER_H__
