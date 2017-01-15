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
* \file   Task.c
* \brief  Tasks management
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include <pthread.h>
#include <sched.h>
#include <sys/prctl.h>

#include "utils/Log.h"
#include "utils/Task.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                            */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "TASK"

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

typedef struct TASK_PRIVATE_DATA_S {
    uint32_t  nbTasks;
} TASK_PRIVATE_DATA_S;

typedef struct TASK_RESERVED_DATA_S {
    sem_t     semQuit;

    sem_t     semStart;
    pthread_t taskId;
} TASK_RESERVED_DATA_S;

/* -------------------------------------------------------------------------------------------- */
/*                                          VARIABLES                                           */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                         PROTOTYPES                                           */
/* -------------------------------------------------------------------------------------------- */

static TASK_ERROR_E create_f (TASK_S *obj, TASK_PARAMS_S *params);
static TASK_ERROR_E destroy_f(TASK_S *obj, TASK_PARAMS_S *params);

static TASK_ERROR_E start_f(TASK_S *obj, TASK_PARAMS_S *params);
static TASK_ERROR_E stop_f (TASK_S *obj, TASK_PARAMS_S *params);

static void* loop(void *args);

/* -------------------------------------------------------------------------------------------- */
/*                                      PUBLIC FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
TASK_ERROR_E Task_Init(TASK_S **obj)
{
    assert(obj && (*obj = calloc(1, sizeof(TASK_S))));
    
    TASK_PRIVATE_DATA_S *pData;
    assert((pData   = calloc(1, sizeof(TASK_PRIVATE_DATA_S))));
    
    /* Initialize obj */
    (*obj)->create  = create_f;
    (*obj)->destroy = destroy_f;
    (*obj)->start   = start_f;
    (*obj)->stop    = stop_f;
    
    pData->nbTasks  = 0;
    
    (*obj)->pData   = (void*)pData;
    
    return TASK_ERROR_NONE;
}

/*!
 *
 */
TASK_ERROR_E Task_UnInit(TASK_S **obj)
{
    assert(obj && *obj && (*obj)->pData);
    
    TASK_PRIVATE_DATA_S *pData = (TASK_PRIVATE_DATA_S*)((*obj)->pData);
    
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
/*                                     PRIVATE FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static TASK_ERROR_E create_f(TASK_S *obj, TASK_PARAMS_S *params)
{
    assert(obj && obj->pData && params);
    
    TASK_RESERVED_DATA_S *reserved;
    assert((reserved = calloc(1, sizeof(TASK_RESERVED_DATA_S))));
    
    if (sem_init(&reserved->semQuit, 0, 0) != 0) {
        Loge("sem_init() failed - %s", strerror(errno));
        goto semQuit_exit;
    }
    
    if (sem_init(&reserved->semStart, 0, 0) != 0) {
        Loge("sem_init() failed - %s", strerror(errno));
        goto semStart_exit;
    }
    
    /* Create loop */
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
        if (pthread_attr_setschedparam(&attr, &schedParam) < 0) { /* Set new sched_param */
            Loge("pthread_attr_setschedparam() failed - %s", strerror(errno));
        }
        if (pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED) < 0) { /* Required to make attr taken into account */
            Loge("pthread_attr_setinheritsched() failed - %s", strerror(errno));
        }
    }
    
    params->reserved = (void*)reserved;

    if (pthread_create(&reserved->taskId, &attr, loop, params) != 0) {
        Loge("pthread_create() failed - %s", strerror(errno));
        goto posix_exit;
    }
    
    (void)pthread_attr_destroy(&attr);
    
    ((TASK_PRIVATE_DATA_S*)(obj->pData))->nbTasks++;
    
    return TASK_ERROR_NONE;

posix_exit:
    (void)pthread_attr_destroy(&attr);
    (void)sem_destroy(&reserved->semStart);

semStart_exit:
    (void)sem_destroy(&reserved->semQuit);

semQuit_exit:
    free(reserved);
    reserved = NULL;
    
    return TASK_ERROR_CREATE;
}

/*!
 *
 */
static TASK_ERROR_E destroy_f(TASK_S *obj, TASK_PARAMS_S *params)
{
    assert(obj && obj->pData && params && params->reserved);
    
    TASK_RESERVED_DATA_S *reserved = (TASK_RESERVED_DATA_S*)(params->reserved);
    
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
    
    ((TASK_PRIVATE_DATA_S*)(obj->pData))->nbTasks--;
    
    return TASK_ERROR_NONE;
}

/*!
 *
 */
static TASK_ERROR_E start_f(TASK_S *obj, TASK_PARAMS_S *params)
{
    assert(obj && obj->pData && params && params->reserved);
    
    sem_post(&((TASK_RESERVED_DATA_S*)(params->reserved))->semStart);
    
    return TASK_ERROR_NONE;
}

/*!
 *
 */
static TASK_ERROR_E stop_f(TASK_S *obj, TASK_PARAMS_S *params)
{
    assert(obj && obj->pData && params && params->reserved);
    
    TASK_RESERVED_DATA_S *reserved = (TASK_RESERVED_DATA_S*)(params->reserved);
    
    sem_post(&reserved->semQuit);
    
    if (pthread_join(reserved->taskId, NULL) != 0) {
        Loge("pthread_join() failed - %s", strerror(errno));
        return TASK_ERROR_STOP;
    }
    
    return TASK_ERROR_NONE;
}

/*!
 *
 */
static void* loop(void *args)
{
    assert(args);
    
    TASK_PARAMS_S *params          = (TASK_PARAMS_S*)args;
    TASK_RESERVED_DATA_S *reserved = (TASK_RESERVED_DATA_S*)(params->reserved);
    
    assert(reserved);
    
    /* Set task's name */
    if (strlen(params->name) != 0) {
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
