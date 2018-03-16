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
* \file ClientsListeners.c
* \brief TODO
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// HEADERS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#include "core/Listeners.h"

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// MACROS ////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "ClientsListeners"

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// TYPES /////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

struct clients_listeners_private_data_s {
    struct buffer_s           buffer;
    struct listeners_params_s *listenersParams;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////// PUBLIC FUNCTIONS PROTOTYPES //////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum listeners_error_e setClientsListeners_f(struct listeners_s *obj);
enum listeners_error_e unsetClientsListeners_f(struct listeners_s *obj);

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////////// CALLBACKS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

static void onClientDataCb(struct client_params_s *params, struct buffer_s *buffer,
                           void *userData);
static void onClientLinkCb(struct client_params_s *params, void *userData);

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////// PUBLIC FUNCTIONS IMPLEMENTATION ///////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
enum listeners_error_e setClientsListeners_f(struct listeners_s *obj)
{
    assert(obj);
    
    struct listeners_params_s *listenersParams = &obj->params;
    struct input_s *input                      = &listenersParams->ctx->input;
    
    if (!input->graphicsConfig.enable && !input->serversConfig.enable) {
        Logw("At least servers or graphics module must be enabled");
    }
    else {
        struct clients_infos_s *clientsInfos = &listenersParams->ctx->params.clientsInfos;
        struct client_params_s *clientParams = NULL;
        uint8_t index;
        
        struct clients_listeners_private_data_s *pData;
        assert((pData = calloc(1, sizeof(struct clients_listeners_private_data_s))));
        pData->listenersParams = listenersParams;
        
        for (index = 0; index < clientsInfos->nbClients; index++) {
            clientParams = &(clientsInfos->clientInfos[index])->clientParams;

            clientParams->onDataReceivedCb = onClientDataCb;
            clientParams->onLinkBrokenCb   = onClientLinkCb;
            clientParams->userData         = pData;
        }
    }
    
    return LISTENERS_ERROR_NONE;
}

/*!
 *
 */
enum listeners_error_e unsetClientsListeners_f(struct listeners_s *obj)
{
    assert(obj);
    
    struct listeners_params_s *listenersParams     = &obj->params;
    struct clients_infos_s *clientsInfos           = &listenersParams->ctx->params.clientsInfos;
    struct client_params_s *clientParams           = NULL;
    struct clients_listeners_private_data_s *pData = NULL;
    
    uint8_t index;
    for (index = 0; index < clientsInfos->nbClients; index++) {
        clientParams = &(clientsInfos->clientInfos[index])->clientParams;
        pData = (struct clients_listeners_private_data_s*)(clientParams->userData);
        clientParams->userData = NULL;
    }
    
    if (pData) {
        free(pData);
    }
    
    return LISTENERS_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////////// CALLBACKS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static void onClientDataCb(struct client_params_s *params,
                           struct buffer_s *buffer, void *userData)
{
    assert(params && buffer && userData);
    
    struct clients_listeners_private_data_s *pData = (struct clients_listeners_private_data_s*)userData;
    struct context_s *ctx                          = pData->listenersParams->ctx;
    struct input_s *input                          = &ctx->input;
    struct graphics_s *graphicsObj                 = ctx->modules.graphicsObj;
    struct server_s *serverObj                     = ctx->modules.serverObj;
    struct graphics_infos_s *graphicsInfos         = &ctx->params.graphicsInfos;
    struct servers_infos_s *serversInfos           = &ctx->params.serversInfos;
    struct server_infos_s *serverInfos             = NULL;
    struct clients_infos_s *clientsInfos           = &ctx->params.clientsInfos;
    struct client_infos_s *clientInfos             = NULL;
    
    uint8_t i, j;
    for (i = 0; i < clientsInfos->nbClients; i++) {
        clientInfos = clientsInfos->clientInfos[i];

        if (strcmp(params->name, clientInfos->clientParams.name) == 0) {
            Logd("Client \"%s\" found at index \"%u\"", params->name, i);
            break;
        }
    }

    if (i == clientsInfos->nbClients) {
        Loge("Client \"%s\" not found", params->name);
        return;
    }

    pData->buffer.data   = buffer->data;
    pData->buffer.length = buffer->length;

    if (graphicsObj && clientInfos->graphicsDest) {
        if (clientInfos->graphicsIndex == -1) {
            for (j = 0; j < graphicsInfos->nbGfxElements; j++) {
                if (strcmp(graphicsInfos->gfxElements[j]->name, clientInfos->graphicsDest) == 0) {
                    Logd("Element \"%s\" found at index \"%u\"", clientInfos->graphicsDest, j);
                    break;
                }
            }
            if (j < graphicsInfos->nbGfxElements) {
                clientInfos->graphicsIndex = j;
            }
            else {
                Loge("Element \"%s\" does not exist", clientInfos->graphicsDest);
            }
        }
        
        if ((graphicsInfos->state == MODULE_STATE_STARTED) && (clientInfos->graphicsIndex != -1)) {
            graphicsObj->setData(graphicsObj, clientInfos->graphicsDest, &pData->buffer);
        }
    }
    
    if (serverObj && clientInfos->serverDest) {
        if (clientInfos->serverIndex == -1) {
            for (j = 0; j < serversInfos->nbServers; j++) {
                serverInfos = serversInfos->serverInfos[j];

                if (strcmp(serverInfos->serverParams.name, clientInfos->serverDest) == 0) {
                    Logd("Server \"%s\" found at index \"%u\"", clientInfos->serverDest, j);
                    break;
                }
            }

            if (j == serversInfos->nbServers) {
                Loge("Server \"%s\" does not exist", clientInfos->serverDest);
                return;
            }

            clientInfos->serverIndex = j;
        }
        else {
            serverInfos = serversInfos->serverInfos[clientInfos->serverIndex];
        }
        
        if (serverInfos->state == MODULE_STATE_STARTED) {
            serverObj->sendData(serverObj, &serverInfos->serverParams, &pData->buffer);
        }
    }
}

/*!
 *
 */
static void onClientLinkCb(struct client_params_s *params, void *userData)
{
    assert(params && userData);
    
    Logd("Client link broken - name : \"%s\"", params->name);
}
