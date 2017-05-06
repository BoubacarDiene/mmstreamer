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
* \file   Main.c
* \brief  Main entry point of the application
* \author Boubacar DIENE
*
* This file is in charge of intializing all modules. To do so, it first parses the main config
* file (see Main.xml) in order to determine which module(s) of mmstreamer are enabled then it
* retrieves their settings from their own config files.
*/

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include "core/Core.h"
#include "core/XmlDefines.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                             */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "MAIN"

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                          VARIABLES                                           */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                         PROTOTYPES                                           */
/* -------------------------------------------------------------------------------------------- */

static void onGeneralCb (void *userData, const char **attrs);
static void onGraphicsCb(void *userData, const char **attrs);
static void onVideosCb  (void *userData, const char **attrs);
static void onServersCb (void *userData, const char **attrs);
static void onClientsCb (void *userData, const char **attrs);

static void onErrorCb(void *userData, int32_t errorCode, const char *errorStr);

/* -------------------------------------------------------------------------------------------- */
/*                                           MAIN                                               */
/* -------------------------------------------------------------------------------------------- */

static void usage(const char * const program)
{
    Loge("Usage: %s [-c <path to Main.xml>]", program);
}

/*!
 * main() function
 */
int main(int argc, char **argv)
{
    int32_t opt   = -1;
    char *mainXml = MAIN_XML_FILE;

    while ((opt = getopt(argc, argv, "c:")) != -1) {
        switch (opt) {
            case 'c':
                mainXml = optarg;
                break;

            default:;
                usage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    int32_t ret = EXIT_FAILURE;

    CONTEXT_S *mCtx;
    assert((mCtx = calloc(1, sizeof(CONTEXT_S))));
    
    PARAMS_S  *params  = &mCtx->params;
    MODULES_S *modules = &mCtx->modules;
    INPUT_S   *input   = &mCtx->input;
    
    if (Parser_Init(&modules->parserObj) != PARSER_ERROR_NONE) {
        Loge("Parser_Init() failed");
        goto parserInit_exit;
    }
    
    Logd("Parsing main config file : \"%s\"", mainXml);
    
    PARSER_TAGS_HANDLER_S tagsHandlers[] = {
    	{ XML_TAG_GENERAL,   onGeneralCb,   NULL,  NULL },
    	{ XML_TAG_GRAPHICS,  onGraphicsCb,  NULL,  NULL },
    	{ XML_TAG_VIDEOS,    onVideosCb,    NULL,  NULL },
    	{ XML_TAG_SERVERS,   onServersCb,   NULL,  NULL },
    	{ XML_TAG_CLIENTS,   onClientsCb,   NULL,  NULL },
    	{ NULL,              NULL,          NULL,  NULL }
    };
    
    PARSER_PARAMS_S parserParams;
    strncpy(parserParams.path, mainXml, sizeof(parserParams.path));
    parserParams.encoding     = PARSER_ENCODING_UTF_8;
    parserParams.tagsHandlers = tagsHandlers;
    parserParams.onErrorCb    = onErrorCb;
    parserParams.userData     = mCtx;
    
    if (modules->parserObj->parse(modules->parserObj, &parserParams) != PARSER_ERROR_NONE) {
        Loge("Failed to parse main config file");
        goto parserParse_exit;
    }
    
    Logd("Initializing modules");
    
    if (input->graphicsEnabled && (Graphics_Init(&modules->graphicsObj) != GRAPHICS_ERROR_NONE)) {
        Loge("Graphics_Init() failed");
        goto graphicsInit_exit;
    }
    
    if (input->videosEnabled && (Video_Init(&modules->videoObj) != VIDEO_ERROR_NONE)) {
        Loge("Video_Init() failed");
        goto videoInit_exit;
    }
    
    if (input->serversEnabled && (Server_Init(&modules->serverObj) != SERVER_ERROR_NONE)) {
        Loge("Server_Init() failed");
        goto serverInit_exit;
    }
    
    if (input->clientsEnabled && (Client_Init(&modules->clientObj) != CLIENT_ERROR_NONE)) {
        Loge("Client_Init() failed");
        goto clientInit_exit;
    }
    
    CORE_S *coreObj = NULL;
    if (Core_Init(&coreObj, mCtx) != CORE_ERROR_NONE) {
        Loge("Core_Init() failed");
        goto coreInit_exit;
    }
    
    Logd("Starting modules");

    uint32_t index;
    
    if (modules->graphicsObj) {
        Logd("Loading graphics params");
        if (coreObj->loadGraphicsParams(coreObj) != CORE_ERROR_NONE) {
            Loge("Failed to load graphics params");
            goto loadGraphicsParams_exit;
        }

        GRAPHICS_INFOS_S *graphicsInfos = &params->graphicsInfos;

        if (input->autoStartGraphics) {
            Logd("Creating graphics drawer");
            if (modules->graphicsObj->createDrawer(modules->graphicsObj, &graphicsInfos->graphicsParams) != GRAPHICS_ERROR_NONE) {
                Loge("createDrawer() failed");
                goto createDrawer_exit;
            }
        
            Logd("Displaying %u graphics elements", graphicsInfos->nbGfxElements);
            if (modules->graphicsObj->drawAllElements(modules->graphicsObj) != GRAPHICS_ERROR_NONE) {
                Loge("Failed to draw all elements");
                goto destroyDrawer_exit;
            }
        }
    }

    uint8_t videoIndex;

    VIDEO_RESOLUTION_S resolution = { 0 };
    VIDEOS_INFOS_S *videosInfos   = NULL;
    VIDEO_DEVICE_S **videoDevices = NULL;
    VIDEO_DEVICE_S *videoDevice   = NULL;
    uint8_t nbDevices             = 0;
    
    if (modules->videoObj) {
        Logd("Loading video params");
        if (coreObj->loadVideosParams(coreObj) != CORE_ERROR_NONE) {
            Loge("Failed to load videos params");
            goto loadVideosParams_exit;
        }
        
        if (input->autoStartVideos) {
            videosInfos  = &params->videosInfos;
            videoDevices = videosInfos->devices;
            nbDevices    = videosInfos->nbDevices;

            for (videoIndex = 0; videoIndex < nbDevices; videoIndex++) {
                videoDevice = videoDevices[videoIndex];

                Logd("Starting video capture on device \"%s\"", videoDevice->videoParams.name);
                if (modules->videoObj->startDeviceCapture(modules->videoObj, &videoDevice->videoParams) != VIDEO_ERROR_NONE) {
                    Loge("startDeviceCapture() failed");
                    goto startDeviceCapture_exit;
                }

                (void)modules->videoObj->getFinalResolution(modules->videoObj, &videoDevice->videoParams, &resolution);
                Logd("Device \"%s\" : width = %u - height = %u", videoDevice->videoParams.name, resolution.width, resolution.height);
            }
        }
    }

    size_t maxBufferSize          = -1;
    uint8_t nbServers             = 0;
    SERVER_PARAMS_S *serverParams = NULL;

    if (modules->serverObj) {
        Logd("Loading servers params");
        if (coreObj->loadServersParams(coreObj) != CORE_ERROR_NONE) {
            Loge("Failed to load servers params");
            goto loadServersParams_exit;
        }

        nbServers = params->serversInfos.nbServers;
        Logd("Starting %u servers", nbServers);

        for (index = 0; index < nbServers; index++) {
            serverParams = params->serversInfos.serverParams[index];

            if (serverParams->maxBufferSize == (size_t)-1) {
                maxBufferSize = -1;

                for (videoIndex = 0; videoIndex < nbDevices; videoIndex++) {
                    videoDevice = videoDevices[videoIndex];
                    if (!videoDevice->serverDest || (strcmp(videoDevice->serverDest, serverParams->name) != 0)) {
                        continue;
                    }
                    (void)modules->videoObj->getMaxBufferSize(modules->videoObj, &videoDevice->videoParams, &maxBufferSize);
                    break;
                }

                serverParams->maxBufferSize = (maxBufferSize > 0 ? maxBufferSize : MAX_BUFFER_SIZE);
                Logd("Server \"%s\"'s buffer size set to %lu bytes", serverParams->name, serverParams->maxBufferSize);
            }

            if (input->autoStartServers
                && modules->serverObj->start(modules->serverObj, serverParams) != SERVER_ERROR_NONE) {
                Loge("Failed to start server %s", serverParams->name);
                goto stopServers_exit;
            }
        }
    }

    uint8_t nbClients             = 0;
    CLIENT_PARAMS_S *clientParams = NULL;

    if (modules->clientObj) {
        Logd("Loading clients params");
        if (coreObj->loadClientsParams(coreObj) != CORE_ERROR_NONE) {
            Loge("Failed to load clients params");
            goto loadClientsParams_exit;
        }
        
        if (input->autoStartClients) {
            nbClients = params->clientsInfos.nbClients;
            Logd("Starting %u clients", nbClients);

            for (index = 0; index < nbClients; index++) {
                clientParams = params->clientsInfos.clientParams[index];

                if (modules->clientObj->start(modules->clientObj, clientParams) != CLIENT_ERROR_NONE) {
                    Loge("Failed to start client %s", clientParams->name);
                    //goto stopClients_exit;
                }
            }
        }
    }

    uint8_t nbVideoListeners           = 0;
    VIDEO_LISTENER_S  **videoListeners = NULL;

    if (modules->videoObj && input->autoStartVideos) {
        for (videoIndex = 0; videoIndex < nbDevices; videoIndex++) {
            videoDevice      = videoDevices[videoIndex];
            nbVideoListeners = videoDevice->nbVideoListeners;
            videoListeners   = videoDevice->videoListeners;

            Logd("Registering %u video listeners on device \"%s\"", nbVideoListeners, videoDevice->videoParams.name);
            for (index = 0; index < nbVideoListeners; index++) {
                if (modules->videoObj->registerListener(modules->videoObj,
                                                        &videoDevice->videoParams,
                                                        videoListeners[index]) != VIDEO_ERROR_NONE) {
                    Loge("Failed to register listener \"%s\"", (videoListeners[index])->name);
                    goto registerVideoListeners_exit;
                }
            }
        }
    }
    
    if (coreObj->keepAppRunning(coreObj, input->keepAliveMethod, input->timeout_s) != CORE_ERROR_NONE) {
        Loge("Error occurred trying to keep app running");
    }
    else {
        Logd("Stopping app");
        ret = EXIT_SUCCESS;
    }
    
registerVideoListeners_exit:
    if (modules->videoObj && input->autoStartVideos) {
        for (videoIndex = 0; videoIndex < nbDevices; videoIndex++) {
            videoDevice      = videoDevices[videoIndex];
            nbVideoListeners = videoDevice->nbVideoListeners;
            videoListeners   = videoDevice->videoListeners;

            Logd("Unregistering %u video listeners on device \"%s\"", nbVideoListeners, videoDevice->videoParams.name);
            for (index = 0; index < nbVideoListeners; index++) {
                (void)modules->videoObj->unregisterListener(modules->videoObj, &videoDevice->videoParams, videoListeners[index]);
            }
        }
    }
    
//stopClients_exit:
    if (modules->clientObj) {
        Logd("Unloading clients params");
        if (input->autoStartClients) {
            for (index = 0; index < nbClients; index++) {
                clientParams = params->clientsInfos.clientParams[index];
                (void)modules->clientObj->stop(modules->clientObj, clientParams);
            }
        }
        (void)coreObj->unloadClientsParams(coreObj);
    }

loadClientsParams_exit:
stopServers_exit:
    if (modules->serverObj) {
        Logd("Unloading servers params");
        if (input->autoStartServers) {
            for (index = 0; index < nbServers; index++) {
                serverParams = params->serversInfos.serverParams[index];
                (void)modules->serverObj->stop(modules->serverObj, serverParams);
            }
        }
        (void)coreObj->unloadServersParams(coreObj);
    }

loadServersParams_exit:
startDeviceCapture_exit:
    if (modules->videoObj) {
        if (input->autoStartVideos) {
            for (videoIndex = 0; videoIndex < nbDevices; videoIndex++) {
                videoDevice = videoDevices[videoIndex];
                Logd("Stopping video capture on device \"%s\"", videoDevice->videoParams.name);
                (void)modules->videoObj->stopDeviceCapture(modules->videoObj, &videoDevice->videoParams);
            }
        }
        Logd("Unloading videos params");
        (void)coreObj->unloadVideosParams(coreObj);
    }

loadVideosParams_exit:
destroyDrawer_exit:
    if (modules->graphicsObj && input->autoStartGraphics) {
        Logd("Destroying elements and drawer");
        (void)modules->graphicsObj->destroyDrawer(modules->graphicsObj);
    }

createDrawer_exit:
    if (modules->graphicsObj) {
        Logd("Unloading graphics params");
        (void)coreObj->unloadGraphicsParams(coreObj);
    }

loadGraphicsParams_exit:
    Logd("Uninitializing coreObj");
    (void)Core_UnInit(&coreObj);

coreInit_exit:
    if (modules->clientObj) {
        Logd("Uninitializing clientObj");
        (void)Client_UnInit(&modules->clientObj);
    }

clientInit_exit:
    if (modules->serverObj) {
        Logd("Uninitializing serverObj");
        (void)Server_UnInit(&modules->serverObj);
    }

serverInit_exit:
    if (modules->videoObj) {
        Logd("Uninitializing videoObj");
        (void)Video_UnInit(&modules->videoObj);
    }

videoInit_exit:
    if (modules->graphicsObj) {
        Logd("Uninitializing graphicsObj");
        (void)Graphics_UnInit(&modules->graphicsObj);
    }

graphicsInit_exit:
parserParse_exit:
    if (input->appDataDir) {
        free(input->appDataDir);
        input->appDataDir = NULL;
    }

    if (input->resRootDir) {
        free(input->resRootDir);
        input->resRootDir = NULL;
    }

    if (input->clientsXml) {
        free(input->clientsXml);
        input->clientsXml = NULL;
    }
    
    if (input->serversXml) {
        free(input->serversXml);
        input->serversXml = NULL;
    }
    
    if (input->videosXml) {
        free(input->videosXml);
        input->videosXml = NULL;
    }
    
    if (input->graphicsXml) {
        free(input->graphicsXml);
        input->graphicsXml = NULL;
    }
    
    if (modules->parserObj) {
        Logd("Uninitializing parser");
        (void)Parser_UnInit(&modules->parserObj);
    }

parserInit_exit:
    free(mCtx);
    mCtx = NULL;
    
    return ret;
}

/* -------------------------------------------------------------------------------------------- */
/*                                           CALLBACKS                                          */
/* -------------------------------------------------------------------------------------------- */

/*!
 * Called when <General> tag is found
 */
static void onGeneralCb(void *userData, const char **attrs)
{
    assert(userData);
	
    CONTEXT_S *ctx      = (CONTEXT_S*)userData;
    PARSER_S *parserObj = ctx->modules.parserObj;
    INPUT_S   *input    = &ctx->input;
    
    PARSER_ATTR_HANDLER_S attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_APP_DATA_DIR,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&input->appDataDir,
    	    .attrGetter.vector = parserObj->getString
        },
    	{
    	    .attrName          = XML_ATTR_RES_ROOT_DIR,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&input->resRootDir,
    	    .attrGetter.vector = parserObj->getString
        },
    	{
    	    .attrName          = XML_ATTR_KEEP_ALIVE_METHOD,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void**)&input->keepAliveMethod,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_TIMEOUT_S,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void**)&input->timeout_s,
    	    .attrGetter.scalar = parserObj->getUint32
        },
    	{
    	    NULL,
    	    PARSER_ATTR_TYPE_NONE,
    	    NULL,
    	    NULL
        }
    };
    
    if (parserObj->getAttributes(parserObj, attrHandlers, attrs) != PARSER_ERROR_NONE) {
    	Loge("Failed to retrieve attributes in \"General\" tag");
    }
}

