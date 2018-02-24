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
* \file Commands.c
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
#define TAG "Commands"

#define COMMANDS_TASK_NAME "commandsTask"

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// TYPES /////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

struct commands_list_element_s {
    time_t                             seconds;
    struct controller_command_s        *command;

    controllers_command_action_done_cb actionDoneCb;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////// PUBLIC FUNCTIONS PROTOTYPES //////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum controllers_error_e initCmdsTask_f(struct controllers_s *obj);
enum controllers_error_e uninitCmdsTask_f(struct controllers_s *obj);
enum controllers_error_e startCmdsTask_f(struct controllers_s *obj);
enum controllers_error_e stopCmdsTask_f(struct controllers_s *obj);

void sendToEngine_f(void *userData, struct controller_command_s *command,
                    controllers_command_action_done_cb actionDoneCb);

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
enum controllers_error_e initCmdsTask_f(struct controllers_s *obj)
{
    assert(obj);

    struct controllers_task_s *cmdsTask = &obj->tasksMngt[CONTROLLERS_TASK_CMDS].task;
    struct input_s *input               = &obj->params.ctx->input;
    uint8_t priority                    = input->ctrlLibsPrio;

    if (input->nbCtrlLibs == 0) {
        Logw("No library provided");
        return CONTROLLERS_ERROR_NONE;
    }

    Logd("Initialize cmdsTask");

    struct list_params_s listParams = {0};
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
    cmdsTask->taskParams.fctData  = obj;
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
enum controllers_error_e uninitCmdsTask_f(struct controllers_s *obj)
{
    assert(obj);

    struct controllers_task_s *cmdsTask = &obj->tasksMngt[CONTROLLERS_TASK_CMDS].task;
    struct input_s *input               = &obj->params.ctx->input;

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
enum controllers_error_e startCmdsTask_f(struct controllers_s *obj)
{
    assert(obj);

    struct controllers_task_s *cmdsTask = &obj->tasksMngt[CONTROLLERS_TASK_CMDS].task;
    struct input_s *input               = &obj->params.ctx->input;

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
enum controllers_error_e stopCmdsTask_f(struct controllers_s *obj)
{
    assert(obj);

    struct controllers_task_s *cmdsTask = &obj->tasksMngt[CONTROLLERS_TASK_CMDS].task;
    struct input_s *input               = &obj->params.ctx->input;

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
void sendToEngine_f(void *userData, struct controller_command_s *command,
                    controllers_command_action_done_cb actionDoneCb)
{
    assert(userData && command);

    struct controllers_lib_s *lib            = (struct controllers_lib_s*)userData;
    struct controllers_s *controllersObj     = (struct controllers_s*)(lib->pData);
    struct controllers_task_s *cmdsTask      = &controllersObj->tasksMngt[CONTROLLERS_TASK_CMDS].task;
    struct list_s *cmdsList                  = cmdsTask->list;
    struct commands_list_element_s *element  = NULL;

    if (cmdsTask->quit) {
        Loge("Controllers are (being) stopped");
        return;
    }

    if (cmdsList->lock(cmdsList) != LIST_ERROR_NONE) {
        Loge("Failed to lock commands list");
        return;
    }

    Logd("Send command - id : \"%u\" / data : \"%s\"", command->id, command->data);

    assert((element = calloc(1, sizeof(struct commands_list_element_s))));
    element->seconds      = time(NULL);
    element->command      = command;
    element->actionDoneCb = actionDoneCb;

    (void)cmdsList->add(cmdsList, (void*)element);

    (void)cmdsList->unlock(cmdsList);

    sem_post(&cmdsTask->sem);
}

/* -------------------------------------------------------------------------------------------- */
/* ///////////////////////////// PRIVATE FUNCTIONS IMPLEMENTATION ///////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

static void taskFct_f(struct task_params_s *params)
{
    assert(params && params->fctData);

    struct controllers_s *controllersObj    = (struct controllers_s*)(params->fctData);
    struct controllers_task_s *cmdsTask     = &controllersObj->tasksMngt[CONTROLLERS_TASK_CMDS].task;
    struct list_s *cmdsList                 = cmdsTask->list;
    struct commands_list_element_s *element = NULL;

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

    struct controller_command_s *cmd = element->command;
    if (controllersObj->params.onCommandCb) {
        Logd("Handling cmd - id : \"%u\" / data : \"%s\"", cmd->id, cmd->data);
        controllersObj->params.onCommandCb(controllersObj->params.userData, cmd);
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

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////////// CALLBACKS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

static uint8_t compareCb(struct list_s *obj, void *elementToCheck, void *userData)
{
    assert(obj && elementToCheck && userData);

    struct commands_list_element_s *element = (struct commands_list_element_s*)elementToCheck;
    time_t secondsOfElementToRemove         = *((time_t*)userData);

    return (element->seconds == secondsOfElementToRemove);
}

static void releaseCb(struct list_s *obj, void *element)
{
    assert(obj && element);

    struct commands_list_element_s *elementToRemove = (struct commands_list_element_s*)element;

    if (elementToRemove->actionDoneCb) {
        elementToRemove->actionDoneCb(elementToRemove->command);
    }

    elementToRemove->command      = NULL;
    elementToRemove->actionDoneCb = NULL;

    free(elementToRemove);
    elementToRemove = NULL;
}
