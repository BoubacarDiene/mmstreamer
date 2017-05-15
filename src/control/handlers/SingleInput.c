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

static void closeApplication(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData);
static void changeLanguage  (HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData);

static void hideElement(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData);
static void showElement(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData);

static void hideGroup(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData);
static void showGroup(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData);

static void setFocus(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData);

static void saveVideoElement(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData);
static void takeScreenshot  (HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData);

static void setClickable   (HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData);
static void setNotClickable(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData);

static void stopGraphics (HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData);
static void startGraphics(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData);

static void stopVideo (HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData);
static void startVideo(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData);

static void stopServer (HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData);
static void startServer(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData);

static void suspendServer(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData);
static void resumeServer (HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData);

static void stopCient  (HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData);
static void startClient(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData);

static void multiInputs(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData);

/* -------------------------------------------------------------------------------------------- */
/*                                          VARIABLES                                           */
/* -------------------------------------------------------------------------------------------- */

CLICK_HANDLERS_S gSingleInputClickHandlers[] = {
	{ "closeApplication",           NULL,             closeApplication },
	{ "changeLanguage",             NULL,             changeLanguage   },
	{ "hideElement",                NULL,             hideElement      },
	{ "showElement",                NULL,             showElement      },
	{ "hideGroup",                  NULL,             hideGroup        },
	{ "showGroup",                  NULL,             showGroup        },
	{ "setFocus",                   NULL,             setFocus         },
	{ "saveVideoElement",           NULL,             saveVideoElement },
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
	{ "multiInputs",                NULL,             multiInputs      },
	{ NULL,                         NULL,             NULL             }
};

uint32_t gNbSingleInputClickHandlers = (uint32_t)(sizeof(gSingleInputClickHandlers) / sizeof(gSingleInputClickHandlers[0]));

