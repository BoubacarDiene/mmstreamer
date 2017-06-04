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

#define CONTROLLER_FORMAT_VALUE "%u"
#define CONTROLLER_FORMAT_TEXT  "%u;%u;%u;%u"
#define CONTROLLER_FORMAT_IMAGE "%u;%d"
#define CONTROLLER_FORMAT_NAV   "%s;%s;%s;%s"
#define CONTROLLER_FORMAT_GFX   "%u;%u;%u"

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

typedef enum   CONTROLLER_ERROR_E     CONTROLLER_ERROR_E;
typedef enum   CONTROLLER_COMMAND_E   CONTROLLER_COMMAND_E;
typedef enum   CONTROLLER_EVENT_E     CONTROLLER_EVENT_E;
typedef enum   CONTROLLER_GFX_E       CONTROLLER_GFX_E;

typedef struct CONTROLLER_COMMAND_S   CONTROLLER_COMMAND_S;
typedef struct CONTROLLER_EVENT_S     CONTROLLER_EVENT_S;
typedef struct CONTROLLER_LIBRARY_S   CONTROLLER_LIBRARY_S;
typedef struct CONTROLLER_FUNCTIONS_S CONTROLLER_FUNCTIONS_S;
typedef struct CONTROLLER_S           CONTROLLER_S;

typedef void (*CONTROLLER_RELEASE_CB)(void *memory);

typedef void (*CONTROLLER_ON_COMMAND_CB)(CONTROLLER_S *obj, void *data);
typedef void (*CONTROLLER_ON_EVENT_CB  )(CONTROLLER_S *obj, CONTROLLER_EVENT_S *event);

typedef CONTROLLER_ERROR_E (*CONTROLLER_INIT_LIBRARY_F  )(CONTROLLER_S **obj, CONTROLLER_FUNCTIONS_S *fcts);
typedef CONTROLLER_ERROR_E (*CONTROLLER_UNINIT_LIBRARY_F)(CONTROLLER_S **obj);

typedef void (*CONTROLLER_REGISTER_EVENTS_F  )(void *userData, int32_t eventsMask);
typedef void (*CONTROLLER_UNREGISTER_EVENTS_F)(void *userData, int32_t eventsMask);

typedef void (*CONTROLLER_SEND_TO_ENGINE_F )(void *userData, CONTROLLER_COMMAND_S *command);
typedef void (*CONTROLLER_SEND_TO_LIBRARY_F)(void *userData, CONTROLLER_LIBRARY_S *library);

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
    CONTROLLER_EVENT_STOPPED   = 1 << 0,
    CONTROLLER_EVENT_STARTED   = 1 << 1,
    CONTROLLER_EVENT_SUSPENDED = 1 << 2,
    CONTROLLER_EVENT_CLICKED   = 1 << 3,

    CONTROLLER_EVENT_ALL       = 0xFF
};

enum CONTROLLER_GFX_E {
    CONTROLLER_GFX_MOVE_LEFT,
    CONTROLLER_GFX_MOVE_UP,
    CONTROLLER_GFX_MOVE_RIGHT,
    CONTROLLER_GFX_MOVE_DOWN,
    CONTROLLER_GFX_CLICK
};

struct CONTROLLER_COMMAND_S {
    CONTROLLER_COMMAND_E  id;
    char                  *data;
    CONTROLLER_RELEASE_CB release;
};

struct CONTROLLER_EVENT_S {
    CONTROLLER_EVENT_E id;
    char               *name;
};

struct CONTROLLER_LIBRARY_S {
    char                  *name;
    void                  *data;
    CONTROLLER_RELEASE_CB release;
};

struct CONTROLLER_FUNCTIONS_S {
    CONTROLLER_REGISTER_EVENTS_F   registerEvents;
    CONTROLLER_UNREGISTER_EVENTS_F unregisterEvents;

    CONTROLLER_SEND_TO_ENGINE_F    sendToEngine;
    CONTROLLER_SEND_TO_LIBRARY_F   sendToLibrary;

    void                           *userData;
};

struct CONTROLLER_S {
    void *pData;
};

#ifdef __cplusplus
}
#endif

#endif //__CONTROLLER_H__
