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
* \file Server.h
* \author Boubacar DIENE
*/

#ifndef __SERVER_H__
#define __SERVER_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// HEADERS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#include "network/LinkHelper.h"

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////// TYPES DECLARATION ///////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum server_accept_mode_e;
enum server_error_e;

struct server_params_s;
struct server_s;

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////////// CALLBACKS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

typedef void (*server_on_client_state_changed_cb)(struct server_params_s *params,
                                                  struct link_s *client,
                                                  enum state_e state, void *userData);

/* -------------------------------------------------------------------------------------------- */
/* ///////////////////////////////////// PUBLIC FUNCTIONS ///////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

typedef enum server_error_e (*server_start_f)(struct server_s *obj,
                                              struct server_params_s *params);
typedef enum server_error_e (*server_stop_f)(struct server_s *obj,
                                             struct server_params_s *params);

/** addReceiver   : Client added to receivers' list => it starts receiving data from server
 * removeReceiver: Remove client from receivers' list but still connected */
typedef enum server_error_e (*server_add_receiver_f)(struct server_s *obj,
                                                     struct server_params_s *params,
                                                     struct link_s *client);
typedef enum server_error_e (*server_remove_receiver_f)(struct server_s *obj,
                                                        struct server_params_s *params,
                                                        struct link_s *client);

typedef enum server_error_e (*server_suspend_sender_f)(struct server_s *obj,
                                                       struct server_params_s *params);
typedef enum server_error_e (*server_resume_sender_f)(struct server_s *obj,
                                                      struct server_params_s *params);

/* Close link with client and release its resources */
typedef enum server_error_e (*server_disconnect_client_f)(struct server_s *obj,
                                                          struct server_params_s *params,
                                                          struct link_s *client);

typedef enum server_error_e (*server_send_data_f)(struct server_s *obj,
                                                  struct server_params_s *params,
                                                  struct buffer_s *buffer);

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// TYPES /////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum server_accept_mode_e {
    SERVER_ACCEPT_MODE_AUTOMATIC, /** Dispatch data to all connected clients i.e no need to call
                                      addReceiver() */
    SERVER_ACCEPT_MODE_MANUAL,    /* Wait until client is added to receivers' list */
};

enum server_error_e {
    SERVER_ERROR_NONE,
    SERVER_ERROR_INIT,
    SERVER_ERROR_UNINIT,
    SERVER_ERROR_LOCK,
    SERVER_ERROR_LIST,
    SERVER_ERROR_START,
    SERVER_ERROR_STOP,
    SERVER_ERROR_PARAMS
};

struct server_params_s {
    char                              name[MAX_NAME_SIZE];
    
    enum stream_type_e                type;
    enum link_type_e                  link;
    enum link_mode_e                  mode;
    enum server_accept_mode_e         acceptMode;
    
    char                              mime[MAX_MIME_SIZE];
    
    union {
        struct recipient_s            server;                          /* Connection-oriented */
        char                          serverSocketName[MAX_NAME_SIZE]; /* Connectionless */
    } recipient;
    
    enum priority_e                   priority;
    uint32_t                          maxClients;
    size_t                            maxBufferSize;
    
    server_on_client_state_changed_cb onClientStateChangedCb;
    
    void                              *userData;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// MAIN CONTEXT /////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

struct server_s {
    server_start_f             start;
    server_stop_f              stop;

    server_add_receiver_f      addReceiver;
    server_remove_receiver_f   removeReceiver;

    server_suspend_sender_f    suspendSender;
    server_resume_sender_f     resumeSender;

    server_disconnect_client_f disconnectClient;

    server_send_data_f         sendData;

    void *pData;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// INITIALIZER //////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum server_error_e Server_Init(struct server_s **obj);
enum server_error_e Server_UnInit(struct server_s **obj);

#ifdef __cplusplus
}
#endif

#endif //__SERVER_H__
