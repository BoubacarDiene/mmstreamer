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
* \file   SingleInput.c
* \brief  TODO
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include "control/Control.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                            */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "HANDLERS-SINGLEINPUT"

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                         PROTOTYPES                                           */
/* -------------------------------------------------------------------------------------------- */

static HANDLERS_ERROR_E closeApplication(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData);
static HANDLERS_ERROR_E changeLanguage  (HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData);

static HANDLERS_ERROR_E hideElement(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData);
static HANDLERS_ERROR_E showElement(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData);

static HANDLERS_ERROR_E hideGroup(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData);
static HANDLERS_ERROR_E showGroup(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData);

static HANDLERS_ERROR_E setFocus(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData);

static HANDLERS_ERROR_E saveVideoElement(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData);
static HANDLERS_ERROR_E takeScreenshot  (HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData);

static HANDLERS_ERROR_E setClickable   (HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData);
static HANDLERS_ERROR_E setNotClickable(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData);

static HANDLERS_ERROR_E stopGraphics (HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData);
static HANDLERS_ERROR_E startGraphics(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData);

static HANDLERS_ERROR_E stopVideo (HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData);
static HANDLERS_ERROR_E startVideo(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData);

static HANDLERS_ERROR_E stopServer (HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData);
static HANDLERS_ERROR_E startServer(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData);

static HANDLERS_ERROR_E suspendServer(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData);
static HANDLERS_ERROR_E resumeServer (HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData);

static HANDLERS_ERROR_E stopCient  (HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData);
static HANDLERS_ERROR_E startClient(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData);

static HANDLERS_ERROR_E multiInputs(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData);

/* -------------------------------------------------------------------------------------------- */
/*                                          VARIABLES                                           */
/* -------------------------------------------------------------------------------------------- */

COMMAND_HANDLERS_S gSingleInputHandlers[] = {
	{ HANDLERS_COMMAND_CLOSE_APPLICATION,           NULL,             closeApplication },
	{ HANDLERS_COMMAND_CHANGE_LANGUAGE,             NULL,             changeLanguage   },
	{ HANDLERS_COMMAND_SAVE_VIDEO_ELEMENT,          NULL,             saveVideoElement },
	{ HANDLERS_COMMAND_TAKE_SCREENSHOT,             NULL,             takeScreenshot   },
	{ HANDLERS_COMMAND_HIDE_ELEMENT,                NULL,             hideElement      },
	{ HANDLERS_COMMAND_SHOW_ELEMENT,                NULL,             showElement      },
	{ HANDLERS_COMMAND_SET_FOCUS,                   NULL,             setFocus         },
	{ HANDLERS_COMMAND_HIDE_GROUP,                  NULL,             hideGroup        },
	{ HANDLERS_COMMAND_SHOW_GROUP,                  NULL,             showGroup        },
	{ HANDLERS_COMMAND_SET_CLICKABLE,               NULL,             setClickable     },
	{ HANDLERS_COMMAND_SET_NOT_CLICKABLE,           NULL,             setNotClickable  },
	{ HANDLERS_COMMAND_STOP_GRAPHICS,               NULL,             stopGraphics     },
	{ HANDLERS_COMMAND_START_GRAPHICS,              NULL,             startGraphics    },
	{ HANDLERS_COMMAND_STOP_VIDEO,                  NULL,             stopVideo        },
	{ HANDLERS_COMMAND_START_VIDEO,                 NULL,             startVideo       },
	{ HANDLERS_COMMAND_STOP_SERVER,                 NULL,             stopServer       },
	{ HANDLERS_COMMAND_START_SERVER,                NULL,             startServer      },
	{ HANDLERS_COMMAND_SUSPEND_SERVER,              NULL,             suspendServer    },
	{ HANDLERS_COMMAND_RESUME_SERVER,               NULL,             resumeServer     },
	{ HANDLERS_COMMAND_STOP_CLIENT,                 NULL,             stopCient        },
	{ HANDLERS_COMMAND_START_CLIENT,                NULL,             startClient      },

	{ HANDLERS_COMMAND_MULTI_INPUTS,                NULL,             multiInputs      },
	{ NULL,                                         NULL,             NULL             }
};

uint32_t gNbSingleInputHandlers = (uint32_t)(sizeof(gSingleInputHandlers) / sizeof(gSingleInputHandlers[0]));

/* -------------------------------------------------------------------------------------------- */
/*                                       COMMAND HANDLERS                                       */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static HANDLERS_ERROR_E closeApplication(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(obj && obj->pData);

    (void)gfxElementName;
    (void)gfxElementData;
    (void)handlerData;

    Logd("Closing application");

    HANDLERS_PRIVATE_DATA_S *pData = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                 = pData->ctx;
    GRAPHICS_S *graphicsObj        = ctx->modules.graphicsObj;
    INPUT_S *input                 = &ctx->input;
    HANDLERS_ERROR_E ret           = HANDLERS_ERROR_NONE;

    switch (input->keepAliveMethod) {
        case KEEP_ALIVE_EVENTS_BASED:
            if (graphicsObj && graphicsObj->quit(graphicsObj) != GRAPHICS_ERROR_NONE) {
                Loge("Graphics quit() failed");
                ret = HANDLERS_ERROR_COMMAND;
            }
            break;

        case KEEP_ALIVE_SEMAPHORE_BASED:
            sem_post(&ctx->keepAliveSem);
            break;

        case KEEP_ALIVE_TIMER_BASED:
            // Wait until timeout
            break;

        default:
            ;
    }

    return ret;
}

