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
* \file   Control.c
* \brief  TODO
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include "control/Control.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                             */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "Control"

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

typedef struct CONTROL_COMMANDS_LIST_S {
    CONTROLLER_COMMAND_E id;
    char                 *str;
} CONTROL_COMMANDS_LIST_S;

typedef struct CONTROL_PRIVATE_DATA_S {
    CONTEXT_S       *ctx;
    
    pthread_mutex_t lock;
    
    HANDLERS_S      *handlersObj;
    CONTROLLERS_S   *controllersObj;
} CONTROL_PRIVATE_DATA_S;

/* -------------------------------------------------------------------------------------------- */
/*                                          VARIABLES                                           */
/* -------------------------------------------------------------------------------------------- */

static CONTROL_COMMANDS_LIST_S gCommandsList[] = {
                                                    { CONTROLLER_COMMAND_CLOSE_APPLICATION,   HANDLERS_COMMAND_CLOSE_APPLICATION  },
                                                    { CONTROLLER_COMMAND_CHANGE_LANGUAGE,     HANDLERS_COMMAND_CHANGE_LANGUAGE    },
                                                    { CONTROLLER_COMMAND_SAVE_VIDEO_ELEMENT,  HANDLERS_COMMAND_SAVE_VIDEO_ELEMENT },
                                                    { CONTROLLER_COMMAND_TAKE_SCREENSHOT,     HANDLERS_COMMAND_TAKE_SCREENSHOT    },
                                                    { CONTROLLER_COMMAND_HIDE_ELEMENT,        HANDLERS_COMMAND_HIDE_ELEMENT       },
                                                    { CONTROLLER_COMMAND_SHOW_ELEMENT,        HANDLERS_COMMAND_SHOW_ELEMENT       },
                                                    { CONTROLLER_COMMAND_SET_FOCUS,           HANDLERS_COMMAND_SET_FOCUS          },
                                                    { CONTROLLER_COMMAND_HIDE_GROUP,          HANDLERS_COMMAND_HIDE_GROUP         },
                                                    { CONTROLLER_COMMAND_SHOW_GROUP,          HANDLERS_COMMAND_SHOW_GROUP         },
                                                    { CONTROLLER_COMMAND_SET_CLICKABLE,       HANDLERS_COMMAND_SET_CLICKABLE      },
                                                    { CONTROLLER_COMMAND_SET_NOT_CLICKABLE,   HANDLERS_COMMAND_SET_NOT_CLICKABLE  },
                                                    { CONTROLLER_COMMAND_STOP_GRAPHICS,       HANDLERS_COMMAND_STOP_GRAPHICS      },
                                                    { CONTROLLER_COMMAND_START_GRAPHICS,      HANDLERS_COMMAND_START_GRAPHICS     },
                                                    { CONTROLLER_COMMAND_STOP_VIDEO,          HANDLERS_COMMAND_STOP_VIDEO         },
                                                    { CONTROLLER_COMMAND_START_VIDEO,         HANDLERS_COMMAND_START_VIDEO        },
                                                    { CONTROLLER_COMMAND_STOP_SERVER,         HANDLERS_COMMAND_STOP_SERVER        },
                                                    { CONTROLLER_COMMAND_START_SERVER,        HANDLERS_COMMAND_START_SERVER       },
                                                    { CONTROLLER_COMMAND_SUSPEND_SERVER,      HANDLERS_COMMAND_SUSPEND_SERVER     },
                                                    { CONTROLLER_COMMAND_RESUME_SERVER,       HANDLERS_COMMAND_RESUME_SERVER      },
                                                    { CONTROLLER_COMMAND_STOP_CLIENT,         HANDLERS_COMMAND_STOP_CLIENT        },
                                                    { CONTROLLER_COMMAND_START_CLIENT,        HANDLERS_COMMAND_START_CLIENT       },
                                                    { CONTROLLER_COMMAND_UPDATE_TEXT,         HANDLERS_COMMAND_UPDATE_TEXT        },
                                                    { CONTROLLER_COMMAND_UPDATE_IMAGE,        HANDLERS_COMMAND_UPDATE_IMAGE       },
                                                    { CONTROLLER_COMMAND_UPDATE_NAV,          HANDLERS_COMMAND_UPDATE_NAV         },
                                                    { CONTROLLER_COMMAND_SEND_GFX_EVENT,      HANDLERS_COMMAND_SEND_GFX_EVENT     }
                                                 };

