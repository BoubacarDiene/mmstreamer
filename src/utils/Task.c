//////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                              //
//              Copyright © 2016, 2018 Boubacar DIENE                                           //
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
* \file Task.c
* \brief Tasks management
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// HEADERS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#include <pthread.h>
#ifdef _POSIX_PRIORITY_SCHEDULING
#include <sched.h>
#endif
#include <sys/prctl.h>

#include "utils/Log.h"
#include "utils/Task.h"

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// MACROS ////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "Task"

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// TYPES /////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

struct task_private_data_s {
    uint32_t nbTasks;
};

struct task_reserved_data_s {
    sem_t     semQuit;

    sem_t     semStart;
    pthread_t taskId;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////// PUBLIC FUNCTIONS PROTOTYPES //////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

static enum task_error_e create_f(struct task_s *obj, struct task_params_s *params);
static enum task_error_e destroy_f(struct task_s *obj, struct task_params_s *params);

static enum task_error_e start_f(struct task_s *obj, struct task_params_s *params);
static enum task_error_e stop_f(struct task_s *obj, struct task_params_s *params);

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////// PRIVATE FUNCTIONS PROTOTYPES /////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

static void* loop(void *args);

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// INITIALIZER //////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
enum task_error_e Task_Init(struct task_s **obj)
{
    assert(obj && (*obj = calloc(1, sizeof(struct task_s))));
    
    struct task_private_data_s *pData;
    assert((pData = calloc(1, sizeof(struct task_private_data_s))));
    
    /* Initialize obj */
    (*obj)->create  = create_f;
    (*obj)->destroy = destroy_f;
    (*obj)->start   = start_f;
    (*obj)->stop    = stop_f;

    (*obj)->pData   = (void*)pData;
    
    return TASK_ERROR_NONE;
}

/*!
 *
 */
enum task_error_e Task_UnInit(struct task_s **obj)
{
    assert(obj && *obj && (*obj)->pData);
    
    struct task_private_data_s *pData = (struct task_private_data_s*)((*obj)->pData);
    
    if (pData->nbTasks != 0) {
        Loge("%u still alive", pData->nbTasks);
        return TASK_ERROR_UNINIT;
    }
    
    free(pData);
    pData = NULL;
    
    free(*obj);
    *obj = NULL;
    
    return TASK_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////// PUBLIC FUNCTIONS IMPLEMENTATION ///////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static enum task_error_e create_f(struct task_s *obj, struct task_params_s *params)
{
    assert(obj && obj->pData && params);
    
    struct task_reserved_data_s *reserved;
    assert((reserved = calloc(1, sizeof(struct task_reserved_data_s))));
    
    if (sem_init(&reserved->semQuit, 0, 0) != 0) {
        Loge("sem_init() failed - %s", strerror(errno));
        goto semQuitExit;
    }
    
    if (sem_init(&reserved->semStart, 0, 0) != 0) {
        Loge("sem_init() failed - %s", strerror(errno));
        goto semStartExit;
    }

    params->reserved = (void*)reserved;

    /* Create loop */
#ifdef _POSIX_PRIORITY_SCHEDULING
    int policy = SCHED_FIFO;
    pthread_attr_t attr;
    struct sched_param schedParam;

    uint8_t updateParams = 1;

    (void)pthread_attr_init(&attr);                        /* Init with default parameters */
    (void)pthread_attr_getschedparam (&attr, &schedParam); /* Get current sched_param */

    switch (params->priority) {
        case PRIORITY_LOWEST:
            schedParam.sched_priority = sched_get_priority_min(policy);
            break;
            
        case PRIORITY_DEFAULT:
            updateParams = 0;
            break;
            
        case PRIORITY_HIGHEST:
            schedParam.sched_priority = sched_get_priority_max(policy);
            break;
            
        default:
            ;
    }
    
    if (updateParams) {
        if (pthread_attr_setschedparam(&attr, &schedParam) < 0) {
            Loge("pthread_attr_setschedparam() failed - %s", strerror(errno));
        }
        if (pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED) < 0) {
            Loge("pthread_attr_setinheritsched() failed - %s", strerror(errno));
        }
    }

    if (pthread_create(&reserved->taskId, &attr, loop, params) != 0) {
        Loge("pthread_create() failed - %s", strerror(errno));
        (void)pthread_attr_destroy(&attr);
        goto pthreadExit;
    }
    
    (void)pthread_attr_destroy(&attr);
#else
    Loge("_POSIX_PRIORITY_SCHEDULING not defined on your system");
    if (pthread_create(&reserved->taskId, NULL, loop, params) != 0) {
        Loge("pthread_create() failed - %s", strerror(errno));
        goto pthreadExit;
    }
#endif
    
    ((struct task_private_data_s*)(obj->pData))->nbTasks++;
    
    return TASK_ERROR_NONE;

pthreadExit:
    (void)sem_destroy(&reserved->semStart);

semStartExit:
    (void)sem_destroy(&reserved->semQuit);

semQuitExit:
    free(reserved);
    reserved = NULL;
    
    return TASK_ERROR_CREATE;
}

/*!
 *
 */
static enum task_error_e destroy_f(struct task_s *obj, struct task_params_s *params)
{
    assert(obj && obj->pData && params && params->reserved);
    
    struct task_reserved_data_s *reserved = (struct task_reserved_data_s*)(params->reserved);
    
    if (sem_destroy(&reserved->semStart) != 0) {
        return TASK_ERROR_DESTROY;
    }
    
    if (sem_destroy(&reserved->semQuit) != 0) {
        return TASK_ERROR_DESTROY;
    }
    
    free(reserved);
    reserved = NULL;
    
    if (params->atExit) {
        params->atExit(params);
    }
    
    ((struct task_private_data_s*)(obj->pData))->nbTasks--;
    
    return TASK_ERROR_NONE;
}

/*!
 *
 */
static enum task_error_e start_f(struct task_s *obj, struct task_params_s *params)
{
    assert(obj && obj->pData && params && params->reserved);
    
    sem_post(&((struct task_reserved_data_s*)(params->reserved))->semStart);
    
    return TASK_ERROR_NONE;
}

/*!
 *
 */
static enum task_error_e stop_f(struct task_s *obj, struct task_params_s *params)
{
    assert(obj && obj->pData && params && params->reserved);
    
    struct task_reserved_data_s *reserved = (struct task_reserved_data_s*)(params->reserved);
    
    sem_post(&reserved->semQuit);
    
    if (pthread_join(reserved->taskId, NULL) != 0) {
        Loge("pthread_join() failed - %s", strerror(errno));
        return TASK_ERROR_STOP;
    }
    
    return TASK_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/*                               PRIVATE FUNCTIONS IMPLEMENTATION                               */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static void* loop(void *args)
{
    assert(args);
    
    struct task_params_s *params          = (struct task_params_s*)args;
    struct task_reserved_data_s *reserved = (struct task_reserved_data_s*)(params->reserved);
    
    assert(reserved);
    
    /* Set task's name */
    if ((params->name)[0] != '\0') {
        Logd("Task : \"%s\"", params->name);
        prctl(PR_SET_NAME, params->name, 0, 0, 0);
    }
    
    /* Wait until start is called */
    sem_wait(&reserved->semStart);
    
    /* Loop */
    while (sem_trywait(&reserved->semQuit) != 0) {
        params->fct(params);
    }
    
    return NULL;
}
