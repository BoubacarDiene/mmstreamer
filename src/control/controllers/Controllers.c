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
* \file   Controllers.c
* \brief  TODO
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include "control/Controllers.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                            */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "Controllers"

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                         PROTOTYPES                                           */
/* -------------------------------------------------------------------------------------------- */

extern CONTROLLERS_ERROR_E loadLibs_f  (CONTROLLERS_S *obj);
extern CONTROLLERS_ERROR_E unloadLibs_f(CONTROLLERS_S *obj);

extern CONTROLLERS_ERROR_E initCmdsTask_f  (CONTROLLERS_S *obj);
extern CONTROLLERS_ERROR_E uninitCmdsTask_f(CONTROLLERS_S *obj);
extern CONTROLLERS_ERROR_E startCmdsTask_f (CONTROLLERS_S *obj);
extern CONTROLLERS_ERROR_E stopCmdsTask_f  (CONTROLLERS_S *obj);

extern CONTROLLERS_ERROR_E initEvtsTask_f  (CONTROLLERS_S *obj);
extern CONTROLLERS_ERROR_E uninitEvtsTask_f(CONTROLLERS_S *obj);
extern CONTROLLERS_ERROR_E startEvtsTask_f (CONTROLLERS_S *obj);
extern CONTROLLERS_ERROR_E stopEvtsTask_f  (CONTROLLERS_S *obj);

extern CONTROLLERS_ERROR_E initLibsTask_f  (CONTROLLERS_S *obj);
extern CONTROLLERS_ERROR_E uninitLibsTask_f(CONTROLLERS_S *obj);
extern CONTROLLERS_ERROR_E startLibsTask_f (CONTROLLERS_S *obj);
extern CONTROLLERS_ERROR_E stopLibsTask_f  (CONTROLLERS_S *obj);

extern CONTROLLERS_ERROR_E notify_f(CONTROLLERS_S *obj, CONTROLLER_EVENT_S *event);

/* -------------------------------------------------------------------------------------------- */
/*                                          VARIABLES                                           */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                      PUBLIC FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
CONTROLLERS_ERROR_E Controllers_Init(CONTROLLERS_S **obj, CONTROLLERS_PARAMS_S *params)
{
    assert(obj && params && (*obj = calloc(1, sizeof(CONTROLLERS_S))));

    CONTROLLERS_PRIVATE_DATA_S *pData;
    assert((pData = calloc(1, sizeof(CONTROLLERS_PRIVATE_DATA_S))));

    (*obj)->loadLibs       = loadLibs_f;
    (*obj)->unloadLibs     = unloadLibs_f;

    (*obj)->initCmdsTask   = initCmdsTask_f;
    (*obj)->uninitCmdsTask = uninitCmdsTask_f;
    (*obj)->startCmdsTask  = startCmdsTask_f;
    (*obj)->stopCmdsTask   = stopCmdsTask_f;

    (*obj)->initEvtsTask   = initEvtsTask_f;
    (*obj)->uninitEvtsTask = uninitEvtsTask_f;
    (*obj)->startEvtsTask  = startEvtsTask_f;
    (*obj)->stopEvtsTask   = stopEvtsTask_f;

    (*obj)->initLibsTask   = initLibsTask_f;
    (*obj)->uninitLibsTask = uninitLibsTask_f;
    (*obj)->startLibsTask  = startLibsTask_f;
    (*obj)->stopLibsTask   = stopLibsTask_f;

    (*obj)->notify         = notify_f;

    pData->params.ctx         = params->ctx;
    pData->params.onCommandCb = params->onCommandCb;
    pData->params.userData    = params->userData;

    (*obj)->pData = (void*)pData;

    return CONTROLLERS_ERROR_NONE;
}

/*!
 *
 */
CONTROLLERS_ERROR_E Controllers_UnInit(CONTROLLERS_S **obj)
{
    assert(obj && *obj && (*obj)->pData);

    CONTROLLERS_PRIVATE_DATA_S *pData = (CONTROLLERS_PRIVATE_DATA_S*)((*obj)->pData);

    pData->params.ctx         = NULL;
    pData->params.onCommandCb = NULL;
    pData->params.userData    = NULL;

    free((*obj)->pData);
    (*obj)->pData = NULL;

    free(*obj);
    *obj = NULL;

    return CONTROLLERS_ERROR_NONE;
}