/*!
 *
 */
static HANDLERS_ERROR_E changeLanguage(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(obj && obj->pData && gfxElementData);

    (void)gfxElementName;

    HANDLERS_PRIVATE_DATA_S *pData      = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                      = pData->ctx;
    CONTROL_ELEMENT_DATA_S *elementData = (CONTROL_ELEMENT_DATA_S*)gfxElementData;
    GRAPHICS_S *graphicsObj             = ctx->modules.graphicsObj;
    GRAPHICS_INFOS_S *graphicsInfos     = &ctx->params.graphicsInfos;
    uint32_t nbGfxElements              = graphicsInfos->nbGfxElements;
    GFX_ELEMENT_S **gfxElements         = graphicsInfos->gfxElements;
    HANDLERS_ERROR_E ret                = HANDLERS_ERROR_NONE;

    if (graphicsInfos->state != MODULE_STATE_STARTED) {
        Logw("Graphics module is not started - current state : %u", graphicsInfos->state);
        return HANDLERS_ERROR_STATE;
    }
    
    char nextLanguage[MIN_STR_SIZE] = { 0 };

    if (!handlerData || (strlen(handlerData) == 0)) {
        elementData->getters.getLanguage(elementData->getters.userData, graphicsInfos->currentLanguage, nextLanguage);
    }
    else {
        strncpy(nextLanguage, handlerData, sizeof(nextLanguage));
    }

    if (strcmp(graphicsInfos->currentLanguage, nextLanguage) == 0) {
        Logw("\"%s\" is already the current language", nextLanguage);
        return ret;
    }

    Logd("Changing language from \"%s\" to \"%s\"", graphicsInfos->currentLanguage, nextLanguage);
    
    uint32_t index;
    GFX_TEXT_S text;
    CONTROL_ELEMENT_DATA_S *data;
    
    for (index = 0; index < nbGfxElements; index++) {
        if (gfxElements[index]->type != GFX_ELEMENT_TYPE_TEXT) {
            continue;
        }
        
        data = (CONTROL_ELEMENT_DATA_S*)gfxElements[index]->pData;
        
        memcpy(&text, &gfxElements[index]->data.text, sizeof(GFX_TEXT_S));
        memset(text.str, '\0', sizeof(text.str));
        elementData->getters.getString(elementData->getters.userData, data->ids.text.stringId, nextLanguage, text.str);
        
        if (graphicsObj->setData(graphicsObj, gfxElements[index]->name, (void*)&text) != GRAPHICS_ERROR_NONE) {
            Loge("setData() failed for element \"%s\"", gfxElements[index]->name);
            ret = HANDLERS_ERROR_COMMAND;
            break;
        }
    }
    
    memset(graphicsInfos->currentLanguage, '\0', MIN_STR_SIZE);
    strcpy(graphicsInfos->currentLanguage, nextLanguage);

    return ret;
}

/*!
 *
 */
static HANDLERS_ERROR_E hideElement(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(obj && obj->pData);

    (void)gfxElementName;
    (void)gfxElementData;

    if (!handlerData) {
        Loge("Handler data is expected");
        return HANDLERS_ERROR_PARAMS;
    }

    Logd("Hiding element \"%s\"", handlerData);

    HANDLERS_PRIVATE_DATA_S *pData  = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                  = pData->ctx;
    GRAPHICS_S *graphicsObj         = ctx->modules.graphicsObj;
    GRAPHICS_INFOS_S *graphicsInfos = &ctx->params.graphicsInfos;

    if (graphicsInfos->state != MODULE_STATE_STARTED) {
        Logw("Graphics module not started - current state : %u", graphicsInfos->state);
        return HANDLERS_ERROR_STATE;
    }

    if (graphicsObj->setVisible(graphicsObj, handlerData, 0) != GRAPHICS_ERROR_NONE) {
        Loge("setVisible() failed for element \"%s\"", handlerData);
        return HANDLERS_ERROR_COMMAND;
    }

    return HANDLERS_ERROR_NONE;
}

/*!
 *
 */
static HANDLERS_ERROR_E showElement(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(obj && obj->pData);

    (void)gfxElementName;
    (void)gfxElementData;

    if (!handlerData) {
        Loge("Handler data is expected");
        return HANDLERS_ERROR_PARAMS;
    }

    Logd("Showing element \"%s\"", handlerData);

    HANDLERS_PRIVATE_DATA_S *pData  = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                  = pData->ctx;
    GRAPHICS_S *graphicsObj         = ctx->modules.graphicsObj;
    GRAPHICS_INFOS_S *graphicsInfos = &ctx->params.graphicsInfos;

    if (graphicsInfos->state != MODULE_STATE_STARTED) {
        Logw("Graphics module not started - current state : %u", graphicsInfos->state);
        return HANDLERS_ERROR_STATE;
    }

    if (graphicsObj->setVisible(graphicsObj, handlerData, 1) != GRAPHICS_ERROR_NONE) {
        Loge("setVisible() failed for element \"%s\"", handlerData);
        return HANDLERS_ERROR_COMMAND;
    }

    return HANDLERS_ERROR_NONE;
}

