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
* \file   ServersListeners.c
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
#define TAG "SERVERS-LISTENERS"

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                         PROTOTYPES                                           */
/* -------------------------------------------------------------------------------------------- */

LISTENERS_ERROR_E setServersListeners_f  (LISTENERS_S *obj);
LISTENERS_ERROR_E unsetServersListeners_f(LISTENERS_S *obj);

static void onClientStateChangedCb(SERVER_PARAMS_S *params, LINK_S *client, STATE_E state, void *userData);

/* -------------------------------------------------------------------------------------------- */
/*                                          VARIABLES                                           */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                          FUNCTIONS                                           */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
LISTENERS_ERROR_E setServersListeners_f(LISTENERS_S *obj)
{
    assert(obj && obj->pData);
    
    LISTENERS_PDATA_S *pData      = (LISTENERS_PDATA_S*)(obj->pData);
    SERVERS_INFOS_S *serversInfos = &pData->ctx->params.serversInfos;
    SERVER_PARAMS_S *serverParams = NULL;
    
    uint8_t index;
    for (index = 0; index < serversInfos->nbServers; index++) {
        serverParams = &(serversInfos->serverInfos[index])->serverParams;

        serverParams->onClientStateChangedCb = onClientStateChangedCb;
        serverParams->userData               = pData;
    }
    
    return LISTENERS_ERROR_NONE;
}

/*!
 *
 */
LISTENERS_ERROR_E unsetServersListeners_f(LISTENERS_S *obj)
{
    assert(obj && obj->pData);
    
    LISTENERS_PDATA_S *pData      = (LISTENERS_PDATA_S*)(obj->pData);
    SERVERS_INFOS_S *serversInfos = &pData->ctx->params.serversInfos;
    SERVER_PARAMS_S *serverParams = NULL;
    
    uint8_t index;
    for (index = 0; index < serversInfos->nbServers; index++) {
        serverParams           = &(serversInfos->serverInfos[index])->serverParams;
        serverParams->userData = NULL;
    }
    
    return LISTENERS_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/*                                          CALLBACKS                                           */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static void onClientStateChangedCb(SERVER_PARAMS_S *params, LINK_S *client, STATE_E state, void *userData)
{
    assert(params && client && userData);
    
    Logd("Server-%s : Client with id %u %s",
        params->name,
        client->id,
        (state == STATE_CONNECTED ? "CONNECTED" : "DISCONNECTED"));
}
