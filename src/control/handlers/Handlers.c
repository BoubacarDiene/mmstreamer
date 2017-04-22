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
* \file   Handlers.c
* \brief  TODO
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include "control/Handlers.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                            */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "HANDLERS"

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                         PROTOTYPES                                           */
/* -------------------------------------------------------------------------------------------- */

static HANDLERS_ERROR_E getClickHandler_f(HANDLERS_S *obj, const char *handlerName, CLICK_HANDLER_F *out);
static HANDLERS_ERROR_E getElementIndex_f(HANDLERS_S *obj, char *elementName, uint32_t *index);
static HANDLERS_ERROR_E getSubstring_f   (HANDLERS_S *obj, const char *haystack, const char *needle, char *out, uint32_t *offset);

/* -------------------------------------------------------------------------------------------- */
/*                                          VARIABLES                                           */
/* -------------------------------------------------------------------------------------------- */

extern CLICK_HANDLERS_S gSingleInputClickHandlers[];
extern uint32_t gNbSingleInputClickHandlers;

extern CLICK_HANDLERS_S gMultiInputsClickHandlers[];
extern uint32_t gNbMultiInputsClickHandlers;

/* -------------------------------------------------------------------------------------------- */
/*                                      PUBLIC FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
HANDLERS_ERROR_E Handlers_Init(HANDLERS_S **obj, CONTEXT_S *ctx)
{
    assert(obj && (*obj = calloc(1, sizeof(HANDLERS_S))));

    HANDLERS_PRIVATE_DATA_S *pData;
    assert((pData = calloc(1, sizeof(HANDLERS_PRIVATE_DATA_S))));

    (*obj)->getClickHandler = getClickHandler_f;
    (*obj)->getElementIndex = getElementIndex_f;
    (*obj)->getSubstring    = getSubstring_f;

    pData->ctx = ctx;

    pData->nbSingleInputClickHandlers = gNbSingleInputClickHandlers;
    pData->singleInputClickHandlers   = gSingleInputClickHandlers;

    pData->nbMultiInputsClickHandlers = gNbMultiInputsClickHandlers;
    pData->multiInputsClickHandlers   = gMultiInputsClickHandlers;

    (*obj)->pData = (void*)pData;

    return HANDLERS_ERROR_NONE;
}

/*!
 *
 */
HANDLERS_ERROR_E Handlers_UnInit(HANDLERS_S **obj)
{
    assert(obj && *obj && (*obj)->pData);

    HANDLERS_PRIVATE_DATA_S *pData = (HANDLERS_PRIVATE_DATA_S*)((*obj)->pData);

    pData->ctx                      = NULL;
    pData->singleInputClickHandlers = NULL;
    pData->multiInputsClickHandlers = NULL;

    free((*obj)->pData);
    (*obj)->pData = NULL;

    free(*obj);
    *obj = NULL;

    return HANDLERS_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/*                                     PRIVATE FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static HANDLERS_ERROR_E getClickHandler_f(HANDLERS_S *obj, const char *handlerName, CLICK_HANDLER_F *out)
{
    assert(obj && obj->pData && handlerName && out);

    HANDLERS_PRIVATE_DATA_S *pData = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);

    uint32_t i = 0;
    while ((i < pData->nbSingleInputClickHandlers)
            && pData->singleInputClickHandlers[i].name
            && (strcmp(pData->singleInputClickHandlers[i].name, handlerName) != 0)) {
        i++;
    }

    if (pData->singleInputClickHandlers[i].name) {
        *out = pData->singleInputClickHandlers[i].fct;
    }

    return HANDLERS_ERROR_NONE;
}

/*!
 *
 */
static HANDLERS_ERROR_E getElementIndex_f(HANDLERS_S *obj, char *elementName, uint32_t *index)
{
    assert(obj && obj->pData && elementName && index);

    HANDLERS_PRIVATE_DATA_S *pData  = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    GRAPHICS_INFOS_S *graphicsInfos = &pData->ctx->params.graphicsInfos;
    uint32_t nbGfxElements          = graphicsInfos->nbGfxElements;
    GFX_ELEMENT_S **gfxElements     = graphicsInfos->gfxElements;

    for (*index = 0; *index < nbGfxElements; (*index)++) {
        if (strcmp(gfxElements[*index]->name, elementName) == 0) {
            break;
        }
    }

    if (*index >= nbGfxElements) {
        Loge("Element \"%s\" not found", elementName);
        return HANDLERS_ERROR_PARAMS;
    }

    return HANDLERS_ERROR_NONE;
}

/*!
 *
 */
HANDLERS_ERROR_E getSubstring_f(HANDLERS_S *obj, const char *haystack, const char *needle, char *out, uint32_t *offset)
{
    assert(obj && obj->pData);

    (void)obj;

    if (!haystack || !needle || !out || !offset) {
        Loge("Bad params");
        return HANDLERS_ERROR_PARAMS;
    }

    char *result = strstr(haystack + *offset, needle);
    if (!result) {
        return HANDLERS_ERROR_PARAMS;
    }

    int32_t len = strlen(haystack + *offset) - strlen(result);
    if (len > 0) {
        strncpy(out, haystack + *offset, len);
    }

    *offset += len + 1;

    return HANDLERS_ERROR_NONE;
}
