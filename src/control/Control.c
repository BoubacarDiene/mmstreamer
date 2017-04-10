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
* \file   Control.c
* \brief  TODO
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include "control/Control.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                            */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "CONTROL"

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

typedef struct CONTROL_PRIVATE_DATA_S {
    CONTEXT_S   *ctx;
} CONTROL_PRIVATE_DATA_S;

/* -------------------------------------------------------------------------------------------- */
/*                                          VARIABLES                                           */
/* -------------------------------------------------------------------------------------------- */

extern CONTROL_CLICK_HANDLERS_S gSingleInputClickHandlers[];
extern uint32_t gNbSingleInputClickHandlers;

/* -------------------------------------------------------------------------------------------- */
/*                                         PROTOTYPES                                           */
/* -------------------------------------------------------------------------------------------- */

CONTROL_ERROR_E initElementData_f  (CONTROL_S *obj, void **data);
CONTROL_ERROR_E uninitElementData_f(CONTROL_S *obj, void **data);

CONTROL_ERROR_E setElementGetters_f  (CONTROL_S *obj, void *data, CONTROL_GETTERS_S *getters);
CONTROL_ERROR_E unsetElementGetters_f(CONTROL_S *obj, void *data);

CONTROL_ERROR_E setElementTextIds_f  (CONTROL_S *obj, void *data, CONTROL_TEXT_IDS_S *textIds);
CONTROL_ERROR_E unsetElementTextIds_f(CONTROL_S *obj, void *data);

CONTROL_ERROR_E setElementImageIds_f  (CONTROL_S *obj, void *data, CONTROL_IMAGE_IDS_S *imageIds);
CONTROL_ERROR_E unsetElementImageIds_f(CONTROL_S *obj, void *data);

CONTROL_ERROR_E setClickHandlers_f  (CONTROL_S *obj, void *data, CONTROL_HANDLERS_S *handlers, uint32_t nbHandlers, uint32_t index);
CONTROL_ERROR_E unsetClickHandlers_f(CONTROL_S *obj, void *data);

CONTROL_ERROR_E handleClick_f(CONTEXT_S *ctx, GFX_EVENT_S *gfxEvent);

/* -------------------------------------------------------------------------------------------- */
/*                                      PUBLIC FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
CONTROL_ERROR_E Control_Init(CONTROL_S **obj, CONTEXT_S *ctx)
{
    assert(obj && ctx && (*obj = calloc(1, sizeof(CONTROL_S))));

    CONTROL_PRIVATE_DATA_S *pData;
    assert((pData = calloc(1, sizeof(CONTROL_PRIVATE_DATA_S))));

    (*obj)->initElementData      = initElementData_f;
    (*obj)->uninitElementData    = uninitElementData_f;

    (*obj)->setElementGetters    = setElementGetters_f;
    (*obj)->unsetElementGetters  = unsetElementGetters_f;

    (*obj)->setElementTextIds    = setElementTextIds_f;
    (*obj)->unsetElementTextIds  = unsetElementTextIds_f;

    (*obj)->setElementImageIds   = setElementImageIds_f;
    (*obj)->unsetElementImageIds = unsetElementImageIds_f;

    (*obj)->setClickHandlers     = setClickHandlers_f;
    (*obj)->unsetClickHandlers   = unsetClickHandlers_f;

    (*obj)->handleClick          = handleClick_f;

    pData->ctx = ctx;

    (*obj)->pData = (void*)pData;

    return CONTROL_ERROR_NONE;
}

/*!
 *
 */
