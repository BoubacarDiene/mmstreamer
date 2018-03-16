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
* \file Control.c
* \brief TODO
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// HEADERS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#include "control/Control.h"

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// MACROS ////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "Control"

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// TYPES /////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

struct control_commands_list_s {
    enum controller_command_e id;
    char                      *str;
    uint8_t                   gfxElementRequired;
    uint8_t                   dataContainsElementName;
};

struct control_private_data_s {
    struct context_s     *ctx;
    
    pthread_mutex_t      lock;
    
    struct handlers_s    *handlersObj;
    struct controllers_s *controllersObj;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////// PUBLIC FUNCTIONS PROTOTYPES //////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

static enum control_error_e initElementData_f(struct control_s *obj, void **data);
static enum control_error_e uninitElementData_f(struct control_s *obj, void **data);

static enum control_error_e setElementGetters_f(struct control_s *obj, void *data,
                                                struct control_getters_s *getters);
static enum control_error_e unsetElementGetters_f(struct control_s *obj, void *data);

static enum control_error_e setElementTextIds_f(struct control_s *obj, void *data,
                                                struct control_text_ids_s *textIds);
static enum control_error_e unsetElementTextIds_f(struct control_s *obj, void *data);

static enum control_error_e setElementImageIds_f(struct control_s *obj, void *data,
                                                 struct control_image_ids_s *imageIds);
static enum control_error_e unsetElementImageIds_f(struct control_s *obj, void *data);

static enum control_error_e setCommandHandlers_f(struct control_s *obj, void *data,
                                                 struct handlers_id_s *handlers,
                                                 uint32_t nbHandlers, uint32_t index);
static enum control_error_e unsetCommandHandlers_f(struct control_s *obj, void *data);

static enum control_error_e loadControllers_f(struct control_s *obj);
static enum control_error_e unloadControllers_f(struct control_s *obj);

static enum control_error_e handleClick_f(struct control_s *obj,
                                          struct gfx_event_s *gfxEvent);
static enum control_error_e handleCommand_f(struct control_s *obj,
                                            struct controllers_command_s *command);

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////////// CALLBACKS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

static void onCommandCb(void *userData, struct controller_command_s *command);
static void onModuleStateChangedCb(void *userData, char *name, enum module_state_e state);

/* -------------------------------------------------------------------------------------------- */
/* ///////////////////////////////////// GLOBAL VARIABLES ///////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

static struct control_commands_list_s gCommandsList[] = {
    { CONTROLLER_COMMAND_CLOSE_APPLICATION,   HANDLERS_COMMAND_CLOSE_APPLICATION,   0,  0 },
    { CONTROLLER_COMMAND_CHANGE_LANGUAGE,     HANDLERS_COMMAND_CHANGE_LANGUAGE,     1,  0 },
    { CONTROLLER_COMMAND_SAVE_VIDEO_ELEMENT,  HANDLERS_COMMAND_SAVE_VIDEO_ELEMENT,  1,  1 },
    { CONTROLLER_COMMAND_TAKE_SCREENSHOT,     HANDLERS_COMMAND_TAKE_SCREENSHOT,     0,  0 },
    { CONTROLLER_COMMAND_HIDE_ELEMENT,        HANDLERS_COMMAND_HIDE_ELEMENT,        0,  0 },
    { CONTROLLER_COMMAND_SHOW_ELEMENT,        HANDLERS_COMMAND_SHOW_ELEMENT,        0,  0 },
    { CONTROLLER_COMMAND_SET_FOCUS,           HANDLERS_COMMAND_SET_FOCUS,           0,  0 },
    { CONTROLLER_COMMAND_HIDE_GROUP,          HANDLERS_COMMAND_HIDE_GROUP,          0,  0 },
    { CONTROLLER_COMMAND_SHOW_GROUP,          HANDLERS_COMMAND_SHOW_GROUP,          0,  0 },
    { CONTROLLER_COMMAND_SET_CLICKABLE,       HANDLERS_COMMAND_SET_CLICKABLE,       0,  0 },
    { CONTROLLER_COMMAND_SET_NOT_CLICKABLE,   HANDLERS_COMMAND_SET_NOT_CLICKABLE,   0,  0 },
    { CONTROLLER_COMMAND_STOP_GRAPHICS,       HANDLERS_COMMAND_STOP_GRAPHICS,       0,  0 },
    { CONTROLLER_COMMAND_START_GRAPHICS,      HANDLERS_COMMAND_START_GRAPHICS,      0,  0 },
    { CONTROLLER_COMMAND_STOP_VIDEO,          HANDLERS_COMMAND_STOP_VIDEO,          0,  0 },
    { CONTROLLER_COMMAND_START_VIDEO,         HANDLERS_COMMAND_START_VIDEO,         0,  0 },
    { CONTROLLER_COMMAND_STOP_SERVER,         HANDLERS_COMMAND_STOP_SERVER,         0,  0 },
    { CONTROLLER_COMMAND_START_SERVER,        HANDLERS_COMMAND_START_SERVER,        0,  0 },
    { CONTROLLER_COMMAND_SUSPEND_SERVER,      HANDLERS_COMMAND_SUSPEND_SERVER,      0,  0 },
    { CONTROLLER_COMMAND_RESUME_SERVER,       HANDLERS_COMMAND_RESUME_SERVER,       0,  0 },
    { CONTROLLER_COMMAND_STOP_CLIENT,         HANDLERS_COMMAND_STOP_CLIENT,         0,  0 },
    { CONTROLLER_COMMAND_START_CLIENT,        HANDLERS_COMMAND_START_CLIENT,        0,  0 },
    { CONTROLLER_COMMAND_UPDATE_TEXT,         HANDLERS_COMMAND_UPDATE_TEXT,         1,  1 },
    { CONTROLLER_COMMAND_UPDATE_IMAGE,        HANDLERS_COMMAND_UPDATE_IMAGE,        1,  1 },
    { CONTROLLER_COMMAND_UPDATE_NAV,          HANDLERS_COMMAND_UPDATE_NAV,          1,  1 },
    { CONTROLLER_COMMAND_SEND_GFX_EVENT,      HANDLERS_COMMAND_SEND_GFX_EVENT,      0,  0 }
};

uint32_t gNbCommands = (uint32_t)(sizeof(gCommandsList) / sizeof(gCommandsList[0]));

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// INITIALIZER //////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
enum control_error_e Control_Init(struct control_s **obj, struct context_s *ctx)
{
    assert(obj && ctx && (*obj = calloc(1, sizeof(struct control_s))));

    struct control_private_data_s *pData;
    assert((pData = calloc(1, sizeof(struct control_private_data_s))));

    if (pthread_mutex_init(&pData->lock, NULL) != 0) {
        Loge("pthread_mutex_init() failed");
        goto lockExit;
    }

    struct handlers_params_s handlersParams = {
        .ctx                    = ctx,
        .onModuleStateChangedCb = onModuleStateChangedCb,
        .userData               = *obj
    };

    if (Handlers_Init(&pData->handlersObj, &handlersParams) != HANDLERS_ERROR_NONE) {
        Loge("Handlers_Init() failed");
        goto HandlersInitExit;
    }

    struct controllers_params_s params = {
        .ctx         = ctx,
        .onCommandCb = onCommandCb,
        .userData    = *obj
    };

    if (Controllers_Init(&pData->controllersObj, &params) != CONTROLLERS_ERROR_NONE) {
        Loge("Controllers_Init() failed");
        goto ControllersInitExit;
    }

    (*obj)->initElementData      = initElementData_f;
    (*obj)->uninitElementData    = uninitElementData_f;

    (*obj)->setElementGetters    = setElementGetters_f;
    (*obj)->unsetElementGetters  = unsetElementGetters_f;

    (*obj)->setElementTextIds    = setElementTextIds_f;
    (*obj)->unsetElementTextIds  = unsetElementTextIds_f;

    (*obj)->setElementImageIds   = setElementImageIds_f;
    (*obj)->unsetElementImageIds = unsetElementImageIds_f;

    (*obj)->setCommandHandlers   = setCommandHandlers_f;
    (*obj)->unsetCommandHandlers = unsetCommandHandlers_f;

    (*obj)->loadControllers      = loadControllers_f;
    (*obj)->unloadControllers    = unloadControllers_f;

    (*obj)->handleClick          = handleClick_f;
    (*obj)->handleCommand        = handleCommand_f;

    pData->ctx = ctx;

    (*obj)->pData = (void*)pData;

    return CONTROL_ERROR_NONE;

ControllersInitExit:
    (void)Handlers_UnInit(&pData->handlersObj);

HandlersInitExit:
    (void)pthread_mutex_destroy(&pData->lock);

lockExit:
    free(pData);
    free(*obj);
    *obj = NULL;

    return CONTROL_ERROR_INIT;
}

/*!
 *
 */
enum control_error_e Control_UnInit(struct control_s **obj)
{
    assert(obj && *obj && (*obj)->pData);

