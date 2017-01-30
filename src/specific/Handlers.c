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
* \file   Handlers.c
* \brief  TODO
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include "specific/Specific.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                            */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "SPECIFIC-HANDLERS"

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

typedef void (*SPECIFIC_CLICK_HANDLER_F)(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData);

typedef struct SPECIFIC_CLICK_HANDLERS_S {
    char                     *name;
    char                     *data;
    SPECIFIC_CLICK_HANDLER_F fct;
} SPECIFIC_CLICK_HANDLERS_S;

typedef struct SPECIFIC_ELEMENT_DATA_S {
    uint32_t                  index;
    
    union {
        SPECIFIC_TEXT_IDS_S   text;
        SPECIFIC_IMAGE_IDS_S  image;
    } ids;
    
    uint32_t                  nbClickHandlers;
    SPECIFIC_CLICK_HANDLERS_S *clickHandlers;
    
    SPECIFIC_GETTERS_S        getters;
} SPECIFIC_ELEMENT_DATA_S;

/* -------------------------------------------------------------------------------------------- */
/*                                         PROTOTYPES                                           */
/* -------------------------------------------------------------------------------------------- */

/* Public functions */
SPECIFIC_ERROR_E initElementData_f  (SPECIFIC_S *obj, void **data);
SPECIFIC_ERROR_E uninitElementData_f(SPECIFIC_S *obj, void **data);

SPECIFIC_ERROR_E setElementGetters_f  (SPECIFIC_S *obj, void *data, SPECIFIC_GETTERS_S *getters);
SPECIFIC_ERROR_E unsetElementGetters_f(SPECIFIC_S *obj, void *data);

SPECIFIC_ERROR_E setElementTextIds_f  (SPECIFIC_S *obj, void *data, SPECIFIC_TEXT_IDS_S *textIds);
SPECIFIC_ERROR_E unsetElementTextIds_f(SPECIFIC_S *obj, void *data);

SPECIFIC_ERROR_E setElementImageIds_f  (SPECIFIC_S *obj, void *data, SPECIFIC_IMAGE_IDS_S *imageIds);
SPECIFIC_ERROR_E unsetElementImageIds_f(SPECIFIC_S *obj, void *data);

SPECIFIC_ERROR_E setClickHandlers_f  (SPECIFIC_S *obj, void *data, SPECIFIC_HANDLERS_S *handlers, uint32_t nbHandlers, uint32_t index);
SPECIFIC_ERROR_E unsetClickHandlers_f(SPECIFIC_S *obj, void *data);

SPECIFIC_ERROR_E handleClick_f(CONTEXT_S *ctx, GFX_EVENT_S *gfxEvent);

/* Click handlers */
static void closeApplication(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData);
static void changeLanguage(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData);

static void hideElement(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData);
static void showElement(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData);

static void hideGroup(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData);
static void showGroup(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData);

static void setFocus(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData);

static void saveVideoFrame(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData);
static void takeScreenshot(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData);

static void setClickable   (CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData);
static void setNotClickable(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData);

static void stopGraphics (CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData);
static void startGraphics(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData);

static void stopVideo (CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData);
static void startVideo(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData);

static void stopServer (CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData);
static void startServer(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData);

static void suspendServer(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData);
static void resumeServer (CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData);

static void stopCient  (CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData);
static void startClient(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData);

/* -------------------------------------------------------------------------------------------- */
/*                                          VARIABLES                                           */
/* -------------------------------------------------------------------------------------------- */

