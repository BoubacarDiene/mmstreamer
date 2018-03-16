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
* \file Main.c
* \brief Main entry point of the application
* \author Boubacar DIENE
*
* This file is in charge of intializing all modules. To do so, it first parses the main config
* file (see Main.xml) in order to determine which module(s) of mmstreamer are enabled then it
* retrieves their settings from their own config files.
*/

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// HEADERS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#include <getopt.h>
#ifdef _POSIX_PRIORITY_SCHEDULING
#include <sched.h>
#else
#include <sys/resource.h>
#endif

#include "core/Core.h"
#include "core/XmlDefines.h"

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// MACROS ////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "Main"

#ifdef _POSIX_PRIORITY_SCHEDULING
#define SCHED_POLICY SCHED_FIFO
#else
#define MIN_NICENESS PRIO_MIN
#define MAX_NICENESS PRIO_MAX
#endif

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// TYPES /////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

struct options_s {
    char    *mainXml;
    int32_t priority;
};

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////////// CALLBACKS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

static void onGeneralCb(void *userData, const char **attrs);
static void onItemCb(void *userData, const char **attrs);
static void onGraphicsCb(void *userData, const char **attrs);
static void onVideosCb(void *userData, const char **attrs);
static void onServersCb(void *userData, const char **attrs);
static void onClientsCb(void *userData, const char **attrs);

static void onControllersStartCb(void *userData, const char **attrs);
static void onControllersEndCb(void *userData);

static void onErrorCb(void *userData, int32_t errorCode, const char *errorStr);

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////// PRIVATE FUNCTIONS PROTOTYPES /////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

static void usage(const char * const program);
static void parseOptions(int argc, char **argv, struct options_s *out);
static void setPriority(struct options_s *in);

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////////// MAIN /////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

