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

#include "specific/Listeners.h"

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
    
    if (!input->graphicsEnabled && !input->serverEnabled) {
        Logw("At least server or graphics module must be enabled");
    }
    else {
        CLIENTS_INFOS_S *clientsInfos = &pData->ctx->params.clientsInfos;
        uint8_t index;
        
        for (index = 0; index < clientsInfos->nbClients; index++) {
            clientsInfos->clientParams[index]->onDataReceivedCb = onClientDataCb;
            clientsInfos->clientParams[index]->onLinkBrokenCb   = onClientLinkCb;
            clientsInfos->clientParams[index]->userData         = pData;
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
    
    LISTENERS_PDATA_S *pData = (LISTENERS_PDATA_S*)(obj->pData);
    
    uint8_t index;
    for (index = 0; index < pData->ctx->params.clientsInfos.nbClients; index++) {
        pData->ctx->params.clientsInfos.clientParams[index]->userData = NULL;
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
    
    LISTENERS_PDATA_S *pData      = (LISTENERS_PDATA_S*)userData;
    INPUT_S *input                = &pData->ctx->input;
    GRAPHICS_S *graphicsObj       = pData->ctx->modules.graphicsObj;
    SERVER_S *serverObj           = pData->ctx->modules.serverObj;
    CLIENTS_INFOS_S *clientsInfos = &pData->ctx->params.clientsInfos;
    
    char **graphicsDest           = clientsInfos->graphicsDest;
    int8_t *graphicsIndex         = clientsInfos->graphicsIndex;
    
    char **serverDest             = clientsInfos->serverDest;
    int8_t *serverIndex           = clientsInfos->serverIndex;
    
    uint8_t i, j;
    for (i = 0; i < clientsInfos->nbClients; i++) {
        if (strcmp(params->name, clientsInfos->clientParams[i]->name) == 0) {
            Logd("Client \"%s\" found at index \"%u\"", params->name, i);
            break;
        }
    }
    
    pData->buffer.data   = buffer->data;
    pData->buffer.length = buffer->length;
    
    if (input->graphicsEnabled && graphicsObj && graphicsDest[i]) {
        GRAPHICS_INFOS_S *graphicsInfos = &pData->ctx->params.graphicsInfos;
            
        if (graphicsIndex[i] == -1) {
            for (j = 0; j < graphicsInfos->nbGfxElements; j++) {
                if (strcmp(graphicsInfos->gfxElements[j]->name, graphicsDest[i]) == 0) {
                    Logd("Element \"%s\" found at index \"%u\"", graphicsDest[i], j);
                    break;
                }
            }
            if (j < graphicsInfos->nbGfxElements) {
                graphicsIndex[i] = j;
            }
            else {
                Loge("Element \"%s\" does not exist", graphicsDest[i]);
            }
        }
        
        if (graphicsIndex[i] != -1) {
            graphicsObj->setData(graphicsObj, graphicsDest[i], &pData->buffer);
        }
    }
    
    if (input->serverEnabled && serverObj && serverDest) {
        SERVERS_INFOS_S *serversInfos = &pData->ctx->params.serversInfos;
        if (serverIndex[i] == -1) {
            for (j = 0; j < serversInfos->nbServers; j++) {
                if (strcmp(serversInfos->serverParams[j]->name, serverDest[i]) == 0) {
                    Logd("Server \"%s\" found at index \"%u\"", serverDest[i], j);
                    break;
                }
            }
            if (j < serversInfos->nbServers) {
                serverIndex[i] = j;
            }
            else {
                Loge("Server \"%s\" does not exist", serverDest[i]);
            }
        }
        
        if (serverIndex[i] != -1) {
            serverObj->sendData(serverObj, serversInfos->serverParams[serverIndex[i]], &pData->buffer);
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
