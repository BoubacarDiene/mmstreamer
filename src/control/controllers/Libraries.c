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
//////////////////////////////////////////////////////////////////////////////////////////////////

/*!
* \file Libraries.c
* \brief TODO
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// HEADERS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#include <dlfcn.h>

#include "control/Controllers.h"

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// MACROS ////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "Libraries"

#define LIBRARIES_TASK_NAME "librariesTask"

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// TYPES /////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

struct libraries_list_element_s {
    time_t                             seconds;
    struct controller_library_s        *library;

    controllers_library_action_done_cb actionDoneCb;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////// PUBLIC FUNCTIONS PROTOTYPES //////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum controllers_error_e initLibsTask_f(struct controllers_s *obj);
enum controllers_error_e uninitLibsTask_f(struct controllers_s *obj);
enum controllers_error_e startLibsTask_f(struct controllers_s *obj);
enum controllers_error_e stopLibsTask_f(struct controllers_s *obj);

extern void registerEvents_f(void *enginePrivateData, int32_t eventsMask);
extern void unregisterEvents_f(void *enginePrivateData, int32_t eventsMask);

void sendToLibrary_f(void *enginePrivateData, struct controller_library_s *library,
                     controllers_library_action_done_cb actionDoneCb);
extern void sendToEngine_f(void *enginePrivateData, struct controller_command_s *command,
                           controllers_command_action_done_cb actionDoneCb);

enum controllers_error_e loadLibs_f(struct controllers_s *obj);
enum controllers_error_e unloadLibs_f(struct controllers_s *obj);

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////// PRIVATE FUNCTIONS PROTOTYPES /////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

static void taskFct_f(struct task_params_s *params);

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////////// CALLBACKS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

static uint8_t compareCb(struct list_s *obj, void *elementToCheck, const void *userData);
static void releaseCb(struct list_s *obj, void *element);

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////// PUBLIC FUNCTIONS IMPLEMENTATION ///////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
enum controllers_error_e initLibsTask_f(struct controllers_s *obj)
{
    ASSERT(obj);

    struct controllers_task_s *libsTask = &obj->tasksMngt[CONTROLLERS_TASK_LIBS].task;
    struct input_s *input               = &obj->params.ctx->input;
    uint8_t priority                    = input->ctrlLibsPrio;

    if (input->nbCtrlLibs == 0) {
        Logw("No library provided");
        return CONTROLLERS_ERROR_NONE;
    }

    Logd("Initialize libsTask");

    struct list_callbacks_s listCallbacks = {0};
    listCallbacks.compareCb = compareCb;
    listCallbacks.releaseCb = releaseCb;
    listCallbacks.browseCb  = NULL;

    if (List_Init(&libsTask->list, &listCallbacks) != LIST_ERROR_NONE) {
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
    libsTask->taskParams.fctData  = obj;
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
enum controllers_error_e uninitLibsTask_f(struct controllers_s *obj)
{
    ASSERT(obj);

    struct controllers_task_s *libsTask = &obj->tasksMngt[CONTROLLERS_TASK_LIBS].task;
    struct input_s *input               = &obj->params.ctx->input;

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
enum controllers_error_e startLibsTask_f(struct controllers_s *obj)
{
    ASSERT(obj);

    struct controllers_task_s *libsTask = &obj->tasksMngt[CONTROLLERS_TASK_LIBS].task;
    struct input_s *input               = &obj->params.ctx->input;

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
enum controllers_error_e stopLibsTask_f(struct controllers_s *obj)
{
    ASSERT(obj);

    struct controllers_task_s *libsTask = &obj->tasksMngt[CONTROLLERS_TASK_LIBS].task;
    struct input_s *input               = &obj->params.ctx->input;

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
void sendToLibrary_f(void *enginePrivateData, struct controller_library_s *library,
                     controllers_library_action_done_cb actionDoneCb)
{
    ASSERT(enginePrivateData && library);

    struct controllers_lib_s *lib            = (struct controllers_lib_s*)enginePrivateData;
    struct controllers_s *controllersObj     = (struct controllers_s*)(lib->pData);
    struct controllers_task_s *libsTask      = &controllersObj->tasksMngt[CONTROLLERS_TASK_LIBS].task;
    struct list_s *libsList                  = libsTask->list;
    struct libraries_list_element_s *element = NULL;

    if (libsTask->quit) {
        Loge("Controllers are (being) stopped");
        return;
    }

    if (libsList->lock(libsList) != LIST_ERROR_NONE) {
        Loge("Failed to lock libraries list");
        return;
    }

    Logd("Send to library : \"%s\"", library->name);

    ASSERT((element = calloc(1, sizeof(struct libraries_list_element_s))));
    element->seconds      = time(NULL);
    element->library      = library;
    element->actionDoneCb = actionDoneCb;

    (void)libsList->add(libsList, (void*)element);

    (void)libsList->unlock(libsList);

    sem_post(&libsTask->sem);
}

/*!
 *
 */
