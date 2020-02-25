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
* \file ClientsLoader.c
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
#define TAG "ClientsLoader"

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////// PUBLIC FUNCTIONS PROTOTYPES //////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum loaders_error_e loadClientsXml_f(struct loaders_s *obj, struct context_s *ctx,
                                      struct xml_clients_s *xmlClients);
enum loaders_error_e unloadClientsXml_f(struct loaders_s *obj, struct xml_clients_s *xmlClients);

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////////// CALLBACKS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

static void onClientStartCb(const void *userData, const char **attrs);
static void onClientEndCb(const void *userData);

static void onGeneralCb(const void *userData, const char **attrs);
static void onInetCb(const void *userData, const char **attrs);
static void onUnixCb(const void *userData, const char **attrs);

static void onErrorCb(const void *userData, int32_t errorCode, const char *errorStr);

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////// PUBLIC FUNCTIONS IMPLEMENTATION ///////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
enum loaders_error_e loadClientsXml_f(struct loaders_s *obj, struct context_s *ctx,
                                      struct xml_clients_s *xmlClients)
{
    ASSERT(obj && ctx && xmlClients);
    
    struct parser_s *parserObj = ctx->parserObj;
    struct input_s *input      = &ctx->input;
    
    xmlClients->reserved = ctx;
    
    Logd("Parsing file : \"%s/%s\"", input->resRootDir, input->clientsConfig.xml);
    
    struct parser_tags_handler_s tagsHandlers[] = {
    	{ XML_TAG_CLIENT,   onClientStartCb,  onClientEndCb,  NULL },
    	{ XML_TAG_GENERAL,  onGeneralCb,      NULL,           NULL },
    	{ XML_TAG_INET,     onInetCb,         NULL,           NULL },
    	{ XML_TAG_UNIX,     onUnixCb,         NULL,           NULL },
    	{ NULL,             NULL,             NULL,           NULL }
    };
    
    struct parser_params_s parserParams;
    snprintf(parserParams.path, sizeof(parserParams.path), "%s/%s",
                                input->resRootDir, input->clientsConfig.xml);
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
enum loaders_error_e unloadClientsXml_f(struct loaders_s *obj, struct xml_clients_s *xmlClients)
{
    ASSERT(obj && xmlClients);
    
    uint8_t index;
    struct xml_client_s *client;
    
    for (index = 0; index < xmlClients->nbClients; index++) {
        client = &xmlClients->clients[index];
        if (client->name) {
            free(client->name);
        }
        if (client->graphicsDest) {
            free(client->graphicsDest);
        }
        if (client->serverDest) {
            free(client->serverDest);
        }
        if (client->serverHost) {
            free(client->serverHost);
        }
        if (client->serverService) {
            free(client->serverService);
        }
        if (client->serverPath) {
            free(client->serverPath);
        }
        if (client->serverSocketName) {
            free(client->serverSocketName);
        }
    }
    
    free(xmlClients->clients);
    xmlClients->clients = NULL;
    
    xmlClients->reserved = NULL;
    
    return LOADERS_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////////// CALLBACKS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static void onClientStartCb(const void *userData, const char **attrs)
{
    ASSERT(userData);
    
    (void)attrs;
    
    struct xml_clients_s *xmlClients = (struct xml_clients_s*)userData;
    
    Logd("Adding client %u", (xmlClients->nbClients + 1));
    
    xmlClients->clients = realloc(xmlClients->clients,
                                  (size_t)(xmlClients->nbClients + 1) * sizeof(struct xml_client_s));
    ASSERT(xmlClients->clients);
    
    memset(&xmlClients->clients[xmlClients->nbClients], 0, sizeof(struct xml_client_s));
}

/*!
 *
 */
static void onClientEndCb(const void *userData)
{
    ASSERT(userData);
    
    struct xml_clients_s *xmlClients = (struct xml_clients_s*)userData;

    xmlClients->nbClients++;
    
    Logd("Client %u added", xmlClients->nbClients);
}

/*!
 *
 */
static void onGeneralCb(const void *userData, const char **attrs)
{
    ASSERT(userData);
    
    struct xml_clients_s *xmlClients = (struct xml_clients_s*)userData;
    struct xml_client_s *client      = &xmlClients->clients[xmlClients->nbClients];
    struct context_s *ctx            = (struct context_s*)xmlClients->reserved;
    struct parser_s *parserObj       = ctx->parserObj;
    
    struct parser_attr_handler_s attrHandlers[] = {
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
    
    if (client->graphicsDest && ((client->graphicsDest)[0] == '\0')) {
        free(client->graphicsDest);
        client->graphicsDest = NULL;
    }
    
    if (client->serverDest && ((client->serverDest)[0] == '\0')) {
        free(client->serverDest);
        client->serverDest = NULL;
    }
}

/*!
 *
 */
static void onInetCb(const void *userData, const char **attrs)
{
    ASSERT(userData);
    
    struct xml_clients_s *xmlClients = (struct xml_clients_s*)userData;
    struct xml_client_s *client      = &xmlClients->clients[xmlClients->nbClients];
    struct context_s *ctx            = (struct context_s*)xmlClients->reserved;
    struct parser_s *parserObj       = ctx->parserObj;
    
    struct parser_attr_handler_s attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_HOST,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&client->serverHost,
    	    .attrGetter.vector = parserObj->getString
        },
    	{
    	    .attrName          = XML_ATTR_SERVICE,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&client->serverService,
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
static void onUnixCb(const void *userData, const char **attrs)
{
    ASSERT(userData);
    
    struct xml_clients_s *xmlClients = (struct xml_clients_s*)userData;
    struct xml_client_s *client      = &xmlClients->clients[xmlClients->nbClients];
    struct context_s *ctx            = (struct context_s*)xmlClients->reserved;
    struct parser_s *parserObj       = ctx->parserObj;
    
    struct parser_attr_handler_s attrHandlers[] = {
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
static void onErrorCb(const void *userData, int32_t errorCode, const char *errorStr)
{
    (void)userData;
    
    Loge("Parsing error - errorCode = %d / errorStr = \"%s\"", errorCode, errorStr);
}