int main(int argc, char **argv)
{
    int32_t ret = EXIT_FAILURE;

    struct options_s options = {
        .mainXml  = MAIN_XML_FILE,
        .priority = 0
    };

    parseOptions(argc, argv, &options);
    setPriority(&options);

    struct context_s *mCtx;
    assert((mCtx = calloc(1, sizeof(struct context_s))));
    
    struct params_s *params   = &mCtx->params;
    struct modules_s *modules = &mCtx->modules;
    struct input_s *input     = &mCtx->input;
    
    if (Parser_Init(&mCtx->parserObj) != PARSER_ERROR_NONE) {
        Loge("Parser_Init() failed");
        goto parserInitExit;
    }
    
    Logd("Parsing main config file : \"%s\"", options.mainXml);
    
    struct parser_tags_handler_s tagsHandlers[] = {
    	{ XML_TAG_GENERAL,      onGeneralCb,           NULL,                NULL },
    	{ XML_TAG_CONTROLLERS,  onControllersStartCb,  onControllersEndCb,  NULL },
    	{ XML_TAG_ITEM,         onItemCb,              NULL,                NULL },
    	{ XML_TAG_GRAPHICS,     onGraphicsCb,          NULL,                NULL },
    	{ XML_TAG_VIDEOS,       onVideosCb,            NULL,                NULL },
    	{ XML_TAG_SERVERS,      onServersCb,           NULL,                NULL },
    	{ XML_TAG_CLIENTS,      onClientsCb,           NULL,                NULL },
    	{ NULL,                 NULL,                  NULL,                NULL }
    };
    
    struct parser_params_s parserParams;
    strncpy(parserParams.path, options.mainXml, sizeof(parserParams.path));
    parserParams.encoding     = PARSER_ENCODING_UTF_8;
    parserParams.tagsHandlers = tagsHandlers;
    parserParams.onErrorCb    = onErrorCb;
    parserParams.userData     = mCtx;
    
    if (mCtx->parserObj->parse(mCtx->parserObj, &parserParams) != PARSER_ERROR_NONE) {
        Loge("Failed to parse main config file");
        goto parserParseExit;
    }
    
    Logd("Initializing modules");
    
    if (input->graphicsConfig.enable
        && (Graphics_Init(&modules->graphicsObj) != GRAPHICS_ERROR_NONE)) {
        Loge("Graphics_Init() failed");
        goto graphicsInitExit;
    }
    
    if (input->videosConfig.enable
        && (Video_Init(&modules->videoObj) != VIDEO_ERROR_NONE)) {
        Loge("Video_Init() failed");
        goto videoInitExit;
    }
    
    if (input->serversConfig.enable
        && (Server_Init(&modules->serverObj) != SERVER_ERROR_NONE)) {
        Loge("Server_Init() failed");
        goto serverInitExit;
    }
    
    if (input->clientsConfig.enable
        && (Client_Init(&modules->clientObj) != CLIENT_ERROR_NONE)) {
        Loge("Client_Init() failed");
        goto clientInitExit;
    }
    
    struct core_s *coreObj = NULL;
    if (Core_Init(&coreObj, mCtx) != CORE_ERROR_NONE) {
        Loge("Core_Init() failed");
        goto coreInitExit;
    }
    
    Logd("Starting modules");

    uint32_t index;
    struct graphics_infos_s *graphicsInfos = &params->graphicsInfos;
    enum graphics_error_e gfxError         = GRAPHICS_ERROR_NONE;
    
    if (modules->graphicsObj) {
        Logd("Loading graphics params");
        if (coreObj->loadGraphicsParams(coreObj) != CORE_ERROR_NONE) {
            Loge("Failed to load graphics params");
            goto loadGraphicsParamsExit;
        }

        graphicsInfos->state = MODULE_STATE_STOPPED;

        if (input->graphicsConfig.autoStart) {
            Logd("Creating graphics drawer");
            gfxError = modules->graphicsObj->createDrawer(modules->graphicsObj,
                                                          &graphicsInfos->graphicsParams);
            if (gfxError != GRAPHICS_ERROR_NONE) {
                Loge("createDrawer() failed");
                goto createDrawerExit;
            }

            /* Drawer is created so graphics module's state must be updated here so as to
             * destroy drawer when stopping the module */
            graphicsInfos->state = MODULE_STATE_STARTED;

            Logd("Displaying %u graphics elements", graphicsInfos->nbGfxElements);
            gfxError = modules->graphicsObj->drawAllElements(modules->graphicsObj);
            if (gfxError != GRAPHICS_ERROR_NONE) {
                Loge("Failed to draw all elements");
                goto destroyDrawerExit;
            }
        }
    }

    uint8_t videoIndex;
    struct video_area_s videoArea        = {0};
    struct videos_infos_s *videosInfos   = NULL;
    struct video_device_s **videoDevices = NULL;
    struct video_device_s *videoDevice   = NULL;
    uint8_t nbDevices                    = 0;
    enum video_error_e videoError        = VIDEO_ERROR_NONE;
    
    if (modules->videoObj) {
        Logd("Loading video params");
        if (coreObj->loadVideosParams(coreObj) != CORE_ERROR_NONE) {
            Loge("Failed to load videos params");
            goto loadVideosParamsExit;
        }

        videosInfos  = &params->videosInfos;
        videoDevices = videosInfos->devices;
        nbDevices    = videosInfos->nbDevices;

        for (videoIndex = 0; videoIndex < nbDevices; videoIndex++) {
            videoDevice        = videoDevices[videoIndex];
            videoDevice->state = MODULE_STATE_STOPPED;

            if (input->videosConfig.autoStart) {
                Logd("Starting video capture on device \"%s\"", videoDevice->videoParams.name);
                videoError = modules->videoObj->startDeviceCapture(modules->videoObj,
                                                                   &videoDevice->videoParams);
                if (videoError != VIDEO_ERROR_NONE) {
                    Loge("startDeviceCapture() failed");
                    goto startDeviceCaptureExit;
                }

                videoDevice->state = MODULE_STATE_STARTED;

                videoError = modules->videoObj->getFinalVideoArea(modules->videoObj,
                                                                  &videoDevice->videoParams,
                                                                  &videoArea);
                if (videoError != VIDEO_ERROR_NONE) {
                    Loge("startDeviceCapture() failed");
                    goto startDeviceCaptureExit;
                }

                Logd("Device \"%s\" : width = %u - height = %u",
                        videoDevice->videoParams.name, videoArea.width, videoArea.height);
            }
        }
    }

    size_t maxBufferSize                 = 0;
    uint8_t nbServers                    = 0;
    struct server_infos_s *serverInfos   = NULL;
    struct server_params_s *serverParams = NULL;

    if (modules->serverObj) {
        Logd("Loading servers params");
        if (coreObj->loadServersParams(coreObj) != CORE_ERROR_NONE) {
            Loge("Failed to load servers params");
            goto loadServersParamsExit;
        }

        nbServers = params->serversInfos.nbServers;
        Logd("Starting %u servers", nbServers);

        for (index = 0; index < nbServers; index++) {
            serverInfos        = params->serversInfos.serverInfos[index];
            serverParams       = &serverInfos->serverParams;
            serverInfos->state = MODULE_STATE_STOPPED;

            serverParams->maxBufferSize = (size_t)input->maxBufferSize;

            if (serverParams->maxBufferSize == (size_t)-1) {
                for (videoIndex = 0; videoIndex < nbDevices; videoIndex++) {
                    videoDevice = videoDevices[videoIndex];
                    if (!videoDevice->serverDest
                        || (strcmp(videoDevice->serverDest, serverParams->name) != 0)) {
                        continue;
                    }
                    (void)modules->videoObj->getMaxBufferSize(modules->videoObj,
                                                              &videoDevice->videoParams,
                                                              &maxBufferSize);
                    break;
                }

                serverParams->maxBufferSize = (maxBufferSize > 0 ? maxBufferSize : MAX_BUFFER_SIZE);
                maxBufferSize = 0;
            }

            Logd("Server \"%s\"'s max buffer size set to %lu bytes",
                    serverParams->name, serverParams->maxBufferSize);

            if (input->serversConfig.autoStart) {
                if (modules->serverObj->start(modules->serverObj,
                                              serverParams) != SERVER_ERROR_NONE) {
                    Loge("Failed to start server %s", serverParams->name);
                    goto stopServersExit;
                }
                serverInfos->state = MODULE_STATE_STARTED;
            }
        }
    }

    uint8_t nbClients                    = 0;
    struct client_infos_s *clientInfos   = NULL;
    struct client_params_s *clientParams = NULL;

    if (modules->clientObj) {
        Logd("Loading clients params");
        if (coreObj->loadClientsParams(coreObj) != CORE_ERROR_NONE) {
            Loge("Failed to load clients params");
            goto loadClientsParamsExit;
        }

        nbClients = params->clientsInfos.nbClients;
        Logd("Starting %u clients", nbClients);

        for (index = 0; index < nbClients; index++) {
            clientInfos        = params->clientsInfos.clientInfos[index];
            clientParams       = &clientInfos->clientParams;
            clientInfos->state = MODULE_STATE_STOPPED;

            clientParams->maxBufferSize = (input->maxBufferSize <= 0 ? MAX_BUFFER_SIZE
                                                                     : (size_t)input->maxBufferSize);
            Logd("Client \"%s\"'s max buffer size set to %lu bytes",
                    clientParams->name, clientParams->maxBufferSize);

            if (input->clientsConfig.autoStart) {
                if (modules->clientObj->start(modules->clientObj,
                                              clientParams) != CLIENT_ERROR_NONE) {
                    Loge("Failed to start client %s", clientParams->name);
                    //goto stopClientsExit;
                }
                else {
                    clientInfos->state = MODULE_STATE_STARTED;
                }
            }
        }
    }

    uint8_t nbVideoListeners                  = 0;
    struct video_listener_s  **videoListeners = NULL;

    if (modules->videoObj && input->videosConfig.autoStart) {
        for (videoIndex = 0; videoIndex < nbDevices; videoIndex++) {
            videoDevice      = videoDevices[videoIndex];
            nbVideoListeners = videoDevice->nbVideoListeners;
            videoListeners   = videoDevice->videoListeners;

            Logd("Registering %u video listeners on device \"%s\"",
                    nbVideoListeners, videoDevice->videoParams.name);
            for (index = 0; index < nbVideoListeners; index++) {
                if (modules->videoObj->registerListener(modules->videoObj,
                                                        &videoDevice->videoParams,
                                                        videoListeners[index]) != VIDEO_ERROR_NONE) {
                    Loge("Failed to register listener \"%s\"", (videoListeners[index])->name);
                    goto registerVideoListenersExit;
                }
            }
        }
    }
    
    if (coreObj->keepAppRunning(coreObj, input->keepAliveMethod,
                                input->timeout_s) != CORE_ERROR_NONE) {
        Loge("Error occurred trying to keep app running");
    }
    else {
        Logd("Stopping app");
        ret = EXIT_SUCCESS;
    }

registerVideoListenersExit:
    if (modules->videoObj) {
        Logd("Unregistering video listeners");
        for (videoIndex = 0; videoIndex < nbDevices; videoIndex++) {
            videoDevice      = videoDevices[videoIndex];
            nbVideoListeners = videoDevice->nbVideoListeners;
            videoListeners   = videoDevice->videoListeners;

            Logd("State of \"%s\" at exit : %u",
                    videoDevice->videoParams.name, videoDevice->state);

            if (videoDevice->state == MODULE_STATE_STARTED) {
                Logd("Unregistering %u video listeners on device \"%s\"",
                        nbVideoListeners, videoDevice->videoParams.name);
                for (index = 0; index < nbVideoListeners; index++) {
                    (void)modules->videoObj->unregisterListener(modules->videoObj,
                                                                &videoDevice->videoParams,
                                                                videoListeners[index]);
                }
            }
        }
    }

//stopClientsExit:
    if (modules->clientObj) {
        for (index = 0; index < nbClients; index++) {
            clientInfos  = params->clientsInfos.clientInfos[index];
            clientParams = &clientInfos->clientParams;

            Logd("State of \"%s\" at exit : %u", clientParams->name, clientInfos->state);

            if (clientInfos->state == MODULE_STATE_STARTED) {
                Logd("Stopping client \"%s\"", clientParams->name);
                (void)modules->clientObj->stop(modules->clientObj, clientParams);
            }

            clientInfos->state = MODULE_STATE_STOPPED;
        }

        Logd("Unloading clients params");
        (void)coreObj->unloadClientsParams(coreObj);
    }

loadClientsParamsExit:
stopServersExit:
    if (modules->serverObj) {
        for (index = 0; index < nbServers; index++) {
            serverInfos  = params->serversInfos.serverInfos[index];
            serverParams = &serverInfos->serverParams;

            Logd("State of \"%s\" at exit : %u", serverParams->name, serverInfos->state);

            if (serverInfos->state == MODULE_STATE_STARTED) {
                Logd("Stopping server \"%s\"", serverParams->name);
                (void)modules->serverObj->stop(modules->serverObj, serverParams);
            }

            serverInfos->state = MODULE_STATE_STOPPED;
        }

        Logd("Unloading servers params");
        (void)coreObj->unloadServersParams(coreObj);
    }

loadServersParamsExit:
startDeviceCaptureExit:
    if (modules->videoObj) {
        for (videoIndex = 0; videoIndex < nbDevices; videoIndex++) {
            videoDevice = videoDevices[videoIndex];

            Logd("State of \"%s\" at exit : %u",
                    videoDevice->videoParams.name, videoDevice->state);

            if (videoDevice->state == MODULE_STATE_STARTED) {
                Logd("Stopping video capture on device \"%s\"", videoDevice->videoParams.name);
                (void)modules->videoObj->stopDeviceCapture(modules->videoObj,
                                                           &videoDevice->videoParams);
            }

            videoDevice->state = MODULE_STATE_STOPPED;
        }

        Logd("Unloading videos params");
        (void)coreObj->unloadVideosParams(coreObj);
    }

loadVideosParamsExit:
destroyDrawerExit:
    if (modules->graphicsObj) {
        Logd("Destroying elements and drawer - graphics module'state : %u", graphicsInfos->state);
        if (graphicsInfos->state == MODULE_STATE_STARTED) {
            (void)modules->graphicsObj->destroyDrawer(modules->graphicsObj);
        }

        graphicsInfos->state = MODULE_STATE_STOPPED;
    }

createDrawerExit:
    if (modules->graphicsObj) {
        Logd("Unloading graphics params");
        (void)coreObj->unloadGraphicsParams(coreObj);
    }

loadGraphicsParamsExit:
    Logd("Uninitializing coreObj");
    (void)Core_UnInit(&coreObj);

coreInitExit:
    if (modules->clientObj) {
        Logd("Uninitializing clientObj");
        (void)Client_UnInit(&modules->clientObj);
    }

clientInitExit:
    if (modules->serverObj) {
        Logd("Uninitializing serverObj");
        (void)Server_UnInit(&modules->serverObj);
    }

serverInitExit:
    if (modules->videoObj) {
        Logd("Uninitializing videoObj");
        (void)Video_UnInit(&modules->videoObj);
    }

videoInitExit:
    if (modules->graphicsObj) {
        Logd("Uninitializing graphicsObj");
        (void)Graphics_UnInit(&modules->graphicsObj);
    }

graphicsInitExit:
parserParseExit:
    if (input->appDataDir) {
        free(input->appDataDir);
    }

    if (input->resRootDir) {
        free(input->resRootDir);
    }

    if (input->libRootDir) {
        free(input->libRootDir);
    }

    uint8_t count;
    while (input->nbCtrlLibs > 0) {
        count = input->nbCtrlLibs - 1;
        if (input->ctrlLibs[count].path) {
            free(input->ctrlLibs[count].path);
        }
        if (input->ctrlLibs[count].initFn) {
            free(input->ctrlLibs[count].initFn);
        }
        if (input->ctrlLibs[count].uninitFn) {
            free(input->ctrlLibs[count].uninitFn);
        }
        if (input->ctrlLibs[count].onCommandCb) {
            free(input->ctrlLibs[count].onCommandCb);
        }
        if (input->ctrlLibs[count].onEventCb) {
            free(input->ctrlLibs[count].onEventCb);
        }
        --input->nbCtrlLibs;
    }

    if (input->ctrlLibs) {
        free(input->ctrlLibs);
    }

    if (input->clientsConfig.xml) {
        free(input->clientsConfig.xml);
    }
    
    if (input->serversConfig.xml) {
        free(input->serversConfig.xml);
    }
    
    if (input->videosConfig.xml) {
        free(input->videosConfig.xml);
    }
    
    if (input->graphicsConfig.xml) {
        free(input->graphicsConfig.xml);
    }
    
    if (mCtx->parserObj) {
        Logd("Uninitializing parser");
        (void)Parser_UnInit(&mCtx->parserObj);
    }

parserInitExit:
    free(mCtx);
    
    return ret;
}

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////////// CALLBACKS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 * Called when <General> tag is found
 */
