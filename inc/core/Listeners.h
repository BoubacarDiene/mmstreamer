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
* \file Listeners.h
* \author Boubacar DIENE
*/

#ifndef __LISTENERS_H__
#define __LISTENERS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// HEADERS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#include "core/Common.h"
#include "control/Control.h"

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////// TYPES DECLARATION ///////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum listeners_error_e;

struct listeners_params_s;
struct listeners_s;

/* -------------------------------------------------------------------------------------------- */
/* ///////////////////////////////////// PUBLIC FUNCTIONS ///////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

typedef enum listeners_error_e (*listeners_set_graphics_listeners_f)(struct listeners_s *obj);
typedef enum listeners_error_e (*listeners_unset_graphics_listeners_f)(struct listeners_s *obj);

typedef enum listeners_error_e (*listeners_set_videos_listeners_f)(struct listeners_s *obj);
typedef enum listeners_error_e (*listeners_unset_videos_listeners_f)(struct listeners_s *obj);

typedef enum listeners_error_e (*listeners_set_servers_listeners_f)(struct listeners_s *obj);
typedef enum listeners_error_e (*listeners_unset_servers_listeners_f)(struct listeners_s *obj);

typedef enum listeners_error_e (*listeners_set_clients_listeners_f)(struct listeners_s *obj);
typedef enum listeners_error_e (*listeners_unset_clients_listeners_f)(struct listeners_s *obj);

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// TYPES /////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum listeners_error_e {
    LISTENERS_ERROR_NONE,
    LISTENERS_ERROR_INIT,
    LISTENERS_ERROR_UNINIT,
    LISTENERS_ERROR_LISTENER
};

struct listeners_params_s {
    struct context_s *ctx;
    struct control_s *controlObj;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// MAIN CONTEXT /////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

struct listeners_s {
    listeners_set_graphics_listeners_f   setGraphicsListeners;
    listeners_unset_graphics_listeners_f unsetGraphicsListeners;
    
    listeners_set_videos_listeners_f     setVideosListeners;
    listeners_unset_videos_listeners_f   unsetVideosListeners;
    
    listeners_set_servers_listeners_f    setServersListeners;
    listeners_unset_servers_listeners_f  unsetServersListeners;
    
    listeners_set_clients_listeners_f    setClientsListeners;
    listeners_unset_clients_listeners_f  unsetClientsListeners;
    
    struct listeners_params_s params;
    void                      *pData;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// INITIALIZER //////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum listeners_error_e Listeners_Init(struct listeners_s **obj, struct listeners_params_s *params);
enum listeners_error_e Listeners_UnInit(struct listeners_s **obj);

#ifdef __cplusplus
}
#endif

#endif //__LISTENERS_H__