enum controllers_error_e loadLibs_f(struct controllers_s *obj)
{
    ASSERT(obj);

    struct input_s *input      = &obj->params.ctx->input;
    struct library_s *ctrlLibs = input->ctrlLibs;

    obj->nbLibs = input->nbCtrlLibs;
    if (obj->nbLibs == 0) {
        Logw("No library provided");
        return CONTROLLERS_ERROR_NONE;
    }

    Logd("Load \"%u\" controller(s)", obj->nbLibs);

    ASSERT((obj->libs = calloc(obj->nbLibs, sizeof(struct controllers_lib_s))));

    struct controller_functions_s fcts;
    fcts.registerEvents   = registerEvents_f;
    fcts.unregisterEvents = unregisterEvents_f;
    fcts.sendToEngine     = sendToEngine_f;
    fcts.sendToLibrary    = sendToLibrary_f;

    struct controllers_lib_s *lib;
    uint8_t index, count;
    for (index = 0; index < obj->nbLibs; ++index) {
        lib = &obj->libs[index];

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

        lib->onCommandCb = dlsym(lib->handle, ctrlLibs[index].onCommandCb);
        if (!lib->onCommandCb) {
            Loge("Failed to load onCommandCb \"%s\"", ctrlLibs[index].onCommandCb);
            goto libExit;
        }

        lib->onEventCb = dlsym(lib->handle, ctrlLibs[index].onEventCb);
        if (!lib->onEventCb) {
            Loge("Failed to load onEventCb \"%s\"", ctrlLibs[index].onEventCb);
            goto libExit;
        }

        lib->pData             = obj;
        fcts.enginePrivateData = lib;
        if (lib->init(&lib->obj, &fcts) != CONTROLLER_ERROR_NONE) {
            Loge("\"%s\" failed", ctrlLibs[index].initFn);
            goto libExit;
        }
    }

    return CONTROLLERS_ERROR_NONE;

libExit:
    while (index > 0) {
        count = (uint8_t)(index - 1);
        lib = &obj->libs[count];

        lib->uninit(&lib->obj);

        if (lib->handle) {
            dlclose(lib->handle);
        }

        if (lib->path) {
            free(lib->path);
        }

        --index;
    }

    free(obj->libs);
    obj->libs = NULL;

    obj->nbLibs = 0;

    return CONTROLLERS_ERROR_LIB;
}

enum controllers_error_e unloadLibs_f(struct controllers_s *obj)
{
    ASSERT(obj);

    if (obj->nbLibs == 0) {
        goto exit;
    }

    Logd("Unload \"%u\" controller(s)", obj->nbLibs);

    struct controllers_lib_s *lib;
    uint8_t index;
    for (index = 0; index < obj->nbLibs; ++index) {
        lib = &obj->libs[index];

        lib->uninit(&lib->obj);

        if (lib->handle) {
            dlclose(lib->handle);
        }

        if (lib->path) {
            free(lib->path);
        }
    }

    free(obj->libs);
    obj->libs = NULL;

exit:
    return CONTROLLERS_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/* ///////////////////////////// PRIVATE FUNCTIONS IMPLEMENTATION ///////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

static void taskFct_f(struct task_params_s *params)
{
    ASSERT(params && params->fctData);

    struct controllers_s *controllersObj     = (struct controllers_s*)(params->fctData);
    struct controllers_task_s *libsTask      = &controllersObj->tasksMngt[CONTROLLERS_TASK_LIBS].task;
    struct list_s *libsList                  = libsTask->list;
    struct libraries_list_element_s *element = NULL;

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

    struct controller_library_s *library = element->library;
    struct controllers_lib_s *lib        = NULL;

    uint8_t i;
    for (i = 0; i < controllersObj->nbLibs; ++i) {
        lib = &controllersObj->libs[i];
        if (strstr(lib->path, library->name)) {
            Logd("\"%s\" found", library->name);
            lib->onCommandCb(lib->obj, library->data);
            break;
        }
    }

    if (i == controllersObj->nbLibs) {
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

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////////// CALLBACKS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

static uint8_t compareCb(struct list_s *obj, void *elementToCheck, const void *userData)
{
    ASSERT(obj && elementToCheck && userData);

    struct libraries_list_element_s *element = (struct libraries_list_element_s*)elementToCheck;
    time_t secondsOfElementToRemove          = *((time_t*)userData);

    return (element->seconds == secondsOfElementToRemove);
}

static void releaseCb(struct list_s *obj, void *element)
{
    ASSERT(obj && element);

    struct libraries_list_element_s *elementToRemove = (struct libraries_list_element_s*)element;

    if (elementToRemove->actionDoneCb) {
        elementToRemove->actionDoneCb(elementToRemove->library);
    }

    free(elementToRemove);
}
