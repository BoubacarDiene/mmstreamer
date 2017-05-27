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
* \file   Controllers.c
* \brief  TODO
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include <time.h>

#include "control/Controllers.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                            */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "Events"

#define EVENTS_TASK_NAME "eventsTask"

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

typedef struct EVENTS_LIST_ELEMENT_S {
    time_t             seconds;
    CONTROLLER_EVENT_S event;
} EVENTS_LIST_ELEMENT_S;

/* -------------------------------------------------------------------------------------------- */
/*                                         PROTOTYPES                                           */
/* -------------------------------------------------------------------------------------------- */

CONTROLLERS_ERROR_E initEvtsTask_f  (CONTROLLERS_S *obj);
CONTROLLERS_ERROR_E uninitEvtsTask_f(CONTROLLERS_S *obj);
CONTROLLERS_ERROR_E startEvtsTask_f (CONTROLLERS_S *obj);
CONTROLLERS_ERROR_E stopEvtsTask_f  (CONTROLLERS_S *obj);

void registerEvents_f  (void *userData, int32_t eventsMask);
void unregisterEvents_f(void *userData, int32_t eventsMask);

static void    taskFct_f(TASK_PARAMS_S *params);
static uint8_t compareCb(LIST_S *obj, void *elementToCheck, void *userData);
static void    releaseCb(LIST_S *obj, void *element);

