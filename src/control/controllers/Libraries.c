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
* \file   Libraries.c
* \brief  TODO
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include <dlfcn.h>

#include "control/Controllers.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                             */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "Libraries"

#define LIBRARIES_TASK_NAME "librariesTask"

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

typedef struct LIBRARIES_LIST_ELEMENT_S {
    time_t               seconds;
    CONTROLLER_LIBRARY_S *library;
} LIBRARIES_LIST_ELEMENT_S;

/* -------------------------------------------------------------------------------------------- */
/*                                         PROTOTYPES                                           */
/* -------------------------------------------------------------------------------------------- */

CONTROLLERS_ERROR_E initLibsTask_f  (CONTROLLERS_S *obj);
CONTROLLERS_ERROR_E uninitLibsTask_f(CONTROLLERS_S *obj);
CONTROLLERS_ERROR_E startLibsTask_f (CONTROLLERS_S *obj);
CONTROLLERS_ERROR_E stopLibsTask_f  (CONTROLLERS_S *obj);

void sendToLibrary_f(void *userData, CONTROLLER_LIBRARY_S *library);

CONTROLLERS_ERROR_E loadLibs_f  (CONTROLLERS_S *obj);
CONTROLLERS_ERROR_E unloadLibs_f(CONTROLLERS_S *obj);

extern void registerEvents_f  (void *userData, int32_t eventsMask);
extern void unregisterEvents_f(void *userData, int32_t eventsMask);

extern void sendToEngine_f (void *userData, CONTROLLER_COMMAND_S *command);

static void    taskFct_f(TASK_PARAMS_S *params);
static uint8_t compareCb(LIST_S *obj, void *elementToCheck, void *userData);
static void    releaseCb(LIST_S *obj, void *element);