static SPECIFIC_CLICK_HANDLERS_S gClickHandlers[] = {
	{ "closeApplication",           NULL,             closeApplication },
	{ "changeLanguage",             NULL,             changeLanguage   },
	{ "hideElement",                NULL,             hideElement      },
	{ "showElement",                NULL,             showElement      },
	{ "hideGroup",                  NULL,             hideGroup        },
	{ "showGroup",                  NULL,             showGroup        },
	{ "setFocus",                   NULL,             setFocus         },
	{ "saveVideoFrame",             NULL,             saveVideoFrame   },
	{ "takeScreenshot",             NULL,             takeScreenshot   },
	{ "setClickable",               NULL,             setClickable     },
	{ "setNotClickable",            NULL,             setNotClickable  },
	{ "stopGraphics",               NULL,             stopGraphics     },
	{ "startGraphics",              NULL,             startGraphics    },
	{ "stopVideo",                  NULL,             stopVideo        },
	{ "startVideo",                 NULL,             startVideo       },
	{ "stopServer",                 NULL,             stopServer       },
	{ "startServer",                NULL,             startServer      },
	{ "suspendServer",              NULL,             suspendServer    },
	{ "resumeServer",               NULL,             resumeServer     },
	{ "stopCient",                  NULL,             stopCient        },
	{ "startClient",                NULL,             startClient      },
	{ NULL,                         NULL,             NULL             }
};

uint32_t gNbClickHandlers = (uint32_t)(sizeof(gClickHandlers) / sizeof(gClickHandlers[0]));

/* -------------------------------------------------------------------------------------------- */
/*                                          FUNCTIONS                                           */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
SPECIFIC_ERROR_E initElementData_f(SPECIFIC_S *obj, void **data)
{
    assert(obj && data);

    SPECIFIC_ELEMENT_DATA_S *elementData;
    assert((elementData = calloc(1, sizeof(SPECIFIC_ELEMENT_DATA_S))));

    *data = elementData;

    return SPECIFIC_ERROR_NONE;
}

/*!
 *
 */
SPECIFIC_ERROR_E uninitElementData_f(SPECIFIC_S *obj, void **data)
{
    assert(obj && data && *data);

    SPECIFIC_ELEMENT_DATA_S *elementData = (SPECIFIC_ELEMENT_DATA_S*)(*data);

    free(elementData);
    elementData = NULL;

    return SPECIFIC_ERROR_NONE;
}

/*!
 *
 */
SPECIFIC_ERROR_E setElementGetters_f(SPECIFIC_S *obj, void *data, SPECIFIC_GETTERS_S *getters)
{
    assert(obj && data && getters);

    SPECIFIC_ELEMENT_DATA_S *elementData = (SPECIFIC_ELEMENT_DATA_S*)data;

    elementData->getters.getString   = getters->getString;
    elementData->getters.getColor    = getters->getColor;
    elementData->getters.getFont     = getters->getFont;
    elementData->getters.getImage    = getters->getImage;
    elementData->getters.getLanguage = getters->getLanguage;

    elementData->getters.userData    = getters->userData;

    return SPECIFIC_ERROR_NONE;
}

/*!
 *
 */
SPECIFIC_ERROR_E unsetElementGetters_f(SPECIFIC_S *obj, void *data)
{
    assert(obj && data);

    SPECIFIC_ELEMENT_DATA_S *elementData = (SPECIFIC_ELEMENT_DATA_S*)data;

    elementData->getters.userData = NULL;

    return SPECIFIC_ERROR_NONE;
}

/*!
 *
 */
SPECIFIC_ERROR_E setElementTextIds_f(SPECIFIC_S *obj, void *data, SPECIFIC_TEXT_IDS_S *textIds)
{
    assert(obj && data && textIds);

    SPECIFIC_ELEMENT_DATA_S *elementData = (SPECIFIC_ELEMENT_DATA_S*)data;

    elementData->ids.text.stringId = textIds->stringId;
    elementData->ids.text.fontId   = textIds->fontId;
    elementData->ids.text.colorId  = textIds->colorId;

    return SPECIFIC_ERROR_NONE;
}

/*!
 *
 */
SPECIFIC_ERROR_E unsetElementTextIds_f(SPECIFIC_S *obj, void *data)
{
    assert(obj && data);

    return SPECIFIC_ERROR_NONE;
}

/*!
 *
 */
