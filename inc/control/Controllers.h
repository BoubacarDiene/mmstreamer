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
* \file   Controllers.h
* \author Boubacar DIENE
*/

#ifndef __CONTROLLERS_H__
#define __CONTROLLERS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include "core/Common.h"

#include "export/Controller.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                             */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEFS                                           */
/* -------------------------------------------------------------------------------------------- */

typedef enum   CONTROLLERS_ERROR_E        CONTROLLERS_ERROR_E;

typedef struct CONTROLLERS_COMMAND_S      CONTROLLERS_COMMAND_S;
typedef struct CONTROLLERS_LIB_S          CONTROLLERS_LIB_S;
typedef struct CONTROLLERS_TASK_S         CONTROLLERS_TASK_S;
typedef struct CONTROLLERS_PARAMS_S       CONTROLLERS_PARAMS_S;
typedef struct CONTROLLERS_PRIVATE_DATA_S CONTROLLERS_PRIVATE_DATA_S;
typedef struct CONTROLLERS_S              CONTROLLERS_S;

typedef void (*CONTROLLERS_ON_COMMAND_CB)(void *userData, CONTROLLER_COMMAND_S *command);

typedef CONTROLLERS_ERROR_E (*CONTROLLERS_LOAD_LIBS_F  )(CONTROLLERS_S *obj);
typedef CONTROLLERS_ERROR_E (*CONTROLLERS_UNLOAD_LIBS_F)(CONTROLLERS_S *obj);

typedef CONTROLLERS_ERROR_E (*CONTROLLERS_INIT_CMDS_TASK_F  )(CONTROLLERS_S *obj);
typedef CONTROLLERS_ERROR_E (*CONTROLLERS_UNINIT_CMDS_TASK_F)(CONTROLLERS_S *obj);
typedef CONTROLLERS_ERROR_E (*CONTROLLERS_START_CMDS_TASK_F )(CONTROLLERS_S *obj);
typedef CONTROLLERS_ERROR_E (*CONTROLLERS_STOP_CMDS_TASK_F  )(CONTROLLERS_S *obj);

typedef CONTROLLERS_ERROR_E (*CONTROLLERS_INIT_EVTS_TASK_F  )(CONTROLLERS_S *obj);
typedef CONTROLLERS_ERROR_E (*CONTROLLERS_UNINIT_EVTS_TASK_F)(CONTROLLERS_S *obj);
typedef CONTROLLERS_ERROR_E (*CONTROLLERS_START_EVTS_TASK_F )(CONTROLLERS_S *obj);
typedef CONTROLLERS_ERROR_E (*CONTROLLERS_STOP_EVTS_TASK_F  )(CONTROLLERS_S *obj);

typedef CONTROLLERS_ERROR_E (*CONTROLLERS_NOTIFY_F)(CONTROLLERS_S *obj, CONTROLLER_EVENT_S *event);

enum CONTROLLERS_ERROR_E {
    CONTROLLERS_ERROR_NONE,
    CONTROLLERS_ERROR_INIT,
    CONTROLLERS_ERROR_UNINIT,
    CONTROLLERS_ERROR_PARAMS,
    CONTROLLERS_ERROR_COMMAND,
    CONTROLLERS_ERROR_TASK,
    CONTROLLERS_ERROR_LIST,
    CONTROLLERS_ERROR_LIB
};

struct CONTROLLERS_COMMAND_S {
    CONTROLLER_COMMAND_E id;

    char                 *gfxElementName;
    void                 *gfxElementData;

    char                 *handlerName;
    char                 *handlerData;
};

struct CONTROLLERS_LIB_S {
    CONTROLLER_S        *obj;
    void                *handle;

    CONTROLLER_INIT_F   init;
    CONTROLLER_UNINIT_F uninit;
    CONTROLLER_NOTIFY_F notify;

    int32_t             eventsMask;

    void                *pData;
};

struct CONTROLLERS_TASK_S {
    volatile uint8_t quit;

    LIST_S           *list;

    TASK_S           *task;
    TASK_PARAMS_S    taskParams;

    pthread_mutex_t  lock;
    sem_t            sem;
};

struct CONTROLLERS_PARAMS_S {
    CONTEXT_S                 *ctx;
    CONTROLLERS_ON_COMMAND_CB onCommandCb;
    void                      *userData;
};

struct CONTROLLERS_PRIVATE_DATA_S {
    CONTROLLERS_PARAMS_S params;

    uint8_t              nbLibs;
    CONTROLLERS_LIB_S    *libs;

    CONTROLLERS_TASK_S   cmdsTask;
    CONTROLLERS_TASK_S   evtsTask;
};

struct CONTROLLERS_S {
    CONTROLLERS_LOAD_LIBS_F        loadLibs;
    CONTROLLERS_UNLOAD_LIBS_F      unloadLibs;

    CONTROLLERS_INIT_CMDS_TASK_F   initCmdsTask;
    CONTROLLERS_UNINIT_CMDS_TASK_F uninitCmdsTask;
    CONTROLLERS_START_CMDS_TASK_F  startCmdsTask;
    CONTROLLERS_STOP_CMDS_TASK_F   stopCmdsTask;

    CONTROLLERS_INIT_EVTS_TASK_F   initEvtsTask;
    CONTROLLERS_UNINIT_EVTS_TASK_F uninitEvtsTask;
    CONTROLLERS_START_EVTS_TASK_F  startEvtsTask;
    CONTROLLERS_STOP_EVTS_TASK_F   stopEvtsTask;

    CONTROLLERS_NOTIFY_F           notify;

    CONTROLLERS_PRIVATE_DATA_S     *pData;
};

/* -------------------------------------------------------------------------------------------- */
/*                                           VARIABLES                                          */
/* -------------------------------------------------------------------------------------------- */

CONTROLLERS_ERROR_E Controllers_Init  (CONTROLLERS_S **obj, CONTROLLERS_PARAMS_S *params);
CONTROLLERS_ERROR_E Controllers_UnInit(CONTROLLERS_S **obj);

#ifdef __cplusplus
}
#endif

#endif //__CONTROLLERS_H__