    struct control_private_data_s *pData = (struct control_private_data_s*)((*obj)->pData);

    (void)Controllers_UnInit(&pData->controllersObj);
    (void)Handlers_UnInit(&pData->handlersObj);

    (void)pthread_mutex_destroy(&pData->lock);

    free(pData);
    free(*obj);
    *obj = NULL;

    return CONTROL_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////// PUBLIC FUNCTIONS IMPLEMENTATION ///////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
enum control_error_e initElementData_f(struct control_s *obj, void **data)
{
    assert(obj && data);

    struct control_element_data_s *elementData;
    assert((elementData = calloc(1, sizeof(struct control_element_data_s))));

    *data = elementData;

    return CONTROL_ERROR_NONE;
}

/*!
 *
 */
enum control_error_e uninitElementData_f(struct control_s *obj, void **data)
{
    assert(obj && data && *data);

    struct control_element_data_s *elementData = (struct control_element_data_s*)(*data);
    free(elementData);

    return CONTROL_ERROR_NONE;
}

/*!
 *
 */
enum control_error_e setElementGetters_f(struct control_s *obj, void *data,
                                         struct control_getters_s *getters)
{
    assert(obj && data && getters);

    ((struct control_element_data_s*)data)->getters = *getters;

    return CONTROL_ERROR_NONE;
}

/*!
 *
 */
enum control_error_e unsetElementGetters_f(struct control_s *obj, void *data)
{
    assert(obj && data);