uint32_t gNbCommands = (uint32_t)(sizeof(gCommandsList) / sizeof(gCommandsList[0]));

/* -------------------------------------------------------------------------------------------- */
/*                                         PROTOTYPES                                           */
/* -------------------------------------------------------------------------------------------- */

static CONTROL_ERROR_E initElementData_f  (CONTROL_S *obj, void **data);
static CONTROL_ERROR_E uninitElementData_f(CONTROL_S *obj, void **data);

static CONTROL_ERROR_E setElementGetters_f  (CONTROL_S *obj, void *data, CONTROL_GETTERS_S *getters);
static CONTROL_ERROR_E unsetElementGetters_f(CONTROL_S *obj, void *data);

static CONTROL_ERROR_E setElementTextIds_f  (CONTROL_S *obj, void *data, CONTROL_TEXT_IDS_S *textIds);
static CONTROL_ERROR_E unsetElementTextIds_f(CONTROL_S *obj, void *data);

static CONTROL_ERROR_E setElementImageIds_f  (CONTROL_S *obj, void *data, CONTROL_IMAGE_IDS_S *imageIds);
static CONTROL_ERROR_E unsetElementImageIds_f(CONTROL_S *obj, void *data);

static CONTROL_ERROR_E setCommandHandlers_f  (CONTROL_S *obj, void *data, HANDLERS_ID_S *handlers, uint32_t nbHandlers, uint32_t index);
static CONTROL_ERROR_E unsetCommandHandlers_f(CONTROL_S *obj, void *data);

static CONTROL_ERROR_E loadControllers_f  (CONTROL_S *obj);
static CONTROL_ERROR_E unloadControllers_f(CONTROL_S *obj);

static CONTROL_ERROR_E handleClick_f  (CONTROL_S *obj, GFX_EVENT_S *gfxEvent);
static CONTROL_ERROR_E handleCommand_f(CONTROL_S *obj, CONTROLLERS_COMMAND_S *command);

static void onCommandCb           (void *userData, CONTROLLER_COMMAND_S *command);
static void onModuleStateChangedCb(void *userData, char *name, MODULE_STATE_E state);

/* -------------------------------------------------------------------------------------------- */
/*                                      PUBLIC FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
CONTROL_ERROR_E Control_Init(CONTROL_S **obj, CONTEXT_S *ctx)
{
    assert(obj && ctx && (*obj = calloc(1, sizeof(CONTROL_S))));

    CONTROL_PRIVATE_DATA_S *pData;
    assert((pData = calloc(1, sizeof(CONTROL_PRIVATE_DATA_S))));

    if (pthread_mutex_init(&pData->lock, NULL) != 0) {
        Loge("pthread_mutex_init() failed");
        goto lockExit;
    }

    HANDLERS_PARAMS_S handlersParams = {
                                            .ctx                    = ctx,
                                            .onModuleStateChangedCb = onModuleStateChangedCb,
                                            .userData               = *obj
                                       };

    if (Handlers_Init(&pData->handlersObj, &handlersParams) != HANDLERS_ERROR_NONE) {
        Loge("Handlers_Init() failed");
        goto HandlersInitExit;
    }

    CONTROLLERS_PARAMS_S params = {
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
    pData = NULL;

    free(*obj);
    *obj = NULL;

    return CONTROL_ERROR_INIT;
}

/*!
 *
 */