/*!
 * Called when <Graphics> tag is found
 */
static void onGraphicsCb(void *userData, const char **attrs)
{
    assert(userData);

    CONTEXT_S *ctx      = (CONTEXT_S*)userData;
    PARSER_S *parserObj = ctx->modules.parserObj;
    INPUT_S   *input    = &ctx->input;
    
    PARSER_ATTR_HANDLER_S attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_ENABLE,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&input->graphicsEnabled,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_AUTO_START,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&input->autoStartGraphics,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_XML_FILE,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&input->graphicsXml,
    	    .attrGetter.vector = parserObj->getString
        },
    	{
    	    NULL,
    	    PARSER_ATTR_TYPE_NONE,
    	    NULL,
    	    NULL
        }
    };
    
    if (parserObj->getAttributes(parserObj, attrHandlers, attrs) != PARSER_ERROR_NONE) {
    	Loge("Failed to retrieve attributes in \"Graphics\" tag");
    }
}

/*!
 * Called when <Videos> tag is found
 */
static void onVideosCb(void *userData, const char **attrs)
{
    assert(userData);

    CONTEXT_S *ctx      = (CONTEXT_S*)userData;
    PARSER_S *parserObj = ctx->modules.parserObj;
    INPUT_S   *input    = &ctx->input;
    
    PARSER_ATTR_HANDLER_S attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_ENABLE,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&input->videosEnabled,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_AUTO_START,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&input->autoStartVideos,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_XML_FILE,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&input->videosXml,
    	    .attrGetter.vector = parserObj->getString
        },
    	{
    	    NULL,
    	    PARSER_ATTR_TYPE_NONE,
    	    NULL,
    	    NULL
        }
    };
    
    if (parserObj->getAttributes(parserObj, attrHandlers, attrs) != PARSER_ERROR_NONE) {
    	Loge("Failed to retrieve attributes in \"Video\" tag");
    }
}

