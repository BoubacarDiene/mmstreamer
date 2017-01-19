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
* \file   Server.h
* \author Boubacar DIENE
*/

#ifndef __SERVER_H__
#define __SERVER_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include "network/LinkHelper.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                             */
/* -------------------------------------------------------------------------------------------- */
                         
/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

typedef enum SERVER_ACCEPT_MODE_E SERVER_ACCEPT_MODE_E;
typedef enum SERVER_ERROR_E       SERVER_ERROR_E;

typedef struct SERVER_PARAMS_S    SERVER_PARAMS_S;
typedef struct SERVER_S           SERVER_S;

typedef void (*ON_CLIENT_STATE_CHANGED_CB)(SERVER_PARAMS_S *params, LINK_S *client, STATE_E state, void *userData);

typedef SERVER_ERROR_E (*SERVER_START_F)(SERVER_S *obj, SERVER_PARAMS_S *params);
typedef SERVER_ERROR_E (*SERVER_STOP_F )(SERVER_S *obj, SERVER_PARAMS_S *params);

typedef SERVER_ERROR_E (*SERVER_ADD_RECEIVER_F)(SERVER_S *obj, SERVER_PARAMS_S *params, LINK_S *client);
typedef SERVER_ERROR_E (*SERVER_REMOVE_RECEIVER_F)(SERVER_S *obj, SERVER_PARAMS_S *params, LINK_S *client);

typedef SERVER_ERROR_E (*SERVER_SUSPEND_SENDER_F)(SERVER_S *obj, SERVER_PARAMS_S *params);
typedef SERVER_ERROR_E (*SERVER_RESUME_SSENDER_F)(SERVER_S *obj, SERVER_PARAMS_S *params);

typedef SERVER_ERROR_E (*SERVER_DISCONNECT_CLIENT_F)(SERVER_S *obj, SERVER_PARAMS_S *params, LINK_S *client);

typedef SERVER_ERROR_E (*SERVER_SEND_DATA_F)(SERVER_S *obj, SERVER_PARAMS_S *params, BUFFER_S *buffer);

enum SERVER_ACCEPT_MODE_E {
    SERVER_ACCEPT_MODE_AUTOMATIC, /* Dispatch data to all connected clients i.e no need to call addReceiver() */
    SERVER_ACCEPT_MODE_MANUAL,    /* Wait until client is added to receivers' list */
};

enum SERVER_ERROR_E {
    SERVER_ERROR_NONE,
    SERVER_ERROR_INIT,
    SERVER_ERROR_UNINIT,
    SERVER_ERROR_LOCK,
    SERVER_ERROR_LIST,
    SERVER_ERROR_START,
    SERVER_ERROR_STOP,
    SERVER_ERROR_PARAMS
};

struct SERVER_PARAMS_S {
    char                       name[MAX_NAME_SIZE];
    
    STREAM_TYPE_E              type;
    LINK_TYPE_E                link;
    LINK_MODE_E                mode;
    SERVER_ACCEPT_MODE_E       acceptMode;
    
    char                       mime[MAX_MIME_SIZE];
    
    union {
        RECIPIENT_S            server;                          /* Used in connection-oriented mode */
        char                   serverSocketName[MAX_NAME_SIZE]; /* Used in connectionless mode */
    } recipient;
    
    PRIORITY_E                 priority;
    uint32_t                   maxClients;
    size_t                     maxBufferSize;
    
    ON_CLIENT_STATE_CHANGED_CB onClientStateChangedCb;
    
    void                       *userData;
};

struct SERVER_S {
    SERVER_START_F             start;
    SERVER_STOP_F              stop;
    
    SERVER_ADD_RECEIVER_F      addReceiver;    /* Client added to receivers' list => it starts receiving data from server */
    SERVER_REMOVE_RECEIVER_F   removeReceiver; // Remove client from receivers' list but still connected
    
    SERVER_SUSPEND_SENDER_F    suspendSender;
    SERVER_SUSPEND_SENDER_F    resumeSender;

    SERVER_DISCONNECT_CLIENT_F disconnectClient; // Close link with client and release its resources
    
    SERVER_SEND_DATA_F         sendData;
    
    void                       *pData;
};

/* -------------------------------------------------------------------------------------------- */
/*                                      PUBLIC FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

SERVER_ERROR_E Server_Init  (SERVER_S **obj);
SERVER_ERROR_E Server_UnInit(SERVER_S **obj);

#ifdef __cplusplus
}
#endif

#endif //__SERVER_H__
