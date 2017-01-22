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
* \file   Listeners.c
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
#define TAG "LISTENERS"

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                         PROTOTYPES                                           */
/* -------------------------------------------------------------------------------------------- */

extern LISTENERS_ERROR_E setGraphicsListeners_f  (LISTENERS_S *obj);
extern LISTENERS_ERROR_E unsetGraphicsListeners_f(LISTENERS_S *obj);

extern LISTENERS_ERROR_E setVideoListeners_f  (LISTENERS_S *obj);
extern LISTENERS_ERROR_E unsetVideoListeners_f(LISTENERS_S *obj);

extern LISTENERS_ERROR_E setServersListeners_f  (LISTENERS_S *obj);
extern LISTENERS_ERROR_E unsetServersListeners_f(LISTENERS_S *obj);

extern LISTENERS_ERROR_E setClientsListeners_f  (LISTENERS_S *obj);
extern LISTENERS_ERROR_E unsetClientsListeners_f(LISTENERS_S *obj);

/* -------------------------------------------------------------------------------------------- */
/*                                          VARIABLES                                           */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                      PUBLIC FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
LISTENERS_ERROR_E Listeners_Init(LISTENERS_S **obj, CONTEXT_S *ctx, SPECIFIC_S *specificObj)
{
    assert(obj && ctx && (*obj = calloc(1, sizeof(LISTENERS_S))));
    
    LISTENERS_PDATA_S *pData;
    assert((pData = calloc(1, sizeof(LISTENERS_PDATA_S))));
    
    (*obj)->setGraphicsListeners   = setGraphicsListeners_f;
    (*obj)->unsetGraphicsListeners = unsetGraphicsListeners_f;
    
    (*obj)->setVideoListeners      = setVideoListeners_f;
    (*obj)->unsetVideoListeners    = unsetVideoListeners_f;
    
    (*obj)->setServersListeners    = setServersListeners_f;
    (*obj)->unsetServersListeners  = unsetServersListeners_f;
    
    (*obj)->setClientsListeners    = setClientsListeners_f;
    (*obj)->unsetClientsListeners  = unsetClientsListeners_f;
    
    pData->ctx         = ctx;
    pData->specificObj = specificObj;
    
    (*obj)->pData = pData;
    
    return LISTENERS_ERROR_NONE;
}

/*!
 *
 */
LISTENERS_ERROR_E Listeners_UnInit(LISTENERS_S **obj)
{
    assert(obj && *obj && (*obj)->pData);
    
    LISTENERS_PDATA_S *pData = (LISTENERS_PDATA_S*)((*obj)->pData);
    
    pData->ctx         = NULL;
    pData->specificObj = NULL;
    
    free(pData);
    pData = NULL;
    
    free(*obj);
    *obj = NULL;
    
    return LISTENERS_ERROR_NONE;
}