    ((struct control_element_data_s*)data)->getters.userData = NULL;

    return CONTROL_ERROR_NONE;
}

/*!
 *
 */
enum control_error_e setElementTextIds_f(struct control_s *obj, void *data,
                                         struct control_text_ids_s *textIds)
{
    assert(obj && data && textIds);

    ((struct control_element_data_s*)data)->ids.text = *textIds;

    return CONTROL_ERROR_NONE;
}

/*!
 *
 */
enum control_error_e unsetElementTextIds_f(struct control_s *obj, void *data)
{
    assert(obj && data);

    return CONTROL_ERROR_NONE;
}

/*!
 *
 */
enum control_error_e setElementImageIds_f(struct control_s *obj, void *data,
                                          struct control_image_ids_s *imageIds)
{
    assert(obj && data && imageIds);

    ((struct control_element_data_s*)data)->ids.image = *imageIds;

    return CONTROL_ERROR_NONE;
}

/*!
 *
 */
enum control_error_e unsetElementImageIds_f(struct control_s *obj, void *data)
{
    assert(obj && data);

    return CONTROL_ERROR_NONE;
}

/*!
 *
 */
enum control_error_e setCommandHandlers_f(struct control_s *obj, void *data,
                                          struct handlers_id_s *handlers,
                                          uint32_t nbHandlers, uint32_t index)
{
    assert(obj && obj->pData && data);

    struct control_private_data_s *pData       = (struct control_private_data_s*)(obj->pData);
    struct control_element_data_s *elementData = (struct control_element_data_s*)data;

    elementData->index             = index;
    elementData->nbCommandHandlers = nbHandlers;

    if (!handlers || (nbHandlers == 0)) {
        return CONTROL_ERROR_PARAMS;
    }

    elementData->commandHandlers = calloc(1, nbHandlers * sizeof(struct handlers_commands_s));
    assert(elementData->commandHandlers);

    uint32_t i;
    for (i = 0; i < nbHandlers; i++) {
        (elementData->commandHandlers[i]).name = strdup((handlers[i]).name);
        (elementData->commandHandlers[i]).data = strdup((handlers[i]).data);

        (void)pData->handlersObj->getCommandHandler(pData->handlersObj,
                                                    (handlers[i]).name,
                                                    &(elementData->commandHandlers[i]).fct);
    }

    return CONTROL_ERROR_NONE;
}

/*!
 *
 */
enum control_error_e unsetCommandHandlers_f(struct control_s *obj, void *data)
{
    assert(obj && data);