/*!
 *
 */
static HANDLERS_ERROR_E hideGroup(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(obj && obj->pData);

    (void)gfxElementName;
    (void)gfxElementData;

    if (!handlerData) {
        Loge("Handler data is expected");
        return HANDLERS_ERROR_PARAMS;
    }

    Logd("Hiding group \"%s\"", handlerData);

    HANDLERS_PRIVATE_DATA_S *pData  = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                  = pData->ctx;
    GRAPHICS_S *graphicsObj         = ctx->modules.graphicsObj;
    GRAPHICS_INFOS_S *graphicsInfos = &ctx->params.graphicsInfos;
    uint32_t nbGfxElements          = graphicsInfos->nbGfxElements;
    GFX_ELEMENT_S **gfxElements     = graphicsInfos->gfxElements;
    HANDLERS_ERROR_E ret            = HANDLERS_ERROR_NONE;

    if (graphicsInfos->state != MODULE_STATE_STARTED) {
        Logw("Graphics module not started - current state : %u", graphicsInfos->state);
        return HANDLERS_ERROR_STATE;
    }
    
    uint32_t index;
    for (index = 0; index < nbGfxElements; index++) {
        if (strncmp(gfxElements[index]->groupName, handlerData, sizeof(gfxElements[index]->groupName)) != 0) {
            continue;
        }

        if (graphicsObj->setVisible(graphicsObj, gfxElements[index]->name, 0) != GRAPHICS_ERROR_NONE) {
            Loge("setVisible() failed for element \"%s\"", gfxElements[index]->name);
            ret = HANDLERS_ERROR_COMMAND;
            break;
        }
    }

    return ret;
}

/*!
 *
 */
static HANDLERS_ERROR_E showGroup(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(obj && obj->pData);

    (void)gfxElementName;
    (void)gfxElementData;

    if (!handlerData) {
        Loge("Handler data is expected");
        return HANDLERS_ERROR_PARAMS;
    }

    Logd("Showing group \"%s\"", handlerData);

    HANDLERS_PRIVATE_DATA_S *pData  = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                  = pData->ctx;
    GRAPHICS_S *graphicsObj         = ctx->modules.graphicsObj;
    GRAPHICS_INFOS_S *graphicsInfos = &ctx->params.graphicsInfos;
    uint32_t nbGfxElements          = graphicsInfos->nbGfxElements;
    GFX_ELEMENT_S **gfxElements     = graphicsInfos->gfxElements;
    HANDLERS_ERROR_E ret            = HANDLERS_ERROR_NONE;

    if (graphicsInfos->state != MODULE_STATE_STARTED) {
        Logw("Graphics module not started - current state : %u", graphicsInfos->state);
        return HANDLERS_ERROR_STATE;
    }

    uint32_t index;
    for (index = 0; index < nbGfxElements; index++) {
        if (strncmp(gfxElements[index]->groupName, handlerData, sizeof(gfxElements[index]->groupName)) != 0) {
            continue;
        }

        if (graphicsObj->setVisible(graphicsObj, gfxElements[index]->name, 1) != GRAPHICS_ERROR_NONE) {
            Loge("setVisible() failed for element \"%s\"", gfxElements[index]->name);
            ret = HANDLERS_ERROR_COMMAND;
            break;
        }
    }

    return ret;
}

/*!
 *
 */
static HANDLERS_ERROR_E setFocus(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(obj && obj->pData);

    (void)gfxElementName;
    (void)gfxElementData;

    if (!handlerData) {
        Loge("Handler data is expected");
        return HANDLERS_ERROR_PARAMS;
    }

    Logd("Giving focus to element \"%s\"", handlerData);

    HANDLERS_PRIVATE_DATA_S *pData  = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                  = pData->ctx;
    GRAPHICS_S *graphicsObj         = ctx->modules.graphicsObj;
    GRAPHICS_INFOS_S *graphicsInfos = &ctx->params.graphicsInfos;

    if (graphicsInfos->state != MODULE_STATE_STARTED) {
        Logw("Graphics module not started - current state : %u", graphicsInfos->state);
        return HANDLERS_ERROR_STATE;
    }

    if (graphicsObj->setFocus(graphicsObj, handlerData) != GRAPHICS_ERROR_NONE) {
        Loge("setFocus() failed for element \"%s\"", handlerData);
        return HANDLERS_ERROR_COMMAND;
    }

    return HANDLERS_ERROR_NONE;
}

/*!
 *
 */
