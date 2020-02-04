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
* \file SingleInput.c
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
#define TAG "SingleInput"

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////// PRIVATE FUNCTIONS PROTOTYPES /////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

static enum handlers_error_e closeApplication(struct handlers_s *obj, char *gfxElementName,
                                              void *gfxElementData, char *handlerData);
static enum handlers_error_e changeLanguage(struct handlers_s *obj, char *gfxElementName,
                                            void *gfxElementData, char *handlerData);

static enum handlers_error_e hideElement(struct handlers_s *obj, char *gfxElementName,
                                         void *gfxElementData, char *handlerData);
static enum handlers_error_e showElement(struct handlers_s *obj, char *gfxElementName,
                                         void *gfxElementData, char *handlerData);

static enum handlers_error_e hideGroup(struct handlers_s *obj, char *gfxElementName,
                                       void *gfxElementData, char *handlerData);
static enum handlers_error_e showGroup(struct handlers_s *obj, char *gfxElementName,
                                       void *gfxElementData, char *handlerData);

static enum handlers_error_e setFocus(struct handlers_s *obj, char *gfxElementName,
                                      void *gfxElementData, char *handlerData);

static enum handlers_error_e takeScreenshot(struct handlers_s *obj, char *gfxElementName,
                                            void *gfxElementData, char *handlerData);

static enum handlers_error_e setClickable(struct handlers_s *obj, char *gfxElementName,
                                          void *gfxElementData, char *handlerData);
static enum handlers_error_e setNotClickable(struct handlers_s *obj, char *gfxElementName,
                                             void *gfxElementData, char *handlerData);

static enum handlers_error_e stopGraphics(struct handlers_s *obj, char *gfxElementName,
                                          void *gfxElementData, char *handlerData);
static enum handlers_error_e startGraphics(struct handlers_s *obj, char *gfxElementName,
                                           void *gfxElementData, char *handlerData);

static enum handlers_error_e stopVideo(struct handlers_s *obj, char *gfxElementName,
                                       void *gfxElementData, char *handlerData);
static enum handlers_error_e startVideo(struct handlers_s *obj, char *gfxElementName,
                                        void *gfxElementData, char *handlerData);

static enum handlers_error_e stopServer(struct handlers_s *obj, char *gfxElementName,
                                        void *gfxElementData, char *handlerData);
static enum handlers_error_e startServer(struct handlers_s *obj, char *gfxElementName,
                                         void *gfxElementData, char *handlerData);

static enum handlers_error_e suspendServer(struct handlers_s *obj, char *gfxElementName,
                                           void *gfxElementData, char *handlerData);
static enum handlers_error_e resumeServer(struct handlers_s *obj, char *gfxElementName,
                                          void *gfxElementData, char *handlerData);

static enum handlers_error_e stopClient(struct handlers_s *obj, char *gfxElementName,
                                        void *gfxElementData, char *handlerData);
static enum handlers_error_e startClient(struct handlers_s *obj, char *gfxElementName,
                                         void *gfxElementData, char *handlerData);

static enum handlers_error_e multiInputs(struct handlers_s *obj, char *gfxElementName,
                                         void *gfxElementData, char *handlerData);

