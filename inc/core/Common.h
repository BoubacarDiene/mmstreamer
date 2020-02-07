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
* \file Common.h
* \author Boubacar DIENE
*/

#ifndef __CORE_COMMON_H__
#define __CORE_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// HEADERS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#include "graphics/Graphics.h"
#include "network/Client.h"
#include "network/Server.h"
#include "video/Video.h"

#include "utils/Common.h"
#include "utils/Log.h"
#include "utils/Parser.h"

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// TYPES /////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum keep_alive_method_e {
    KEEP_ALIVE_EVENTS_BASED,
    KEEP_ALIVE_SEMAPHORE_BASED,
    KEEP_ALIVE_TIMER_BASED
};

enum module_state_e {
    MODULE_STATE_STOPPED   = 1 << 0,
    MODULE_STATE_STARTED   = 1 << 1,
    MODULE_STATE_SUSPENDED = 1 << 2
};

struct graphics_infos_s {
    enum module_state_e      state;

    char                     *currentLanguage;
    uint32_t                 nbGfxElements;
    struct gfx_element_s     **gfxElements;
    struct graphics_params_s graphicsParams;
};

struct video_device_s {
    enum module_state_e     state;

    uint8_t                 nbVideoListeners;
    struct video_listener_s **videoListeners;
    struct video_params_s   videoParams;

    char                    *graphicsDest;
    int32_t                 graphicsIndex;

    char                    *serverDest;
    int32_t                 serverIndex;
};

struct videos_infos_s {
    uint8_t               nbDevices;
    struct video_device_s **devices;
};

struct server_infos_s {
    enum module_state_e    state;
    struct server_params_s serverParams;
};

struct servers_infos_s {
    uint8_t               nbServers;
    struct server_infos_s **serverInfos;
};

struct client_infos_s {
    enum module_state_e    state;
    struct client_params_s clientParams;
    
    char                   *graphicsDest;
    int32_t                graphicsIndex;
    
    char                   *serverDest;
    int32_t                serverIndex;
};

struct clients_infos_s {
    uint8_t               nbClients;
    struct client_infos_s **clientInfos;
};

struct library_s {
    char *path;

    char *initFn;
    char *uninitFn;
    char *onCommandCb;
    char *onEventCb;
};

struct params_s {
    struct graphics_infos_s graphicsInfos;
    struct videos_infos_s   videosInfos;
    struct servers_infos_s  serversInfos;
    struct clients_infos_s  clientsInfos;
};

struct module_config_s {
    uint8_t          enable;
    uint8_t          autoStart;
    char             *xml;
};

struct modules_s {
    struct graphics_s *graphicsObj;
    struct video_s    *videoObj;
    struct server_s   *serverObj;
    struct client_s   *clientObj;
};

struct input_s {
    char                   *appDataDir;
    char                   *resRootDir;
    char                   *libRootDir;
    
    uint8_t                keepAliveMethod;
    uint32_t               timeout_s;
    int32_t                maxBufferSize;

    uint8_t                nbCtrlLibs;
    struct library_s       *ctrlLibs;
    uint8_t                ctrlLibsPrio;

    struct module_config_s graphicsConfig;
    struct module_config_s videosConfig;
    struct module_config_s serversConfig;
    struct module_config_s clientsConfig;
};

struct context_s {
    struct params_s  params;
    struct modules_s modules;
    struct input_s   input;

    struct parser_s  *parserObj;

    sem_t            keepAliveSem;
};

#ifdef __cplusplus
}
#endif

#endif //__CORE_COMMON_H__