    struct control_element_data_s *elementData = (struct control_element_data_s*)data;

    uint32_t i;
    for (i = 0; i < elementData->nbCommandHandlers; i++) {
        if ((elementData->commandHandlers[i]).name) {
            free((elementData->commandHandlers[i]).name);
        }
        if ((elementData->commandHandlers[i]).data) {
            free((elementData->commandHandlers[i]).data);
        }
    }

    if (elementData->commandHandlers) {
        free(elementData->commandHandlers);
        elementData->commandHandlers = NULL;
    }

    return CONTROL_ERROR_NONE;
}

/*!
 *
 */
static enum control_error_e loadControllers_f(struct control_s *obj)
{
    assert(obj && obj->pData);

    struct control_private_data_s *pData      = (struct control_private_data_s*)(obj->pData);
    struct controllers_s *controllersObj      = pData->controllersObj;
    enum controllers_error_e controllersError = CONTROLLERS_ERROR_NONE;

    controllersError = controllersObj->tasksMngt[CONTROLLERS_TASK_CMDS].init(controllersObj);
    if (controllersError != CONTROLLERS_ERROR_NONE) {
        Loge("initCmdsTask() failed");
        return CONTROL_ERROR_UNKNOWN;
    }

    controllersError = controllersObj->tasksMngt[CONTROLLERS_TASK_EVTS].init(controllersObj);
    if (controllersError != CONTROLLERS_ERROR_NONE) {
        Loge("initEvtsTask() failed");
        goto initEvtsTaskExit;
    }

    controllersError = controllersObj->tasksMngt[CONTROLLERS_TASK_LIBS].init(controllersObj);
    if (controllersError != CONTROLLERS_ERROR_NONE) {
        Loge("initLibsTask() failed");
        goto initLibsTaskExit;
    }

    controllersError = controllersObj->loadLibs(controllersObj);
    if (controllersError != CONTROLLERS_ERROR_NONE) {
        Loge("loadLibs() failed");
        goto loadLibsExit;
    }

    controllersError = controllersObj->tasksMngt[CONTROLLERS_TASK_CMDS].start(controllersObj);
    if (controllersError != CONTROLLERS_ERROR_NONE) {
        Loge("startCmdsTask() failed");
        goto startCmdsTaskExit;
    }

    controllersError = controllersObj->tasksMngt[CONTROLLERS_TASK_EVTS].start(controllersObj);
    if (controllersError != CONTROLLERS_ERROR_NONE) {
        Loge("startEvtsTask() failed");
        goto startEvtsTaskExit;
    }

    controllersError = controllersObj->tasksMngt[CONTROLLERS_TASK_LIBS].start(controllersObj);
    if (controllersError != CONTROLLERS_ERROR_NONE) {
        Loge("startLibsTask() failed");
        goto startLibsTaskExit;
    }

    return CONTROL_ERROR_NONE;

startLibsTaskExit:
	(void)controllersObj->tasksMngt[CONTROLLERS_TASK_EVTS].stop(controllersObj);

startEvtsTaskExit:
    (void)controllersObj->tasksMngt[CONTROLLERS_TASK_CMDS].stop(controllersObj);

startCmdsTaskExit:
    (void)controllersObj->unloadLibs(controllersObj);

loadLibsExit:
	(void)controllersObj->tasksMngt[CONTROLLERS_TASK_LIBS].uninit(controllersObj);

initLibsTaskExit:
    (void)controllersObj->tasksMngt[CONTROLLERS_TASK_EVTS].uninit(controllersObj);

initEvtsTaskExit:
    (void)controllersObj->tasksMngt[CONTROLLERS_TASK_CMDS].uninit(controllersObj);

    return CONTROL_ERROR_UNKNOWN;
}

/*!
 *
 */
static enum control_error_e unloadControllers_f(struct control_s *obj)
{
    assert(obj && obj->pData);