/* -------------------------------------------------------------------------------------------- */
/* ///////////////////////////////////// GLOBAL VARIABLES ///////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

struct handlers_commands_s gSingleInputHandlers[] = {
	{ HANDLERS_COMMAND_CLOSE_APPLICATION,   NULL,  closeApplication },
	{ HANDLERS_COMMAND_CHANGE_LANGUAGE,     NULL,  changeLanguage   },
	{ HANDLERS_COMMAND_TAKE_SCREENSHOT,     NULL,  takeScreenshot   },
	{ HANDLERS_COMMAND_HIDE_ELEMENT,        NULL,  hideElement      },
	{ HANDLERS_COMMAND_SHOW_ELEMENT,        NULL,  showElement      },
	{ HANDLERS_COMMAND_SET_FOCUS,           NULL,  setFocus         },
	{ HANDLERS_COMMAND_HIDE_GROUP,          NULL,  hideGroup        },
	{ HANDLERS_COMMAND_SHOW_GROUP,          NULL,  showGroup        },
	{ HANDLERS_COMMAND_SET_CLICKABLE,       NULL,  setClickable     },
	{ HANDLERS_COMMAND_SET_NOT_CLICKABLE,   NULL,  setNotClickable  },
	{ HANDLERS_COMMAND_STOP_GRAPHICS,       NULL,  stopGraphics     },
	{ HANDLERS_COMMAND_START_GRAPHICS,      NULL,  startGraphics    },
	{ HANDLERS_COMMAND_STOP_VIDEO,          NULL,  stopVideo        },
	{ HANDLERS_COMMAND_START_VIDEO,         NULL,  startVideo       },
	{ HANDLERS_COMMAND_STOP_SERVER,         NULL,  stopServer       },
	{ HANDLERS_COMMAND_START_SERVER,        NULL,  startServer      },
	{ HANDLERS_COMMAND_SUSPEND_SERVER,      NULL,  suspendServer    },
	{ HANDLERS_COMMAND_RESUME_SERVER,       NULL,  resumeServer     },
	{ HANDLERS_COMMAND_STOP_CLIENT,         NULL,  stopClient       },
	{ HANDLERS_COMMAND_START_CLIENT,        NULL,  startClient      },

	{ HANDLERS_COMMAND_MULTI_INPUTS,        NULL,  multiInputs      },
	{ NULL,                                 NULL,  NULL             }
};

uint32_t gNbSingleInputHandlers = (uint32_t)(sizeof(gSingleInputHandlers)
                                           / sizeof(gSingleInputHandlers[0]));

/* -------------------------------------------------------------------------------------------- */
/* ///////////////////////////// PRIVATE FUNCTIONS IMPLEMENTATION ///////////////////////////// */
/* -------------------------------------------------------------------------------------------- */


/*!
 *
 */
