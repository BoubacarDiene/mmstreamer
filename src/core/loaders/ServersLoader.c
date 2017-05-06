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
* \file   ServersLoader.c
* \brief  TODO
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include "core/Loaders.h"
#include "core/XmlDefines.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                            */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "SERVERS-LOADER"

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                         PROTOTYPES                                           */
/* -------------------------------------------------------------------------------------------- */

LOADERS_ERROR_E loadServersXml_f  (LOADERS_S *obj, CONTEXT_S *ctx, XML_SERVERS_S *xmlServers);
LOADERS_ERROR_E unloadServersXml_f(LOADERS_S *obj, XML_SERVERS_S *xmlServers);

static void onServerStartCb(void *userData, const char **attrs);
static void onServerEndCb  (void *userData);

static void onGeneralCb    (void *userData, const char **attrs);
static void onInetCb       (void *userData, const char **attrs);
static void onUnixCb       (void *userData, const char **attrs);

static void onErrorCb(void *userData, int32_t errorCode, const char *errorStr);

/* -------------------------------------------------------------------------------------------- */
/*                                          VARIABLES                                           */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                          FUNCTIONS                                           */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
LOADERS_ERROR_E loadServersXml_f(LOADERS_S *obj, CONTEXT_S *ctx, XML_SERVERS_S *xmlServers)
{
    assert(obj && ctx && xmlServers);
    
    PARSER_S *parserObj  = ctx->modules.parserObj;
    INPUT_S *input       = &ctx->input;
    
    xmlServers->reserved = ctx;
    
    Logd("Parsing file : \"%s/%s\"", input->resRootDir, input->serversXml);
    
    PARSER_TAGS_HANDLER_S tagsHandlers[] = {
    	{ XML_TAG_SERVER,   onServerStartCb,  onServerEndCb,  NULL },
    	{ XML_TAG_GENERAL,  onGeneralCb,      NULL,           NULL },
    	{ XML_TAG_INET,     onInetCb,         NULL,           NULL },
    	{ XML_TAG_UNIX,     onUnixCb,         NULL,           NULL },
    	{ NULL,             NULL,             NULL,           NULL }
    };
    
    PARSER_PARAMS_S parserParams;
    snprintf(parserParams.path, sizeof(parserParams.path), "%s/%s", input->resRootDir, input->serversXml);
    parserParams.encoding     = PARSER_ENCODING_UTF_8;
    parserParams.tagsHandlers = tagsHandlers;
    parserParams.onErrorCb    = onErrorCb;
    parserParams.userData     = xmlServers;
    
    if (parserObj->parse(parserObj, &parserParams) != PARSER_ERROR_NONE) {
        Loge("Failed to parse file");
        xmlServers->reserved = NULL;
        return LOADERS_ERROR_XML;
    }
    
    return LOADERS_ERROR_NONE;
}

/*!
 *
 */
LOADERS_ERROR_E unloadServersXml_f(LOADERS_S *obj, XML_SERVERS_S *xmlServers)
{
    assert(obj && xmlServers);
    
    uint8_t index;
    XML_SERVER_S *server;
    
    for (index = 0; index < xmlServers->nbServers; index++) {
        server = &xmlServers->servers[index];
        if (server->name) {
            free(server->name);
            server->name = NULL;
        }
        if (server->mime) {
            free(server->mime);
            server->mime = NULL;
        }
        if (server->host) {
            free(server->host);
            server->host = NULL;
        }
        if (server->service) {
            free(server->service);
            server->service = NULL;
        }
        if (server->path) {
            free(server->path);
            server->path = NULL;
        }
        if (server->socketName) {
            free(server->socketName);
            server->socketName = NULL;
        }
    }
    
    xmlServers->reserved = NULL;
    
    return LOADERS_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/*                                           CALLBACKS                                          */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static void onServerStartCb(void *userData, const char **attrs)
{
    assert(userData);
    
    (void)attrs;
    
    XML_SERVERS_S *xmlServers = (XML_SERVERS_S*)userData;
    
    Logd("Adding server %u", (xmlServers->nbServers + 1));
    
    xmlServers->servers = realloc(xmlServers->servers, (xmlServers->nbServers + 1) * sizeof(XML_SERVER_S));
    assert(xmlServers->servers);
    
    memset(&xmlServers->servers[xmlServers->nbServers], '\0', sizeof(XML_SERVER_S));
}

/*!
 *
 */
static void onServerEndCb(void *userData)
{
    assert(userData);
    
    XML_SERVERS_S *xmlServers = (XML_SERVERS_S*)userData;

    xmlServers->nbServers++;
    
    Logd("Server %u added", xmlServers->nbServers);
}

/*!
 *
 */
static void onGeneralCb(void *userData, const char **attrs)
{
    assert(userData);
    
    XML_SERVERS_S *xmlServers = (XML_SERVERS_S*)userData;
    XML_SERVER_S *server      = &xmlServers->servers[xmlServers->nbServers];
    CONTEXT_S *ctx            = (CONTEXT_S*)xmlServers->reserved;
    PARSER_S *parserObj       = ctx->modules.parserObj;
    
    PARSER_ATTR_HANDLER_S attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_NAME,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&server->name,
    	    .attrGetter.vector = parserObj->getString
        },
    	{
    	    .attrName          = XML_ATTR_TYPE,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&server->type,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_LINK,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&server->link,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_MODE,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&server->mode,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_ACCEPT_MODE,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&server->acceptMode,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_PRIORITY,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&server->priority,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_MAX_CLIENTS,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&server->maxClients,
    	    .attrGetter.scalar = parserObj->getUint32
        },
    	{
    	    .attrName          = XML_ATTR_MIME,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&server->mime,
    	    .attrGetter.vector = parserObj->getString
        },
    	{
    	    .attrName          = XML_ATTR_MAX_BUFFER_SIZE,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&server->maxBufferSize,
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
 *
 */
static void onInetCb(void *userData, const char **attrs)
{
    assert(userData);
    
    XML_SERVERS_S *xmlServers = (XML_SERVERS_S*)userData;
    XML_SERVER_S *server      = &xmlServers->servers[xmlServers->nbServers];
    CONTEXT_S *ctx            = (CONTEXT_S*)xmlServers->reserved;
    PARSER_S *parserObj       = ctx->modules.parserObj;
    
    PARSER_ATTR_HANDLER_S attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_HOST,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&server->host,
    	    .attrGetter.vector = parserObj->getString
        },
    	{
    	    .attrName          = XML_ATTR_SERVICE,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void*)&server->service,
    	    .attrGetter.vector = parserObj->getString
        },
    	{
    	    .attrName          = XML_ATTR_PATH,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void*)&server->path,
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
    	Loge("Failed to retrieve attributes in \"Inet\" tag");
    }
}

/*!
 *
 */
static void onUnixCb(void *userData, const char **attrs)
{
    assert(userData);
    
    XML_SERVERS_S *xmlServers = (XML_SERVERS_S*)userData;
    XML_SERVER_S *server      = &xmlServers->servers[xmlServers->nbServers];
    CONTEXT_S *ctx            = (CONTEXT_S*)xmlServers->reserved;
    PARSER_S *parserObj       = ctx->modules.parserObj;
    
    PARSER_ATTR_HANDLER_S attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_SOCKET_NAME,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&server->socketName,
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
    	Loge("Failed to retrieve attributes in \"Unix\" tag");
    }
}

/*!
 *
 */
static void onErrorCb(void *userData, int32_t errorCode, const char *errorStr)
{
    (void)userData;
    
    Loge("Parsing error - errorCode = %d / errorStr = \"%s\"", errorCode, errorStr);
}