/*!
 * Called when <Servers> tag is found
 */
static void onServersCb(void *userData, const char **attrs)
{
    assert(userData);

    CONTEXT_S *ctx      = (CONTEXT_S*)userData;
    PARSER_S *parserObj = ctx->modules.parserObj;
    INPUT_S   *input    = &ctx->input;
    
    PARSER_ATTR_HANDLER_S attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_ENABLE,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&input->serversEnabled,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_AUTO_START,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&input->autoStartServers,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_XML_FILE,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&input->serversXml,
    	    .attrGetter.vector = parserObj->getString
        },
    	{
    	    NULL,
    	    PARSER_ATTR_TYPE_NONE,
    	    NULL,
    	    NULL
        }
    };
    
    if (parserObj->getAttributes(parserObj, attrHandlers, attrs) != PARSER_ERROR_NONE) {
    	Loge("Failed to retrieve attributes in \"Servers\" tag");
    }
}

/*!
 * Called when <Clients> tag is found
 */
static void onClientsCb(void *userData, const char **attrs)
{
    assert(userData);

    CONTEXT_S *ctx      = (CONTEXT_S*)userData;
    PARSER_S *parserObj = ctx->modules.parserObj;
    INPUT_S   *input    = &ctx->input;
    
    PARSER_ATTR_HANDLER_S attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_ENABLE,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&input->clientsEnabled,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_AUTO_START,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&input->autoStartClients,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_XML_FILE,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&input->clientsXml,
    	    .attrGetter.vector = parserObj->getString
        },
    	{
    	    NULL,
    	    PARSER_ATTR_TYPE_NONE,
    	    NULL,
    	    NULL
        }
    };

    if (parserObj->getAttributes(parserObj, attrHandlers, attrs) != PARSER_ERROR_NONE) {
        Loge("Failed to retrieve attributes in \"Clients\" tag");
    }
}

/**
 * Called when parsing failed
 */
static void onErrorCb(void *userData, int32_t errorCode, const char *errorStr)
{
    (void)userData;

    Loge("Parsing error - errorCode = %d / errorStr = \"%s\"", errorCode, errorStr);
}