static HANDLERS_ERROR_E saveVideoElement(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(obj && obj->pData);

    (void)gfxElementName;
    (void)gfxElementData;

    if (!handlerData) {
        Loge("Handler data is expected");
        return HANDLERS_ERROR_PARAMS;
    }

    Logd("Saving video element \"%s\"", handlerData);

    HANDLERS_PRIVATE_DATA_S *pData  = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                  = pData->ctx;
    GRAPHICS_S *graphicsObj         = ctx->modules.graphicsObj;
    GRAPHICS_INFOS_S *graphicsInfos = &ctx->params.graphicsInfos;
    INPUT_S *input                  = &ctx->input;

    if (graphicsInfos->state != MODULE_STATE_STARTED) {
        Logw("Graphics module not started - current state : %u", graphicsInfos->state);
        return HANDLERS_ERROR_STATE;
    }

    GFX_IMAGE_S image;
    struct stat st;

    if (stat(input->appDataDir, &st) < 0) {
        Logd("Creating direcory : \"%s\"", input->appDataDir);
        if (mkdir(input->appDataDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {
            Loge("%s", strerror(errno));
            return HANDLERS_ERROR_IO;
        }
    }

    sprintf(image.path, "%s/picture_%ld.bmp", input->appDataDir, time(NULL));
    image.format = GFX_IMAGE_FORMAT_BMP;

    if (graphicsObj->saveVideoElement(graphicsObj, handlerData, &image) != GRAPHICS_ERROR_NONE) {
        Loge("saveVideoElement() failed for element \"%s\"", handlerData);
        return HANDLERS_ERROR_COMMAND;
    }

    return HANDLERS_ERROR_NONE;
}

/*!
 *
 */
static HANDLERS_ERROR_E takeScreenshot(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(obj && obj->pData);

    (void)gfxElementName;
    (void)gfxElementData;

    if (!handlerData) {
        Loge("Handler data is expected");
        return HANDLERS_ERROR_PARAMS;
    }

    Logd("Taking screnshot - Image format : \"%s\"", handlerData);

    HANDLERS_PRIVATE_DATA_S *pData  = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                  = pData->ctx;
    GRAPHICS_S *graphicsObj         = ctx->modules.graphicsObj;
    GRAPHICS_INFOS_S *graphicsInfos = &ctx->params.graphicsInfos;
    INPUT_S *input                  = &ctx->input;

    if (graphicsInfos->state != MODULE_STATE_STARTED) {
        Logw("Graphics module not started - current state : %u", graphicsInfos->state);
        return HANDLERS_ERROR_STATE;
    }

    int32_t imageFormat = atoi(handlerData);
    GFX_IMAGE_S image;
    struct stat st;

    if (stat(input->appDataDir, &st) < 0) {
        Logd("Creating direcory : \"%s\"", input->appDataDir);
        if (mkdir(input->appDataDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {
            Loge("%s", strerror(errno));
            return HANDLERS_ERROR_IO;
        }
    }

    switch (imageFormat) {
        case GFX_IMAGE_FORMAT_JPG:
            sprintf(image.path, "%s/screenshot_%ld.jpeg", input->appDataDir, time(NULL));
            image.format = GFX_IMAGE_FORMAT_JPG;
            break;

        case GFX_IMAGE_FORMAT_PNG:
            sprintf(image.path, "%s/screenshot_%ld.png", input->appDataDir, time(NULL));
            image.format = GFX_IMAGE_FORMAT_PNG;
            break;

        default:
            sprintf(image.path, "%s/screenshot_%ld.bmp", input->appDataDir, time(NULL));
            image.format = GFX_IMAGE_FORMAT_BMP;
    }

    if (graphicsObj->takeScreenshot(graphicsObj, &image) != GRAPHICS_ERROR_NONE) {
        Loge("takeScreenshot() failed for element \"%s\"", handlerData);
        return HANDLERS_ERROR_COMMAND;
    }

    return HANDLERS_ERROR_NONE;
}

/*!
 *
 */
static HANDLERS_ERROR_E setClickable(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(obj && obj->pData);

    (void)gfxElementName;
    (void)gfxElementData;

    if (!handlerData) {
        Loge("Handler data is expected");
        return HANDLERS_ERROR_PARAMS;
    }

    Logd("Setting element \"%s\" as clickable", handlerData);

    HANDLERS_PRIVATE_DATA_S *pData  = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                  = pData->ctx;
    GRAPHICS_S *graphicsObj         = ctx->modules.graphicsObj;
    GRAPHICS_INFOS_S *graphicsInfos = &ctx->params.graphicsInfos;

    if (graphicsInfos->state != MODULE_STATE_STARTED) {
        Logw("Graphics module not started - current state : %u", graphicsInfos->state);
        return HANDLERS_ERROR_STATE;
    }

    if (graphicsObj->setClickable(graphicsObj, handlerData, 1) != GRAPHICS_ERROR_NONE) {
        Loge("setClickable() failed for element \"%s\"", handlerData);
        return HANDLERS_ERROR_COMMAND;
    }

    return HANDLERS_ERROR_NONE;
}

/*!
 *
 */
static HANDLERS_ERROR_E setNotClickable(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(obj && obj->pData);

    (void)gfxElementName;
    (void)gfxElementData;

    if (!handlerData) {
        Loge("Handler data is expected");
        return HANDLERS_ERROR_PARAMS;
    }

    Logd("Setting element \"%s\" as not clickable", handlerData);

    HANDLERS_PRIVATE_DATA_S *pData  = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                  = pData->ctx;
    GRAPHICS_S *graphicsObj         = ctx->modules.graphicsObj;
    GRAPHICS_INFOS_S *graphicsInfos = &ctx->params.graphicsInfos;

    if (graphicsInfos->state != MODULE_STATE_STARTED) {
        Logw("Graphics module not started - current state : %u", graphicsInfos->state);
        return HANDLERS_ERROR_STATE;
    }

    if (graphicsObj->setClickable(graphicsObj, handlerData, 0) != GRAPHICS_ERROR_NONE) {
        Loge("setClickable() failed for element \"%s\"", handlerData);
        return HANDLERS_ERROR_COMMAND;
    }

    return HANDLERS_ERROR_NONE;
}

/*!
 *
 */
static HANDLERS_ERROR_E stopGraphics(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(obj && obj->pData);

    (void)gfxElementName;
    (void)gfxElementData;
    (void)handlerData;

    Logd("Stopping graphics module");

    HANDLERS_PRIVATE_DATA_S *pData  = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                  = pData->ctx;
    GRAPHICS_S *graphicsObj         = ctx->modules.graphicsObj;
    GRAPHICS_INFOS_S *graphicsInfos = &ctx->params.graphicsInfos;

    if (graphicsInfos->state != MODULE_STATE_STARTED) {
        Logw("Graphics module not started - current state : %u", graphicsInfos->state);
        return HANDLERS_ERROR_STATE;
    }

    if (graphicsObj->destroyDrawer(graphicsObj) != GRAPHICS_ERROR_NONE) {
        Loge("destroyDrawer() failed");
        return HANDLERS_ERROR_COMMAND;
    }

    graphicsInfos->state = MODULE_STATE_STOPPED;

    return HANDLERS_ERROR_NONE;
}

/*!
 *
 */
static HANDLERS_ERROR_E startGraphics(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(obj && obj->pData);

    (void)gfxElementName;
    (void)gfxElementData;
    (void)handlerData;

    Logd("Starting graphics module");

    HANDLERS_PRIVATE_DATA_S *pData    = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                    = pData->ctx;
    GRAPHICS_S *graphicsObj           = ctx->modules.graphicsObj;
    GRAPHICS_INFOS_S *graphicsInfos   = &ctx->params.graphicsInfos;
    GRAPHICS_PARAMS_S *graphicsParams = &ctx->params.graphicsInfos.graphicsParams;

    if (graphicsInfos->state == MODULE_STATE_STARTED) {
        Logw("Graphics module is already started");
        return HANDLERS_ERROR_STATE;
    }

    if (graphicsObj->createDrawer(graphicsObj, graphicsParams) != GRAPHICS_ERROR_NONE) {
        Loge("createDrawer() failed");
        return HANDLERS_ERROR_COMMAND;
    }

    /* Drawer is created so graphics module's state must be updated here so as to destroy
     * drawer when stopping the module */
    graphicsInfos->state = MODULE_STATE_STARTED;

    if (graphicsObj->drawAllElements(graphicsObj) != GRAPHICS_ERROR_NONE) {
        Loge("drawAllElements() failed");
        return HANDLERS_ERROR_COMMAND;
    }

    return HANDLERS_ERROR_NONE;
}

/*!
 *
 */
static HANDLERS_ERROR_E stopVideo(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(obj && obj->pData);

    (void)gfxElementName;
    (void)gfxElementData;

    if (!handlerData) {
        Loge("Handler data is expected");
        return HANDLERS_ERROR_PARAMS;
    }

    Logd("Stopping video \"%s\"", handlerData);

    HANDLERS_PRIVATE_DATA_S *pData = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                 = pData->ctx;
    VIDEO_S *videoObj              = ctx->modules.videoObj;
    VIDEOS_INFOS_S *videosInfos    = &ctx->params.videosInfos;
    VIDEO_DEVICE_S **videoDevices  = videosInfos->devices;
    uint8_t nbDevices              = videosInfos->nbDevices;
    VIDEO_DEVICE_S *videoDevice    = NULL;
    HANDLERS_ERROR_E ret           = HANDLERS_ERROR_NONE;

    uint8_t nbVideoListeners           = 0;
    VIDEO_LISTENER_S  **videoListeners = NULL;

    uint32_t videoIndex, listenerIndex;
    for (videoIndex = 0; videoIndex < nbDevices; videoIndex++) {
        videoDevice = videoDevices[videoIndex];

        if (strcmp(videoDevice->videoParams.name, handlerData) == 0) {
            Logd("Video device \"%s\" found", videoDevice->videoParams.name);

            if (videoDevice->state != MODULE_STATE_STARTED) {
                Logw("Video device \"%s\" is not started", videoDevice->videoParams.name);
                break;
            }

            nbVideoListeners = videoDevice->nbVideoListeners;
            videoListeners   = videoDevice->videoListeners;

            for (listenerIndex = 0; listenerIndex < nbVideoListeners; listenerIndex++) {
                if (videoObj->unregisterListener(videoObj, &videoDevice->videoParams, videoListeners[listenerIndex]) != VIDEO_ERROR_NONE) {
                    Loge("unregisterListener() failed - \"%s\"", (videoListeners[listenerIndex])->name);
                    ret = HANDLERS_ERROR_COMMAND;
                }
            }

            if (videoObj->stopDeviceCapture(videoObj, &videoDevice->videoParams) != VIDEO_ERROR_NONE) {
                Loge("stopDeviceCapture() failed - \"%s\"", videoDevice->videoParams.name);
                ret = HANDLERS_ERROR_COMMAND;
            }

            if (ret == HANDLERS_ERROR_NONE) {
                videoDevice->state = MODULE_STATE_STOPPED;
            }
            break;
        }
    }

    return ret;
}

/*!
 *
 */
static HANDLERS_ERROR_E startVideo(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(obj && obj->pData);

    (void)gfxElementName;
    (void)gfxElementData;

    if (!handlerData) {
        Loge("Handler data is expected");
        return HANDLERS_ERROR_PARAMS;
    }

    Logd("Starting video \"%s\"", handlerData);

    HANDLERS_PRIVATE_DATA_S *pData = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                 = pData->ctx;
    VIDEO_S *videoObj              = ctx->modules.videoObj;
    VIDEOS_INFOS_S *videosInfos    = &ctx->params.videosInfos;
    VIDEO_DEVICE_S **videoDevices  = videosInfos->devices;
    uint8_t nbDevices              = videosInfos->nbDevices;
    VIDEO_DEVICE_S *videoDevice    = NULL;
    HANDLERS_ERROR_E ret           = HANDLERS_ERROR_NONE;

    size_t maxBufferSize           = -1;
    VIDEO_AREA_S videoArea         = { 0 };

    uint8_t nbVideoListeners           = 0;
    VIDEO_LISTENER_S  **videoListeners = NULL;

    uint32_t videoIndex, listenerIndex;
    for (videoIndex = 0; videoIndex < nbDevices; videoIndex++) {
        videoDevice = videoDevices[videoIndex];

        if (strcmp(videoDevice->videoParams.name, handlerData) == 0) {
            Logd("Video device \"%s\" found", videoDevice->videoParams.name);

            if (videoDevice->state == MODULE_STATE_STARTED) {
                Logw("Video device \"%s\" is already started", videoDevice->videoParams.name);
                break;
            }

            if (videoObj->startDeviceCapture(videoObj, &videoDevice->videoParams) != VIDEO_ERROR_NONE) {
                Loge("startDeviceCapture() failed - \"%s\"", videoDevice->videoParams.name);
                return HANDLERS_ERROR_COMMAND;
            }

            /* Capture is started so the state of this video device must be updated here so as to
             * stop capture when stop is called */
            videoDevice->state = MODULE_STATE_STARTED;

            (void)videoObj->getMaxBufferSize(videoObj, &videoDevice->videoParams, &maxBufferSize);
            (void)videoObj->getFinalVideoArea(videoObj, &videoDevice->videoParams, &videoArea);

            Logd("maxBufferSize = %lu bytes / width = %u - height = %u", maxBufferSize, videoArea.width, videoArea.height);

            nbVideoListeners = videoDevice->nbVideoListeners;
            videoListeners   = videoDevice->videoListeners;

            for (listenerIndex = 0; listenerIndex < nbVideoListeners; listenerIndex++) {
                if (videoObj->registerListener(videoObj, &videoDevice->videoParams, videoListeners[listenerIndex]) != VIDEO_ERROR_NONE) {
                    Loge("Failed to register listener \"%s\"", (videoListeners[listenerIndex])->name);
                    ret = HANDLERS_ERROR_COMMAND;
                }
            }
            break;
        }
    }

    return ret;
}

/*!
 *
 */
static HANDLERS_ERROR_E stopServer(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(obj && obj->pData);

    (void)gfxElementName;
    (void)gfxElementData;

    if (!handlerData) {
        Loge("Handler data is expected");
        return HANDLERS_ERROR_PARAMS;
    }

    Logd("Starting server \"%s\"", handlerData);

    HANDLERS_PRIVATE_DATA_S *pData = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                 = pData->ctx;
    SERVER_S *serverObj            = ctx->modules.serverObj;
    SERVERS_INFOS_S *serversInfos  = &ctx->params.serversInfos;
    SERVER_INFOS_S *serverInfos    = NULL;
    HANDLERS_ERROR_E ret           = HANDLERS_ERROR_NONE;

    uint32_t index;
    for (index = 0; index < serversInfos->nbServers; index++) {
        serverInfos = serversInfos->serverInfos[index];

        if (strcmp(serverInfos->serverParams.name, handlerData) == 0) {
            Logd("Server \"%s\" found", serverInfos->serverParams.name);

            if (serverInfos->state == MODULE_STATE_STOPPED) {
                Logw("Server \"%s\" is not started", serverInfos->serverParams.name);
                break;
            }

            if (serverObj->stop(serverObj, &serverInfos->serverParams) != SERVER_ERROR_NONE) {
                Loge("Failed to stop server \"%s\"", serverInfos->serverParams.name);
                ret = HANDLERS_ERROR_COMMAND;
            }

            if (ret == HANDLERS_ERROR_NONE) {
                serverInfos->state = MODULE_STATE_STOPPED;
            }
            break;
        }
    }

    return ret;
}

/*!
 *
 */
static HANDLERS_ERROR_E startServer(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(obj && obj->pData);

    (void)gfxElementName;
    (void)gfxElementData;

    if (!handlerData) {
        Loge("Handler data is expected");
        return HANDLERS_ERROR_PARAMS;
    }

    Logd("Starting server \"%s\"", handlerData);

    HANDLERS_PRIVATE_DATA_S *pData = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                 = pData->ctx;
    SERVER_S *serverObj            = ctx->modules.serverObj;
    SERVERS_INFOS_S *serversInfos  = &ctx->params.serversInfos;
    SERVER_INFOS_S *serverInfos    = NULL;
    HANDLERS_ERROR_E ret           = HANDLERS_ERROR_NONE;

    uint32_t index;
    for (index = 0; index < serversInfos->nbServers; index++) {
        serverInfos = serversInfos->serverInfos[index];

        if (strcmp(serverInfos->serverParams.name, handlerData) == 0) {
            Logd("Server \"%s\" found", serverInfos->serverParams.name);

            if (serverInfos->state != MODULE_STATE_STOPPED) {
                Logw("Server \"%s\" is already started", serverInfos->serverParams.name);
                break;
            }

            if (serverObj->start(serverObj, &serverInfos->serverParams) != SERVER_ERROR_NONE) {
                Loge("Failed to start server \"%s\"", serverInfos->serverParams.name);
                ret = HANDLERS_ERROR_COMMAND;
            }

            if (ret == HANDLERS_ERROR_NONE) {
                serverInfos->state = MODULE_STATE_STARTED;
            }
            break;
        }
    }

    return ret;
}

/*!
 *
 */
static HANDLERS_ERROR_E suspendServer(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(obj && obj->pData);

    (void)gfxElementName;
    (void)gfxElementData;

    if (!handlerData) {
        Loge("Handler data is expected");
        return HANDLERS_ERROR_PARAMS;
    }

    Logd("Suspending server \"%s\"", handlerData);

    HANDLERS_PRIVATE_DATA_S *pData = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                 = pData->ctx;
    SERVER_S *serverObj            = ctx->modules.serverObj;
    SERVERS_INFOS_S *serversInfos  = &ctx->params.serversInfos;
    SERVER_INFOS_S *serverInfos    = NULL;
    HANDLERS_ERROR_E ret           = HANDLERS_ERROR_NONE;

    uint32_t index;
    for (index = 0; index < serversInfos->nbServers; index++) {
        serverInfos = serversInfos->serverInfos[index];

        if (strcmp(serverInfos->serverParams.name, handlerData) == 0) {
            Logd("Server \"%s\" found", serverInfos->serverParams.name);

            if (serverInfos->state == MODULE_STATE_STOPPED) {
                Logw("Server \"%s\" is currently stopped", serverInfos->serverParams.name);
                break;
            }

            if (serverInfos->state == MODULE_STATE_SUSPENDED) {
                Logw("Server \"%s\" is already suspended", serverInfos->serverParams.name);
                break;
            }

            if (serverObj->suspendSender(serverObj, &serverInfos->serverParams) != SERVER_ERROR_NONE) {
                Loge("Failed to suspend server \"%s\"", serverInfos->serverParams.name);
                ret = HANDLERS_ERROR_COMMAND;
            }

            if (ret == HANDLERS_ERROR_NONE) {
                serverInfos->state = MODULE_STATE_SUSPENDED;
            }
            break;
        }
    }

    return ret;
}

/*!
 *
 */
static HANDLERS_ERROR_E resumeServer(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(obj && obj->pData);

    (void)gfxElementName;
    (void)gfxElementData;

    if (!handlerData) {
        Loge("Handler data is expected");
        return HANDLERS_ERROR_PARAMS;
    }

    Logd("Resuming server \"%s\"", handlerData);

    HANDLERS_PRIVATE_DATA_S *pData = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                 = pData->ctx;
    SERVER_S *serverObj            = ctx->modules.serverObj;
    SERVERS_INFOS_S *serversInfos  = &ctx->params.serversInfos;
    SERVER_INFOS_S *serverInfos    = NULL;
    HANDLERS_ERROR_E ret           = HANDLERS_ERROR_NONE;

    uint32_t index;
    for (index = 0; index < serversInfos->nbServers; index++) {
        serverInfos = serversInfos->serverInfos[index];

        if (strcmp(serverInfos->serverParams.name, handlerData) == 0) {
            Logd("Server \"%s\" found", serverInfos->serverParams.name);

            if (serverInfos->state == MODULE_STATE_STOPPED) {
                Logw("Server \"%s\" is currently stopped", serverInfos->serverParams.name);
                break;
            }

            if (serverInfos->state != MODULE_STATE_SUSPENDED) {
                Logw("Server \"%s\" was not suspended", serverInfos->serverParams.name);
                break;
            }

            if (serverObj->resumeSender(serverObj, &serverInfos->serverParams) != SERVER_ERROR_NONE) {
                Loge("Failed to resume server \"%s\"", serverInfos->serverParams.name);
                ret = HANDLERS_ERROR_COMMAND;
            }

            if (ret == HANDLERS_ERROR_NONE) {
                serverInfos->state = MODULE_STATE_STARTED;
            }
            break;
        }
    }

    return ret;
}

/*!
 *
 */
static HANDLERS_ERROR_E stopCient(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(obj && obj->pData);

    (void)gfxElementName;
    (void)gfxElementData;

    if (!handlerData) {
        Loge("Handler data is expected");
        return HANDLERS_ERROR_PARAMS;
    }

    Logd("Stopping client \"%s\"", handlerData);

    HANDLERS_PRIVATE_DATA_S *pData = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                 = pData->ctx;
    CLIENT_S *clientObj            = ctx->modules.clientObj;
    CLIENTS_INFOS_S *clientsInfos  = &ctx->params.clientsInfos;
    CLIENT_INFOS_S *clientInfos    = NULL;
    HANDLERS_ERROR_E ret           = HANDLERS_ERROR_NONE;

    uint32_t index;
    for (index = 0; index < clientsInfos->nbClients; index++) {
        clientInfos = clientsInfos->clientInfos[index];

        if (strcmp(clientInfos->clientParams.name, handlerData) == 0) {
            Logd("Client \"%s\" found", clientInfos->clientParams.name);

            if (clientInfos->state == MODULE_STATE_STOPPED) {
                Logw("Client \"%s\" is already stopped", clientInfos->clientParams.name);
                break;
            }

            if (clientObj->stop(clientObj, &clientInfos->clientParams) != CLIENT_ERROR_NONE) {
                Loge("failed to stop client \"%s\"", clientInfos->clientParams.name);
                ret = HANDLERS_ERROR_COMMAND;
            }

            if (ret == HANDLERS_ERROR_NONE) {
                clientInfos->state = MODULE_STATE_STOPPED;
            }
            break;
        }
    }

    return ret;
}

/*!
 *
 */
static HANDLERS_ERROR_E startClient(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(obj && obj->pData);

    (void)gfxElementName;
    (void)gfxElementData;

    if (!handlerData) {
        Loge("Handler data is expected");
        return HANDLERS_ERROR_PARAMS;
    }

    Logd("Starting client \"%s\"", handlerData);

    HANDLERS_PRIVATE_DATA_S *pData = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                 = pData->ctx;
    CLIENT_S *clientObj            = ctx->modules.clientObj;
    CLIENTS_INFOS_S *clientsInfos  = &ctx->params.clientsInfos;
    CLIENT_INFOS_S *clientInfos    = NULL;
    HANDLERS_ERROR_E ret           = HANDLERS_ERROR_NONE;

    uint32_t index;
    for (index = 0; index < clientsInfos->nbClients; index++) {
        clientInfos = clientsInfos->clientInfos[index];

        if (strcmp(clientInfos->clientParams.name, handlerData) == 0) {
            Logd("Client \"%s\" found", clientInfos->clientParams.name);

            if (clientInfos->state != MODULE_STATE_STOPPED) {
                Logw("Client \"%s\" is already started", clientInfos->clientParams.name);
                break;
            }
            if (clientObj->start(clientObj, &clientInfos->clientParams) != CLIENT_ERROR_NONE) {
                Loge("failed to start client \"%s\"", clientInfos->clientParams.name);
                ret = HANDLERS_ERROR_COMMAND;
            }

            if (ret == HANDLERS_ERROR_NONE) {
                clientInfos->state = MODULE_STATE_STARTED;
            }
            break;
        }
    }

    return ret;
}

/*!
 *
 */
static HANDLERS_ERROR_E multiInputs(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(obj && obj->pData);

    (void)gfxElementName;
    (void)gfxElementData;

    if (!handlerData) {
        Loge("Handler data is expected");
        return HANDLERS_ERROR_PARAMS;
    }

    Logd("Handling multi inputs request - data : \"%s\"", handlerData);

    HANDLERS_PRIVATE_DATA_S *pData = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                 = pData->ctx;

    uint32_t offset                  = 0;
    char functionName[MAX_NAME_SIZE] = { 0 };
    char targetName[MAX_NAME_SIZE]   = { 0 };

    if (obj->getSubstring(obj, handlerData, ";", functionName, &offset) != HANDLERS_ERROR_NONE) {
        Loge("Bad format. Expected: customFunctionName;targetName;param1;param2;...");
        return HANDLERS_ERROR_PARAMS;
    }

    if (obj->getSubstring(obj, handlerData, ";", targetName, &offset) != HANDLERS_ERROR_NONE) {
        strncpy(targetName, handlerData + offset, sizeof(targetName));
    }

    Logd("Function : \"%s\" / Target : \"%s\"", functionName, targetName);

    uint32_t index;
    for (index = 0; index < pData->nbMultiInputsHandlers; index++) {
        if (strcmp(pData->multiInputsHandlers[index].name, functionName) == 0) {
            break;
        }
    }

    if (index >= pData->nbMultiInputsHandlers) {
        Loge("Method \"%s\" not found", functionName);
        return HANDLERS_ERROR_PARAMS;
    }

    if (!pData->multiInputsHandlers[index].fct) {
        Loge("Method \"%s\" not defined", functionName);
        return HANDLERS_ERROR_PARAMS;
    }

    return pData->multiInputsHandlers[index].fct(obj, targetName, NULL, handlerData + offset);
}
