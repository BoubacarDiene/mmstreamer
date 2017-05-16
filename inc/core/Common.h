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
* \file   Common.h
* \author Boubacar DIENE
*/

#ifndef __CORE_COMMON_H__
#define __CORE_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include "graphics/Graphics.h"
#include "network/Client.h"
#include "network/Server.h"
#include "video/Video.h"

#include "utils/Common.h"
#include "utils/Log.h"
#include "utils/Parser.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                             */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEFS                                           */
/* -------------------------------------------------------------------------------------------- */

typedef enum   KEEP_ALIVE_METHOD_E KEEP_ALIVE_METHOD_E;
typedef enum   MODULE_STATE_E      MODULE_STATE_E;

typedef struct GRAPHICS_INFOS_S    GRAPHICS_INFOS_S;

typedef struct VIDEO_DEVICE_S      VIDEO_DEVICE_S;
typedef struct VIDEOS_INFOS_S      VIDEOS_INFOS_S;

typedef struct SERVER_INFOS_S      SERVER_INFOS_S;
typedef struct SERVERS_INFOS_S     SERVERS_INFOS_S;

typedef struct CLIENT_INFOS_S      CLIENT_INFOS_S;
typedef struct CLIENTS_INFOS_S     CLIENTS_INFOS_S;

typedef struct PARAMS_S            PARAMS_S;
typedef struct MODULE_CONFIG_S     MODULE_CONFIG_S;
typedef struct MODULES_S           MODULES_S;
typedef struct INPUT_S             INPUT_S;
typedef struct CONTEXT_S           CONTEXT_S;

enum KEEP_ALIVE_METHOD_E {
    KEEP_ALIVE_EVENTS_BASED,
    KEEP_ALIVE_SEMAPHORE_BASED,
    KEEP_ALIVE_TIMER_BASED
};

enum MODULE_STATE_E {
    MODULE_STATE_STOPPED,
    MODULE_STATE_STARTED,
    MODULE_STATE_SUSPENDED
};

struct GRAPHICS_INFOS_S {
    MODULE_STATE_E    state;

    char              *currentLanguage;
    uint32_t          nbGfxElements;
    GFX_ELEMENT_S     **gfxElements;
    GRAPHICS_PARAMS_S graphicsParams;
};

struct VIDEO_DEVICE_S {
    MODULE_STATE_E    state;

    uint8_t           nbVideoListeners;
    VIDEO_LISTENER_S  **videoListeners;
    VIDEO_PARAMS_S    videoParams;

    char              *graphicsDest;
    int8_t            graphicsIndex;

    char              *serverDest;
    int8_t            serverIndex;
};

struct VIDEOS_INFOS_S {
    uint8_t        nbDevices;
    VIDEO_DEVICE_S **devices;
};

struct SERVER_INFOS_S {
    MODULE_STATE_E    state;
    SERVER_PARAMS_S   serverParams;
};

struct SERVERS_INFOS_S {
    uint8_t           nbServers;
    SERVER_INFOS_S    **serverInfos;
};

struct CLIENT_INFOS_S {
    MODULE_STATE_E    state;
    CLIENT_PARAMS_S   clientParams;
    
    char              *graphicsDest;
    int8_t            graphicsIndex;
    
    char              *serverDest;
    int8_t            serverIndex;
};

struct CLIENTS_INFOS_S {
    uint8_t           nbClients;
    CLIENT_INFOS_S    **clientInfos;
};

struct PARAMS_S {
    GRAPHICS_INFOS_S graphicsInfos;
    VIDEOS_INFOS_S   videosInfos;
    SERVERS_INFOS_S  serversInfos;
    CLIENTS_INFOS_S  clientsInfos;
};

struct MODULE_CONFIG_S {
    uint8_t          enable;
    uint8_t          autoStart;
    char             *xml;
};

struct MODULES_S {
    GRAPHICS_S *graphicsObj;
    VIDEO_S    *videoObj;
    SERVER_S   *serverObj;
    CLIENT_S   *clientObj;
};

struct INPUT_S {
    char            *appDataDir;
    char            *resRootDir;
    
    uint8_t         keepAliveMethod;
    uint32_t        timeout_s;

    MODULE_CONFIG_S graphicsConfig;
    MODULE_CONFIG_S videosConfig;
    MODULE_CONFIG_S serversConfig;
    MODULE_CONFIG_S clientsConfig;
};

struct CONTEXT_S {
    PARAMS_S   params;
    MODULES_S  modules;
    INPUT_S    input;

    PARSER_S   *parserObj;

    sem_t      keepAliveSem;
};

/* -------------------------------------------------------------------------------------------- */
/*                                           VARIABLES                                          */
/* -------------------------------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif //__CORE_COMMON_H__
