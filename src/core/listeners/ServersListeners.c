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
* \file ServersListeners.c
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
#define TAG "ServersListeners"

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////// PUBLIC FUNCTIONS PROTOTYPES //////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum listeners_error_e setServersListeners_f(struct listeners_s *obj);
enum listeners_error_e unsetServersListeners_f(struct listeners_s *obj);

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////////// CALLBACKS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

static void onClientStateChangedCb(struct server_params_s *params, struct link_s *client,
                                   enum state_e state, void *userData);

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////// PUBLIC FUNCTIONS IMPLEMENTATION ///////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
enum listeners_error_e setServersListeners_f(struct listeners_s *obj)
{
    assert(obj);
    
    struct listeners_params_s *listenersParams = &obj->params;
    struct servers_infos_s *serversInfos       = &listenersParams->ctx->params.serversInfos;
    struct server_params_s *serverParams       = NULL;
    
    uint8_t index;
    for (index = 0; index < serversInfos->nbServers; index++) {
        serverParams = &(serversInfos->serverInfos[index])->serverParams;

        serverParams->onClientStateChangedCb = onClientStateChangedCb;
        serverParams->userData               = listenersParams;
    }
    
    return LISTENERS_ERROR_NONE;
}

/*!
 *
 */
enum listeners_error_e unsetServersListeners_f(struct listeners_s *obj)
{
    assert(obj);
    
    struct listeners_params_s *listenersParams = &obj->params;
    struct servers_infos_s *serversInfos       = &listenersParams->ctx->params.serversInfos;
    struct server_params_s *serverParams       = NULL;
    
    uint8_t index;
    for (index = 0; index < serversInfos->nbServers; index++) {
        serverParams           = &(serversInfos->serverInfos[index])->serverParams;
        serverParams->userData = NULL;
    }
    
    return LISTENERS_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////////// CALLBACKS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static void onClientStateChangedCb(struct server_params_s *params, struct link_s *client,
                                   enum state_e state, void *userData)
{
    assert(params && client && userData);
    
    Logd("Server-%s : Client with id %u %s",
        params->name,
        client->id,
        (state == STATE_CONNECTED ? "CONNECTED" : "DISCONNECTED"));
}
