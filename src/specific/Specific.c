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

extern VIDEO_CONFIG_S gVideoConfig[];
extern uint32_t gNbVideoConfigs;

extern SPECIFIC_CLICK_HANDLERS_S gGenericClickHandlers[];
extern uint32_t gNbGenericClickHandlers;

/* -------------------------------------------------------------------------------------------- */
/*                                         PROTOTYPES                                           */
/* -------------------------------------------------------------------------------------------- */

SPECIFIC_ERROR_E getVideoConfig_f(SPECIFIC_S *obj, VIDEO_CONFIG_S *config, uint32_t configChoice);

SPECIFIC_ERROR_E initElementData_f  (SPECIFIC_S *obj, void **data);
SPECIFIC_ERROR_E uninitElementData_f(SPECIFIC_S *obj, void **data);

SPECIFIC_ERROR_E setElementGetters_f  (SPECIFIC_S *obj, void *data, SPECIFIC_GETTERS_S *getters);
SPECIFIC_ERROR_E unsetElementGetters_f(SPECIFIC_S *obj, void *data);

SPECIFIC_ERROR_E setElementTextIds_f  (SPECIFIC_S *obj, void *data, SPECIFIC_TEXT_IDS_S *textIds);
SPECIFIC_ERROR_E unsetElementTextIds_f(SPECIFIC_S *obj, void *data);

SPECIFIC_ERROR_E setElementImageIds_f  (SPECIFIC_S *obj, void *data, SPECIFIC_IMAGE_IDS_S *imageIds);
SPECIFIC_ERROR_E unsetElementImageIds_f(SPECIFIC_S *obj, void *data);

SPECIFIC_ERROR_E setClickHandlers_f  (SPECIFIC_S *obj, void *data, SPECIFIC_HANDLERS_S *handlers, uint32_t nbHandlers, uint32_t index);
SPECIFIC_ERROR_E unsetClickHandlers_f(SPECIFIC_S *obj, void *data);

SPECIFIC_ERROR_E handleClick_f(CONTEXT_S *ctx, GFX_EVENT_S *gfxEvent);

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

    (*obj)->setClickHandlers     = setClickHandlers_f;
    (*obj)->unsetClickHandlers   = unsetClickHandlers_f;

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

/* -------------------------------------------------------------------------------------------- */
/*                                     PRIVATE FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

SPECIFIC_ERROR_E getVideoConfig_f(SPECIFIC_S *obj, VIDEO_CONFIG_S *config, uint32_t configChoice)
{
    assert(obj && config);

    if (configChoice >= gNbVideoConfigs) {
        Loge("Bad choice %u / Nb video configs : %u", configChoice, gNbVideoConfigs);
        return SPECIFIC_ERROR_PARAMS;
    }

    config->caps        = gVideoConfig[configChoice].caps;
    config->type        = gVideoConfig[configChoice].type;
    config->pixelformat = gVideoConfig[configChoice].pixelformat;
    config->colorspace  = gVideoConfig[configChoice].colorspace;
    config->memory      = gVideoConfig[configChoice].memory;
    config->awaitMode   = gVideoConfig[configChoice].awaitMode;

    return SPECIFIC_ERROR_NONE;
}

/*!
 *
 */
SPECIFIC_ERROR_E initElementData_f(SPECIFIC_S *obj, void **data)
{
    assert(obj && data);

    SPECIFIC_ELEMENT_DATA_S *elementData;
    assert((elementData = calloc(1, sizeof(SPECIFIC_ELEMENT_DATA_S))));

    *data = elementData;

    return SPECIFIC_ERROR_NONE;
}

/*!
 *
 */
SPECIFIC_ERROR_E uninitElementData_f(SPECIFIC_S *obj, void **data)
{
    assert(obj && data && *data);

    SPECIFIC_ELEMENT_DATA_S *elementData = (SPECIFIC_ELEMENT_DATA_S*)(*data);

    free(elementData);
    elementData = NULL;

    return SPECIFIC_ERROR_NONE;
}

/*!
 *
 */
SPECIFIC_ERROR_E setElementGetters_f(SPECIFIC_S *obj, void *data, SPECIFIC_GETTERS_S *getters)
{
    assert(obj && data && getters);

    SPECIFIC_ELEMENT_DATA_S *elementData = (SPECIFIC_ELEMENT_DATA_S*)data;

    elementData->getters.getString   = getters->getString;
    elementData->getters.getColor    = getters->getColor;
    elementData->getters.getFont     = getters->getFont;
    elementData->getters.getImage    = getters->getImage;
    elementData->getters.getLanguage = getters->getLanguage;

    elementData->getters.userData    = getters->userData;

    return SPECIFIC_ERROR_NONE;
}

