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
* \file   Commands.c
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
#define TAG "Commands"

#define COMMANDS_TASK_NAME "commandsTask"

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

typedef struct COMMANDS_LIST_ELEMENT_S {
    time_t               seconds;
    CONTROLLER_COMMAND_S command;
} COMMANDS_LIST_ELEMENT_S;

/* -------------------------------------------------------------------------------------------- */
/*                                         PROTOTYPES                                           */
/* -------------------------------------------------------------------------------------------- */

CONTROLLERS_ERROR_E initCmdsTask_f  (CONTROLLERS_S *obj);
CONTROLLERS_ERROR_E uninitCmdsTask_f(CONTROLLERS_S *obj);
CONTROLLERS_ERROR_E startCmdsTask_f (CONTROLLERS_S *obj);
CONTROLLERS_ERROR_E stopCmdsTask_f  (CONTROLLERS_S *obj);

void sendCommand_f(void *userData, CONTROLLER_COMMAND_S *command);

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
CONTROLLERS_ERROR_E initCmdsTask_f(CONTROLLERS_S *obj)
{
    assert(obj && obj->pData);

    CONTROLLERS_PRIVATE_DATA_S *pData = (CONTROLLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTROLLERS_TASK_S *cmdsTask      = &pData->cmdsTask;
    INPUT_S *input                    = &pData->params.ctx->input;
    uint8_t priority                  = input->ctrlLibsPrio;

    if (input->nbCtrlLibs == 0) {
        Logw("No library provided");
        return CONTROLLERS_ERROR_NONE;
    }

    Logd("Initialize cmdsTask");

    LIST_PARAMS_S listParams = { 0 };
    listParams.compareCb = compareCb;
    listParams.releaseCb = releaseCb;
    listParams.browseCb  = NULL;

    if (List_Init(&cmdsTask->list, &listParams) != LIST_ERROR_NONE) {
        Loge("List_Init() failed");
        goto listExit;
    }

    if (Task_Init(&cmdsTask->task) != TASK_ERROR_NONE) {
        Loge("Task_Init() failed");
        goto taskExit;
    }

    if (sem_init(&cmdsTask->sem, 0, 0) != 0) {
        Loge("sem_init() failed");
        goto semExit;
    }

    strcpy(cmdsTask->taskParams.name, COMMANDS_TASK_NAME);
    cmdsTask->taskParams.priority = priority;
    cmdsTask->taskParams.fct      = taskFct_f;
    cmdsTask->taskParams.fctData  = pData;
    cmdsTask->taskParams.userData = NULL;
    cmdsTask->taskParams.atExit   = NULL;

    if (cmdsTask->task->create(cmdsTask->task, &cmdsTask->taskParams) != TASK_ERROR_NONE) {
        Loge("Failed to create cmds task");
        goto taskCreateExit;
    }

    return CONTROLLERS_ERROR_NONE;

taskCreateExit:
    (void)sem_destroy(&cmdsTask->sem);
    
semExit:
    (void)Task_UnInit(&cmdsTask->task);

taskExit:
    (void)List_UnInit(&cmdsTask->list);

listExit:
    return CONTROLLERS_ERROR_TASK;
}

/*!
 *
 */
CONTROLLERS_ERROR_E uninitCmdsTask_f(CONTROLLERS_S *obj)
{
    assert(obj && obj->pData);

    CONTROLLERS_PRIVATE_DATA_S *pData = (CONTROLLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTROLLERS_TASK_S *cmdsTask      = &pData->cmdsTask;
    INPUT_S *input                    = &pData->params.ctx->input;

    if (input->nbCtrlLibs == 0) {
        Logw("No library provided");
        return CONTROLLERS_ERROR_NONE;
    }

    Logd("Uninitialize cmdsTask");

    (void)cmdsTask->list->lock(cmdsTask->list);
    (void)cmdsTask->list->removeAll(cmdsTask->list);
    (void)cmdsTask->list->unlock(cmdsTask->list);

    (void)cmdsTask->task->destroy(cmdsTask->task, &cmdsTask->taskParams);

    (void)sem_destroy(&cmdsTask->sem);

    (void)Task_UnInit(&cmdsTask->task);
    (void)List_UnInit(&cmdsTask->list);

    return CONTROLLERS_ERROR_NONE;
}

/*!
 *
 */
CONTROLLERS_ERROR_E startCmdsTask_f(CONTROLLERS_S *obj)
{
    assert(obj && obj->pData);

    CONTROLLERS_PRIVATE_DATA_S *pData = (CONTROLLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTROLLERS_TASK_S *cmdsTask      = &pData->cmdsTask;
    INPUT_S *input                    = &pData->params.ctx->input;

    if (input->nbCtrlLibs == 0) {
        Logw("No library provided");
        return CONTROLLERS_ERROR_NONE;
    }

    Logd("Start cmdsTask");

    if (cmdsTask->task->start(cmdsTask->task, &cmdsTask->taskParams) != TASK_ERROR_NONE) {
        Loge("Failed to start cmdsTask");
        return CONTROLLERS_ERROR_TASK;
    }

    return CONTROLLERS_ERROR_NONE;
}

/*!
 *
 */
CONTROLLERS_ERROR_E stopCmdsTask_f(CONTROLLERS_S *obj)
{
    assert(obj && obj->pData);

    CONTROLLERS_PRIVATE_DATA_S *pData = (CONTROLLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTROLLERS_TASK_S *cmdsTask      = &pData->cmdsTask;
    INPUT_S *input                    = &pData->params.ctx->input;

    if (input->nbCtrlLibs == 0) {
        Logw("No library provided");
        return CONTROLLERS_ERROR_NONE;
    }

    Logd("Stop cmdsTask");

    cmdsTask->quit = 1;
    sem_post(&cmdsTask->sem);

    if (cmdsTask->task->stop(cmdsTask->task, &cmdsTask->taskParams) != TASK_ERROR_NONE) {
        Loge("Failed to stop cmdsTask");
        return CONTROLLERS_ERROR_TASK;
    }

    return CONTROLLERS_ERROR_NONE;
}

/*!
 *+
 */
void sendCommand_f(void *userData, CONTROLLER_COMMAND_S *command)
{
    assert(userData && command);

    CONTROLLERS_LIB_S *lib            = (CONTROLLERS_LIB_S*)userData;
    CONTROLLERS_PRIVATE_DATA_S *pData = (CONTROLLERS_PRIVATE_DATA_S*)(lib->pData);
    CONTROLLERS_TASK_S *cmdsTask      = &pData->cmdsTask;
    LIST_S *cmdsList                  = cmdsTask->list;
    COMMANDS_LIST_ELEMENT_S *element  = NULL;

    if (cmdsTask->quit) {
        Loge("Controllers are (being) stopped");
        return;
    }

    if (cmdsList->lock(cmdsList) != LIST_ERROR_NONE) {
        Loge("Failed to lock commands list");
        return;
    }

    Logd("Send command - id : \"%u\"", command->id);

    assert((element = calloc(1, sizeof(COMMANDS_LIST_ELEMENT_S))));
    element->seconds = time(NULL);
    memcpy(&element->command, command, sizeof(CONTROLLER_COMMAND_S));

    (void)cmdsList->add(cmdsList, (void*)element);

    (void)cmdsList->unlock(cmdsList);

    sem_post(&cmdsTask->sem);
}

/* -------------------------------------------------------------------------------------------- */
/*                                     PRIVATE FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

static void taskFct_f(TASK_PARAMS_S *params)
{
    assert(params && params->fctData);

    CONTROLLERS_PRIVATE_DATA_S *pData = (CONTROLLERS_PRIVATE_DATA_S*)(params->fctData);
    CONTROLLERS_TASK_S *cmdsTask      = &pData->cmdsTask;
    LIST_S *cmdsList                  = cmdsTask->list;
    COMMANDS_LIST_ELEMENT_S *element  = NULL;

    if (cmdsTask->quit) {
        return;
    }

    sem_wait(&cmdsTask->sem);

    if (cmdsTask->quit) {
        return;
    }

    if (cmdsList->lock(cmdsList) != LIST_ERROR_NONE) {
        Loge("Failed to lock commands list");
        goto lockExit;
    }

    if (cmdsList->getElement(cmdsList, (void**)&element) != LIST_ERROR_NONE) {
        Loge("Failed to retrieve element from commands list");
        goto getElementExit;
    }

    (void)cmdsList->unlock(cmdsList);

    CONTROLLERS_ON_COMMAND_CB cb = pData->params.onCommandCb;
    void *userData               = pData->params.userData;
    if (cb) {
        cb(userData, &element->command);
    }

    (void)cmdsList->lock(cmdsList);
    (void)cmdsList->remove(cmdsList, (void*)&element->seconds);
    (void)cmdsList->unlock(cmdsList);

    return;

getElementExit:
    (void)cmdsList->unlock(cmdsList);

lockExit:
    sem_post(&cmdsTask->sem); // Force retry
}

static uint8_t compareCb(LIST_S *obj, void *elementToCheck, void *userData)
{
    assert(obj && elementToCheck && userData);

    COMMANDS_LIST_ELEMENT_S *element = (COMMANDS_LIST_ELEMENT_S*)elementToCheck;
    time_t secondsOfElementToRemove  = *((time_t*)userData);

    return (element->seconds == secondsOfElementToRemove);
}

static void releaseCb(LIST_S *obj, void *element)
{
    assert(obj && element);

    COMMANDS_LIST_ELEMENT_S *elementToRemove = (COMMANDS_LIST_ELEMENT_S*)element;

    free(elementToRemove);
    elementToRemove = NULL;
}
