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
* \file Events.c
* \brief TODO
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// HEADERS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#include <time.h>

#include "control/Controllers.h"

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// MACROS ////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "Events"

#define EVENTS_TASK_NAME "eventsTask"

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// TYPES /////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

struct events_list_element_s {
    time_t                    seconds;
    struct controller_event_s event;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////// PUBLIC FUNCTIONS PROTOTYPES //////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum controllers_error_e initEvtsTask_f(struct controllers_s *obj);
enum controllers_error_e uninitEvtsTask_f(struct controllers_s *obj);
enum controllers_error_e startEvtsTask_f(struct controllers_s *obj);
enum controllers_error_e stopEvtsTask_f(struct controllers_s *obj);

enum controllers_error_e notify_f(struct controllers_s *obj, struct controller_event_s *event);

void registerEvents_f(void *userData, int32_t eventsMask);
void unregisterEvents_f(void *userData, int32_t eventsMask);

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////// PRIVATE FUNCTIONS PROTOTYPES /////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

static void taskFct_f(struct task_params_s *params);

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////////// CALLBACKS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

static uint8_t compareCb(struct list_s *obj, void *elementToCheck, void *userData);
static void releaseCb(struct list_s *obj, void *element);

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////// PUBLIC FUNCTIONS IMPLEMENTATION ///////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
enum controllers_error_e initEvtsTask_f(struct controllers_s *obj)
{
    assert(obj);

    struct controllers_task_s *evtsTask = &obj->tasksMngt[CONTROLLERS_TASK_EVTS].task;
    struct input_s *input               = &obj->params.ctx->input;
    uint8_t priority                    = input->ctrlLibsPrio;

    if (input->nbCtrlLibs == 0) {
        Logw("No library provided");
        return CONTROLLERS_ERROR_NONE;
    }

    Logd("Initialize evtsTask");

    struct list_params_s listParams = {0};
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
    evtsTask->taskParams.fctData  = obj;
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
enum controllers_error_e uninitEvtsTask_f(struct controllers_s *obj)
{
    assert(obj);

    struct controllers_task_s *evtsTask = &obj->tasksMngt[CONTROLLERS_TASK_EVTS].task;
    struct input_s *input               = &obj->params.ctx->input;

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
enum controllers_error_e startEvtsTask_f(struct controllers_s *obj)
{
    assert(obj);

    struct controllers_task_s *evtsTask = &obj->tasksMngt[CONTROLLERS_TASK_EVTS].task;
    struct input_s *input               = &obj->params.ctx->input;

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
enum controllers_error_e stopEvtsTask_f(struct controllers_s *obj)
{
    assert(obj);

    struct controllers_task_s *evtsTask = &obj->tasksMngt[CONTROLLERS_TASK_EVTS].task;
    struct input_s *input               = &obj->params.ctx->input;

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
enum controllers_error_e notify_f(struct controllers_s *obj, struct controller_event_s *event)
{
    assert(obj && event);

    struct controllers_task_s *evtsTask   = &obj->tasksMngt[CONTROLLERS_TASK_EVTS].task;
    struct list_s *evtsList               = evtsTask->list;
    struct events_list_element_s *element = NULL;
    struct input_s *input                 = &obj->params.ctx->input;

    if (obj->nbLibs == 0) {
        Logw("No library loaded");
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

    assert((element = calloc(1, sizeof(struct events_list_element_s))));
    element->seconds    = time(NULL);
    element->event.id   = event->id;
    element->event.name = strdup(event->name);

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

    struct controllers_lib_s *lib        = (struct controllers_lib_s*)userData;
    struct controllers_s *controllersObj = (struct controllers_s*)(lib->pData);
    struct controllers_task_s *evtsTask  = &controllersObj->tasksMngt[CONTROLLERS_TASK_EVTS].task;

    if (pthread_mutex_lock(&evtsTask->lock) != 0) {
        Loge("pthread_mutex_lock() failed");
        return;
    }

    Logd("Register events : %d", eventsMask);

    lib->eventsMask |= (eventsMask);

    (void)pthread_mutex_unlock(&evtsTask->lock);
}

/*!
 *
 */
void unregisterEvents_f(void *userData, int32_t eventsMask)
{
    assert(userData);

    struct controllers_lib_s *lib        = (struct controllers_lib_s*)userData;
    struct controllers_s *controllersObj = (struct controllers_s*)(lib->pData);
    struct controllers_task_s *evtsTask  = &controllersObj->tasksMngt[CONTROLLERS_TASK_EVTS].task;

    if (pthread_mutex_lock(&evtsTask->lock) != 0) {
        Loge("pthread_mutex_lock() failed");
        return;
    }

    Logd("Unregister events : %d", eventsMask);

    lib->eventsMask &= ~(eventsMask);

    (void)pthread_mutex_unlock(&evtsTask->lock);
}

/* -------------------------------------------------------------------------------------------- */
/* ///////////////////////////// PRIVATE FUNCTIONS IMPLEMENTATION ///////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

static void taskFct_f(struct task_params_s *params)
{
    assert(params && params->fctData);

    struct controllers_s *controllersObj  = (struct controllers_s*)(params->fctData);
    struct controllers_task_s *evtsTask   = &controllersObj->tasksMngt[CONTROLLERS_TASK_EVTS].task;
    struct list_s *evtsList               = evtsTask->list;
    struct events_list_element_s *element = NULL;

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

    struct controllers_lib_s *lib = NULL;
    uint8_t i;
    for (i = 0; i < controllersObj->nbLibs; ++i) {
        lib = &controllersObj->libs[i];
        if (element->event.id & lib->eventsMask) {
            (void)pthread_mutex_unlock(&evtsTask->lock);
            lib->onEventCb(lib->obj, &element->event);
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

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////////// CALLBACKS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

static uint8_t compareCb(struct list_s *obj, void *elementToCheck, void *userData)
{
    assert(obj && elementToCheck && userData);

    struct events_list_element_s *element = (struct events_list_element_s*)elementToCheck;
    time_t secondsOfElementToRemove       = *((time_t*)userData);

    return (element->seconds == secondsOfElementToRemove);
}

static void releaseCb(struct list_s *obj, void *element)
{
    assert(obj && element);

    struct events_list_element_s *elementToRemove = (struct events_list_element_s*)element;
    struct controller_event_s *event              = &elementToRemove->event;

    if (event->name) {
        free(event->name);
        event->name = NULL;
    }

    free(elementToRemove);
    elementToRemove = NULL;
}