SPECIFIC_ERROR_E setElementImageIds_f(SPECIFIC_S *obj, void *data, SPECIFIC_IMAGE_IDS_S *imageIds)
{
    assert(obj && data && imageIds);

    SPECIFIC_ELEMENT_DATA_S *elementData = (SPECIFIC_ELEMENT_DATA_S*)data;

    elementData->ids.image.imageId       = imageIds->imageId;
    elementData->ids.image.hiddenColorId = imageIds->hiddenColorId;

    return SPECIFIC_ERROR_NONE;
}

/*!
 *
 */
SPECIFIC_ERROR_E unsetElementImageIds_f(SPECIFIC_S *obj, void *data)
{
    assert(obj && data);

    return SPECIFIC_ERROR_NONE;
}

/*!
 *
 */
SPECIFIC_ERROR_E setClickHandlers_f(SPECIFIC_S *obj, void *data, SPECIFIC_HANDLERS_S *handlers, uint32_t nbHandlers, uint32_t index)
{
    assert(obj && data);

    if (!handlers || (nbHandlers == 0)) {
        return SPECIFIC_ERROR_PARAMS;
    }

    SPECIFIC_ELEMENT_DATA_S *elementData = (SPECIFIC_ELEMENT_DATA_S*)data;

    elementData->index           = index;
    elementData->nbClickHandlers = nbHandlers;

    assert((elementData->clickHandlers = calloc(1, nbHandlers * sizeof(SPECIFIC_CLICK_HANDLERS_S))));

    uint32_t i, j;
    for (i = 0; i < nbHandlers; i++) {
        (elementData->clickHandlers[i]).name = strdup((handlers[i]).name);
        (elementData->clickHandlers[i]).data = strdup((handlers[i]).data);

        j = 0;
        while ((j < gNbClickHandlers)
                && gClickHandlers[j].name
                && (strcmp(gClickHandlers[j].name, (handlers[i]).name) != 0)) {
            j++;
        }

        if (gClickHandlers[j].name) {
            (elementData->clickHandlers[i]).fct = gClickHandlers[j].fct;
        }
    }

    return SPECIFIC_ERROR_NONE;
}

/*!
 *
 */
SPECIFIC_ERROR_E unsetClickHandlers_f(SPECIFIC_S *obj, void *data)
{
    assert(obj && data);

    SPECIFIC_ELEMENT_DATA_S *elementData = (SPECIFIC_ELEMENT_DATA_S*)data;

    uint32_t i;
    for (i = 0; i < elementData->nbClickHandlers; i++) {
        if ((elementData->clickHandlers[i]).name) {
            free((elementData->clickHandlers[i]).name);
            (elementData->clickHandlers[i]).name = NULL;
        }
        if ((elementData->clickHandlers[i]).data) {
            free((elementData->clickHandlers[i]).data);
            (elementData->clickHandlers[i]).data = NULL;
        }
        (elementData->clickHandlers[i]).fct = NULL;
    }

    elementData->clickHandlers = NULL;

    return SPECIFIC_ERROR_NONE;
}

/*!
 *
 */
