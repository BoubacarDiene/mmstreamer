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
* \file   ClientsLoader.c
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
#define TAG "CLIENTS-LOADER"

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                         PROTOTYPES                                           */
/* -------------------------------------------------------------------------------------------- */

LOADERS_ERROR_E loadClientsXml_f  (LOADERS_S *obj, CONTEXT_S *ctx, XML_CLIENTS_S *xmlClients);
LOADERS_ERROR_E unloadClientsXml_f(LOADERS_S *obj, XML_CLIENTS_S *xmlClients);

static void onClientStartCb(void *userData, const char **attrs);
static void onClientEndCb  (void *userData);

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
LOADERS_ERROR_E loadClientsXml_f(LOADERS_S *obj, CONTEXT_S *ctx, XML_CLIENTS_S *xmlClients)
{
    assert(obj && ctx && xmlClients);
    
    PARSER_S *parserObj  = ctx->modules.parserObj;
    INPUT_S *input       = &ctx->input;
    
    xmlClients->reserved = ctx;
    
    Logd("Parsing file : \"%s/%s\"", input->xmlRootDir, input->clientsXml);
    
    PARSER_TAGS_HANDLER_S tagsHandlers[] = {
    	{ XML_TAG_CLIENT,   onClientStartCb,  onClientEndCb,  NULL },
    	{ XML_TAG_GENERAL,  onGeneralCb,      NULL,           NULL },
    	{ XML_TAG_INET,     onInetCb,         NULL,           NULL },
    	{ XML_TAG_UNIX,     onUnixCb,         NULL,           NULL },
    	{ NULL,             NULL,             NULL,           NULL }
    };
    
    PARSER_PARAMS_S parserParams;
    snprintf(parserParams.path, sizeof(parserParams.path), "%s/%s", input->xmlRootDir, input->clientsXml);
    parserParams.encoding     = PARSER_ENCODING_UTF_8;
    parserParams.tagsHandlers = tagsHandlers;
    parserParams.onErrorCb    = onErrorCb;
    parserParams.userData     = xmlClients;
    
    if (parserObj->parse(parserObj, &parserParams) != PARSER_ERROR_NONE) {
        Loge("Failed to parse file");
        xmlClients->reserved = NULL;
        return LOADERS_ERROR_XML;
    }
    
    return LOADERS_ERROR_NONE;
}

/*!
 *
 */
LOADERS_ERROR_E unloadClientsXml_f(LOADERS_S *obj, XML_CLIENTS_S *xmlClients)
{
    assert(obj && xmlClients);
    
    uint8_t index;
    XML_CLIENT_S *client;
    
    for (index = 0; index < xmlClients->nbClients; index++) {
        client = &xmlClients->clients[index];
        if (client->name) {
            free(client->name);
            client->name = NULL;
        }
        if (client->graphicsDest) {
            free(client->graphicsDest);
            client->graphicsDest = NULL;
        }
        if (client->serverDest) {
            free(client->serverDest);
            client->serverDest = NULL;
        }
        if (client->serverHost) {
            free(client->serverHost);
            client->serverHost = NULL;
        }
        if (client->serverService) {
            free(client->serverService);
            client->serverService = NULL;
        }
        if (client->serverPath) {
            free(client->serverPath);
            client->serverPath = NULL;
        }
        if (client->serverSocketName) {
            free(client->serverSocketName);
            client->serverSocketName = NULL;
        }
    }
    
    xmlClients->reserved = NULL;
    
    return LOADERS_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/*                                           CALLBACKS                                          */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static void onClientStartCb(void *userData, const char **attrs)
{
    assert(userData);
    
    (void)attrs;
    
    XML_CLIENTS_S *xmlClients = (XML_CLIENTS_S*)userData;
    
    Logd("Adding client %u", (xmlClients->nbClients + 1));
    
    xmlClients->clients = realloc(xmlClients->clients, (xmlClients->nbClients + 1) * sizeof(XML_CLIENT_S));
    assert(xmlClients->clients);
    
    memset(&xmlClients->clients[xmlClients->nbClients], '\0', sizeof(XML_CLIENT_S));
}

/*!
 *
 */
static void onClientEndCb(void *userData)
{
    assert(userData);
    
    XML_CLIENTS_S *xmlClients = (XML_CLIENTS_S*)userData;

    xmlClients->nbClients++;
    
    Logd("Client %u added", xmlClients->nbClients);
}

/*!
 *
 */
static void onGeneralCb(void *userData, const char **attrs)
{
    assert(userData);
    
    XML_CLIENTS_S *xmlClients = (XML_CLIENTS_S*)userData;
    XML_CLIENT_S *client      = &xmlClients->clients[xmlClients->nbClients];
    CONTEXT_S *ctx            = (CONTEXT_S*)xmlClients->reserved;
    PARSER_S *parserObj       = ctx->modules.parserObj;
    
    PARSER_ATTR_HANDLER_S attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_NAME,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&client->name,
    	    .attrGetter.vector = parserObj->getString
        },
    	{
    	    .attrName          = XML_ATTR_TYPE,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&client->type,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_LINK,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&client->link,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_MODE,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&client->mode,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_PRIORITY,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&client->priority,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_GFX_DEST,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&client->graphicsDest,
    	    .attrGetter.vector = parserObj->getString
        },
    	{
    	    .attrName          = XML_ATTR_SERVER_DEST,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&client->serverDest,
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
    
    if (client->graphicsDest && (strlen(client->graphicsDest) == 0)) {
        free(client->graphicsDest);
        client->graphicsDest = NULL;
    }
    
    if (client->serverDest && (strlen(client->serverDest) == 0)) {
        free(client->serverDest);
        client->serverDest = NULL;
    }
}

/*!
 *
 */
static void onInetCb(void *userData, const char **attrs)
{
    assert(userData);
    
    XML_CLIENTS_S *xmlClients = (XML_CLIENTS_S*)userData;
    XML_CLIENT_S *client      = &xmlClients->clients[xmlClients->nbClients];
    CONTEXT_S *ctx            = (CONTEXT_S*)xmlClients->reserved;
    PARSER_S *parserObj       = ctx->modules.parserObj;
    
    PARSER_ATTR_HANDLER_S attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_HOST,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&client->serverHost,
    	    .attrGetter.vector = parserObj->getString
        },
    	{
    	    .attrName          = XML_ATTR_SERVICE,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void*)&client->serverService,
    	    .attrGetter.vector = parserObj->getString
        },
    	{
    	    .attrName          = XML_ATTR_PATH,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&client->serverPath,
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
    
    XML_CLIENTS_S *xmlClients = (XML_CLIENTS_S*)userData;
    XML_CLIENT_S *client      = &xmlClients->clients[xmlClients->nbClients];
    CONTEXT_S *ctx            = (CONTEXT_S*)xmlClients->reserved;
    PARSER_S *parserObj       = ctx->modules.parserObj;
    
    PARSER_ATTR_HANDLER_S attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_SERVER_SOCKET_NAME,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&client->serverSocketName,
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
