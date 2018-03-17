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
* \file Client.h
* \author Boubacar DIENE
*/

#ifndef __CLIENT_H__
#define __CLIENT_H__

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

enum client_error_e;

struct client_params_s;
struct client_s;

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////////// CALLBACKS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

typedef void (*client_on_data_received_cb)(struct client_params_s *params,
                                           struct buffer_s *buffer, void *userData);
typedef void (*client_on_link_broken_cb)(struct client_params_s *params, void *userData);

/* -------------------------------------------------------------------------------------------- */
/* ///////////////////////////////////// PUBLIC FUNCTIONS ///////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

typedef enum client_error_e (*client_start_f)(struct client_s *obj,
                                              struct client_params_s *params);
typedef enum client_error_e (*client_stop_f)(struct client_s *obj,
                                             struct client_params_s *params);

typedef enum client_error_e (*client_send_data_f)(struct client_s *obj,
                                                  struct client_params_s *params,
                                                  struct buffer_s *buffer);

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// TYPES /////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum client_error_e {
    CLIENT_ERROR_NONE,
    CLIENT_ERROR_INIT,
    CLIENT_ERROR_UNINIT,
    CLIENT_ERROR_LOCK,
    CLIENT_ERROR_START,
    CLIENT_ERROR_STOP,
    CLIENT_ERROR_PARAMS,
    CLIENT_ERROR_LIST
};

struct client_params_s {
    char                       name[MAX_NAME_SIZE];
    
    enum stream_type_e         type;
    enum link_type_e           link;
    enum link_mode_e           mode;
    
    union {
        struct recipient_s     server;                          // Connection-oriented mode
        char                   serverSocketName[MAX_NAME_SIZE]; // Connectionless mode
    } recipient;
    
    enum priority_e            priority;
    size_t                     maxBufferSize;
    
    client_on_data_received_cb onDataReceivedCb;
    client_on_link_broken_cb   onLinkBrokenCb;
    
    void                       *userData;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// MAIN CONTEXT /////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

struct client_s {
    client_start_f     start;
    client_stop_f      stop;

    client_send_data_f sendData;
    
    void *pData;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// INITIALIZER //////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum client_error_e Client_Init(struct client_s **obj);
enum client_error_e Client_UnInit(struct client_s **obj);

#ifdef __cplusplus
}
#endif

#endif //__CLIENT_H__