/* -------------------------------------------------------------------------------------------- */
/*                                         VARIABLES                                            */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                      PUBLIC FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
CONTROLLERS_ERROR_E initLibsTask_f(CONTROLLERS_S *obj)
{
    assert(obj && obj->pData);

    CONTROLLERS_PRIVATE_DATA_S *pData = (CONTROLLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTROLLERS_TASK_S *libsTask      = &pData->libsTask;
    INPUT_S *input                    = &pData->params.ctx->input;
    uint8_t priority                  = input->ctrlLibsPrio;

    if (input->nbCtrlLibs == 0) {
        Logw("No library provided");
        return CONTROLLERS_ERROR_NONE;
    }

    Logd("Initialize libsTask");

    LIST_PARAMS_S listParams = { 0 };
    listParams.compareCb = compareCb;
    listParams.releaseCb = releaseCb;
    listParams.browseCb  = NULL;

    if (List_Init(&libsTask->list, &listParams) != LIST_ERROR_NONE) {
        Loge("List_Init() failed");
        goto listExit;
    }

    if (Task_Init(&libsTask->task) != TASK_ERROR_NONE) {
        Loge("Task_Init() failed");
        goto taskExit;
    }

    if (sem_init(&libsTask->sem, 0, 0) != 0) {
        Loge("sem_init() failed");
        goto semExit;
    }

    strcpy(libsTask->taskParams.name, LIBRARIES_TASK_NAME);
    libsTask->taskParams.priority = priority;
    libsTask->taskParams.fct      = taskFct_f;
    libsTask->taskParams.fctData  = pData;
    libsTask->taskParams.userData = NULL;
    libsTask->taskParams.atExit   = NULL;

    if (libsTask->task->create(libsTask->task, &libsTask->taskParams) != TASK_ERROR_NONE) {
        Loge("Failed to create libs task");
        goto taskCreateExit;
    }

    return CONTROLLERS_ERROR_NONE;

taskCreateExit:
    (void)sem_destroy(&libsTask->sem);

semExit:
    (void)Task_UnInit(&libsTask->task);

taskExit:
    (void)List_UnInit(&libsTask->list);

listExit:
    return CONTROLLERS_ERROR_TASK;
}

/*!
 *
 */
CONTROLLERS_ERROR_E uninitLibsTask_f(CONTROLLERS_S *obj)
{
    assert(obj && obj->pData);

    CONTROLLERS_PRIVATE_DATA_S *pData = (CONTROLLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTROLLERS_TASK_S *libsTask      = &pData->libsTask;
    INPUT_S *input                    = &pData->params.ctx->input;

    if (input->nbCtrlLibs == 0) {
        Logw("No library provided");
        return CONTROLLERS_ERROR_NONE;
    }

    Logd("Uninitialize libsTask");

    (void)libsTask->list->lock(libsTask->list);
    (void)libsTask->list->removeAll(libsTask->list);
    (void)libsTask->list->unlock(libsTask->list);

    (void)libsTask->task->destroy(libsTask->task, &libsTask->taskParams);

    (void)sem_destroy(&libsTask->sem);

    (void)Task_UnInit(&libsTask->task);
    (void)List_UnInit(&libsTask->list);

    return CONTROLLERS_ERROR_NONE;
}

/*!
 *
 */
CONTROLLERS_ERROR_E startLibsTask_f(CONTROLLERS_S *obj)
{
    assert(obj && obj->pData);

    CONTROLLERS_PRIVATE_DATA_S *pData = (CONTROLLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTROLLERS_TASK_S *libsTask      = &pData->libsTask;
    INPUT_S *input                    = &pData->params.ctx->input;

    if (input->nbCtrlLibs == 0) {
        Logw("No library provided");
        return CONTROLLERS_ERROR_NONE;
    }

    Logd("Start libsTask");

    if (libsTask->task->start(libsTask->task, &libsTask->taskParams) != TASK_ERROR_NONE) {
        Loge("Failed to start libsTask");
        return CONTROLLERS_ERROR_TASK;
    }

    return CONTROLLERS_ERROR_NONE;
}

/*!
 *
 */
CONTROLLERS_ERROR_E stopLibsTask_f(CONTROLLERS_S *obj)
{
    assert(obj && obj->pData);

    CONTROLLERS_PRIVATE_DATA_S *pData = (CONTROLLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTROLLERS_TASK_S *libsTask      = &pData->libsTask;
    INPUT_S *input                    = &pData->params.ctx->input;

    if (input->nbCtrlLibs == 0) {
        Logw("No library provided");
        return CONTROLLERS_ERROR_NONE;
    }

    Logd("Stop libsTask");

    libsTask->quit = 1;
    sem_post(&libsTask->sem);

    if (libsTask->task->stop(libsTask->task, &libsTask->taskParams) != TASK_ERROR_NONE) {
        Loge("Failed to stop libsTask");
        return CONTROLLERS_ERROR_TASK;
    }

    return CONTROLLERS_ERROR_NONE;
}

/*!
 *
 */
void sendToLibrary_f(void *userData, CONTROLLER_LIBRARY_S *library)
{
    assert(userData && library);

    CONTROLLERS_LIB_S *lib            = (CONTROLLERS_LIB_S*)userData;
    CONTROLLERS_PRIVATE_DATA_S *pData = (CONTROLLERS_PRIVATE_DATA_S*)(lib->pData);
    CONTROLLERS_TASK_S *libsTask      = &pData->libsTask;
    LIST_S *libsList                  = libsTask->list;
    LIBRARIES_LIST_ELEMENT_S *element = NULL;

    if (libsTask->quit) {
        Loge("Controllers are (being) stopped");
        return;
    }

    if (libsList->lock(libsList) != LIST_ERROR_NONE) {
        Loge("Failed to lock libraries list");
        return;
    }

    Logd("Send to library : \"%s\"", library->name);

    assert((element = calloc(1, sizeof(LIBRARIES_LIST_ELEMENT_S))));
    element->seconds = time(NULL);
    element->library = library;

    (void)libsList->add(libsList, (void*)element);

    (void)libsList->unlock(libsList);

    sem_post(&libsTask->sem);
}

/*!
 *
 */
CONTROLLERS_ERROR_E loadLibs_f(CONTROLLERS_S *obj)
{
    assert(obj && obj->pData);

    CONTROLLERS_PRIVATE_DATA_S *pData = (CONTROLLERS_PRIVATE_DATA_S*)(obj->pData);
    INPUT_S *input                    = &pData->params.ctx->input;
    LIBRARY_S *ctrlLibs               = input->ctrlLibs;
    uint8_t priority                  = input->ctrlLibsPrio;

    pData->nbLibs = input->nbCtrlLibs;
    if (pData->nbLibs == 0) {
        Logw("No library provided");
        return CONTROLLERS_ERROR_NONE;
    }

    Logd("Load \"%u\" controller(s)", pData->nbLibs);

    assert((pData->libs = (CONTROLLERS_LIB_S*)calloc(pData->nbLibs, sizeof(CONTROLLERS_LIB_S))));

    CONTROLLER_FUNCTIONS_S fcts;
    fcts.registerEvents   = registerEvents_f;
    fcts.unregisterEvents = unregisterEvents_f;
    fcts.sendToEngine     = sendToEngine_f;
    fcts.sendToLibrary    = sendToLibrary_f;

    CONTROLLERS_LIB_S *lib;
    uint8_t index, count;
    for (index = 0; index < pData->nbLibs; ++index) {
        lib = &pData->libs[index];

        if (access(ctrlLibs[index].path, F_OK) != 0) {
            Loge("\"%s\" not found", ctrlLibs[index].path);
            goto libExit;
        }

        lib->path = strdup(ctrlLibs[index].path);

        lib->handle = dlopen(ctrlLibs[index].path, RTLD_LAZY);
        if (!lib->handle) {
            Loge("Failed to load library \"%s\"", ctrlLibs[index].path);
            goto libExit;
        }

        lib->init = dlsym(lib->handle, ctrlLibs[index].initFn);
        if (!lib->init) {
            Loge("Failed to load init function \"%s\"", ctrlLibs[index].initFn);
            goto libExit;
        }

        lib->uninit = dlsym(lib->handle, ctrlLibs[index].uninitFn);
        if (!lib->uninit) {
            Loge("Failed to load uninit function \"%s\"", ctrlLibs[index].uninitFn);
            goto libExit;
        }

        lib->onCommand = dlsym(lib->handle, ctrlLibs[index].onCommandCb);
        if (!lib->onCommand) {
            Loge("Failed to load onCommandCb \"%s\"", ctrlLibs[index].onCommandCb);
            goto libExit;
        }

        lib->onEvent = dlsym(lib->handle, ctrlLibs[index].onEventCb);
        if (!lib->onEvent) {
            Loge("Failed to load onEventCb \"%s\"", ctrlLibs[index].onEventCb);
            goto libExit;
        }

        lib->pData    = pData;
        fcts.userData = lib;
        if (lib->init(&lib->obj, &fcts) != CONTROLLER_ERROR_NONE) {
            Loge("\"%s\" failed", ctrlLibs[index].initFn);
            goto libExit;
        }
    }

    return CONTROLLERS_ERROR_NONE;

libExit:
    while (index > 0) {
        count = index - 1;
        lib = &pData->libs[count];

        lib->uninit(&lib->obj);
        lib->obj = NULL;

        lib->init      = NULL;
        lib->uninit    = NULL;
        lib->onCommand = NULL;
        lib->onEvent   = NULL;
        lib->pData     = NULL;

        if (lib->handle) {
            dlclose(lib->handle);
            lib->handle = NULL;
        }

        if (lib->path) {
            free(lib->path);
            lib->path = NULL;
        }

        --index;
    }

    free(pData->libs);
    pData->libs = NULL;

    pData->nbLibs = 0;

    return CONTROLLERS_ERROR_LIB;
}

CONTROLLERS_ERROR_E unloadLibs_f(CONTROLLERS_S *obj)
{
    assert(obj && obj->pData);

    CONTROLLERS_PRIVATE_DATA_S *pData = (CONTROLLERS_PRIVATE_DATA_S*)(obj->pData);

    if (pData->nbLibs == 0) {
        goto exit;
    }

    Logd("Unload \"%u\" controller(s)", pData->nbLibs);

    CONTROLLERS_LIB_S *lib;
    uint8_t index;
    for (index = 0; index < pData->nbLibs; ++index) {
        lib = &pData->libs[index];

        lib->uninit(&lib->obj);
        lib->obj = NULL;

        lib->init      = NULL;
        lib->uninit    = NULL;
        lib->onCommand = NULL;
        lib->onEvent   = NULL;
        lib->pData     = NULL;

        if (lib->handle) {
            dlclose(lib->handle);
            lib->handle = NULL;
        }

        if (lib->path) {
            free(lib->path);
            lib->path = NULL;
        }
    }

    free(pData->libs);
    pData->libs = NULL;

exit:
    return CONTROLLERS_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/*                                     PRIVATE FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

static void taskFct_f(TASK_PARAMS_S *params)
{
    assert(params && params->fctData);

    CONTROLLERS_PRIVATE_DATA_S *pData = (CONTROLLERS_PRIVATE_DATA_S*)(params->fctData);
    CONTROLLERS_TASK_S *libsTask      = &pData->libsTask;
    LIST_S *libsList                  = libsTask->list;
    LIBRARIES_LIST_ELEMENT_S *element = NULL;

    if (libsTask->quit) {
        return;
    }

    sem_wait(&libsTask->sem);

    if (libsTask->quit) {
        return;
    }

    if (libsList->lock(libsList) != LIST_ERROR_NONE) {
        Loge("Failed to lock libraries list");
        goto lockExit;
    }

    if (libsList->getElement(libsList, (void**)&element) != LIST_ERROR_NONE) {
        Loge("Failed to retrieve element from libraries list");
        goto getElementExit;
    }

    (void)libsList->unlock(libsList);

    CONTROLLER_LIBRARY_S *library = element->library;
    CONTROLLERS_LIB_S *lib        = NULL;

    uint8_t i;
    for (i = 0; i < pData->nbLibs; ++i) {
        lib = &pData->libs[i];
        if (strstr(lib->path, library->name)) {
            Logd("\"%s\" found", library->name);
            lib->onCommand(lib->obj, library->data);
            break;
        }
    }

    if (i == pData->nbLibs) {
        Loge("\"%s\" not found", library->name);
    }

    (void)libsList->lock(libsList);
    (void)libsList->remove(libsList, (void*)&element->seconds);
    (void)libsList->unlock(libsList);

    return;

getElementExit:
    (void)libsList->unlock(libsList);

lockExit:
    sem_post(&libsTask->sem); // Force retry
}

static uint8_t compareCb(LIST_S *obj, void *elementToCheck, void *userData)
{
    assert(obj && elementToCheck && userData);

    LIBRARIES_LIST_ELEMENT_S *element = (LIBRARIES_LIST_ELEMENT_S*)elementToCheck;
    time_t secondsOfElementToRemove   = *((time_t*)userData);

    return (element->seconds == secondsOfElementToRemove);
}

static void releaseCb(LIST_S *obj, void *element)
{
    assert(obj && element);

    LIBRARIES_LIST_ELEMENT_S *elementToRemove = (LIBRARIES_LIST_ELEMENT_S*)element;
    CONTROLLER_LIBRARY_S *library             = elementToRemove->library;

    if (library->release) {
        library->release((void*)library);
    }

    elementToRemove->library = NULL;

    free(elementToRemove);
    elementToRemove = NULL;
}
