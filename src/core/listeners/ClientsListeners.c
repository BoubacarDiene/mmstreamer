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
* \file   ClientsListeners.c
* \brief  TODO
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include "core/Listeners.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                            */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "CLIENTS-LISTENERS"

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                         PROTOTYPES                                           */
/* -------------------------------------------------------------------------------------------- */

LISTENERS_ERROR_E setClientsListeners_f  (LISTENERS_S *obj);
LISTENERS_ERROR_E unsetClientsListeners_f(LISTENERS_S *obj);

static void onClientDataCb(CLIENT_PARAMS_S *params, BUFFER_S *buffer, void *userData);
static void onClientLinkCb(CLIENT_PARAMS_S *params, void *userData);

/* -------------------------------------------------------------------------------------------- */
/*                                          VARIABLES                                           */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                          FUNCTIONS                                           */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
LISTENERS_ERROR_E setClientsListeners_f(LISTENERS_S *obj)
{
    assert(obj && obj->pData);
    
    LISTENERS_PDATA_S *pData = (LISTENERS_PDATA_S*)(obj->pData);
    INPUT_S *input           = &pData->ctx->input;
    
    if (!input->graphicsConfig.enable && !input->serversConfig.enable) {
        Logw("At least servers or graphics module must be enabled");
    }
    else {
        CLIENTS_INFOS_S *clientsInfos = &pData->ctx->params.clientsInfos;
        CLIENT_PARAMS_S *clientParams = NULL;
        uint8_t index;
        
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
LISTENERS_ERROR_E unsetClientsListeners_f(LISTENERS_S *obj)
{
    assert(obj && obj->pData);
    
    LISTENERS_PDATA_S *pData      = (LISTENERS_PDATA_S*)(obj->pData);
    CLIENTS_INFOS_S *clientsInfos = &pData->ctx->params.clientsInfos;
    
    uint8_t index;
    for (index = 0; index < clientsInfos->nbClients; index++) {
        (clientsInfos->clientInfos[index])->clientParams.userData = NULL;
    }
    
    return LISTENERS_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/*                                          CALLBACKS                                           */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static void onClientDataCb(CLIENT_PARAMS_S *params, BUFFER_S *buffer, void *userData)
{
    assert(params && buffer && userData);
    
    LISTENERS_PDATA_S *pData        = (LISTENERS_PDATA_S*)userData;
    INPUT_S *input                  = &pData->ctx->input;
    GRAPHICS_S *graphicsObj         = pData->ctx->modules.graphicsObj;
    SERVER_S *serverObj             = pData->ctx->modules.serverObj;
    GRAPHICS_INFOS_S *graphicsInfos = &pData->ctx->params.graphicsInfos;
    SERVERS_INFOS_S *serversInfos   = &pData->ctx->params.serversInfos;
    SERVER_INFOS_S *serverInfos     = NULL;
    CLIENTS_INFOS_S *clientsInfos   = &pData->ctx->params.clientsInfos;
    CLIENT_INFOS_S *clientInfos     = NULL;
    
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
static void onClientLinkCb(CLIENT_PARAMS_S *params, void *userData)
{
    assert(params && userData);
    
    Logd("Client link broken - name : \"%s\"", params->name);
}