static void onGeneralCb(void *userData, const char **attrs)
{
    assert(userData);
	
    struct context_s *ctx      = (struct context_s*)userData;
    struct parser_s *parserObj = ctx->parserObj;
    struct input_s *input      = &ctx->input;
    
    struct parser_attr_handler_s attrHandlers[] = {
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
    	    .attrName          = XML_ATTR_LIB_ROOT_DIR,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&input->libRootDir,
    	    .attrGetter.vector = parserObj->getString
        },
    	{
    	    .attrName          = XML_ATTR_KEEP_ALIVE_METHOD,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&input->keepAliveMethod,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_TIMEOUT_S,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&input->timeout_s,
    	    .attrGetter.scalar = parserObj->getUint32
        },
    	{
    	    .attrName          = XML_ATTR_MAX_BUFFER_SIZE,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&input->maxBufferSize,
    	    .attrGetter.scalar = parserObj->getInt32
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
 * Called when <Controllers> tag is found
 */
static void onControllersStartCb(void *userData, const char **attrs)
{
    assert(userData);

    (void)attrs;

    Logd("Start parsing Controllers");

    struct context_s *ctx      = (struct context_s*)userData;
    struct parser_s *parserObj = ctx->parserObj;
    struct input_s *input      = &ctx->input;

    struct parser_attr_handler_s attrHandlers[] = {
        {
            .attrName          = XML_ATTR_PRIORITY,
            .attrType          = PARSER_ATTR_TYPE_SCALAR,
            .attrValue.scalar  = (void*)&input->ctrlLibsPrio,
            .attrGetter.scalar = parserObj->getUint8
        },
        {
            NULL,
            PARSER_ATTR_TYPE_NONE,
            NULL,
            NULL
        }
    };

    if (parserObj->getAttributes(parserObj, attrHandlers, attrs) != PARSER_ERROR_NONE) {
        Logd("Failed to retrieve attributes in \"Controllers\" tag");
    }

    Logd("Control library - priority : \"%u\"", input->ctrlLibsPrio);
}

/*!
 * Called when </Controllers> tag is found
 */
static void onControllersEndCb(void *userData)
{
    assert(userData);

    Logd("End parsing Controllers");
}

/*!
 * Called when <Item> tag is found in <Controllers> parent tag
 */
static void onItemCb(void *userData, const char **attrs)
{
    assert(userData);

    struct context_s *ctx      = (struct context_s*)userData;
    struct parser_s *parserObj = ctx->parserObj;
    struct input_s *input      = &ctx->input;

    Logd("Adding control library %u", (input->nbCtrlLibs + 1));

    input->ctrlLibs = realloc(input->ctrlLibs, (input->nbCtrlLibs + 1) * sizeof(struct library_s));
    assert(input->ctrlLibs);

    memset(&input->ctrlLibs[input->nbCtrlLibs], '\0', sizeof(struct library_s));

    struct library_s *ctrlLib = &input->ctrlLibs[input->nbCtrlLibs];

    struct parser_attr_handler_s attrHandlers[] = {
        {
            .attrName          = XML_ATTR_LIB_NAME,
            .attrType          = PARSER_ATTR_TYPE_VECTOR,
            .attrValue.vector  = (void**)&ctrlLib->path,
            .attrGetter.vector = parserObj->getString
        },
        {
            .attrName          = XML_ATTR_INIT_FN,
            .attrType          = PARSER_ATTR_TYPE_VECTOR,
            .attrValue.vector  = (void**)&ctrlLib->initFn,
            .attrGetter.vector = parserObj->getString
        },
        {
            .attrName          = XML_ATTR_UNINIT_FN,
            .attrType          = PARSER_ATTR_TYPE_VECTOR,
            .attrValue.vector  = (void**)&ctrlLib->uninitFn,
            .attrGetter.vector = parserObj->getString
        },
        {
            .attrName          = XML_ATTR_ON_COMMAND_CB,
            .attrType          = PARSER_ATTR_TYPE_VECTOR,
            .attrValue.vector  = (void**)&ctrlLib->onCommandCb,
            .attrGetter.vector = parserObj->getString
        },
        {
            .attrName          = XML_ATTR_ON_EVENT_CB,
            .attrType          = PARSER_ATTR_TYPE_VECTOR,
            .attrValue.vector  = (void**)&ctrlLib->onEventCb,
            .attrGetter.vector = parserObj->getString
        },
        {
            NULL,
            PARSER_ATTR_TYPE_NONE,
            NULL,
            NULL
        }
    };

    ++input->nbCtrlLibs;

    if (parserObj->getAttributes(parserObj, attrHandlers, attrs) != PARSER_ERROR_NONE) {
        Logd("Failed to retrieve attributes in \"Item\" tag");
    }

    if (ctrlLib->path) {
        char *temp   = strdup(ctrlLib->path);
        uint32_t len = (strlen(temp) + strlen(input->libRootDir) + 2) * sizeof(char);

        assert((ctrlLib->path = realloc(ctrlLib->path, len)));
        memset(ctrlLib->path, '\0', len);

        snprintf(ctrlLib->path, len, "%s/%s", input->libRootDir, temp);

        free(temp);
    }

    Logd("Control library - path : \"%s\" / \
                            initFn : \"%s\" / uninitFn : \"%s\" / \
                            onCommandCb : \"%s\" / onEventCb ; \"%s\"",
                            ctrlLib->path,
                            ctrlLib->initFn, ctrlLib->uninitFn,
                            ctrlLib->onCommandCb, ctrlLib->onEventCb);
}

/*!
 * Called when <Graphics> tag is found
 */
static void onGraphicsCb(void *userData, const char **attrs)
{
    assert(userData);

    struct context_s *ctx      = (struct context_s*)userData;
    struct parser_s *parserObj = ctx->parserObj;
    struct input_s *input      = &ctx->input;
    
    struct parser_attr_handler_s attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_ENABLE,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&input->graphicsConfig.enable,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_AUTO_START,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&input->graphicsConfig.autoStart,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_XML_FILE,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&input->graphicsConfig.xml,
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

    struct context_s *ctx      = (struct context_s*)userData;
    struct parser_s *parserObj = ctx->parserObj;
    struct input_s *input      = &ctx->input;
    
    struct parser_attr_handler_s attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_ENABLE,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&input->videosConfig.enable,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_AUTO_START,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&input->videosConfig.autoStart,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_XML_FILE,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&input->videosConfig.xml,
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

    struct context_s *ctx      = (struct context_s*)userData;
    struct parser_s *parserObj = ctx->parserObj;
    struct input_s *input      = &ctx->input;
    
    struct parser_attr_handler_s attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_ENABLE,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&input->serversConfig.enable,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_AUTO_START,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&input->serversConfig.autoStart,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_XML_FILE,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&input->serversConfig.xml,
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

    struct context_s *ctx      = (struct context_s*)userData;
    struct parser_s *parserObj = ctx->parserObj;
    struct input_s *input      = &ctx->input;
    
    struct parser_attr_handler_s attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_ENABLE,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&input->clientsConfig.enable,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_AUTO_START,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&input->clientsConfig.autoStart,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_XML_FILE,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&input->clientsConfig.xml,
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

/* -------------------------------------------------------------------------------------------- */
/* ///////////////////////////// PRIVATE FUNCTIONS IMPLEMENTATION ///////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

static void usage(const char * const program)
{
#ifdef _POSIX_PRIORITY_SCHEDULING
    Loge("Usage: %s [-f|--mainXml <path to Main.xml>][-p|--priority <%d --> %d>]",
            program, sched_get_priority_min(SCHED_POLICY), sched_get_priority_max(SCHED_POLICY));
#else
    Loge("Usage: %s [-f|--mainXml <path to Main.xml>][-n|--niceness <%d --> %d>]",
            program, MIN_NICENESS, MAX_NICENESS);
#endif
}

static void parseOptions(int argc, char **argv, struct options_s *out)
{
    int opt = 0;

    const char* const short_options    = "f:p:n:";
    const struct option long_options[] = {
        { "mainXml",   required_argument,  NULL,  'f' },
        { "priority",  required_argument,  NULL,  'p' },
        { "niceness",  required_argument,  NULL,  'n' },
        { NULL,        0,                  NULL,   0  }
    };

    while (opt != -1) {
        opt = getopt_long(argc, argv, short_options, long_options, NULL);

        switch (opt) {
            case -1:
                Logd("No more option is set");
                break;

            case 'f':
                out->mainXml = optarg;
                break;

            case 'p':
            case 'n':
                out->priority = atoi(optarg);
                break;

            default:
                usage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }
}

static void setPriority(struct options_s *in)
{
#ifdef _POSIX_PRIORITY_SCHEDULING
    if ((in->priority < sched_get_priority_min(SCHED_POLICY))
        || (in->priority > sched_get_priority_max(SCHED_POLICY))) {
        return;
    }

    Logd("Setting priority to \"%d\" with policy \"%d\"", in->priority, SCHED_POLICY);
    struct sched_param param = { .sched_priority = in->priority };
    if (sched_setscheduler(getpid(), SCHED_POLICY, &param) < 0) {
        Loge("sched_setscheduler() failed - %s", strerror(errno));
    }
#else
    if ((in->priority < MIN_NICENESS) || (in->priority > MAX_NICENESS)) {
        return;
    }

    Logd("Setting niceness to \"%d\"", in->priority);
    if (setpriority(PRIO_PROCESS, getpid(), in->priority) < 0) {
        Loge("setpriority() failed - %s", strerror(errno));
    }
#endif
}