/* -------------------------------------------------------------------------------------------- */
/*                                          VARIABLES                                           */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                      PUBLIC FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
CONTROLLERS_ERROR_E initEvtsTask_f(CONTROLLERS_S *obj)
{
    assert(obj && obj->pData);

    CONTROLLERS_PRIVATE_DATA_S *pData = (CONTROLLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTROLLERS_TASK_S *evtsTask      = &pData->evtsTask;
    INPUT_S *input                    = &pData->params.ctx->input;
    uint8_t priority                  = input->ctrlLibsPrio;

    if (input->nbCtrlLibs == 0) {
        Logw("No library provided");
        return CONTROLLERS_ERROR_NONE;
    }

    Logd("Initialize evtsTask");

    LIST_PARAMS_S listParams = { 0 };
    listParams.compareCb = compareCb;
    listParams.releaseCb = releaseCb;
    listParams.browseCb  = NULL;

    if (List_Init(&evtsTask->list, &listParams) != LIST_ERROR_NONE) {
        Loge("List_Init() failed");
        goto listExit;
    }

    if (Task_Init(&evtsTask->task) != TASK_ERROR_NONE) {
        Loge("Task_Init() failed");
        goto taskExit;
    }

    if (pthread_mutex_init(&evtsTask->lock, NULL) != 0) {
        Loge("pthread_mutex_init() failed");
        goto lockExit;
    }

    if (sem_init(&evtsTask->sem, 0, 0) != 0) {
        Loge("sem_init() failed");
        goto semExit;
    }

    strcpy(evtsTask->taskParams.name, EVENTS_TASK_NAME);
    evtsTask->taskParams.priority = priority;
    evtsTask->taskParams.fct      = taskFct_f;
    evtsTask->taskParams.fctData  = pData;
    evtsTask->taskParams.userData = NULL;
    evtsTask->taskParams.atExit   = NULL;

    if (evtsTask->task->create(evtsTask->task, &evtsTask->taskParams) != TASK_ERROR_NONE) {
        Loge("Failed to create evts task");
        goto taskCreateExit;
    }

    return CONTROLLERS_ERROR_NONE;

taskCreateExit:
    (void)sem_destroy(&evtsTask->sem);
    
semExit:
    (void)pthread_mutex_destroy(&evtsTask->lock);

lockExit:
    (void)Task_UnInit(&evtsTask->task);

taskExit:
    (void)List_UnInit(&evtsTask->list);

listExit:
    return CONTROLLERS_ERROR_TASK;
}

/*!
 *
 */
CONTROLLERS_ERROR_E uninitEvtsTask_f(CONTROLLERS_S *obj)
{
    assert(obj && obj->pData);

    CONTROLLERS_PRIVATE_DATA_S *pData = (CONTROLLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTROLLERS_TASK_S *evtsTask      = &pData->evtsTask;
    INPUT_S *input                    = &pData->params.ctx->input;

    if (input->nbCtrlLibs == 0) {
        Logw("No library provided");
        return CONTROLLERS_ERROR_NONE;
    }

    Logd("Uninitialize evtsTask");

    (void)evtsTask->list->lock(evtsTask->list);
    (void)evtsTask->list->removeAll(evtsTask->list);
    (void)evtsTask->list->unlock(evtsTask->list);

    (void)evtsTask->task->destroy(evtsTask->task, &evtsTask->taskParams);

    (void)sem_destroy(&evtsTask->sem);
    (void)pthread_mutex_destroy(&evtsTask->lock);

    (void)Task_UnInit(&evtsTask->task);
    (void)List_UnInit(&evtsTask->list);

    return CONTROLLERS_ERROR_NONE;
}

/*!
 *
 */
CONTROLLERS_ERROR_E startEvtsTask_f(CONTROLLERS_S *obj)
{
    assert(obj && obj->pData);

    CONTROLLERS_PRIVATE_DATA_S *pData = (CONTROLLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTROLLERS_TASK_S *evtsTask      = &pData->evtsTask;
    INPUT_S *input                    = &pData->params.ctx->input;

    if (input->nbCtrlLibs == 0) {
        Logw("No library provided");
        return CONTROLLERS_ERROR_NONE;
    }

    Logd("Start evtsTask");

    if (evtsTask->task->start(evtsTask->task, &evtsTask->taskParams) != TASK_ERROR_NONE) {
        Loge("Failed to start evtsTask");
        return CONTROLLERS_ERROR_TASK;
    }

    return CONTROLLERS_ERROR_NONE;
}

/*!
 *
 */
CONTROLLERS_ERROR_E stopEvtsTask_f(CONTROLLERS_S *obj)
{
    assert(obj && obj->pData);

    CONTROLLERS_PRIVATE_DATA_S *pData = (CONTROLLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTROLLERS_TASK_S *evtsTask      = &pData->evtsTask;
    INPUT_S *input                    = &pData->params.ctx->input;

    if (input->nbCtrlLibs == 0) {
        Logw("No library provided");
        return CONTROLLERS_ERROR_NONE;
    }

    Logd("Stop evtsTask");

    evtsTask->quit = 1;
    sem_post(&evtsTask->sem);

    if (evtsTask->task->stop(evtsTask->task, &evtsTask->taskParams) != TASK_ERROR_NONE) {
        Loge("Failed to stop evtsTask");
        return CONTROLLERS_ERROR_TASK;
    }

    return CONTROLLERS_ERROR_NONE;
}

/*!
 *
 */
CONTROLLERS_ERROR_E notify_f(CONTROLLERS_S *obj, CONTROLLER_EVENT_S *event)
{
    assert(obj && obj->pData && event);

    CONTROLLERS_PRIVATE_DATA_S *pData = (CONTROLLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTROLLERS_TASK_S *evtsTask      = &pData->evtsTask;
    LIST_S *evtsList                  = evtsTask->list;
    EVENTS_LIST_ELEMENT_S *element    = NULL;
    INPUT_S *input                    = &pData->params.ctx->input;

    if (pData->nbLibs == 0) {
        Logw("No library provided");
        return CONTROLLERS_ERROR_NONE;
    }

    if (evtsTask->quit) {
        Loge("Controllers are (being) stopped");
        return CONTROLLERS_ERROR_INIT;
    }

    if (evtsList->lock(evtsList) != LIST_ERROR_NONE) {
        Loge("Failed to lock events list");
        return CONTROLLERS_ERROR_LIST;
    }

    Logd("Notify contoller - event id : \"%u\"", event->id);

    assert((element = calloc(1, sizeof(EVENTS_LIST_ELEMENT_S))));
    element->seconds = time(NULL);
    memcpy(&element->event, event, sizeof(CONTROLLER_EVENT_S));

    (void)evtsList->add(evtsList, (void*)element);

    (void)evtsList->unlock(evtsList);

    sem_post(&evtsTask->sem);

    return CONTROLLERS_ERROR_NONE;
}

/*!
 *
 */
void registerEvents_f(void *userData, int32_t eventsMask)
{
    assert(userData);

    CONTROLLERS_LIB_S *lib            = (CONTROLLERS_LIB_S*)userData;
    CONTROLLERS_PRIVATE_DATA_S *pData = lib->pData;
    CONTROLLERS_TASK_S *evtsTask      = &pData->evtsTask;

    if (pthread_mutex_lock(&evtsTask->lock) != 0) {
        Loge("pthread_mutex_lock() failed");
        return;
    }

    Logd("Register events : %d", eventsMask);

    lib->eventsMask |= eventsMask;

    (void)pthread_mutex_unlock(&evtsTask->lock);
}

/*!
 *
 */
void unregisterEvents_f(void *userData, int32_t eventsMask)
{
    assert(userData);

    CONTROLLERS_LIB_S *lib            = (CONTROLLERS_LIB_S*)userData;
    CONTROLLERS_PRIVATE_DATA_S *pData = lib->pData;
    CONTROLLERS_TASK_S *evtsTask      = &pData->evtsTask;

    if (pthread_mutex_lock(&evtsTask->lock) != 0) {
        Loge("pthread_mutex_lock() failed");
        return;
    }

    Logd("Unregister events : %d", eventsMask);

    lib->eventsMask &= ~(eventsMask);

    (void)pthread_mutex_unlock(&evtsTask->lock);
}

/* -------------------------------------------------------------------------------------------- */
/*                                     PRIVATE FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

static void taskFct_f(TASK_PARAMS_S *params)
{
    assert(params && params->fctData);

    CONTROLLERS_PRIVATE_DATA_S *pData = (CONTROLLERS_PRIVATE_DATA_S*)(params->fctData);
    CONTROLLERS_TASK_S *evtsTask      = &pData->evtsTask;
    LIST_S *evtsList                  = evtsTask->list;
    EVENTS_LIST_ELEMENT_S *element    = NULL;

    if (evtsTask->quit) {
        return;
    }

    sem_wait(&evtsTask->sem);

    if (evtsTask->quit) {
        return;
    }

    if (evtsList->lock(evtsList) != LIST_ERROR_NONE) {
        Loge("Failed to lock events list");
        goto lockExit;
    }

    if (evtsList->getElement(evtsList, (void**)&element) != LIST_ERROR_NONE) {
        Loge("Failed to retrieve element from events list");
        goto getElementExit;
    }

    (void)evtsList->unlock(evtsList);

    if (pthread_mutex_lock(&evtsTask->lock) != 0) {
        Loge("pthread_mutex_lock() failed");
        goto lockExit;
    }

    CONTROLLERS_LIB_S *lib = NULL;
    uint8_t i;
    for (i = 0; i < pData->nbLibs; ++i) {
        lib = &pData->libs[i];
        if (element->event.id & lib->eventsMask) {
            (void)pthread_mutex_unlock(&evtsTask->lock);
            lib->notify(lib->obj, &element->event);
            (void)pthread_mutex_lock(&evtsTask->lock);
            break;
        }
    }

    (void)pthread_mutex_unlock(&evtsTask->lock);

    (void)evtsList->lock(evtsList);
    (void)evtsList->remove(evtsList, (void*)&element->seconds);
    (void)evtsList->unlock(evtsList);

    return;

getElementExit:
    (void)evtsList->unlock(evtsList);

lockExit:
    sem_post(&evtsTask->sem); // Force retry
}

static uint8_t compareCb(LIST_S *obj, void *elementToCheck, void *userData)
{
    assert(obj && elementToCheck && userData);

    EVENTS_LIST_ELEMENT_S *element  = (EVENTS_LIST_ELEMENT_S*)elementToCheck;
    time_t secondsOfElementToRemove = *((time_t*)userData);

    return (element->seconds == secondsOfElementToRemove);
}

static void releaseCb(LIST_S *obj, void *element)
{
    assert(obj && element);

    EVENTS_LIST_ELEMENT_S *elementToRemove = (EVENTS_LIST_ELEMENT_S*)element;

    free(elementToRemove);
    elementToRemove = NULL;
}
