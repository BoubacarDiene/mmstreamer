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
* \file   Core.h
* \author Boubacar DIENE
*/

#ifndef __CORE_H__
#define __CORE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include "core/Common.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                            */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

typedef enum CORE_ERROR_E             CORE_ERROR_E;
typedef enum CORE_KEEP_ALIVE_METHOD_E CORE_KEEP_ALIVE_METHOD_E;

typedef struct CORE_S                 CORE_S;

typedef CORE_ERROR_E (*CORE_LOAD_ALL_PARAMS_F  )(CORE_S *obj);
typedef CORE_ERROR_E (*CORE_UNLOAD_ALL_PARAMS_F)(CORE_S *obj);

typedef CORE_ERROR_E (*CORE_LOAD_GRAPHICS_PARAMS_F  )(CORE_S *obj);
typedef CORE_ERROR_E (*CORE_UNLOAD_GRAPHICS_PARAMS_F)(CORE_S *obj);

typedef CORE_ERROR_E (*CORE_LOAD_VIDEO_PARAMS_F  )(CORE_S *obj);
typedef CORE_ERROR_E (*CORE_UNLOAD_VIDEO_PARAMS_F)(CORE_S *obj);

typedef CORE_ERROR_E (*CORE_LOAD_SERVERS_PARAMS_F  )(CORE_S *obj);
typedef CORE_ERROR_E (*CORE_UNLOAD_SERVERS_PARAMS_F)(CORE_S *obj);

typedef CORE_ERROR_E (*CORE_LOAD_CLIENTS_PARAMS_F  )(CORE_S *obj);
typedef CORE_ERROR_E (*CORE_UNLOAD_CLIENTS_PARAMS_F)(CORE_S *obj);

typedef CORE_ERROR_E (*CORE_KEEP_APP_RUNNING_F)(CORE_S *obj, CORE_KEEP_ALIVE_METHOD_E method, uint32_t timeout_s);

enum CORE_ERROR_E {
    CORE_ERROR_NONE,
    CORE_ERROR_INIT,
    CORE_ERROR_UNINIT,
    CORE_ERROR_PARAMS,
    CORE_ERROR_XML,
    CORE_ERROR_KEEP_ALIVE
};

enum CORE_KEEP_ALIVE_METHOD_E {
    CORE_KEEP_ALIVE_EVENTS_BASED,
    CORE_KEEP_ALIVE_SEMAPHORE_BASED,
    CORE_KEEP_ALIVE_TIMER_BASED
};

struct CORE_S {
    CORE_LOAD_ALL_PARAMS_F        loadAllParams;
    CORE_UNLOAD_ALL_PARAMS_F      unloadAllParams;
    
    CORE_LOAD_GRAPHICS_PARAMS_F   loadGraphicsParams;
    CORE_UNLOAD_GRAPHICS_PARAMS_F unloadGraphicsParams;
    
    CORE_LOAD_VIDEO_PARAMS_F      loadVideoParams;
    CORE_UNLOAD_VIDEO_PARAMS_F    unloadVideoParams;
    
    CORE_LOAD_SERVERS_PARAMS_F    loadServersParams;
    CORE_UNLOAD_SERVERS_PARAMS_F  unloadServersParams;
    
    CORE_LOAD_CLIENTS_PARAMS_F    loadClientsParams;
    CORE_UNLOAD_CLIENTS_PARAMS_F  unloadClientsParams;
    
    CORE_KEEP_APP_RUNNING_F       keepAppRunning;
    
    void                              *pData;
};

/* -------------------------------------------------------------------------------------------- */
/*                                      PUBLIC FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

CORE_ERROR_E Core_Init  (CORE_S **obj, CONTEXT_S *ctx);
CORE_ERROR_E Core_UnInit(CORE_S **obj);

#ifdef __cplusplus
}
#endif

#endif //__CORE_H__