static enum handlers_error_e closeApplication(struct handlers_s *obj, char *gfxElementName,
                                              void *gfxElementData, char *handlerData)
{
    ASSERT(obj && obj->pData);

    (void)gfxElementName;
    (void)gfxElementData;
    (void)handlerData;

    Logd("Closing application");

    struct handlers_private_data_s *pData = (struct handlers_private_data_s*)(obj->pData);
    struct context_s *ctx                 = pData->handlersParams.ctx;
    struct graphics_s *graphicsObj        = ctx->modules.graphicsObj;
    struct input_s *input                 = &ctx->input;
    enum handlers_error_e ret             = HANDLERS_ERROR_NONE;

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
static enum handlers_error_e changeLanguage(struct handlers_s *obj, char *gfxElementName,
                                            void *gfxElementData, char *handlerData)
{
    ASSERT(obj && obj->pData && gfxElementData);

    (void)gfxElementName;

    struct handlers_private_data_s *pData      = (struct handlers_private_data_s*)(obj->pData);
    struct context_s *ctx                      = pData->handlersParams.ctx;
    struct control_element_data_s *elementData = (struct control_element_data_s*)gfxElementData;
    struct graphics_s *graphicsObj             = ctx->modules.graphicsObj;
    struct graphics_infos_s *graphicsInfos     = &ctx->params.graphicsInfos;
    uint32_t nbGfxElements                     = graphicsInfos->nbGfxElements;
    struct gfx_element_s **gfxElements         = graphicsInfos->gfxElements;
    enum handlers_error_e ret                  = HANDLERS_ERROR_NONE;

    if (graphicsInfos->state != MODULE_STATE_STARTED) {
        Logw("Graphics module is not started - current state : %u", graphicsInfos->state);
        return HANDLERS_ERROR_STATE;
    }
    
    char nextLanguage[MIN_STR_SIZE] = {0};

    if (!handlerData || (handlerData[0] == '\0')) {
        elementData->getters.getLanguage(elementData->getters.userData,
                                         graphicsInfos->currentLanguage, nextLanguage);
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
    struct gfx_text_s text;
    struct control_element_data_s *data;
    char str[MAX_STR_SIZE] = {0};
    
    for (index = 0; index < nbGfxElements; index++) {
        if (gfxElements[index]->type != GFX_ELEMENT_TYPE_TEXT) {
            continue;
        }
        
        data = (struct control_element_data_s*)gfxElements[index]->pData;
        
        memset(str, '\0', sizeof(str));
        elementData->getters.getString(elementData->getters.userData, data->ids.text.stringId,
                                                                      nextLanguage, str);
        if (str[0] == '\0') {
            Logw("Translation not available for language \"%s\"", nextLanguage);
            continue;
        }

        memcpy(&text, &gfxElements[index]->data.text, sizeof(struct gfx_text_s));
        if (strncmp(text.str, str, sizeof(str)) == 0) {
            Logw("Translated text is the same for language \"%s\"", nextLanguage);
            continue;
        }

        snprintf(text.str, sizeof(text.str), "%s", str);

        if (graphicsObj->setData(graphicsObj, gfxElements[index]->name,
                                              (void*)&text) != GRAPHICS_ERROR_NONE) {
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
static enum handlers_error_e hideElement(struct handlers_s *obj, char *gfxElementName,
                                         void *gfxElementData, char *handlerData)
{
    ASSERT(obj && obj->pData);

    (void)gfxElementName;
    (void)gfxElementData;

    if (!handlerData) {
        Loge("Handler data is expected");
        return HANDLERS_ERROR_PARAMS;
    }

    Logd("Hiding element \"%s\"", handlerData);

    struct handlers_private_data_s *pData  = (struct handlers_private_data_s*)(obj->pData);
    struct context_s *ctx                  = pData->handlersParams.ctx;
    struct graphics_s *graphicsObj         = ctx->modules.graphicsObj;
    struct graphics_infos_s *graphicsInfos = &ctx->params.graphicsInfos;

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
static enum handlers_error_e showElement(struct handlers_s *obj, char *gfxElementName,
                                         void *gfxElementData, char *handlerData)
{
    ASSERT(obj && obj->pData);

    (void)gfxElementName;
    (void)gfxElementData;

    if (!handlerData) {
        Loge("Handler data is expected");
        return HANDLERS_ERROR_PARAMS;
    }

    Logd("Showing element \"%s\"", handlerData);

    struct handlers_private_data_s *pData  = (struct handlers_private_data_s*)(obj->pData);
    struct context_s *ctx                  = pData->handlersParams.ctx;
    struct graphics_s *graphicsObj         = ctx->modules.graphicsObj;
    struct graphics_infos_s *graphicsInfos = &ctx->params.graphicsInfos;

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
static enum handlers_error_e hideGroup(struct handlers_s *obj, char *gfxElementName,
                                       void *gfxElementData, char *handlerData)
{
    ASSERT(obj && obj->pData);

    (void)gfxElementName;
    (void)gfxElementData;

    if (!handlerData) {
        Loge("Handler data is expected");
        return HANDLERS_ERROR_PARAMS;
    }

    Logd("Hiding group \"%s\"", handlerData);

    struct handlers_private_data_s *pData  = (struct handlers_private_data_s*)(obj->pData);
    struct context_s *ctx                  = pData->handlersParams.ctx;
    struct graphics_s *graphicsObj         = ctx->modules.graphicsObj;
    struct graphics_infos_s *graphicsInfos = &ctx->params.graphicsInfos;
    uint32_t nbGfxElements                 = graphicsInfos->nbGfxElements;
    struct gfx_element_s **gfxElements     = graphicsInfos->gfxElements;
    enum handlers_error_e ret              = HANDLERS_ERROR_NONE;

    if (graphicsInfos->state != MODULE_STATE_STARTED) {
        Logw("Graphics module not started - current state : %u", graphicsInfos->state);
        return HANDLERS_ERROR_STATE;
    }
    
    uint32_t index;
    for (index = 0; index < nbGfxElements; index++) {
        if (strncmp(gfxElements[index]->groupName, handlerData,
                                                   sizeof(gfxElements[index]->groupName)) != 0) {
            continue;
        }

        if (graphicsObj->setVisible(graphicsObj,
                                    gfxElements[index]->name, 0) != GRAPHICS_ERROR_NONE) {
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
static enum handlers_error_e showGroup(struct handlers_s *obj, char *gfxElementName,
                                       void *gfxElementData, char *handlerData)
{
    ASSERT(obj && obj->pData);

    (void)gfxElementName;
    (void)gfxElementData;

    if (!handlerData) {
        Loge("Handler data is expected");
        return HANDLERS_ERROR_PARAMS;
    }

    Logd("Showing group \"%s\"", handlerData);

    struct handlers_private_data_s *pData  = (struct handlers_private_data_s*)(obj->pData);
    struct context_s *ctx                  = pData->handlersParams.ctx;
    struct graphics_s *graphicsObj         = ctx->modules.graphicsObj;
    struct graphics_infos_s *graphicsInfos = &ctx->params.graphicsInfos;
    uint32_t nbGfxElements                 = graphicsInfos->nbGfxElements;
    struct gfx_element_s **gfxElements     = graphicsInfos->gfxElements;
    enum handlers_error_e ret              = HANDLERS_ERROR_NONE;

    if (graphicsInfos->state != MODULE_STATE_STARTED) {
        Logw("Graphics module not started - current state : %u", graphicsInfos->state);
        return HANDLERS_ERROR_STATE;
    }

    uint32_t index;
    for (index = 0; index < nbGfxElements; index++) {
        if (strncmp(gfxElements[index]->groupName, handlerData,
                                                   sizeof(gfxElements[index]->groupName)) != 0) {
            continue;
        }

        if (graphicsObj->setVisible(graphicsObj,
                                    gfxElements[index]->name, 1) != GRAPHICS_ERROR_NONE) {
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
static enum handlers_error_e setFocus(struct handlers_s *obj, char *gfxElementName,
                                      void *gfxElementData, char *handlerData)
{
    ASSERT(obj && obj->pData);

    (void)gfxElementName;
    (void)gfxElementData;

    if (!handlerData) {
        Loge("Handler data is expected");
        return HANDLERS_ERROR_PARAMS;
    }

    Logd("Giving focus to element \"%s\"", handlerData);

    struct handlers_private_data_s *pData  = (struct handlers_private_data_s*)(obj->pData);
    struct context_s *ctx                  = pData->handlersParams.ctx;
    struct graphics_s *graphicsObj         = ctx->modules.graphicsObj;
    struct graphics_infos_s *graphicsInfos = &ctx->params.graphicsInfos;

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
static enum handlers_error_e takeScreenshot(struct handlers_s *obj, char *gfxElementName,
                                            void *gfxElementData, char *handlerData)
{
    ASSERT(obj && obj->pData);

    (void)gfxElementName;
    (void)gfxElementData;

    if (!handlerData) {
        Loge("Handler data is expected");
        return HANDLERS_ERROR_PARAMS;
    }

    Logd("Taking screnshot - Image format : \"%s\"", handlerData);

    struct handlers_private_data_s *pData  = (struct handlers_private_data_s*)(obj->pData);
    struct context_s *ctx                  = pData->handlersParams.ctx;
    struct graphics_s *graphicsObj         = ctx->modules.graphicsObj;
    struct graphics_infos_s *graphicsInfos = &ctx->params.graphicsInfos;
    struct input_s *input                  = &ctx->input;

    if (graphicsInfos->state != MODULE_STATE_STARTED) {
        Logw("Graphics module not started - current state : %u", graphicsInfos->state);
        return HANDLERS_ERROR_STATE;
    }

    uint32_t imageFormat = (uint32_t)atoi(handlerData);
    struct gfx_image_s image;
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
static enum handlers_error_e setClickable(struct handlers_s *obj, char *gfxElementName,
                                          void *gfxElementData, char *handlerData)
{
    ASSERT(obj && obj->pData);

    (void)gfxElementName;
    (void)gfxElementData;

    if (!handlerData) {
        Loge("Handler data is expected");
        return HANDLERS_ERROR_PARAMS;
    }

    Logd("Setting element \"%s\" as clickable", handlerData);

    struct handlers_private_data_s *pData  = (struct handlers_private_data_s*)(obj->pData);
    struct context_s *ctx                  = pData->handlersParams.ctx;
    struct graphics_s *graphicsObj         = ctx->modules.graphicsObj;
    struct graphics_infos_s *graphicsInfos = &ctx->params.graphicsInfos;

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
static enum handlers_error_e setNotClickable(struct handlers_s *obj, char *gfxElementName,
                                             void *gfxElementData, char *handlerData)
{
    ASSERT(obj && obj->pData);

    (void)gfxElementName;
    (void)gfxElementData;

    if (!handlerData) {
        Loge("Handler data is expected");
        return HANDLERS_ERROR_PARAMS;
    }

    Logd("Setting element \"%s\" as not clickable", handlerData);

    struct handlers_private_data_s *pData  = (struct handlers_private_data_s*)(obj->pData);
    struct context_s *ctx                  = pData->handlersParams.ctx;
    struct graphics_s *graphicsObj         = ctx->modules.graphicsObj;
    struct graphics_infos_s *graphicsInfos = &ctx->params.graphicsInfos;

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
static enum handlers_error_e stopGraphics(struct handlers_s *obj, char *gfxElementName,
                                          void *gfxElementData, char *handlerData)
{
    ASSERT(obj && obj->pData);

    (void)gfxElementName;
    (void)gfxElementData;
    (void)handlerData;

    Logd("Stopping graphics module");

    struct handlers_private_data_s *pData    = (struct handlers_private_data_s*)(obj->pData);
    struct handlers_params_s *hParams        = &pData->handlersParams;
    struct context_s *ctx                    = hParams->ctx;
    struct graphics_s *graphicsObj           = ctx->modules.graphicsObj;
    struct graphics_infos_s *graphicsInfos   = &ctx->params.graphicsInfos;
    struct graphics_params_s *graphicsParams = &ctx->params.graphicsInfos.graphicsParams;

    if (graphicsInfos->state != MODULE_STATE_STARTED) {
        Logw("Graphics module not started - current state : %u", graphicsInfos->state);
        return HANDLERS_ERROR_STATE;
    }

    if (graphicsObj->destroyDrawer(graphicsObj) != GRAPHICS_ERROR_NONE) {
        Loge("destroyDrawer() failed");
        return HANDLERS_ERROR_COMMAND;
    }

    graphicsInfos->state = MODULE_STATE_STOPPED;

    if (hParams->onModuleStateChangedCb) {
        hParams->onModuleStateChangedCb(hParams->userData,
                                        graphicsParams->screenParams.name,
                                        graphicsInfos->state);
    }

    return HANDLERS_ERROR_NONE;
}

/*!
 *
 */
static enum handlers_error_e startGraphics(struct handlers_s *obj, char *gfxElementName,
                                           void *gfxElementData, char *handlerData)
{
    ASSERT(obj && obj->pData);

    (void)gfxElementName;
    (void)gfxElementData;
    (void)handlerData;

    Logd("Starting graphics module");

    struct handlers_private_data_s *pData    = (struct handlers_private_data_s*)(obj->pData);
    struct handlers_params_s *hParams        = &pData->handlersParams;
    struct context_s *ctx                    = hParams->ctx;
    struct graphics_s *graphicsObj           = ctx->modules.graphicsObj;
    struct graphics_infos_s *graphicsInfos   = &ctx->params.graphicsInfos;
    struct graphics_params_s *graphicsParams = &ctx->params.graphicsInfos.graphicsParams;
    enum handlers_error_e ret                = HANDLERS_ERROR_NONE;

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
        ret = HANDLERS_ERROR_COMMAND;
    }

    if (hParams->onModuleStateChangedCb) {
        hParams->onModuleStateChangedCb(hParams->userData,
                                        graphicsParams->screenParams.name,
                                        graphicsInfos->state);
    }

    return ret;
}

/*!
 *
 */
static enum handlers_error_e stopVideo(struct handlers_s *obj, char *gfxElementName,
                                       void *gfxElementData, char *handlerData)
{
    ASSERT(obj && obj->pData);

    (void)gfxElementName;
    (void)gfxElementData;

    if (!handlerData) {
        Loge("Handler data is expected");
        return HANDLERS_ERROR_PARAMS;
    }

    Logd("Stopping video \"%s\"", handlerData);

    struct handlers_private_data_s *pData = (struct handlers_private_data_s*)(obj->pData);
    struct handlers_params_s *hParams     = &pData->handlersParams;
    struct context_s *ctx                 = hParams->ctx;
    struct video_s *videoObj              = ctx->modules.videoObj;
    struct videos_infos_s *videosInfos    = &ctx->params.videosInfos;
    struct video_device_s **videoDevices  = videosInfos->devices;
    uint8_t nbDevices                     = videosInfos->nbDevices;
    struct video_device_s *videoDevice    = NULL;
    enum handlers_error_e ret             = HANDLERS_ERROR_NONE;

    uint8_t nbVideoListeners                  = 0;
    struct video_listener_s  **videoListeners = NULL;

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
                if (videoObj->unregisterListener(videoObj, &videoDevice->videoParams,
                                                           videoListeners[listenerIndex])
                                                           != VIDEO_ERROR_NONE) {
                    Loge("unregisterListener() failed - \"%s\"",
                            (videoListeners[listenerIndex])->name);
                    ret = HANDLERS_ERROR_COMMAND;
                }
            }

            if (videoObj->stopDeviceCapture(videoObj,
                                            &videoDevice->videoParams) != VIDEO_ERROR_NONE) {
                Loge("stopDeviceCapture() failed - \"%s\"", videoDevice->videoParams.name);
                ret = HANDLERS_ERROR_COMMAND;
            }

            if (ret == HANDLERS_ERROR_NONE) {
                videoDevice->state = MODULE_STATE_STOPPED;

                if (hParams->onModuleStateChangedCb) {
                    hParams->onModuleStateChangedCb(hParams->userData,
                                                    videoDevice->videoParams.name,
                                                    videoDevice->state);
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
static enum handlers_error_e startVideo(struct handlers_s *obj, char *gfxElementName,
                                        void *gfxElementData, char *handlerData)
{
    ASSERT(obj && obj->pData);

    (void)gfxElementName;
    (void)gfxElementData;

    if (!handlerData) {
        Loge("Handler data is expected");
        return HANDLERS_ERROR_PARAMS;
    }

    Logd("Starting video \"%s\"", handlerData);

    struct handlers_private_data_s *pData = (struct handlers_private_data_s*)(obj->pData);
    struct handlers_params_s *hParams     = &pData->handlersParams;
    struct context_s *ctx                 = hParams->ctx;
    struct video_s *videoObj              = ctx->modules.videoObj;
    struct videos_infos_s *videosInfos    = &ctx->params.videosInfos;
    struct video_device_s **videoDevices  = videosInfos->devices;
    uint8_t nbDevices                     = videosInfos->nbDevices;
    struct video_device_s *videoDevice    = NULL;
    enum handlers_error_e ret             = HANDLERS_ERROR_NONE;

    size_t maxBufferSize          = -1;
    struct video_area_s videoArea = {0};

    uint8_t nbVideoListeners                  = 0;
    struct video_listener_s  **videoListeners = NULL;

    uint32_t videoIndex, listenerIndex;
    for (videoIndex = 0; videoIndex < nbDevices; videoIndex++) {
        videoDevice = videoDevices[videoIndex];

        if (strcmp(videoDevice->videoParams.name, handlerData) == 0) {
            Logd("Video device \"%s\" found", videoDevice->videoParams.name);

            if (videoDevice->state == MODULE_STATE_STARTED) {
                Logw("Video device \"%s\" is already started", videoDevice->videoParams.name);
                break;
            }

            if (videoObj->startDeviceCapture(videoObj,
                                             &videoDevice->videoParams) != VIDEO_ERROR_NONE) {
                Loge("startDeviceCapture() failed - \"%s\"", videoDevice->videoParams.name);
                return HANDLERS_ERROR_COMMAND;
            }

            /* Capture is started so the state of this video device must be updated here so as to
             * stop capture when stop is called */
            videoDevice->state = MODULE_STATE_STARTED;

            (void)videoObj->getMaxBufferSize(videoObj, &videoDevice->videoParams, &maxBufferSize);
            (void)videoObj->getFinalVideoArea(videoObj, &videoDevice->videoParams, &videoArea);

            Logd("maxBufferSize = %lu bytes / width = %u - height = %u",
                    maxBufferSize, videoArea.width, videoArea.height);

            nbVideoListeners = videoDevice->nbVideoListeners;
            videoListeners   = videoDevice->videoListeners;

            for (listenerIndex = 0; listenerIndex < nbVideoListeners; listenerIndex++) {
                if (videoObj->registerListener(videoObj,
                                               &videoDevice->videoParams,
                                               videoListeners[listenerIndex]) != VIDEO_ERROR_NONE) {
                    Loge("Failed to register listener \"%s\"",
                            (videoListeners[listenerIndex])->name);
                    ret = HANDLERS_ERROR_COMMAND;
                }
            }

            if (hParams->onModuleStateChangedCb) {
                hParams->onModuleStateChangedCb(hParams->userData,
                                                videoDevice->videoParams.name,
                                                videoDevice->state);
            }
            break;
        }
    }

    return ret;
}

/*!
 *
 */
static enum handlers_error_e stopServer(struct handlers_s *obj, char *gfxElementName,
                                        void *gfxElementData, char *handlerData)
{
    ASSERT(obj && obj->pData);

    (void)gfxElementName;
    (void)gfxElementData;

    if (!handlerData) {
        Loge("Handler data is expected");
        return HANDLERS_ERROR_PARAMS;
    }

    Logd("Starting server \"%s\"", handlerData);

    struct handlers_private_data_s *pData = (struct handlers_private_data_s*)(obj->pData);
    struct handlers_params_s *hParams     = &pData->handlersParams;
    struct context_s *ctx                 = hParams->ctx;
    struct server_s *serverObj            = ctx->modules.serverObj;
    struct servers_infos_s *serversInfos  = &ctx->params.serversInfos;
    struct server_infos_s *serverInfos    = NULL;
    enum handlers_error_e ret             = HANDLERS_ERROR_NONE;

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

                if (hParams->onModuleStateChangedCb) {
                    hParams->onModuleStateChangedCb(hParams->userData,
                                                    serverInfos->serverParams.name,
                                                    serverInfos->state);
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
static enum handlers_error_e startServer(struct handlers_s *obj, char *gfxElementName,
                                         void *gfxElementData, char *handlerData)
{
    ASSERT(obj && obj->pData);

    (void)gfxElementName;
    (void)gfxElementData;

    if (!handlerData) {
        Loge("Handler data is expected");
        return HANDLERS_ERROR_PARAMS;
    }

    Logd("Starting server \"%s\"", handlerData);

    struct handlers_private_data_s *pData = (struct handlers_private_data_s*)(obj->pData);
    struct handlers_params_s *hParams     = &pData->handlersParams;
    struct context_s *ctx                 = hParams->ctx;
    struct server_s *serverObj            = ctx->modules.serverObj;
    struct servers_infos_s *serversInfos  = &ctx->params.serversInfos;
    struct server_infos_s *serverInfos    = NULL;
    enum handlers_error_e ret             = HANDLERS_ERROR_NONE;

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

                if (hParams->onModuleStateChangedCb) {
                    hParams->onModuleStateChangedCb(hParams->userData,
                                                    serverInfos->serverParams.name,
                                                    serverInfos->state);
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
static enum handlers_error_e suspendServer(struct handlers_s *obj, char *gfxElementName,
                                           void *gfxElementData, char *handlerData)
{
    ASSERT(obj && obj->pData);

    (void)gfxElementName;
    (void)gfxElementData;

    if (!handlerData) {
        Loge("Handler data is expected");
        return HANDLERS_ERROR_PARAMS;
    }

    Logd("Suspending server \"%s\"", handlerData);

    struct handlers_private_data_s *pData = (struct handlers_private_data_s*)(obj->pData);
    struct handlers_params_s *hParams     = &pData->handlersParams;
    struct context_s *ctx                 = hParams->ctx;
    struct server_s *serverObj            = ctx->modules.serverObj;
    struct servers_infos_s *serversInfos  = &ctx->params.serversInfos;
    struct server_infos_s *serverInfos    = NULL;
    enum handlers_error_e ret             = HANDLERS_ERROR_NONE;

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

            if (serverObj->suspendSender(serverObj,
                                         &serverInfos->serverParams) != SERVER_ERROR_NONE) {
                Loge("Failed to suspend server \"%s\"", serverInfos->serverParams.name);
                ret = HANDLERS_ERROR_COMMAND;
            }

            if (ret == HANDLERS_ERROR_NONE) {
                serverInfos->state = MODULE_STATE_SUSPENDED;

                if (hParams->onModuleStateChangedCb) {
                    hParams->onModuleStateChangedCb(hParams->userData,
                                                    serverInfos->serverParams.name,
                                                    serverInfos->state);
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
static enum handlers_error_e resumeServer(struct handlers_s *obj, char *gfxElementName,
                                          void *gfxElementData, char *handlerData)
{
    ASSERT(obj && obj->pData);

    (void)gfxElementName;
    (void)gfxElementData;

    if (!handlerData) {
        Loge("Handler data is expected");
        return HANDLERS_ERROR_PARAMS;
    }

    Logd("Resuming server \"%s\"", handlerData);

    struct handlers_private_data_s *pData = (struct handlers_private_data_s*)(obj->pData);
    struct handlers_params_s *hParams     = &pData->handlersParams;
    struct context_s *ctx                 = hParams->ctx;
    struct server_s *serverObj            = ctx->modules.serverObj;
    struct servers_infos_s *serversInfos  = &ctx->params.serversInfos;
    struct server_infos_s *serverInfos    = NULL;
    enum handlers_error_e ret             = HANDLERS_ERROR_NONE;

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

            if (serverObj->resumeSender(serverObj,
                                        &serverInfos->serverParams) != SERVER_ERROR_NONE) {
                Loge("Failed to resume server \"%s\"", serverInfos->serverParams.name);
                ret = HANDLERS_ERROR_COMMAND;
            }

            if (ret == HANDLERS_ERROR_NONE) {
                serverInfos->state = MODULE_STATE_STARTED;

                if (hParams->onModuleStateChangedCb) {
                    hParams->onModuleStateChangedCb(hParams->userData,
                                                    serverInfos->serverParams.name,
                                                    serverInfos->state);
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
static enum handlers_error_e stopClient(struct handlers_s *obj, char *gfxElementName,
                                        void *gfxElementData, char *handlerData)
{
    ASSERT(obj && obj->pData);

    (void)gfxElementName;
    (void)gfxElementData;

    if (!handlerData) {
        Loge("Handler data is expected");
        return HANDLERS_ERROR_PARAMS;
    }

    Logd("Stopping client \"%s\"", handlerData);

    struct handlers_private_data_s *pData = (struct handlers_private_data_s*)(obj->pData);
    struct handlers_params_s *hParams     = &pData->handlersParams;
    struct context_s *ctx                 = hParams->ctx;
    struct client_s *clientObj            = ctx->modules.clientObj;
    struct clients_infos_s *clientsInfos  = &ctx->params.clientsInfos;
    struct client_infos_s *clientInfos    = NULL;
    enum handlers_error_e ret             = HANDLERS_ERROR_NONE;

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

                if (hParams->onModuleStateChangedCb) {
                    hParams->onModuleStateChangedCb(hParams->userData,
                                                    clientInfos->clientParams.name,
                                                    clientInfos->state);
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
static enum handlers_error_e startClient(struct handlers_s *obj, char *gfxElementName,
                                         void *gfxElementData, char *handlerData)
{
    ASSERT(obj && obj->pData);

    (void)gfxElementName;
    (void)gfxElementData;

    if (!handlerData) {
        Loge("Handler data is expected");
        return HANDLERS_ERROR_PARAMS;
    }

    Logd("Starting client \"%s\"", handlerData);

    struct handlers_private_data_s *pData = (struct handlers_private_data_s*)(obj->pData);
    struct handlers_params_s *hParams     = &pData->handlersParams;
    struct context_s *ctx                 = hParams->ctx;
    struct client_s *clientObj            = ctx->modules.clientObj;
    struct clients_infos_s *clientsInfos  = &ctx->params.clientsInfos;
    struct client_infos_s *clientInfos    = NULL;
    enum handlers_error_e ret             = HANDLERS_ERROR_NONE;

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

                if (hParams->onModuleStateChangedCb) {
                    hParams->onModuleStateChangedCb(hParams->userData,
                                                    clientInfos->clientParams.name,
                                                    clientInfos->state);
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
static enum handlers_error_e multiInputs(struct handlers_s *obj, char *gfxElementName,
                                         void *gfxElementData, char *handlerData)
{
    ASSERT(obj && obj->pData);

    (void)gfxElementName;
    (void)gfxElementData;

    if (!handlerData) {
        Loge("Handler data is expected");
        return HANDLERS_ERROR_PARAMS;
    }

    Logd("Handling multi inputs request - data : \"%s\"", handlerData);

    struct handlers_private_data_s *pData = (struct handlers_private_data_s*)(obj->pData);

    uint32_t offset                  = 0;
    char functionName[MAX_NAME_SIZE] = {0};
    char targetName[MAX_NAME_SIZE]   = {0};

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