    enum control_error_e ret                  = CONTROL_ERROR_NONE;
    enum controllers_error_e controllersError = CONTROLLERS_ERROR_NONE;
    struct control_private_data_s *pData      = (struct control_private_data_s*)(obj->pData);
    struct controllers_s *controllersObj      = pData->controllersObj;

    controllersError = controllersObj->tasksMngt[CONTROLLERS_TASK_LIBS].stop(controllersObj);
    if (controllersError != CONTROLLERS_ERROR_NONE) {
        Loge("stopLibsTask() failed");
        ret = CONTROL_ERROR_UNKNOWN;
    }

    controllersError = controllersObj->tasksMngt[CONTROLLERS_TASK_EVTS].stop(controllersObj);
    if (controllersError != CONTROLLERS_ERROR_NONE) {
        Loge("stopEvtsTask() failed");
        ret = CONTROL_ERROR_UNKNOWN;
    }

    controllersError = controllersObj->tasksMngt[CONTROLLERS_TASK_CMDS].stop(controllersObj);
    if (controllersError != CONTROLLERS_ERROR_NONE) {
        Loge("stopCmdsTask() failed");
        ret = CONTROL_ERROR_UNKNOWN;
    }

    controllersError = controllersObj->unloadLibs(controllersObj);
    if (controllersError != CONTROLLERS_ERROR_NONE) {
        Loge("unloadLibs() failed");
        ret = CONTROL_ERROR_UNKNOWN;
    }

    controllersError = controllersObj->tasksMngt[CONTROLLERS_TASK_LIBS].uninit(controllersObj);
    if (controllersError != CONTROLLERS_ERROR_NONE) {
        Loge("uninitLibsTask() failed");
        ret = CONTROL_ERROR_UNKNOWN;
    }

    controllersError = controllersObj->tasksMngt[CONTROLLERS_TASK_EVTS].uninit(controllersObj);
    if (controllersError != CONTROLLERS_ERROR_NONE) {
        Loge("uninitEvtsTask() failed");
        ret = CONTROL_ERROR_UNKNOWN;
    }

    controllersError = controllersObj->tasksMngt[CONTROLLERS_TASK_CMDS].uninit(controllersObj);
    if (controllersError != CONTROLLERS_ERROR_NONE) {
        Loge("uninitCmdsTask() failed");
        ret = CONTROL_ERROR_UNKNOWN;
    }

    return ret;
}

/*!
 *
 */
enum control_error_e handleClick_f(struct control_s *obj, struct gfx_event_s *gfxEvent)
{
    assert(obj && obj->pData && gfxEvent);

    struct control_private_data_s *pData       = (struct control_private_data_s*)(obj->pData);
    struct controllers_s *controllersObj       = pData->controllersObj;
    struct control_element_data_s *elementData = (struct control_element_data_s*)gfxEvent->gfxElementPData;
    struct context_s *ctx                      = pData->ctx;

    if (!elementData) {
        return CONTROL_ERROR_PARAMS;
    }

    if (pthread_mutex_lock(&pData->lock) != 0) {
        Loge("pthread_mutex_lock() failed");
        return CONTROL_ERROR_LOCK;
    }

    uint32_t i;
    for (i = 0; i < elementData->nbCommandHandlers; i++) {
        if ((elementData->commandHandlers[i]).fct) {
            Logd("Calling command handler : %s", (elementData->commandHandlers[i]).name);
            (void)(elementData->commandHandlers[i]).fct(pData->handlersObj,
                                                        gfxEvent->gfxElementName,
                                                        elementData,
                                                        (elementData->commandHandlers[i]).data);
        }
    }

    struct controller_event_s event = {0};
    event.id   = CONTROLLER_EVENT_CLICKED;
    event.name = gfxEvent->gfxElementName;

    (void)controllersObj->notify(controllersObj, &event);

    (void)pthread_mutex_unlock(&pData->lock);

    return CONTROL_ERROR_NONE;
}

/*!
 *
 */
static enum control_error_e handleCommand_f(struct control_s *obj,
                                            struct controllers_command_s *command)
{
    assert(obj && obj->pData && command);