/*!
 *
 */
SPECIFIC_ERROR_E unsetElementGetters_f(SPECIFIC_S *obj, void *data)
{
    assert(obj && data);

    SPECIFIC_ELEMENT_DATA_S *elementData = (SPECIFIC_ELEMENT_DATA_S*)data;

    elementData->getters.userData = NULL;

    return SPECIFIC_ERROR_NONE;
}

/*!
 *
 */
SPECIFIC_ERROR_E setElementTextIds_f(SPECIFIC_S *obj, void *data, SPECIFIC_TEXT_IDS_S *textIds)
{
    assert(obj && data && textIds);

    SPECIFIC_ELEMENT_DATA_S *elementData = (SPECIFIC_ELEMENT_DATA_S*)data;

    elementData->ids.text.stringId = textIds->stringId;
    elementData->ids.text.fontId   = textIds->fontId;
    elementData->ids.text.colorId  = textIds->colorId;

    return SPECIFIC_ERROR_NONE;
}

/*!
 *
 */
SPECIFIC_ERROR_E unsetElementTextIds_f(SPECIFIC_S *obj, void *data)
{
    assert(obj && data);

    return SPECIFIC_ERROR_NONE;
}

/*!
 *
 */
SPECIFIC_ERROR_E setElementImageIds_f(SPECIFIC_S *obj, void *data, SPECIFIC_IMAGE_IDS_S *imageIds)
{
    assert(obj && data && imageIds);

    SPECIFIC_ELEMENT_DATA_S *elementData = (SPECIFIC_ELEMENT_DATA_S*)data;

    elementData->ids.image.imageId       = imageIds->imageId;
    elementData->ids.image.hiddenColorId = imageIds->hiddenColorId;

    return SPECIFIC_ERROR_NONE;
}

/*!
 *
 */
SPECIFIC_ERROR_E unsetElementImageIds_f(SPECIFIC_S *obj, void *data)
{
    assert(obj && data);

    return SPECIFIC_ERROR_NONE;
}

/*!
 *
 */
SPECIFIC_ERROR_E setClickHandlers_f(SPECIFIC_S *obj, void *data, SPECIFIC_HANDLERS_S *handlers, uint32_t nbHandlers, uint32_t index)
{
    assert(obj && data);

    if (!handlers || (nbHandlers == 0)) {
        return SPECIFIC_ERROR_PARAMS;
    }

    SPECIFIC_ELEMENT_DATA_S *elementData = (SPECIFIC_ELEMENT_DATA_S*)data;

    elementData->index           = index;
    elementData->nbClickHandlers = nbHandlers;

    assert((elementData->clickHandlers = calloc(1, nbHandlers * sizeof(SPECIFIC_CLICK_HANDLERS_S))));

    uint32_t i, j;
    for (i = 0; i < nbHandlers; i++) {
        (elementData->clickHandlers[i]).name = strdup((handlers[i]).name);
        (elementData->clickHandlers[i]).data = strdup((handlers[i]).data);

        j = 0;
        while ((j < gNbGenericClickHandlers)
                && gGenericClickHandlers[j].name
                && (strcmp(gGenericClickHandlers[j].name, (handlers[i]).name) != 0)) {
            j++;
        }

        if (gGenericClickHandlers[j].name) {
            (elementData->clickHandlers[i]).fct = gGenericClickHandlers[j].fct;
        }
    }

    return SPECIFIC_ERROR_NONE;
}

/*!
 *
 */
SPECIFIC_ERROR_E unsetClickHandlers_f(SPECIFIC_S *obj, void *data)
{
    assert(obj && data);

    SPECIFIC_ELEMENT_DATA_S *elementData = (SPECIFIC_ELEMENT_DATA_S*)data;

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

    return SPECIFIC_ERROR_NONE;
}

/*!
 *
 */
SPECIFIC_ERROR_E handleClick_f(CONTEXT_S *ctx, GFX_EVENT_S *gfxEvent)
{
    assert(ctx && gfxEvent);

    SPECIFIC_ELEMENT_DATA_S *elementData = (SPECIFIC_ELEMENT_DATA_S*)gfxEvent->gfxElementPData;

    if (!elementData) {
        return SPECIFIC_ERROR_PARAMS;
    }

    uint32_t i;
    for (i = 0; i < elementData->nbClickHandlers; i++) {
        if ((elementData->clickHandlers[i]).fct) {
            Logd("Calling click handler : %s", (elementData->clickHandlers[i]).name);
            (elementData->clickHandlers[i]).fct(ctx, gfxEvent->gfxElementName, elementData, (elementData->clickHandlers[i]).data);
        }
    }

    return SPECIFIC_ERROR_NONE;
}
