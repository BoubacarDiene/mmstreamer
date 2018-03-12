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
* \file ServersLoader.c
* \brief TODO
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// HEADERS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#include "core/Loaders.h"
#include "core/XmlDefines.h"

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// MACROS ////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "ServersLoader"

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////// PUBLIC FUNCTIONS PROTOTYPES //////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum loaders_error_e loadServersXml_f(struct loaders_s *obj, struct context_s *ctx,
                                      struct xml_servers_s *xmlServers);
enum loaders_error_e unloadServersXml_f(struct loaders_s *obj, struct xml_servers_s *xmlServers);

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////////// CALLBACKS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

static void onServerStartCb(void *userData, const char **attrs);
static void onServerEndCb(void *userData);

static void onGeneralCb(void *userData, const char **attrs);
static void onInetCb(void *userData, const char **attrs);
static void onUnixCb(void *userData, const char **attrs);

static void onErrorCb(void *userData, int32_t errorCode, const char *errorStr);

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////// PUBLIC FUNCTIONS IMPLEMENTATION ///////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
enum loaders_error_e loadServersXml_f(struct loaders_s *obj, struct context_s *ctx,
                                      struct xml_servers_s *xmlServers)
{
    assert(obj && ctx && xmlServers);
    
    struct parser_s *parserObj = ctx->parserObj;
    struct input_s *input      = &ctx->input;
    
    xmlServers->reserved = ctx;
    
    Logd("Parsing file : \"%s/%s\"", input->resRootDir, input->serversConfig.xml);
    
    struct parser_tags_handler_s tagsHandlers[] = {
    	{ XML_TAG_SERVER,   onServerStartCb,  onServerEndCb,  NULL },
    	{ XML_TAG_GENERAL,  onGeneralCb,      NULL,           NULL },
    	{ XML_TAG_INET,     onInetCb,         NULL,           NULL },
    	{ XML_TAG_UNIX,     onUnixCb,         NULL,           NULL },
    	{ NULL,             NULL,             NULL,           NULL }
    };
    
    struct parser_params_s parserParams;
    snprintf(parserParams.path, sizeof(parserParams.path), "%s/%s",
                                input->resRootDir, input->serversConfig.xml);
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
enum loaders_error_e unloadServersXml_f(struct loaders_s *obj, struct xml_servers_s *xmlServers)
{
    assert(obj && xmlServers);
    
    uint8_t index;
    struct xml_server_s *server;
    
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
    
    free(xmlServers->servers);
    xmlServers->servers = NULL;
    
    xmlServers->reserved = NULL;
    
    return LOADERS_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////////// CALLBACKS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static void onServerStartCb(void *userData, const char **attrs)
{
    assert(userData);
    
    (void)attrs;
    
    struct xml_servers_s *xmlServers = (struct xml_servers_s*)userData;
    
    Logd("Adding server %u", (xmlServers->nbServers + 1));
    
    xmlServers->servers = realloc(xmlServers->servers,
                                  (xmlServers->nbServers + 1) * sizeof(struct xml_server_s));
    assert(xmlServers->servers);
    
    memset(&xmlServers->servers[xmlServers->nbServers], 0, sizeof(struct xml_server_s));
}

/*!
 *
 */
static void onServerEndCb(void *userData)
{
    assert(userData);
    
    struct xml_servers_s *xmlServers = (struct xml_servers_s*)userData;

    xmlServers->nbServers++;
    
    Logd("Server %u added", xmlServers->nbServers);
}

/*!
 *
 */
static void onGeneralCb(void *userData, const char **attrs)
{
    assert(userData);
    
    struct xml_servers_s *xmlServers = (struct xml_servers_s*)userData;
    struct xml_server_s *server      = &xmlServers->servers[xmlServers->nbServers];
    struct context_s *ctx            = (struct context_s*)xmlServers->reserved;
    struct parser_s *parserObj       = ctx->parserObj;
    
    struct parser_attr_handler_s attrHandlers[] = {
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
    
    struct xml_servers_s *xmlServers = (struct xml_servers_s*)userData;
    struct xml_server_s *server      = &xmlServers->servers[xmlServers->nbServers];
    struct context_s *ctx            = (struct context_s*)xmlServers->reserved;
    struct parser_s *parserObj       = ctx->parserObj;
    
    struct parser_attr_handler_s attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_HOST,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&server->host,
    	    .attrGetter.vector = parserObj->getString
        },
    	{
    	    .attrName          = XML_ATTR_SERVICE,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&server->service,
    	    .attrGetter.vector = parserObj->getString
        },
    	{
    	    .attrName          = XML_ATTR_PATH,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&server->path,
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
    
    struct xml_servers_s *xmlServers = (struct xml_servers_s*)userData;
    struct xml_server_s *server      = &xmlServers->servers[xmlServers->nbServers];
    struct context_s *ctx            = (struct context_s*)xmlServers->reserved;
    struct parser_s *parserObj       = ctx->parserObj;
    
    struct parser_attr_handler_s attrHandlers[] = {
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
