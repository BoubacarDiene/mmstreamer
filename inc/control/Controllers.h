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
///////////////////////////////////////////////////////////////////////////////////////////////////

/*!
* \file Controllers.h
* \author Boubacar DIENE
*/

#ifndef __CONTROLLERS_H__
#define __CONTROLLERS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// HEADERS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#include "core/Common.h"

#include "export/Controller.h"

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////// TYPES DECLARATION ///////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum controllers_error_e;

struct controllers_command_s;
struct controllers_lib_s;
struct controllers_task_s;
struct controllers_params_s;
struct controllers_s;

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////////// CALLBACKS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

typedef void (*controllers_on_command_cb)(void *userData, struct controller_command_s *command);

typedef void (*controllers_command_action_done_cb)(struct controller_command_s *command);
typedef void (*controllers_library_action_done_cb)(struct controller_library_s *library);

/* -------------------------------------------------------------------------------------------- */
/* ///////////////////////////////////// PUBLIC FUNCTIONS ///////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

typedef enum controllers_error_e (*controllers_load_libs_f)(struct controllers_s *obj);
typedef enum controllers_error_e (*controllers_unload_libs_f)(struct controllers_s *obj);

typedef enum controllers_error_e (*controllers_init_task_f)(struct controllers_s *obj);
typedef enum controllers_error_e (*controllers_uninit_task_f)(struct controllers_s *obj);
typedef enum controllers_error_e (*controllers_start_task_f)(struct controllers_s *obj);
typedef enum controllers_error_e (*controllers_stop_task_f)(struct controllers_s *obj);

typedef enum controllers_error_e (*controllers_notify_f)(struct controllers_s *obj,
                                                         struct controller_event_s *event);

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// TYPES /////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum controllers_error_e {
    CONTROLLERS_ERROR_NONE,
    CONTROLLERS_ERROR_INIT,
    CONTROLLERS_ERROR_UNINIT,
    CONTROLLERS_ERROR_PARAMS,
    CONTROLLERS_ERROR_COMMAND,
    CONTROLLERS_ERROR_TASK,
    CONTROLLERS_ERROR_LIST,
    CONTROLLERS_ERROR_LIB
};

enum controllers_task_e {
    CONTROLLERS_TASK_CMDS,
    CONTROLLERS_TASK_EVTS,
    CONTROLLERS_TASK_LIBS,
    CONTROLLERS_TASK_COUNT
};

struct controllers_command_s {
    enum controller_command_e id;

    char                      *gfxElementName;
    void                      *gfxElementData;

    char                      *handlerName;
    char                      *handlerData;
};

struct controllers_lib_s {
    char                     *path;

    struct controller_s      *obj;
    void                     *handle;

    controller_init_f        init;
    controller_uninit_f      uninit;

    controller_on_command_cb onCommandCb;
    controller_on_event_cb   onEventCb;

    int32_t                  eventsMask;

    void                     *pData;
};

struct controllers_task_s {
    volatile uint8_t     quit;

    struct list_s        *list;

    struct task_s        *task;
    struct task_params_s taskParams;

    pthread_mutex_t      lock;
    sem_t                sem;
};

struct controllers_tasks_mngt_s {
    controllers_init_task_f   init;
    controllers_uninit_task_f uninit;
    controllers_start_task_f  start;
    controllers_stop_task_f   stop;

    struct controllers_task_s task;
};

struct controllers_params_s {
    struct context_s          *ctx;
    controllers_on_command_cb onCommandCb;
    void                      *userData;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// MAIN CONTEXT /////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

struct controllers_s {
    controllers_load_libs_f   loadLibs;
    controllers_unload_libs_f unloadLibs;

    controllers_notify_f      notify;

    struct controllers_tasks_mngt_s tasksMngt[CONTROLLERS_TASK_COUNT];
    struct controllers_params_s     params;

    uint8_t                         nbLibs;
    struct controllers_lib_s        *libs;

    void                            *pData;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// INITIALIZER //////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum controllers_error_e Controllers_Init(struct controllers_s **obj,
                                          struct controllers_params_s *params);
enum controllers_error_e Controllers_UnInit(struct controllers_s **obj);

#ifdef __cplusplus
}
#endif

#endif //__CONTROLLERS_H__