CONTROL_ERROR_E Control_UnInit(CONTROL_S **obj)
{
    assert(obj && *obj && (*obj)->pData);

    CONTROL_PRIVATE_DATA_S *pData = (CONTROL_PRIVATE_DATA_S*)((*obj)->pData);

    (void)Controllers_UnInit(&pData->controllersObj);
    (void)Handlers_UnInit(&pData->handlersObj);

    (void)pthread_mutex_destroy(&pData->lock);

    pData->ctx = NULL;

    free((*obj)->pData);
    (*obj)->pData = NULL;

    free(*obj);
    *obj = NULL;

    return CONTROL_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/*                                     PRIVATE FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
CONTROL_ERROR_E initElementData_f(CONTROL_S *obj, void **data)
{
    assert(obj && data);

    CONTROL_ELEMENT_DATA_S *elementData;
    assert((elementData = calloc(1, sizeof(CONTROL_ELEMENT_DATA_S))));

    *data = elementData;

    return CONTROL_ERROR_NONE;
}

/*!
 *
 */
CONTROL_ERROR_E uninitElementData_f(CONTROL_S *obj, void **data)
{
    assert(obj && data && *data);

    CONTROL_ELEMENT_DATA_S *elementData = (CONTROL_ELEMENT_DATA_S*)(*data);

    free(elementData);
    elementData = NULL;

    return CONTROL_ERROR_NONE;
}

/*!
 *
 */
CONTROL_ERROR_E setElementGetters_f(CONTROL_S *obj, void *data, CONTROL_GETTERS_S *getters)
{
    assert(obj && data && getters);

    CONTROL_ELEMENT_DATA_S *elementData = (CONTROL_ELEMENT_DATA_S*)data;

    elementData->getters.getString   = getters->getString;
    elementData->getters.getColor    = getters->getColor;
    elementData->getters.getFont     = getters->getFont;
    elementData->getters.getImage    = getters->getImage;
    elementData->getters.getLanguage = getters->getLanguage;

    elementData->getters.userData    = getters->userData;

    return CONTROL_ERROR_NONE;
}

/*!
 *
 */
CONTROL_ERROR_E unsetElementGetters_f(CONTROL_S *obj, void *data)
{
    assert(obj && data);

    CONTROL_ELEMENT_DATA_S *elementData = (CONTROL_ELEMENT_DATA_S*)data;

    elementData->getters.userData = NULL;

    return CONTROL_ERROR_NONE;
}

/*!
 *
 */
CONTROL_ERROR_E setElementTextIds_f(CONTROL_S *obj, void *data, CONTROL_TEXT_IDS_S *textIds)
{
    assert(obj && data && textIds);

    CONTROL_ELEMENT_DATA_S *elementData = (CONTROL_ELEMENT_DATA_S*)data;

    elementData->ids.text.stringId = textIds->stringId;
    elementData->ids.text.fontId   = textIds->fontId;
    elementData->ids.text.colorId  = textIds->colorId;

    return CONTROL_ERROR_NONE;
}

/*!
 *
 */
CONTROL_ERROR_E unsetElementTextIds_f(CONTROL_S *obj, void *data)
{
    assert(obj && data);

    return CONTROL_ERROR_NONE;
}

/*!
 *
 */
CONTROL_ERROR_E setElementImageIds_f(CONTROL_S *obj, void *data, CONTROL_IMAGE_IDS_S *imageIds)
{
    assert(obj && data && imageIds);

    CONTROL_ELEMENT_DATA_S *elementData = (CONTROL_ELEMENT_DATA_S*)data;

    elementData->ids.image.imageId       = imageIds->imageId;
    elementData->ids.image.hiddenColorId = imageIds->hiddenColorId;

    return CONTROL_ERROR_NONE;
}

/*!
 *
 */
CONTROL_ERROR_E unsetElementImageIds_f(CONTROL_S *obj, void *data)
{
    assert(obj && data);

    return CONTROL_ERROR_NONE;
}

/*!
 *
 */
CONTROL_ERROR_E setCommandHandlers_f(CONTROL_S *obj, void *data, HANDLERS_ID_S *handlers, uint32_t nbHandlers, uint32_t index)
{
    assert(obj && obj->pData && data);

    CONTROL_PRIVATE_DATA_S *pData       = (CONTROL_PRIVATE_DATA_S*)(obj->pData);
    CONTROL_ELEMENT_DATA_S *elementData = (CONTROL_ELEMENT_DATA_S*)data;

    elementData->index             = index;
    elementData->nbCommandHandlers = nbHandlers;

    if (!handlers || (nbHandlers == 0)) {
        return CONTROL_ERROR_PARAMS;
    }

    assert((elementData->commandHandlers = calloc(1, nbHandlers * sizeof(HANDLERS_COMMANDS_S))));

    uint32_t i;
    for (i = 0; i < nbHandlers; i++) {
        (elementData->commandHandlers[i]).name = strdup((handlers[i]).name);
        (elementData->commandHandlers[i]).data = strdup((handlers[i]).data);

        (void)pData->handlersObj->getCommandHandler(
                                                    pData->handlersObj,
                                                    (handlers[i]).name, &(elementData->commandHandlers[i]).fct
                                                   );
    }

    return CONTROL_ERROR_NONE;
}

/*!
 *
 */
CONTROL_ERROR_E unsetCommandHandlers_f(CONTROL_S *obj, void *data)
{
    assert(obj && data);

    CONTROL_ELEMENT_DATA_S *elementData = (CONTROL_ELEMENT_DATA_S*)data;

    uint32_t i;
    for (i = 0; i < elementData->nbCommandHandlers; i++) {
        if ((elementData->commandHandlers[i]).name) {
            free((elementData->commandHandlers[i]).name);
            (elementData->commandHandlers[i]).name = NULL;
        }
        if ((elementData->commandHandlers[i]).data) {
            free((elementData->commandHandlers[i]).data);
            (elementData->commandHandlers[i]).data = NULL;
        }
        (elementData->commandHandlers[i]).fct = NULL;
    }

    elementData->commandHandlers = NULL;

    return CONTROL_ERROR_NONE;
}

/*!
 *
 */
static CONTROL_ERROR_E loadControllers_f(CONTROL_S *obj)
{
    assert(obj && obj->pData);

    CONTROL_PRIVATE_DATA_S *pData = (CONTROL_PRIVATE_DATA_S*)(obj->pData);
    CONTROLLERS_S *controllersObj = pData->controllersObj;

     if (controllersObj->initCmdsTask(controllersObj) != CONTROLLERS_ERROR_NONE) {
        Loge("initCmdsTask() failed");
        return CONTROL_ERROR_UNKNOWN;
    }

    if (controllersObj->initEvtsTask(controllersObj) != CONTROLLERS_ERROR_NONE) {
        Loge("initEvtsTask() failed");
        goto initEvtsTaskExit;
    }

    if (controllersObj->initLibsTask(controllersObj) != CONTROLLERS_ERROR_NONE) {
        Loge("initLibsTask() failed");
        goto initLibsTaskExit;
    }

    if (controllersObj->loadLibs(controllersObj) != CONTROLLERS_ERROR_NONE) {
        Loge("loadLibs() failed");
        goto loadLibsExit;
    }

    if (controllersObj->startCmdsTask(controllersObj) != CONTROLLERS_ERROR_NONE) {
        Loge("startCmdsTask() failed");
        goto startCmdsTaskExit;
    }

    if (controllersObj->startEvtsTask(controllersObj) != CONTROLLERS_ERROR_NONE) {
        Loge("startEvtsTask() failed");
        goto startEvtsTaskExit;
    }

    if (controllersObj->startLibsTask(controllersObj) != CONTROLLERS_ERROR_NONE) {
        Loge("startLibsTask() failed");
        goto startLibsTaskExit;
    }

    return CONTROL_ERROR_NONE;

startLibsTaskExit:
	(void)controllersObj->stopEvtsTask(controllersObj);

startEvtsTaskExit:
    (void)controllersObj->stopCmdsTask(controllersObj);

startCmdsTaskExit:
    (void)controllersObj->unloadLibs(controllersObj);

loadLibsExit:
	(void)controllersObj->uninitLibsTask(controllersObj);

initLibsTaskExit:
    (void)controllersObj->uninitEvtsTask(controllersObj);

initEvtsTaskExit:
    (void)controllersObj->uninitCmdsTask(controllersObj);

    return CONTROL_ERROR_UNKNOWN;
}

/*!
 *
 */
static CONTROL_ERROR_E unloadControllers_f(CONTROL_S *obj)
{
    assert(obj && obj->pData);

    CONTROL_ERROR_E ret           = CONTROL_ERROR_NONE;
    CONTROL_PRIVATE_DATA_S *pData = (CONTROL_PRIVATE_DATA_S*)(obj->pData);
    CONTROLLERS_S *controllersObj = pData->controllersObj;

    if (controllersObj->stopLibsTask(controllersObj) != CONTROLLERS_ERROR_NONE) {
        Loge("stopLibsTask() failed");
        ret = CONTROL_ERROR_UNKNOWN;
    }

    if (controllersObj->stopEvtsTask(controllersObj) != CONTROLLERS_ERROR_NONE) {
        Loge("stopEvtsTask() failed");
        ret = CONTROL_ERROR_UNKNOWN;
    }

    if (controllersObj->stopCmdsTask(controllersObj) != CONTROLLERS_ERROR_NONE) {
        Loge("stopCmdsTask() failed");
        ret = CONTROL_ERROR_UNKNOWN;
    }

    if (controllersObj->unloadLibs(controllersObj) != CONTROLLERS_ERROR_NONE) {
        Loge("unloadLibs() failed");
        ret = CONTROL_ERROR_UNKNOWN;
    }

    if (controllersObj->uninitLibsTask(controllersObj) != CONTROLLERS_ERROR_NONE) {
        Loge("uninitLibsTask() failed");
        ret = CONTROL_ERROR_UNKNOWN;
    }

    if (controllersObj->uninitEvtsTask(controllersObj) != CONTROLLERS_ERROR_NONE) {
        Loge("uninitEvtsTask() failed");
        ret = CONTROL_ERROR_UNKNOWN;
    }

    if (controllersObj->uninitCmdsTask(controllersObj) != CONTROLLERS_ERROR_NONE) {
        Loge("uninitCmdsTask() failed");
        ret = CONTROL_ERROR_UNKNOWN;
    }

    return ret;
}

/*!
 *
 */
CONTROL_ERROR_E handleClick_f(CONTROL_S *obj, GFX_EVENT_S *gfxEvent)
{
    assert(obj && obj->pData && gfxEvent);

    CONTROL_PRIVATE_DATA_S *pData       = (CONTROL_PRIVATE_DATA_S*)(obj->pData);
    CONTROLLERS_S *controllersObj       = pData->controllersObj;
    CONTROL_ELEMENT_DATA_S *elementData = (CONTROL_ELEMENT_DATA_S*)gfxEvent->gfxElementPData;
    CONTEXT_S *ctx                      = pData->ctx;

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
            (void)(elementData->commandHandlers[i]).fct(pData->handlersObj, gfxEvent->gfxElementName,
                                                        elementData, (elementData->commandHandlers[i]).data);
        }
    }

    CONTROLLER_EVENT_S event = { 0 };
    event.id   = CONTROLLER_EVENT_CLICKED;
    event.name = gfxEvent->gfxElementName;

    (void)controllersObj->notify(controllersObj, &event);

    (void)pthread_mutex_unlock(&pData->lock);

    return CONTROL_ERROR_NONE;
}

