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
* \file   Client.h
* \author Boubacar DIENE
*/

#ifndef __CLIENT_H__
#define __CLIENT_H__

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

typedef enum   CLIENT_ERROR_E  CLIENT_ERROR_E;

typedef struct CLIENT_PARAMS_S CLIENT_PARAMS_S;
typedef struct CLIENT_S        CLIENT_S;

typedef void (*ON_DATA_RECEIVED_CB)(CLIENT_PARAMS_S *params, BUFFER_S *buffer, void *userData);
typedef void (*ON_LINK_BROKEN_CB  )(CLIENT_PARAMS_S *params, void *userData);

typedef CLIENT_ERROR_E (*CLIENT_START_F)(CLIENT_S *obj, CLIENT_PARAMS_S *params);
typedef CLIENT_ERROR_E (*CLIENT_STOP_F )(CLIENT_S *obj, CLIENT_PARAMS_S *params);

typedef CLIENT_ERROR_E (*CLIENT_SEND_DATA_F)(CLIENT_S *obj, CLIENT_PARAMS_S *params, BUFFER_S *buffer);

enum CLIENT_ERROR_E {
    CLIENT_ERROR_NONE,
    CLIENT_ERROR_INIT,
    CLIENT_ERROR_UNINIT,
    CLIENT_ERROR_LOCK,
    CLIENT_ERROR_START,
    CLIENT_ERROR_STOP,
    CLIENT_ERROR_PARAMS,
    CLIENT_ERROR_LIST
};

struct CLIENT_PARAMS_S {
    char                name[MAX_NAME_SIZE];
    
    STREAM_TYPE_E       type;
    LINK_TYPE_E         link;
    LINK_MODE_E         mode;
    
    union {
        RECIPIENT_S     server;                          /* Used in connection-oriented mode */
        char            serverSocketName[MAX_NAME_SIZE]; /* Used in connectionless mode */
    } recipient;
    
    PRIORITY_E          priority;
    size_t              maxBufferSize;
    
    ON_DATA_RECEIVED_CB onDataReceivedCb;
    ON_LINK_BROKEN_CB   onLinkBrokenCb;
    
    void                *userData;
};

struct CLIENT_S {
    CLIENT_START_F     start;
    CLIENT_STOP_F      stop;
    
    CLIENT_SEND_DATA_F sendData;
    
    void               *pData;
};

/* -------------------------------------------------------------------------------------------- */
/*                                      PUBLIC FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

CLIENT_ERROR_E Client_Init  (CLIENT_S **obj);
CLIENT_ERROR_E Client_UnInit(CLIENT_S **obj);

#ifdef __cplusplus
}
#endif

#endif //__CLIENT_H__
