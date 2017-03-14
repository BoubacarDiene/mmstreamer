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
* \file   Generic.c
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
#define TAG "SPECIFIC-GENERIC"

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                         PROTOTYPES                                           */
/* -------------------------------------------------------------------------------------------- */

static void closeApplication(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData);
static void changeLanguage  (CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData);

static void hideElement(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData);
static void showElement(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData);

static void hideGroup(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData);
static void showGroup(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData);

static void setFocus(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData);

static void saveVideoElement(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData);
static void takeScreenshot  (CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData);

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

static void customHandler(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData);


extern SPECIFIC_ERROR_E callCustomHandler(CONTEXT_S *ctx, char *functionName, char *targetName, char *handlerData);
extern SPECIFIC_ERROR_E getSubstring     (CONTEXT_S *ctx, const char *haystack, const char *needle, char *out, uint32_t *offset);

/* -------------------------------------------------------------------------------------------- */
/*                                          VARIABLES                                           */
/* -------------------------------------------------------------------------------------------- */

SPECIFIC_CLICK_HANDLERS_S gGenericClickHandlers[] = {
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
	{ "customHandler",              NULL,             customHandler    },
	{ NULL,                         NULL,             NULL             }
};

uint32_t gNbGenericClickHandlers = (uint32_t)(sizeof(gGenericClickHandlers) / sizeof(gGenericClickHandlers[0]));

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
            // Wait until timeout
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
    
    char nextLanguage[MIN_STR_SIZE]      = { 0 };

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
static void saveVideoElement(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData)
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

    Logd("Handling click on element \"%s\"", gfxElementName);

    if (!handlerData) {
        Loge("Handler data is expected");
        return;
    }

    VIDEO_S *videoObj             = ctx->modules.videoObj;
    VIDEOS_INFOS_S *videosInfos   = &ctx->params.videosInfos;
    VIDEO_DEVICE_S **videoDevices = videosInfos->devices;
    uint8_t nbDevices             = videosInfos->nbDevices;
    VIDEO_DEVICE_S *videoDevice   = NULL;

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
static void startVideo(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(ctx && gfxElementName && gfxElementData);

    Logd("Handling click on element \"%s\"", gfxElementName);

    if (!handlerData) {
        Loge("Handler data is expected");
        return;
    }

    VIDEO_S *videoObj             = ctx->modules.videoObj;
    VIDEOS_INFOS_S *videosInfos   = &ctx->params.videosInfos;
    VIDEO_DEVICE_S **videoDevices = videosInfos->devices;
    uint8_t nbDevices             = videosInfos->nbDevices;
    VIDEO_DEVICE_S *videoDevice   = NULL;

    size_t maxBufferSize          = -1;
    VIDEO_RESOLUTION_S resolution = { 0 };

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
            (void)videoObj->getFinalResolution(videoObj, &videoDevice->videoParams, &resolution);

            Logd("maxBufferSize = %lu bytes / width = %u - height = %u", maxBufferSize, resolution.width, resolution.height);

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

/*!
 *
 */
static void customHandler(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(ctx && gfxElementName && gfxElementData);

    Logd("Handling click on element \"%s\"", gfxElementName);

    if (!handlerData) {
        Loge("Handler data is expected");
        return;
    }

    uint32_t offset                  = 0;
    char functionName[MAX_NAME_SIZE] = { 0 };
    char targetName[MAX_NAME_SIZE]   = { 0 };

    if (getSubstring(ctx, handlerData, ";", functionName, &offset) != SPECIFIC_ERROR_NONE) {
        Loge("Bad format. Expected: customFunctionName;targetName;param1;param2;...");
        return;
    }

    if (getSubstring(ctx, handlerData, ";", targetName, &offset) != SPECIFIC_ERROR_NONE) {
        strncpy(targetName, handlerData + offset, sizeof(targetName));
    }

    Logd("Function : \"%s\" / Target : \"%s\"", functionName, targetName);

    (void)callCustomHandler(ctx, functionName, targetName, handlerData + offset);
}