/* -------------------------------------------------------------------------------------------- */
/*                                        CLICK HANDLERS                                        */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static void closeApplication(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(obj && obj->pData && gfxElementName && gfxElementData);

    (void)handlerData;

    Logd("Handling click on element \"%s\"", gfxElementName);

    HANDLERS_PRIVATE_DATA_S *pData = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                 = pData->ctx;
    GRAPHICS_S *graphicsObj        = ctx->modules.graphicsObj;
    INPUT_S *input                 = &ctx->input;

    switch (input->keepAliveMethod) {
        case KEEP_ALIVE_EVENTS_BASED:
            (void)graphicsObj->quit(graphicsObj);
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
}

/*!
 *
 */
static void changeLanguage(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(obj && obj->pData && gfxElementName && gfxElementData);

    Logd("Handling click on element \"%s\"", gfxElementName);

    HANDLERS_PRIVATE_DATA_S *pData      = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                      = pData->ctx;
    CONTROL_ELEMENT_DATA_S *elementData = (CONTROL_ELEMENT_DATA_S*)gfxElementData;
    GRAPHICS_S *graphicsObj             = ctx->modules.graphicsObj;
    GRAPHICS_INFOS_S *graphicsInfos     = &ctx->params.graphicsInfos;
    uint32_t nbGfxElements              = graphicsInfos->nbGfxElements;
    GFX_ELEMENT_S **gfxElements         = graphicsInfos->gfxElements;
    
    char nextLanguage[MIN_STR_SIZE]     = { 0 };

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
    CONTROL_ELEMENT_DATA_S *data;
    
    for (index = 0; index < nbGfxElements; index++) {
        if (gfxElements[index]->type != GFX_ELEMENT_TYPE_TEXT) {
            continue;
        }
        
        data = (CONTROL_ELEMENT_DATA_S*)gfxElements[index]->pData;
        
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
static void hideElement(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(obj && obj->pData && gfxElementName && gfxElementData);

    Logd("Handling click on element \"%s\"", gfxElementName);

    if (!handlerData) {
        Loge("Handler data is expected");
        return;
    }

    HANDLERS_PRIVATE_DATA_S *pData = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                 = pData->ctx;
    GRAPHICS_S *graphicsObj        = ctx->modules.graphicsObj;

    (void)graphicsObj->setVisible(graphicsObj, handlerData, 0);
}

/*!
 *
 */
static void showElement(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(obj && obj->pData && gfxElementName && gfxElementData);

    Logd("Handling click on element \"%s\"", gfxElementName);

    if (!handlerData) {
        Loge("Handler data is expected");
        return;
    }

    HANDLERS_PRIVATE_DATA_S *pData = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                 = pData->ctx;
    GRAPHICS_S *graphicsObj        = ctx->modules.graphicsObj;

    (void)graphicsObj->setVisible(graphicsObj, handlerData, 1);
}

/*!
 *
 */
static void hideGroup(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(obj && obj->pData && gfxElementName && gfxElementData);

    Logd("Handling click on element \"%s\"", gfxElementName);

    if (!handlerData) {
        Loge("Handler data is expected");
        return;
    }

    HANDLERS_PRIVATE_DATA_S *pData  = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                  = pData->ctx;
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
static void showGroup(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(obj && obj->pData && gfxElementName && gfxElementData);

    Logd("Handling click on element \"%s\"", gfxElementName);

    if (!handlerData) {
        Loge("Handler data is expected");
        return;
    }

    HANDLERS_PRIVATE_DATA_S *pData  = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                  = pData->ctx;
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
static void setFocus(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(obj && obj->pData && gfxElementName && gfxElementData);

    Logd("Handling click on element \"%s\"", gfxElementName);

    if (!handlerData) {
        Loge("Handler data is expected");
        return;
    }

    HANDLERS_PRIVATE_DATA_S *pData = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                 = pData->ctx;
    GRAPHICS_S *graphicsObj        = ctx->modules.graphicsObj;

    (void)graphicsObj->setFocus(graphicsObj, handlerData);
}

/*!
 *
 */
static void saveVideoElement(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(obj && obj->pData && gfxElementName && gfxElementData);

    Logd("Handling click on element \"%s\"", gfxElementName);

    if (!handlerData) {
        Loge("Handler data is expected");
        return;
    }

    HANDLERS_PRIVATE_DATA_S *pData      = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                      = pData->ctx;
    CONTROL_ELEMENT_DATA_S *elementData = (CONTROL_ELEMENT_DATA_S*)gfxElementData;
    GRAPHICS_S *graphicsObj             = ctx->modules.graphicsObj;
    INPUT_S *input                      = &ctx->input;

    GFX_IMAGE_S image;
    struct stat st;

    if (stat(input->appDataDir, &st) < 0) {
        Logd("Creating direcory : \"%s\"", input->appDataDir);
        if (mkdir(input->appDataDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {
            Loge("%s", strerror(errno));
            return;
        }
    }

    sprintf(image.path, "%s/picture_%ld.bmp", input->appDataDir, time(NULL));
    image.format = GFX_IMAGE_FORMAT_BMP;

    (void)graphicsObj->saveVideoElement(graphicsObj, handlerData, &image);
}

/*!
 *
 */
static void takeScreenshot(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(obj && obj->pData && gfxElementName && gfxElementData);

    Logd("Handling click on element \"%s\"", gfxElementName);

    if (!handlerData) {
        Loge("Handler data is expected");
        return;
    }

    HANDLERS_PRIVATE_DATA_S *pData      = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                      = pData->ctx;
    CONTROL_ELEMENT_DATA_S *elementData = (CONTROL_ELEMENT_DATA_S*)gfxElementData;
    GRAPHICS_S *graphicsObj             = ctx->modules.graphicsObj;
    INPUT_S *input                      = &ctx->input;

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
static void setClickable(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(obj && obj->pData && gfxElementName && gfxElementData);

    Logd("Handling click on element \"%s\"", gfxElementName);

    if (!handlerData) {
        Loge("Handler data is expected");
        return;
    }

    HANDLERS_PRIVATE_DATA_S *pData = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                 = pData->ctx;
    GRAPHICS_S *graphicsObj        = ctx->modules.graphicsObj;

    (void)graphicsObj->setClickable(graphicsObj, handlerData, 1);
}

/*!
 *
 */
static void setNotClickable(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(obj && obj->pData && gfxElementName && gfxElementData);

    Logd("Handling click on element \"%s\"", gfxElementName);

    if (!handlerData) {
        Loge("Handler data is expected");
        return;
    }

    HANDLERS_PRIVATE_DATA_S *pData = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                 = pData->ctx;
    GRAPHICS_S *graphicsObj        = ctx->modules.graphicsObj;

    (void)graphicsObj->setClickable(graphicsObj, handlerData, 0);
}

/*!
 *
 */
static void stopGraphics(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(obj && obj->pData && gfxElementName && gfxElementData);

    (void)handlerData;

    Logd("Handling click on element \"%s\"", gfxElementName);

    HANDLERS_PRIVATE_DATA_S *pData = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                 = pData->ctx;
    GRAPHICS_S *graphicsObj        = ctx->modules.graphicsObj;

    (void)graphicsObj->destroyDrawer(graphicsObj);
}

/*!
 *
 */
static void startGraphics(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(obj && obj->pData && gfxElementName && gfxElementData);

    (void)handlerData;

    Logd("Handling click on element \"%s\"", gfxElementName);

    HANDLERS_PRIVATE_DATA_S *pData    = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                    = pData->ctx;
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
static void stopVideo(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(obj && obj->pData && gfxElementName && gfxElementData);

    Logd("Handling click on element \"%s\"", gfxElementName);

    if (!handlerData) {
        Loge("Handler data is expected");
        return;
    }

    HANDLERS_PRIVATE_DATA_S *pData = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                 = pData->ctx;
    VIDEO_S *videoObj              = ctx->modules.videoObj;
    VIDEOS_INFOS_S *videosInfos    = &ctx->params.videosInfos;
    VIDEO_DEVICE_S **videoDevices  = videosInfos->devices;
    uint8_t nbDevices              = videosInfos->nbDevices;
    VIDEO_DEVICE_S *videoDevice    = NULL;

    uint8_t nbVideoListeners           = 0;
    VIDEO_LISTENER_S  **videoListeners = NULL;

    uint32_t videoIndex, listenerIndex;
    for (videoIndex = 0; videoIndex < nbDevices; videoIndex++) {
        videoDevice = videoDevices[videoIndex];

        if (strcmp(videoDevice->videoParams.name, handlerData) == 0) {
            Logd("Video device \"%s\" found", videoDevice->videoParams.name);

            nbVideoListeners = videoDevice->nbVideoListeners;
            videoListeners   = videoDevice->videoListeners;

            for (listenerIndex = 0; listenerIndex < nbVideoListeners; listenerIndex++) {
                (void)videoObj->unregisterListener(videoObj, &videoDevice->videoParams, videoListeners[listenerIndex]);
            }

            (void)videoObj->stopDeviceCapture(videoObj, &videoDevice->videoParams);
            break;
        }
    }
}

/*!
 *
 */
static void startVideo(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(obj && obj->pData && gfxElementName && gfxElementData);

    Logd("Handling click on element \"%s\"", gfxElementName);

    if (!handlerData) {
        Loge("Handler data is expected");
        return;
    }

    HANDLERS_PRIVATE_DATA_S *pData = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                 = pData->ctx;
    VIDEO_S *videoObj              = ctx->modules.videoObj;
    VIDEOS_INFOS_S *videosInfos    = &ctx->params.videosInfos;
    VIDEO_DEVICE_S **videoDevices  = videosInfos->devices;
    uint8_t nbDevices              = videosInfos->nbDevices;
    VIDEO_DEVICE_S *videoDevice    = NULL;

    size_t maxBufferSize           = -1;
    VIDEO_AREA_S videoArea         = { 0 };

    uint8_t nbVideoListeners           = 0;
    VIDEO_LISTENER_S  **videoListeners = NULL;

    uint32_t videoIndex, listenerIndex;
    for (videoIndex = 0; videoIndex < nbDevices; videoIndex++) {
        videoDevice = videoDevices[videoIndex];

        if (strcmp(videoDevice->videoParams.name, handlerData) == 0) {
            Logd("Video device \"%s\" found", videoDevice->videoParams.name);

            if (videoObj->startDeviceCapture(videoObj, &videoDevice->videoParams) != VIDEO_ERROR_NONE) {
                return;
            }

            (void)videoObj->getMaxBufferSize(videoObj, &videoDevice->videoParams, &maxBufferSize);
            (void)videoObj->getFinalVideoArea(videoObj, &videoDevice->videoParams, &videoArea);

            Logd("maxBufferSize = %lu bytes / width = %u - height = %u", maxBufferSize, videoArea.width, videoArea.height);

            nbVideoListeners = videoDevice->nbVideoListeners;
            videoListeners   = videoDevice->videoListeners;

            for (listenerIndex = 0; listenerIndex < nbVideoListeners; listenerIndex++) {
                if (videoObj->registerListener(videoObj, &videoDevice->videoParams, videoListeners[listenerIndex]) != VIDEO_ERROR_NONE) {
                    Loge("Failed to register listener \"%s\"", (videoListeners[listenerIndex])->name);
                }
            }
            break;
        }
    }
}

/*!
 *
 */
static void stopServer(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(obj && obj->pData && gfxElementName && gfxElementData);

    Logd("Handling click on element \"%s\"", gfxElementName);

    if (!handlerData) {
        Loge("Handler data is expected");
        return;
    }

    HANDLERS_PRIVATE_DATA_S *pData = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                 = pData->ctx;
    SERVER_S *serverObj            = ctx->modules.serverObj;
    SERVERS_INFOS_S *serversInfos  = &ctx->params.serversInfos;

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
static void startServer(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(obj && obj->pData && gfxElementName && gfxElementData);

    Logd("Handling click on element \"%s\"", gfxElementName);

    if (!handlerData) {
        Loge("Handler data is expected");
        return;
    }

    HANDLERS_PRIVATE_DATA_S *pData = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                 = pData->ctx;
    SERVER_S *serverObj            = ctx->modules.serverObj;
    SERVERS_INFOS_S *serversInfos  = &ctx->params.serversInfos;

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
static void suspendServer(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(obj && obj->pData && gfxElementName && gfxElementData);

    Logd("Handling click on element \"%s\"", gfxElementName);

    if (!handlerData) {
        Loge("Handler data is expected");
        return;
    }

    HANDLERS_PRIVATE_DATA_S *pData = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                 = pData->ctx;
    SERVER_S *serverObj            = ctx->modules.serverObj;
    SERVERS_INFOS_S *serversInfos  = &ctx->params.serversInfos;

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
static void resumeServer(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(obj && obj->pData && gfxElementName && gfxElementData);

    Logd("Handling click on element \"%s\"", gfxElementName);

    if (!handlerData) {
        Loge("Handler data is expected");
        return;
    }

    HANDLERS_PRIVATE_DATA_S *pData = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                 = pData->ctx;
    SERVER_S *serverObj            = ctx->modules.serverObj;
    SERVERS_INFOS_S *serversInfos  = &ctx->params.serversInfos;

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
static void stopCient(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(obj && obj->pData && gfxElementName && gfxElementData);

    Logd("Handling click on element \"%s\"", gfxElementName);

    if (!handlerData) {
        Loge("Handler data is expected");
        return;
    }

    HANDLERS_PRIVATE_DATA_S *pData = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                 = pData->ctx;
    CLIENT_S *clientObj            = ctx->modules.clientObj;
    CLIENTS_INFOS_S *clientsInfos  = &ctx->params.clientsInfos;

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
static void startClient(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(obj && obj->pData && gfxElementName && gfxElementData);

    Logd("Handling click on element \"%s\"", gfxElementName);

    if (!handlerData) {
        Loge("Handler data is expected");
        return;
    }

    HANDLERS_PRIVATE_DATA_S *pData = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                 = pData->ctx;
    CLIENT_S *clientObj            = ctx->modules.clientObj;
    CLIENTS_INFOS_S *clientsInfos  = &ctx->params.clientsInfos;

    uint32_t index;
    for (index = 0; index < clientsInfos->nbClients; index++) {
        if (strcmp((clientsInfos->clientParams[index])->name, handlerData) == 0) {
            (void)clientObj->start(clientObj, clientsInfos->clientParams[index]);
            break;
        }
    }
}

/*!
 *
 */
static void multiInputs(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(obj && obj->pData && gfxElementName && gfxElementData);

    Logd("Handling click on element \"%s\"", gfxElementName);

    if (!handlerData) {
        Loge("Handler data is expected");
        return;
    }

    HANDLERS_PRIVATE_DATA_S *pData = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                 = pData->ctx;

    uint32_t offset                  = 0;
    char functionName[MAX_NAME_SIZE] = { 0 };
    char targetName[MAX_NAME_SIZE]   = { 0 };

    if (obj->getSubstring(obj, handlerData, ";", functionName, &offset) != HANDLERS_ERROR_NONE) {
        Loge("Bad format. Expected: customFunctionName;targetName;param1;param2;...");
        return;
    }

    if (obj->getSubstring(obj, handlerData, ";", targetName, &offset) != HANDLERS_ERROR_NONE) {
        strncpy(targetName, handlerData + offset, sizeof(targetName));
    }

    Logd("Function : \"%s\" / Target : \"%s\"", functionName, targetName);

    uint32_t index;
    for (index = 0; index < pData->nbMultiInputsClickHandlers; index++) {
        if (strcmp(pData->multiInputsClickHandlers[index].name, functionName) == 0) {
            break;
        }
    }

    if (index >= pData->nbMultiInputsClickHandlers) {
        Loge("Method \"%s\" not found", functionName);
        return;
    }

    if (!pData->multiInputsClickHandlers[index].fct) {
        Loge("Method \"%s\" not defined", functionName);
        return;
    }

    pData->multiInputsClickHandlers[index].fct(obj, targetName, NULL, handlerData + offset);
}