CONTROL_ERROR_E Control_UnInit(CONTROL_S **obj)
{
    assert(obj && *obj && (*obj)->pData);

    CONTROL_PRIVATE_DATA_S *pData = (CONTROL_PRIVATE_DATA_S*)((*obj)->pData);

    pData->ctx = NULL;

    free((*obj)->pData);
    (*obj)->pData = NULL;

    free(*obj);
    *obj = NULL;

    return CONTROL_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/*                                     PRIVATE FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
CONTROL_ERROR_E initElementData_f(CONTROL_S *obj, void **data)
{
    assert(obj && data);

    CONTROL_ELEMENT_DATA_S *elementData;
    assert((elementData = calloc(1, sizeof(CONTROL_ELEMENT_DATA_S))));

    *data = elementData;

    return CONTROL_ERROR_NONE;
}

/*!
 *
 */
CONTROL_ERROR_E uninitElementData_f(CONTROL_S *obj, void **data)
{
    assert(obj && data && *data);

    CONTROL_ELEMENT_DATA_S *elementData = (CONTROL_ELEMENT_DATA_S*)(*data);

    free(elementData);
    elementData = NULL;

    return CONTROL_ERROR_NONE;
}

/*!
 *
 */
CONTROL_ERROR_E setElementGetters_f(CONTROL_S *obj, void *data, CONTROL_GETTERS_S *getters)
{
    assert(obj && data && getters);

    CONTROL_ELEMENT_DATA_S *elementData = (CONTROL_ELEMENT_DATA_S*)data;

    elementData->getters.getString   = getters->getString;
    elementData->getters.getColor    = getters->getColor;
    elementData->getters.getFont     = getters->getFont;
    elementData->getters.getImage    = getters->getImage;
    elementData->getters.getLanguage = getters->getLanguage;

    elementData->getters.userData    = getters->userData;

    return CONTROL_ERROR_NONE;
}

/*!
 *
 */
CONTROL_ERROR_E unsetElementGetters_f(CONTROL_S *obj, void *data)
{
    assert(obj && data);

    CONTROL_ELEMENT_DATA_S *elementData = (CONTROL_ELEMENT_DATA_S*)data;

    elementData->getters.userData = NULL;

    return CONTROL_ERROR_NONE;
}

/*!
 *
 */
CONTROL_ERROR_E setElementTextIds_f(CONTROL_S *obj, void *data, CONTROL_TEXT_IDS_S *textIds)
{
    assert(obj && data && textIds);

    CONTROL_ELEMENT_DATA_S *elementData = (CONTROL_ELEMENT_DATA_S*)data;

    elementData->ids.text.stringId = textIds->stringId;
    elementData->ids.text.fontId   = textIds->fontId;
    elementData->ids.text.colorId  = textIds->colorId;

    return CONTROL_ERROR_NONE;
}

/*!
 *
 */
CONTROL_ERROR_E unsetElementTextIds_f(CONTROL_S *obj, void *data)
{
    assert(obj && data);

    return CONTROL_ERROR_NONE;
}

/*!
 *
 */
CONTROL_ERROR_E setElementImageIds_f(CONTROL_S *obj, void *data, CONTROL_IMAGE_IDS_S *imageIds)
{
    assert(obj && data && imageIds);

    CONTROL_ELEMENT_DATA_S *elementData = (CONTROL_ELEMENT_DATA_S*)data;

    elementData->ids.image.imageId       = imageIds->imageId;
    elementData->ids.image.hiddenColorId = imageIds->hiddenColorId;

    return CONTROL_ERROR_NONE;
}

/*!
 *
 */
CONTROL_ERROR_E unsetElementImageIds_f(CONTROL_S *obj, void *data)
{
    assert(obj && data);

    return CONTROL_ERROR_NONE;
}

/*!
 *
 */
CONTROL_ERROR_E setClickHandlers_f(CONTROL_S *obj, void *data, CONTROL_HANDLERS_S *handlers, uint32_t nbHandlers, uint32_t index)
{
    assert(obj && data);

    CONTROL_ELEMENT_DATA_S *elementData = (CONTROL_ELEMENT_DATA_S*)data;

    elementData->index           = index;
    elementData->nbClickHandlers = nbHandlers;

    if (!handlers || (nbHandlers == 0)) {
        return CONTROL_ERROR_PARAMS;
    }

    assert((elementData->clickHandlers = calloc(1, nbHandlers * sizeof(CONTROL_CLICK_HANDLERS_S))));

    uint32_t i, j;
    for (i = 0; i < nbHandlers; i++) {
        (elementData->clickHandlers[i]).name = strdup((handlers[i]).name);
        (elementData->clickHandlers[i]).data = strdup((handlers[i]).data);

        j = 0;
        while ((j < gNbSingleInputClickHandlers)
                && gSingleInputClickHandlers[j].name
                && (strcmp(gSingleInputClickHandlers[j].name, (handlers[i]).name) != 0)) {
            j++;
        }

        if (gSingleInputClickHandlers[j].name) {
            (elementData->clickHandlers[i]).fct = gSingleInputClickHandlers[j].fct;
        }
    }

    return CONTROL_ERROR_NONE;
}

/*!
 *
 */
CONTROL_ERROR_E unsetClickHandlers_f(CONTROL_S *obj, void *data)
{
    assert(obj && data);

    CONTROL_ELEMENT_DATA_S *elementData = (CONTROL_ELEMENT_DATA_S*)data;

    uint32_t i;
    for (i = 0; i < elementData->nbClickHandlers; i++) {
        if ((elementData->clickHandlers[i]).name) {
            free((elementData->clickHandlers[i]).name);
            (elementData->clickHandlers[i]).name = NULL;
        }
        if ((elementData->clickHandlers[i]).data) {
            free((elementData->clickHandlers[i]).data);
            (elementData->clickHandlers[i]).data = NULL;
        }
        (elementData->clickHandlers[i]).fct = NULL;
    }

    elementData->clickHandlers = NULL;

    return CONTROL_ERROR_NONE;
}

/*!
 *
 */
CONTROL_ERROR_E handleClick_f(CONTEXT_S *ctx, GFX_EVENT_S *gfxEvent)
{
    assert(ctx && gfxEvent);

    CONTROL_ELEMENT_DATA_S *elementData = (CONTROL_ELEMENT_DATA_S*)gfxEvent->gfxElementPData;

    if (!elementData) {
        return CONTROL_ERROR_PARAMS;
    }

    uint32_t i;
    for (i = 0; i < elementData->nbClickHandlers; i++) {
        if ((elementData->clickHandlers[i]).fct) {
            Logd("Calling click handler : %s", (elementData->clickHandlers[i]).name);
            (elementData->clickHandlers[i]).fct(ctx, gfxEvent->gfxElementName, elementData, (elementData->clickHandlers[i]).data);
        }
    }

    return CONTROL_ERROR_NONE;
}
