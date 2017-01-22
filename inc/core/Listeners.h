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
* \file   Listeners.h
* \author Boubacar DIENE
*/

#ifndef __LISTENERS_H__
#define __LISTENERS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include "core/Common.h"
#include "specific/Specific.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                            */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

typedef enum   LISTENERS_ERROR_E LISTENERS_ERROR_E;

typedef struct LISTENERS_PDATA_S LISTENERS_PDATA_S;
typedef struct LISTENERS_S       LISTENERS_S;

typedef LISTENERS_ERROR_E (*LISTENERS_SET_GRAPHICS_LISTENERS_F  )(LISTENERS_S *obj);
typedef LISTENERS_ERROR_E (*LISTENERS_UNSET_GRAPHICS_LISTENERS_F)(LISTENERS_S *obj);

typedef LISTENERS_ERROR_E (*LISTENERS_SET_VIDEO_LISTENERS_F  )(LISTENERS_S *obj);
typedef LISTENERS_ERROR_E (*LISTENERS_UNSET_VIDEO_LISTENERS_F)(LISTENERS_S *obj);

typedef LISTENERS_ERROR_E (*LISTENERS_SET_SERVERS_LISTENERS_F  )(LISTENERS_S *obj);
typedef LISTENERS_ERROR_E (*LISTENERS_UNSET_SERVERS_LISTENERS_F)(LISTENERS_S *obj);

typedef LISTENERS_ERROR_E (*LISTENERS_SET_CLIENTS_LISTENERS_F  )(LISTENERS_S *obj);
typedef LISTENERS_ERROR_E (*LISTENERS_UNSET_CLIENTS_LISTENERS_F)(LISTENERS_S *obj);

enum LISTENERS_ERROR_E {
    LISTENERS_ERROR_NONE,
    LISTENERS_ERROR_INIT,
    LISTENERS_ERROR_UNINIT,
    LISTENERS_ERROR_LISTENER
};

struct LISTENERS_PDATA_S {
    CONTEXT_S  *ctx;
    SPECIFIC_S *specificObj;
    BUFFER_S   buffer;
};

struct LISTENERS_S {
    LISTENERS_SET_GRAPHICS_LISTENERS_F   setGraphicsListeners;
    LISTENERS_UNSET_GRAPHICS_LISTENERS_F unsetGraphicsListeners;
    
    LISTENERS_SET_VIDEO_LISTENERS_F      setVideoListeners;
    LISTENERS_UNSET_VIDEO_LISTENERS_F    unsetVideoListeners;
    
    LISTENERS_SET_SERVERS_LISTENERS_F    setServersListeners;
    LISTENERS_UNSET_SERVERS_LISTENERS_F  unsetServersListeners;
    
    LISTENERS_SET_CLIENTS_LISTENERS_F    setClientsListeners;
    LISTENERS_UNSET_CLIENTS_LISTENERS_F  unsetClientsListeners;
    
    LISTENERS_PDATA_S                    *pData;
};

/* -------------------------------------------------------------------------------------------- */
/*                                      PUBLIC FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

LISTENERS_ERROR_E Listeners_Init  (LISTENERS_S **obj, CONTEXT_S *ctx, SPECIFIC_S *specificObj);
LISTENERS_ERROR_E Listeners_UnInit(LISTENERS_S **obj);

#ifdef __cplusplus
}
#endif

#endif //__LISTENERS_H__
