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
* \file Core.h
* \author Boubacar DIENE
*/

#ifndef __CORE_H__
#define __CORE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// HEADERS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#include "core/Common.h"

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////// TYPES DECLARATION ///////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum core_error_e;

struct core_s;

/* -------------------------------------------------------------------------------------------- */
/* ///////////////////////////////////// PUBLIC FUNCTIONS ///////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

typedef enum core_error_e (*core_load_all_params_f)(struct core_s *obj);
typedef enum core_error_e (*core_unload_all_params_f)(struct core_s *obj);

typedef enum core_error_e (*core_load_graphics_params_f)(struct core_s *obj);
typedef enum core_error_e (*core_unload_graphics_params_f)(struct core_s *obj);

typedef enum core_error_e (*core_load_videos_params_f)(struct core_s *obj);
typedef enum core_error_e (*core_unload_videos_params_f)(struct core_s *obj);

typedef enum core_error_e (*core_load_servers_params_f)(struct core_s *obj);
typedef enum core_error_e (*core_unload_servers_params_f)(struct core_s *obj);

typedef enum core_error_e (*core_load_clients_params_f)(struct core_s *obj);
typedef enum core_error_e (*core_unload_clients_params_f)(struct core_s *obj);

typedef enum core_error_e (*core_keep_app_running_f)(struct core_s *obj,
                                                     enum keep_alive_method_e method,
                                                     uint32_t timeout_s);

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// TYPES /////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum core_error_e {
    CORE_ERROR_NONE,
    CORE_ERROR_INIT,
    CORE_ERROR_UNINIT,
    CORE_ERROR_PARAMS,
    CORE_ERROR_XML,
    CORE_ERROR_KEEP_ALIVE
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// MAIN CONTEXT /////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

struct core_s {
    core_load_all_params_f        loadAllParams;
    core_unload_all_params_f      unloadAllParams;
    
    core_load_graphics_params_f   loadGraphicsParams;
    core_unload_graphics_params_f unloadGraphicsParams;
    
    core_load_videos_params_f     loadVideosParams;
    core_unload_videos_params_f   unloadVideosParams;
    
    core_load_servers_params_f    loadServersParams;
    core_unload_servers_params_f  unloadServersParams;
    
    core_load_clients_params_f    loadClientsParams;
    core_unload_clients_params_f  unloadClientsParams;
    
    core_keep_app_running_f       keepAppRunning;
    
    void *pData;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// INITIALIZER //////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum core_error_e Core_Init(struct core_s **obj, struct context_s *ctx);
enum core_error_e Core_UnInit(struct core_s **obj);

#ifdef __cplusplus
}
#endif

#endif //__CORE_H__
