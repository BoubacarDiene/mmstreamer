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
* \file   Specific.h
* \author Boubacar DIENE
*/

#ifndef __SPECIFIC_H__
#define __SPECIFIC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include "specific/Common.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                            */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

typedef enum SPECIFIC_ERROR_E             SPECIFIC_ERROR_E;
typedef enum SPECIFIC_KEEP_ALIVE_METHOD_E SPECIFIC_KEEP_ALIVE_METHOD_E;

typedef struct SPECIFIC_S                 SPECIFIC_S;

typedef SPECIFIC_ERROR_E (*SPECIFIC_LOAD_ALL_PARAMS_F  )(SPECIFIC_S *obj);
typedef SPECIFIC_ERROR_E (*SPECIFIC_UNLOAD_ALL_PARAMS_F)(SPECIFIC_S *obj);

typedef SPECIFIC_ERROR_E (*SPECIFIC_LOAD_GRAPHICS_PARAMS_F  )(SPECIFIC_S *obj);
typedef SPECIFIC_ERROR_E (*SPECIFIC_UNLOAD_GRAPHICS_PARAMS_F)(SPECIFIC_S *obj);

typedef SPECIFIC_ERROR_E (*SPECIFIC_LOAD_VIDEO_PARAMS_F  )(SPECIFIC_S *obj);
typedef SPECIFIC_ERROR_E (*SPECIFIC_UNLOAD_VIDEO_PARAMS_F)(SPECIFIC_S *obj);

typedef SPECIFIC_ERROR_E (*SPECIFIC_LOAD_SERVERS_PARAMS_F  )(SPECIFIC_S *obj);
typedef SPECIFIC_ERROR_E (*SPECIFIC_UNLOAD_SERVERS_PARAMS_F)(SPECIFIC_S *obj);

typedef SPECIFIC_ERROR_E (*SPECIFIC_LOAD_CLIENTS_PARAMS_F  )(SPECIFIC_S *obj);
typedef SPECIFIC_ERROR_E (*SPECIFIC_UNLOAD_CLIENTS_PARAMS_F)(SPECIFIC_S *obj);

typedef SPECIFIC_ERROR_E (*SPECIFIC_KEEP_APP_RUNNING_F)(SPECIFIC_S *obj, SPECIFIC_KEEP_ALIVE_METHOD_E method, uint32_t timeout_s);

enum SPECIFIC_ERROR_E {
    SPECIFIC_ERROR_NONE,
    SPECIFIC_ERROR_INIT,
    SPECIFIC_ERROR_UNINIT,
    SPECIFIC_ERROR_PARAMS,
    SPECIFIC_ERROR_XML,
    SPECIFIC_ERROR_KEEP_ALIVE
};

enum SPECIFIC_KEEP_ALIVE_METHOD_E {
    SPECIFIC_KEEP_ALIVE_EVENTS_BASED,
    SPECIFIC_KEEP_ALIVE_SEMAPHORE_BASED,
    SPECIFIC_KEEP_ALIVE_TIMER_BASED
};

struct SPECIFIC_S {
    SPECIFIC_LOAD_ALL_PARAMS_F        loadAllParams;
    SPECIFIC_UNLOAD_ALL_PARAMS_F      unloadAllParams;
    
    SPECIFIC_LOAD_GRAPHICS_PARAMS_F   loadGraphicsParams;
    SPECIFIC_UNLOAD_GRAPHICS_PARAMS_F unloadGraphicsParams;
    
    SPECIFIC_LOAD_VIDEO_PARAMS_F      loadVideoParams;
    SPECIFIC_UNLOAD_VIDEO_PARAMS_F    unloadVideoParams;
    
    SPECIFIC_LOAD_SERVERS_PARAMS_F    loadServersParams;
    SPECIFIC_UNLOAD_SERVERS_PARAMS_F  unloadServersParams;
    
    SPECIFIC_LOAD_CLIENTS_PARAMS_F    loadClientsParams;
    SPECIFIC_UNLOAD_CLIENTS_PARAMS_F  unloadClientsParams;
    
    SPECIFIC_KEEP_APP_RUNNING_F       keepAppRunning;
    
    void                              *pData;
};

/* -------------------------------------------------------------------------------------------- */
/*                                      PUBLIC FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

SPECIFIC_ERROR_E Specific_Init  (SPECIFIC_S **obj, CONTEXT_S *ctx);
SPECIFIC_ERROR_E Specific_UnInit(SPECIFIC_S **obj);

#ifdef __cplusplus
}
#endif

#endif //__SPECIFIC_H__
