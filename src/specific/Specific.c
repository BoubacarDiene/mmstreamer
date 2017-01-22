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
* \file   Specific.c
* \brief  TODO
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include "specific/Specific.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                            */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "SPECIFIC"

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

typedef struct SPECIFIC_PRIVATE_DATA_S {
    CONTEXT_S   *ctx;
} SPECIFIC_PRIVATE_DATA_S;

/* -------------------------------------------------------------------------------------------- */
/*                                          VARIABLES                                           */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                         PROTOTYPES                                           */
/* -------------------------------------------------------------------------------------------- */

extern SPECIFIC_ERROR_E getVideoConfig_f(SPECIFIC_S *obj, VIDEO_CONFIG_S *config, uint32_t configChoice);

extern SPECIFIC_ERROR_E initElementData_f  (SPECIFIC_S *obj, void **data);
extern SPECIFIC_ERROR_E uninitElementData_f(SPECIFIC_S *obj, void **data);

extern SPECIFIC_ERROR_E setElementGetters_f  (SPECIFIC_S *obj, void *data, SPECIFIC_GETTERS_S *getters);
extern SPECIFIC_ERROR_E unsetElementGetters_f(SPECIFIC_S *obj, void *data);

extern SPECIFIC_ERROR_E setElementTextIds_f  (SPECIFIC_S *obj, void *data, SPECIFIC_TEXT_IDS_S *textIds);
extern SPECIFIC_ERROR_E unsetElementTextIds_f(SPECIFIC_S *obj, void *data);

extern SPECIFIC_ERROR_E setElementImageIds_f  (SPECIFIC_S *obj, void *data, SPECIFIC_IMAGE_IDS_S *imageIds);
extern SPECIFIC_ERROR_E unsetElementImageIds_f(SPECIFIC_S *obj, void *data);

extern SPECIFIC_ERROR_E setClickHandler_f  (SPECIFIC_S *obj, void *data, char *clickHandlerName, uint32_t index);
extern SPECIFIC_ERROR_E unsetClickHandler_f(SPECIFIC_S *obj, void *data);

extern SPECIFIC_ERROR_E handleClick_f(CONTEXT_S *ctx, GFX_EVENT_S *gfxEvent);

/* -------------------------------------------------------------------------------------------- */
/*                                      PUBLIC FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
SPECIFIC_ERROR_E Specific_Init(SPECIFIC_S **obj, CONTEXT_S *ctx)
{
    assert(obj && ctx && (*obj = calloc(1, sizeof(SPECIFIC_S))));

    SPECIFIC_PRIVATE_DATA_S *pData;
    assert((pData = calloc(1, sizeof(SPECIFIC_PRIVATE_DATA_S))));

    (*obj)->getVideoConfig       = getVideoConfig_f;

    (*obj)->initElementData      = initElementData_f;
    (*obj)->uninitElementData    = uninitElementData_f;

    (*obj)->setElementGetters    = setElementGetters_f;
    (*obj)->unsetElementGetters  = unsetElementGetters_f;

    (*obj)->setElementTextIds    = setElementTextIds_f;
    (*obj)->unsetElementTextIds  = unsetElementTextIds_f;

    (*obj)->setElementImageIds   = setElementImageIds_f;
    (*obj)->unsetElementImageIds = unsetElementImageIds_f;

    (*obj)->setClickHandler      = setClickHandler_f;
    (*obj)->unsetClickHandler    = unsetClickHandler_f;

    (*obj)->handleClick          = handleClick_f;

    pData->ctx = ctx;

    (*obj)->pData = (void*)pData;

    return SPECIFIC_ERROR_NONE;
}

/*!
 *
 */
SPECIFIC_ERROR_E Specific_UnInit(SPECIFIC_S **obj)
{
    assert(obj && *obj && (*obj)->pData);

    SPECIFIC_PRIVATE_DATA_S *pData = (SPECIFIC_PRIVATE_DATA_S*)((*obj)->pData);

    pData->ctx = NULL;

    free((*obj)->pData);
    (*obj)->pData = NULL;

    free(*obj);
    *obj = NULL;

    return SPECIFIC_ERROR_NONE;
}
