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

#include "specific/Specific.h"
#include "specific/XmlDefines.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                            */
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
static void onVideoCb   (void *userData, const char **attrs);
static void onServersCb (void *userData, const char **attrs);
static void onClientsCb (void *userData, const char **attrs);

static void onErrorCb(void *userData, int32_t errorCode, const char *errorStr);

/* -------------------------------------------------------------------------------------------- */
/*                                           MAIN                                               */
/* -------------------------------------------------------------------------------------------- */

/*!
 * main() function
 */
int main(void)
{
    int ret = EXIT_FAILURE;
    
    CONTEXT_S *mCtx;
    assert((mCtx = calloc(1, sizeof(CONTEXT_S))));
    
    PARAMS_S  *params  = &mCtx->params;
    MODULES_S *modules = &mCtx->modules;
    INPUT_S   *input   = &mCtx->input;
    
    if (Parser_Init(&modules->parserObj) != PARSER_ERROR_NONE) {
        Loge("Parser_Init() failed");
        goto parserInit;
    }
    
    Logd("Parsing main config file : \"%s\"", MAIN_CONFIG_FILE);
    
    PARSER_TAGS_HANDLER_S tagsHandlers[] = {
    	{ XML_TAG_GENERAL,   onGeneralCb,   NULL,  NULL },
    	{ XML_TAG_GRAPHICS,  onGraphicsCb,  NULL,  NULL },
    	{ XML_TAG_VIDEO,     onVideoCb,     NULL,  NULL },
    	{ XML_TAG_SERVERS,   onServersCb,   NULL,  NULL },
    	{ XML_TAG_CLIENTS,   onClientsCb,   NULL,  NULL },
    	{ NULL,              NULL,          NULL,  NULL }
    };
    
    PARSER_PARAMS_S parserParams;
    strncpy(parserParams.path, MAIN_CONFIG_FILE, sizeof(parserParams.path));
    parserParams.encoding     = PARSER_ENCODING_UTF_8;
    parserParams.tagsHandlers = tagsHandlers;
    parserParams.onErrorCb    = onErrorCb;
    parserParams.userData     = mCtx;
    
    if (modules->parserObj->parse(modules->parserObj, &parserParams) != PARSER_ERROR_NONE) {
        Loge("Failed to parse main config file");
        goto parserParse;
    }
    
    Logd("Initializing modules");
    
    if (input->graphicsEnabled && (Graphics_Init(&modules->graphicsObj) != GRAPHICS_ERROR_NONE)) {
        Loge("Graphics_Init() failed");
        goto graphicsInit;
    }
    
    if (input->videoEnabled && (Video_Init(&modules->videoObj) != VIDEO_ERROR_NONE)) {
        Loge("Video_Init() failed");
        goto videoInit;
    }
    
    if (input->serverEnabled && (Server_Init(&modules->serverObj) != SERVER_ERROR_NONE)) {
        Loge("Server_Init() failed");
        goto serverInit;
    }
    
    if (input->clientEnabled && (Client_Init(&modules->clientObj) != CLIENT_ERROR_NONE)) {
        Loge("Client_Init() failed");
        goto clientInit;
    }
    
    SPECIFIC_S *specificObj = NULL;
    if (Specific_Init(&specificObj, mCtx) != SPECIFIC_ERROR_NONE) {
        Loge("Specific_Init() failed");
        goto specificInit;
    }
    
    Logd("Starting modules");

    uint32_t index;
    
    if (modules->graphicsObj) {
        Logd("Loading graphics params");
        if (specificObj->loadGraphicsParams(specificObj) != SPECIFIC_ERROR_NONE) {
            Loge("Failed to load graphics params");
            goto loadGraphicsParams;
        }
        
        if (input->autoStartGraphics) {
            Logd("Creating graphics drawer");
            if (modules->graphicsObj->createDrawer(modules->graphicsObj, &params->graphicsInfos.graphicsParams) != GRAPHICS_ERROR_NONE) {
                Loge("createDrawer() failed");
                goto createDrawer;
            }
        
            Logd("Displaying %u graphics elements", params->graphicsInfos.nbGfxElements);
            if (modules->graphicsObj->drawAllElements(modules->graphicsObj) != GRAPHICS_ERROR_NONE) {
                Loge("Failed to draw all elements");
                goto destroyDrawer;
            }
        }
    }
    
    size_t maxBufferSize = -1;
    VIDEO_RESOLUTION_S resolution;
    
    if (modules->videoObj) {
        Logd("Loading video params");
        if (specificObj->loadVideoParams(specificObj) != SPECIFIC_ERROR_NONE) {
            Loge("Failed to load video params");
            goto loadVideoParams;
        }
        
        if (input->autoStartVideo) {
            Logd("Starting device capture");
            if (modules->videoObj->startDeviceCapture(modules->videoObj, &params->videoInfos.videoParams) != VIDEO_ERROR_NONE) {
                Loge("startDeviceCapture() failed");
                goto startDeviceCapture;
            }
            
            (void)modules->videoObj->getMaxBufferSize(modules->videoObj, &maxBufferSize);
            (void)modules->videoObj->getFinalResolution(modules->videoObj, &resolution);
            
            Logd("maxBufferSize = %lu bytes / width = %u - height = %u", maxBufferSize, resolution.width, resolution.height);
        }
    }
    
    if (modules->serverObj) {
        Logd("Loading servers params");
        if (specificObj->loadServersParams(specificObj) != SPECIFIC_ERROR_NONE) {
            Loge("Failed to load servers params");
            goto loadServersParams;
        }
        
        Logd("Starting %u servers", params->serversInfos.nbServers);
        for (index = 0; index < params->serversInfos.nbServers; index++) {
            if ((params->serversInfos.serverParams[index])->maxBufferSize == (size_t)-1) {
                if (maxBufferSize > 0) {
                    (params->serversInfos.serverParams[index])->maxBufferSize = maxBufferSize;
                }
                else {
                    (params->serversInfos.serverParams[index])->maxBufferSize = MAX_BUFFER_SIZE;
                }
                Logd("Server %s's buffer size set to %lu bytes",
                        (params->serversInfos.serverParams[index])->name,
                        (params->serversInfos.serverParams[index])->maxBufferSize);
            }

            if (input->autoStartServer
                && modules->serverObj->start(modules->serverObj, params->serversInfos.serverParams[index]) != SERVER_ERROR_NONE) {
                Loge("Failed to start server %s", (params->serversInfos.serverParams[index])->name);
                goto stop_servers;
            }
        }
    }
    
    if (modules->clientObj) {
        Logd("Loading clients params");
        if (specificObj->loadClientsParams(specificObj) != SPECIFIC_ERROR_NONE) {
            Loge("Failed to load clients params");
            goto loadClientsParams;
        }
        
        if (input->autoStartClient) {
            Logd("Starting %u clients", params->clientsInfos.nbClients);
            for (index = 0; index < params->clientsInfos.nbClients; index++) {
                if (modules->clientObj->start(modules->clientObj, params->clientsInfos.clientParams[index]) != CLIENT_ERROR_NONE) {
                    Loge("Failed to start client %s", (params->clientsInfos.clientParams[index])->name);
                    //goto stopClients;
                }
            }
        }
    }
    
    if (modules->videoObj && input->autoStartVideo) {
        Logd("Registering video listeners");
        for (index = 0; index < params->videoInfos.nbVideoListeners; index++) {
            if (modules->videoObj->registerListener(modules->videoObj, params->videoInfos.videoListeners[index]) != VIDEO_ERROR_NONE) {
                Loge("Failed to register listener \"%s\"", (params->videoInfos.videoListeners[index])->name);
                goto registerVideoListeners;
            }
        }
    }
    
    if (specificObj->keepAppRunning(specificObj, input->keepAliveMethod, input->timeout_s) != SPECIFIC_ERROR_NONE) {
        Loge("Error occurred trying to keep app running");
    }
    else {
        Logd("Stopping app");
        ret = EXIT_SUCCESS;
    }
    
registerVideoListeners:
    if (modules->videoObj && input->autoStartVideo) {
        Logd("Unregistering video listeners");
        for (index = 0; index < params->videoInfos.nbVideoListeners; index++) {
            (void)modules->videoObj->unregisterListener(modules->videoObj, params->videoInfos.videoListeners[index]);
        }
    }
    
stopClients:
    if (modules->clientObj) {
        Logd("Unloading clients params");
        if (input->autoStartClient) {
            for (index = 0; index < params->clientsInfos.nbClients; index++) {
                (void)modules->clientObj->stop(modules->clientObj, params->clientsInfos.clientParams[index]);
            }
        }
        (void)specificObj->unloadClientsParams(specificObj);
    }

loadClientsParams:
stop_servers:
    if (modules->serverObj) {
        Logd("Unloading servers params");
        if (input->autoStartServer) {
            for (index = 0; index < params->serversInfos.nbServers; index++) {
                (void)modules->serverObj->stop(modules->serverObj, params->serversInfos.serverParams[index]);
            }
        }
        (void)specificObj->unloadServersParams(specificObj);
    }

loadServersParams:
    if (modules->videoObj && input->autoStartVideo) {
        Logd("Stopping device capture");
        (void)modules->videoObj->stopDeviceCapture(modules->videoObj);
    }
    
startDeviceCapture:
    if (modules->videoObj) {
        Logd("Unloading video params");
        (void)specificObj->unloadVideoParams(specificObj);
    }

loadVideoParams:
destroyDrawer:
    if (modules->graphicsObj && !input->autoStartGraphics) {
        Logd("Destroying elements and drawer");
        (void)modules->graphicsObj->destroyDrawer(modules->graphicsObj);
    }

createDrawer:
    if (modules->graphicsObj) {
        Logd("Unloading graphics params");
        (void)specificObj->unloadGraphicsParams(specificObj);
    }

loadGraphicsParams:
    Logd("Uninitializing specificObj");
    (void)Specific_UnInit(&specificObj);

specificInit:
    if (modules->clientObj) {
        Logd("Uninitializing clientObj");
        (void)Client_UnInit(&modules->clientObj);
    }

clientInit:
    if (modules->serverObj) {
        Logd("Uninitializing serverObj");
        (void)Server_UnInit(&modules->serverObj);
    }

serverInit:
    if (modules->videoObj) {
        Logd("Uninitializing videoObj");
        (void)Video_UnInit(&modules->videoObj);
    }

videoInit:
    if (modules->graphicsObj) {
        Logd("Uninitializing graphicsObj");
        (void)Graphics_UnInit(&modules->graphicsObj);
    }

graphicsInit:
parserParse:
    if (input->clientsXml) {
        free(input->clientsXml);
        input->clientsXml = NULL;
    }
    
    if (input->serversXml) {
        free(input->serversXml);
        input->serversXml = NULL;
    }
    
    if (input->videoXml) {
        free(input->videoXml);
        input->videoXml = NULL;
    }
    
    if (input->graphicsXml) {
        free(input->graphicsXml);
        input->graphicsXml = NULL;
    }
    
    if (modules->parserObj) {
        Logd("Uninitializing parser");
        (void)Parser_UnInit(&modules->parserObj);
    }

parserInit:   
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
    	    .attrName          = XML_ATTR_XML_ROOT_DIR,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&input->xmlRootDir,
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
 * Called when <Video> tag is found
 */
static void onVideoCb(void *userData, const char **attrs)
{
    assert(userData);

    CONTEXT_S *ctx      = (CONTEXT_S*)userData;
    PARSER_S *parserObj = ctx->modules.parserObj;
    INPUT_S   *input    = &ctx->input;
    
    PARSER_ATTR_HANDLER_S attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_ENABLE,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&input->videoEnabled,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_AUTO_START,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&input->autoStartVideo,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_XML_FILE,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&input->videoXml,
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
    	    .attrValue.scalar  = (void*)&input->serverEnabled,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_AUTO_START,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&input->autoStartServer,
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
    	    .attrValue.scalar  = (void*)&input->clientEnabled,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_AUTO_START,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&input->autoStartClient,
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