/*!
 *
 */
static CONTROL_ERROR_E handleCommand_f(CONTROL_S *obj, CONTROLLERS_COMMAND_S *command)
{
    assert(obj && obj->pData && command);

    CONTROL_PRIVATE_DATA_S *pData = (CONTROL_PRIVATE_DATA_S*)(obj->pData);
    HANDLERS_S *handlersObj       = pData->handlersObj;
    HANDLERS_COMMAND_F handlerCmd = NULL;
    CONTROL_ERROR_E ret           = CONTROL_ERROR_NONE;

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
/*                                            CALLBACKS                                         */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static void onCommandCb(void *userData, CONTROLLER_COMMAND_S *command)
{
    assert(userData && command);

    CONTROL_S *obj                  = (CONTROL_S*)userData;
    CONTROL_PRIVATE_DATA_S *pData   = (CONTROL_PRIVATE_DATA_S*)(obj->pData);
    CONTROLLERS_S *controllersObj   = pData->controllersObj;
    GRAPHICS_INFOS_S *graphicsInfos = &pData->ctx->params.graphicsInfos;
    uint32_t nbGfxElements          = graphicsInfos->nbGfxElements;
    GFX_ELEMENT_S **gfxElements     = graphicsInfos->gfxElements;
    CONTROLLERS_COMMAND_S cmd       = { 0 };

    char gfxElementName[MAX_NAME_SIZE] = { 0 };
    char handlerName[MAX_NAME_SIZE]    = { 0 };
    char handlerData[MIN_STR_SIZE]     = { 0 };

    cmd.id = command->id;

    /* handlerName */
    uint32_t index;
    if ((command->id < gNbCommands) && (gCommandsList[command->id].id == command->id)) {
        strcpy(handlerName, gCommandsList[command->id].str);
    }
    else {
        for (index = 0; index < gNbCommands; ++index) {
            if (gCommandsList[index].id != command->id) {
                continue;
            }
            strcpy(handlerName, gCommandsList[index].str);
            break;
        }

        if (index == gNbCommands) {
            Loge("Command \"%u\" not found", command->id);
            return;
        }
    }

    /* handlerData */
    switch (command->id) {
        case CONTROLLER_COMMAND_CHANGE_LANGUAGE:
            strcpy(gfxElementName, gfxElements[0]->name);
            // No break
        case CONTROLLER_COMMAND_TAKE_SCREENSHOT:
        case CONTROLLER_COMMAND_SAVE_VIDEO_ELEMENT:
        case CONTROLLER_COMMAND_HIDE_ELEMENT:
        case CONTROLLER_COMMAND_SHOW_ELEMENT:
        case CONTROLLER_COMMAND_SET_FOCUS:
        case CONTROLLER_COMMAND_HIDE_GROUP:
        case CONTROLLER_COMMAND_SHOW_GROUP:
        case CONTROLLER_COMMAND_SET_CLICKABLE:
        case CONTROLLER_COMMAND_SET_NOT_CLICKABLE:
        case CONTROLLER_COMMAND_STOP_VIDEO:
        case CONTROLLER_COMMAND_START_VIDEO:
        case CONTROLLER_COMMAND_STOP_SERVER:
        case CONTROLLER_COMMAND_START_SERVER:
        case CONTROLLER_COMMAND_SUSPEND_SERVER:
        case CONTROLLER_COMMAND_RESUME_SERVER:
        case CONTROLLER_COMMAND_STOP_CLIENT:
        case CONTROLLER_COMMAND_START_CLIENT:
        case CONTROLLER_COMMAND_UPDATE_TEXT:
        case CONTROLLER_COMMAND_UPDATE_IMAGE:
        case CONTROLLER_COMMAND_UPDATE_NAV:
        case CONTROLLER_COMMAND_SEND_GFX_EVENT:
            strcpy(handlerData, command->data);
            break;

        case CONTROLLER_COMMAND_CLOSE_APPLICATION:
        case CONTROLLER_COMMAND_STOP_GRAPHICS:
        case CONTROLLER_COMMAND_START_GRAPHICS:
            break;

        default:
            ;
    }

    /* gfxElementName && gfxElementData */
    if (strlen(gfxElementName) != 0) {
        for (index = 0; index < nbGfxElements; ++index) {
            if (strcmp(gfxElements[index]->name, gfxElementName) != 0) {
                continue;
            }

            cmd.gfxElementName = gfxElementName;
            cmd.gfxElementData = gfxElements[index]->pData;
            break;
        }
    }

    cmd.handlerName = handlerName;

    if (strlen(handlerData) != 0) {
        cmd.handlerData = handlerData;
    }

    /* Call handler */
    (void)handleCommand_f((CONTROL_S*)userData, &cmd);
}

/*!
 *
 */
static void onModuleStateChangedCb(void *userData, char *name, MODULE_STATE_E state)
{
    assert(userData && name);

    CONTROL_S *obj                = (CONTROL_S*)userData;
    CONTROL_PRIVATE_DATA_S *pData = (CONTROL_PRIVATE_DATA_S*)(obj->pData);
    CONTROLLERS_S *controllersObj = pData->controllersObj;

    CONTROLLER_EVENT_S event;
    event.id   = state;
    event.name = name;

    (void)controllersObj->notify(controllersObj, &event);
}