SPECIFIC_ERROR_E handleClick_f(CONTEXT_S *ctx, GFX_EVENT_S *gfxEvent)
{
    assert(ctx && gfxEvent);

    SPECIFIC_ELEMENT_DATA_S *elementData = (SPECIFIC_ELEMENT_DATA_S*)gfxEvent->gfxElementPData;

    if (!elementData) {
        return SPECIFIC_ERROR_PARAMS;
    }

    uint32_t i;
    for (i = 0; i < elementData->nbClickHandlers; i++) {
        if ((elementData->clickHandlers[i]).fct) {
            Logd("Calling click handler : %s", (elementData->clickHandlers[i]).name);
            (elementData->clickHandlers[i]).fct(ctx, gfxEvent->gfxElementName, elementData, (elementData->clickHandlers[i]).data);
        }
    }

    return SPECIFIC_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/*                                        CLICK HANDLERS                                        */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static void closeApplication(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(ctx && gfxElementName && gfxElementData);

    (void)handlerData;

    Logd("Handling click on element \"%s\"", gfxElementName);

    GRAPHICS_S *graphicsObj = ctx->modules.graphicsObj;
    INPUT_S *input          = &ctx->input;

    switch (input->keepAliveMethod) {
        case KEEP_ALIVE_EVENTS_BASED:
            (void)graphicsObj->quit(graphicsObj);
            break;

        case KEEP_ALIVE_SEMAPHORE_BASED:
            sem_post(&ctx->keepAliveSem);
            break;

        case KEEP_ALIVE_TIMER_BASED:
            sem_post(&ctx->keepAliveSem);
            break;

        default:
            ;
    }
}

/*!
 *
 */
static void changeLanguage(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(ctx && gfxElementName && gfxElementData);

    Logd("Handling click on element \"%s\"", gfxElementName);
    
    SPECIFIC_ELEMENT_DATA_S *elementData = (SPECIFIC_ELEMENT_DATA_S*)gfxElementData;
    GRAPHICS_S *graphicsObj              = ctx->modules.graphicsObj;
    GRAPHICS_INFOS_S *graphicsInfos      = &ctx->params.graphicsInfos;
    uint32_t nbGfxElements               = graphicsInfos->nbGfxElements;
    GFX_ELEMENT_S **gfxElements          = graphicsInfos->gfxElements;
    
    char nextLanguage[MIN_STR_SIZE];
    memset(nextLanguage, '\0', MIN_STR_SIZE);

    if (!handlerData || (strlen(handlerData) == 0)) {
        elementData->getters.getLanguage(elementData->getters.userData, graphicsInfos->currentLanguage, nextLanguage);
    }
    else {
        strncpy(nextLanguage, handlerData, sizeof(nextLanguage));
    }

    if (strcmp(graphicsInfos->currentLanguage, nextLanguage) == 0) {
        Logw("\"%s\" is already the current language", nextLanguage);
        return;
    }

    Logd("Changing language from \"%s\" to \"%s\"", graphicsInfos->currentLanguage, nextLanguage);
    
    uint32_t index;
    GFX_TEXT_S text;
    SPECIFIC_ELEMENT_DATA_S *data;
    
    for (index = 0; index < nbGfxElements; index++) {
        if (gfxElements[index]->type != GFX_ELEMENT_TYPE_TEXT) {
            continue;
        }
        
        data = (SPECIFIC_ELEMENT_DATA_S*)gfxElements[index]->pData;
        
        memcpy(&text, &gfxElements[index]->data.text, sizeof(GFX_TEXT_S));
        memset(text.str, '\0', sizeof(text.str));
        elementData->getters.getString(elementData->getters.userData, data->ids.text.stringId, nextLanguage, text.str);
        
        (void)graphicsObj->setData(graphicsObj, gfxElements[index]->name, (void*)&text);
    }
    
    memset(graphicsInfos->currentLanguage, '\0', MIN_STR_SIZE);
    strcpy(graphicsInfos->currentLanguage, nextLanguage);
}

/*!
 *
 */
static void hideElement(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(ctx && gfxElementName && gfxElementData);

    Logd("Handling click on element \"%s\"", gfxElementName);

    if (!handlerData) {
        Loge("Handler data is expected");
        return;
    }

    GRAPHICS_S *graphicsObj = ctx->modules.graphicsObj;

    (void)graphicsObj->setVisible(graphicsObj, handlerData, 0);
}

/*!
 *
 */
static void showElement(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(ctx && gfxElementName && gfxElementData);

    Logd("Handling click on element \"%s\"", gfxElementName);

    if (!handlerData) {
        Loge("Handler data is expected");
        return;
    }

    GRAPHICS_S *graphicsObj = ctx->modules.graphicsObj;

    (void)graphicsObj->setVisible(graphicsObj, handlerData, 1);
}

/*!
 *
 */
static void hideGroup(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(ctx && gfxElementName && gfxElementData);

    Logd("Handling click on element \"%s\"", gfxElementName);

    if (!handlerData) {
        Loge("Handler data is expected");
        return;
    }

    GRAPHICS_S *graphicsObj         = ctx->modules.graphicsObj;
    GRAPHICS_INFOS_S *graphicsInfos = &ctx->params.graphicsInfos;
    uint32_t nbGfxElements          = graphicsInfos->nbGfxElements;
    GFX_ELEMENT_S **gfxElements     = graphicsInfos->gfxElements;
    
    uint32_t index;
    for (index = 0; index < nbGfxElements; index++) {
        if (strncmp(gfxElements[index]->groupName, handlerData, sizeof(gfxElements[index]->groupName)) != 0) {
            continue;
        }

        (void)graphicsObj->setVisible(graphicsObj, gfxElements[index]->name, 0);
    }
}

/*!
 *
 */
static void showGroup(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(ctx && gfxElementName && gfxElementData);

    Logd("Handling click on element \"%s\"", gfxElementName);

    if (!handlerData) {
        Loge("Handler data is expected");
        return;
    }

    GRAPHICS_S *graphicsObj         = ctx->modules.graphicsObj;
    GRAPHICS_INFOS_S *graphicsInfos = &ctx->params.graphicsInfos;
    uint32_t nbGfxElements          = graphicsInfos->nbGfxElements;
    GFX_ELEMENT_S **gfxElements     = graphicsInfos->gfxElements;

    uint32_t index;
    for (index = 0; index < nbGfxElements; index++) {
        if (strncmp(gfxElements[index]->groupName, handlerData, sizeof(gfxElements[index]->groupName)) != 0) {
            continue;
        }

        (void)graphicsObj->setVisible(graphicsObj, gfxElements[index]->name, 1);
    }
}

/*!
 *
 */
static void setFocus(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(ctx && gfxElementName && gfxElementData);

    Logd("Handling click on element \"%s\"", gfxElementName);

    if (!handlerData) {
        Loge("Handler data is expected");
        return;
    }

    GRAPHICS_S *graphicsObj = ctx->modules.graphicsObj;

    (void)graphicsObj->setFocus(graphicsObj, handlerData);
}

/*!
 *
 */
static void saveVideoFrame(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(ctx && gfxElementName && gfxElementData);

    Logd("Handling click on element \"%s\"", gfxElementName);

    if (!handlerData) {
        Loge("Handler data is expected");
        return;
    }
    
    SPECIFIC_ELEMENT_DATA_S *elementData = (SPECIFIC_ELEMENT_DATA_S*)gfxElementData;
    GRAPHICS_S *graphicsObj              = ctx->modules.graphicsObj;
    INPUT_S *input                       = &ctx->input;

    int32_t imageFormat = atoi(handlerData);
    GFX_IMAGE_S image;
    struct stat st;

    if (stat(input->appDataDir, &st) < 0) {
        Logd("Creating direcory : \"%s\"", input->appDataDir);
        if (mkdir(input->appDataDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {
            Loge("%s", strerror(errno));
            return;
        }
    }

    switch (imageFormat) {
        case GFX_IMAGE_FORMAT_JPG:
            sprintf(image.path, "%s/picture_%ld.jpeg", input->appDataDir, time(NULL));
            image.format = GFX_IMAGE_FORMAT_JPG;
            break;

        case GFX_IMAGE_FORMAT_PNG:
            sprintf(image.path, "%s/picture_%ld.png", input->appDataDir, time(NULL));
            image.format = GFX_IMAGE_FORMAT_PNG;
            break;

        default:
            sprintf(image.path, "%s/picture_%ld.bmp", input->appDataDir, time(NULL));
            image.format = GFX_IMAGE_FORMAT_BMP;
    }

    (void)graphicsObj->saveVideoFrame(graphicsObj, NULL, &image);
}

/*!
 *
 */
static void takeScreenshot(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(ctx && gfxElementName && gfxElementData);

    Logd("Handling click on element \"%s\"", gfxElementName);

    if (!handlerData) {
        Loge("Handler data is expected");
        return;
    }

    SPECIFIC_ELEMENT_DATA_S *elementData = (SPECIFIC_ELEMENT_DATA_S*)gfxElementData;
    GRAPHICS_S *graphicsObj              = ctx->modules.graphicsObj;
    INPUT_S *input                       = &ctx->input;

    int32_t imageFormat = atoi(handlerData);
    GFX_IMAGE_S image;
    struct stat st;

    if (stat(input->appDataDir, &st) < 0) {
        Logd("Creating direcory : \"%s\"", input->appDataDir);
        if (mkdir(input->appDataDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {
            Loge("%s", strerror(errno));
            return;
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

    (void)graphicsObj->takeScreenshot(graphicsObj, &image);
}

/*!
 *
 */
static void setClickable(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(ctx && gfxElementName && gfxElementData);

    Logd("Handling click on element \"%s\"", gfxElementName);

    if (!handlerData) {
        Loge("Handler data is expected");
        return;
    }

    GRAPHICS_S *graphicsObj = ctx->modules.graphicsObj;

    (void)graphicsObj->setClickable(graphicsObj, handlerData, 1);
}

/*!
 *
 */
static void setNotClickable(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(ctx && gfxElementName && gfxElementData);

    Logd("Handling click on element \"%s\"", gfxElementName);

    if (!handlerData) {
        Loge("Handler data is expected");
        return;
    }

    GRAPHICS_S *graphicsObj = ctx->modules.graphicsObj;

    (void)graphicsObj->setClickable(graphicsObj, handlerData, 0);
}

/*!
 *
 */
static void stopGraphics(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(ctx && gfxElementName && gfxElementData);

    (void)handlerData;

    Logd("Handling click on element \"%s\"", gfxElementName);

    GRAPHICS_S *graphicsObj = ctx->modules.graphicsObj;

    (void)graphicsObj->destroyDrawer(graphicsObj);
}

/*!
 *
 */
static void startGraphics(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(ctx && gfxElementName && gfxElementData);

    (void)handlerData;

    Logd("Handling click on element \"%s\"", gfxElementName);

    GRAPHICS_S *graphicsObj           = ctx->modules.graphicsObj;
    GRAPHICS_PARAMS_S *graphicsParams = &ctx->params.graphicsInfos.graphicsParams;

    if (graphicsObj->createDrawer(graphicsObj, graphicsParams) != GRAPHICS_ERROR_NONE) {
        Loge("createDrawer() failed");
        return;
    }

    (void)graphicsObj->drawAllElements(graphicsObj);
}

/*!
 *
 */
static void stopVideo(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(ctx && gfxElementName && gfxElementData);

    (void)handlerData;

    Logd("Handling click on element \"%s\"", gfxElementName);

    VIDEO_S *videoObj         = ctx->modules.videoObj;
    VIDEO_INFOS_S *videoInfos = &ctx->params.videoInfos;

    uint32_t index;
    for (index = 0; index < videoInfos->nbVideoListeners; index++) {
        (void)videoObj->unregisterListener(videoObj, videoInfos->videoListeners[index]);
    }

    (void)videoObj->stopDeviceCapture(videoObj);
}

/*!
 *
 */
static void startVideo(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(ctx && gfxElementName && gfxElementData);

    (void)handlerData;

    Logd("Handling click on element \"%s\"", gfxElementName);

    VIDEO_S *videoObj           = ctx->modules.videoObj;
    VIDEO_INFOS_S *videoInfos   = &ctx->params.videoInfos;
    VIDEO_PARAMS_S *videoParams = &videoInfos->videoParams;

    if (videoObj->startDeviceCapture(videoObj, videoParams) != VIDEO_ERROR_NONE) {
        return;
    }

    size_t maxBufferSize = -1;
    VIDEO_RESOLUTION_S resolution;

    (void)videoObj->getMaxBufferSize(videoObj, &maxBufferSize);
    (void)videoObj->getFinalResolution(videoObj, &resolution);

    Logd("maxBufferSize = %lu bytes / width = %u - height = %u", maxBufferSize, resolution.width, resolution.height);

    uint32_t index;
    for (index = 0; index < videoInfos->nbVideoListeners; index++) {
        if (videoObj->registerListener(videoObj, videoInfos->videoListeners[index]) != VIDEO_ERROR_NONE) {
            Loge("Failed to register listener \"%s\"", (videoInfos->videoListeners[index])->name);
        }
    }
}

/*!
 *
 */
static void stopServer(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(ctx && gfxElementName && gfxElementData);

    Logd("Handling click on element \"%s\"", gfxElementName);

    if (!handlerData) {
        Loge("Handler data is expected");
        return;
    }

    SERVER_S *serverObj           = ctx->modules.serverObj;
    SERVERS_INFOS_S *serversInfos = &ctx->params.serversInfos;

    uint32_t index;
    for (index = 0; index < serversInfos->nbServers; index++) {
        if (strcmp((serversInfos->serverParams[index])->name, handlerData) == 0) {
            (void)serverObj->stop(serverObj, serversInfos->serverParams[index]);
            break;
        }
    }
}

/*!
 *
 */
static void startServer(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(ctx && gfxElementName && gfxElementData);

    Logd("Handling click on element \"%s\"", gfxElementName);

    if (!handlerData) {
        Loge("Handler data is expected");
        return;
    }

    SERVER_S *serverObj           = ctx->modules.serverObj;
    SERVERS_INFOS_S *serversInfos = &ctx->params.serversInfos;

    uint32_t index;
    for (index = 0; index < serversInfos->nbServers; index++) {
        if (strcmp((serversInfos->serverParams[index])->name, handlerData) == 0) {
            (void)serverObj->start(serverObj, serversInfos->serverParams[index]);
            break;
        }
    }
}

/*!
 *
 */
static void suspendServer(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(ctx && gfxElementName && gfxElementData);

    Logd("Handling click on element \"%s\"", gfxElementName);

    if (!handlerData) {
        Loge("Handler data is expected");
        return;
    }

    SERVER_S *serverObj           = ctx->modules.serverObj;
    SERVERS_INFOS_S *serversInfos = &ctx->params.serversInfos;

    uint32_t index;
    for (index = 0; index < serversInfos->nbServers; index++) {
        if (strcmp((serversInfos->serverParams[index])->name, handlerData) == 0) {
            (void)serverObj->suspendSender(serverObj, serversInfos->serverParams[index]);
            break;
        }
    }
}

/*!
 *
 */
static void resumeServer(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(ctx && gfxElementName && gfxElementData);

    Logd("Handling click on element \"%s\"", gfxElementName);

    if (!handlerData) {
        Loge("Handler data is expected");
        return;
    }
    
    SERVER_S *serverObj           = ctx->modules.serverObj;
    SERVERS_INFOS_S *serversInfos = &ctx->params.serversInfos;

    uint32_t index;
    for (index = 0; index < serversInfos->nbServers; index++) {
        if (strcmp((serversInfos->serverParams[index])->name, handlerData) == 0) {
            (void)serverObj->resumeSender(serverObj, serversInfos->serverParams[index]);
            break;
        }
    }
}

/*!
 *
 */
static void stopCient(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(ctx && gfxElementName && gfxElementData);

    Logd("Handling click on element \"%s\"", gfxElementName);

    if (!handlerData) {
        Loge("Handler data is expected");
        return;
    }

    CLIENT_S *clientObj           = ctx->modules.clientObj;
    CLIENTS_INFOS_S *clientsInfos = &ctx->params.clientsInfos;

    uint32_t index;
    for (index = 0; index < clientsInfos->nbClients; index++) {
        if (strcmp((clientsInfos->clientParams[index])->name, handlerData) == 0) {
            (void)clientObj->stop(clientObj, clientsInfos->clientParams[index]);
            break;
        }
    }
}

/*!
 *
 */
static void startClient(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(ctx && gfxElementName && gfxElementData);

    Logd("Handling click on element \"%s\"", gfxElementName);

    if (!handlerData) {
        Loge("Handler data is expected");
        return;
    }

    CLIENT_S *clientObj           = ctx->modules.clientObj;
    CLIENTS_INFOS_S *clientsInfos = &ctx->params.clientsInfos;

    uint32_t index;
    for (index = 0; index < clientsInfos->nbClients; index++) {
        if (strcmp((clientsInfos->clientParams[index])->name, handlerData) == 0) {
            (void)clientObj->start(clientObj, clientsInfos->clientParams[index]);
            break;
        }
    }
}