    struct control_private_data_s *pData = (struct control_private_data_s*)(obj->pData);
    struct handlers_s *handlersObj       = pData->handlersObj;
    handlers_command_f handlerCmd        = NULL;
    enum control_error_e ret             = CONTROL_ERROR_NONE;

    if (pthread_mutex_lock(&pData->lock) != 0) {
        Loge("pthread_mutex_lock() failed");
        return CONTROL_ERROR_LOCK;
    }

    (void)handlersObj->getCommandHandler(handlersObj, command->handlerName, &handlerCmd);
    if (handlerCmd
        && (handlerCmd(handlersObj,
                       command->gfxElementName,
                       command->gfxElementData,
                       command->handlerData) != HANDLERS_ERROR_NONE)) {
        ret = CONTROL_ERROR_UNKNOWN;
    }

    (void)pthread_mutex_unlock(&pData->lock);

    return ret;
}

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////////// CALLBACKS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static void onCommandCb(void *userData, struct controller_command_s *command)
{
    assert(userData && command);

    struct control_s *obj                  = (struct control_s*)userData;
    struct control_private_data_s *pData   = (struct control_private_data_s*)(obj->pData);
    struct controllers_s *controllersObj   = pData->controllersObj;
    struct graphics_infos_s *graphicsInfos = &pData->ctx->params.graphicsInfos;
    uint32_t nbGfxElements                 = graphicsInfos->nbGfxElements;
    struct gfx_element_s **gfxElements     = graphicsInfos->gfxElements;
    struct controllers_command_s cmd       = {0};

    char gfxElementName[MAX_NAME_SIZE]     = {0};
    char handlerName[MAX_NAME_SIZE]        = {0};
    char handlerData[MIN_STR_SIZE]         = {0};

    /* Get name of handler that will treat this request */
    uint32_t index = command->id;
    if ((command->id >= gNbCommands) || (gCommandsList[command->id].id != command->id)) {
        for (index = 0; index < gNbCommands; ++index) {
            if (gCommandsList[index].id != command->id) {
                continue;
            }
            break;
        }

        if (index == gNbCommands) {
            Loge("Command \"%u\" not found", command->id);
            return;
        }
    }

    snprintf(handlerName, sizeof(handlerName), "%s", gCommandsList[index].str);
    cmd.id          = command->id;
    cmd.handlerName = handlerName;

    /* Prepare command */
    uint32_t offset = 0;
    if (gCommandsList[index].dataContainsElementName || gCommandsList[index].gfxElementRequired) {
        // Set elementName
        if (gCommandsList[index].dataContainsElementName) {
            if (!command->data) {
                Loge("data must be not NULL");
                return;
            }
            enum handlers_error_e hError = pData->handlersObj->getSubstring(pData->handlersObj,
                                                                            command->data,
                                                                            ";",
                                                                            gfxElementName,
                                                                            &offset);
            if (hError != HANDLERS_ERROR_NONE) {
                Loge("Bad format. Expected: elementName;param1;param2;...");
                return;
            }
        }
        else {
            snprintf(gfxElementName, sizeof(gfxElementName), "%s", gfxElements[0]->name);
        }

        // Find corresponding gfx element
        for (index = 0; index < nbGfxElements; ++index) {
            if (strcmp(gfxElements[index]->name, gfxElementName) != 0) {
                continue;
            }
            cmd.gfxElementName = gfxElementName;
            cmd.gfxElementData = gfxElements[index]->pData;
            break;
        }
    }

    if (command->data) {
        snprintf(handlerData, sizeof(handlerData), "%s", command->data + offset);
        cmd.handlerData = handlerData;
    }

    /* Call handler */
    (void)handleCommand_f((struct control_s*)userData, &cmd);
}

/*!
 *
 */
static void onModuleStateChangedCb(void *userData, char *name, enum module_state_e state)
{
    assert(userData && name);

    struct control_s *obj                = (struct control_s*)userData;
    struct control_private_data_s *pData = (struct control_private_data_s*)(obj->pData);
    struct controllers_s *controllersObj = pData->controllersObj;

    struct controller_event_s event;
    event.id   = state;
    event.name = name;

    (void)controllersObj->notify(controllersObj, &event);
}
