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
* \file   Custom.c
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
#define TAG "SPECIFIC-CUSTOM"

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                         PROTOTYPES                                           */
/* -------------------------------------------------------------------------------------------- */

SPECIFIC_ERROR_E callCustomHandler(CONTEXT_S *ctx, char *functionName, char *targetName, char *handlerData);
SPECIFIC_ERROR_E getSubstring     (CONTEXT_S *ctx, const char *haystack, const char *needle, char *out, uint32_t *offset);

static SPECIFIC_ERROR_E getElementIndex(CONTEXT_S *ctx, char *elementName, uint32_t *index);

static void updateText (CONTEXT_S *ctx, char *targetName, void *pData, char *handlerData);
static void updateImage(CONTEXT_S *ctx, char *targetName, void *pData, char *handlerData);
static void updateNav  (CONTEXT_S *ctx, char *targetName, void *pData, char *handlerData);

/* -------------------------------------------------------------------------------------------- */
/*                                          VARIABLES                                           */
/* -------------------------------------------------------------------------------------------- */

SPECIFIC_CLICK_HANDLERS_S gCustomClickHandlers[] = {
	{ "updateText",                     NULL,             updateText             },
	{ "updateImage",                    NULL,             updateImage            },
	{ "updateNav",                      NULL,             updateNav              },
	{ NULL,                             NULL,             NULL                   }
};

uint32_t gNbCustomClickHandlers = (uint32_t)(sizeof(gCustomClickHandlers) / sizeof(gCustomClickHandlers[0]));

/* -------------------------------------------------------------------------------------------- */
/*                                      PUBLIC FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
SPECIFIC_ERROR_E callCustomHandler(CONTEXT_S *ctx, char *functionName, char *targetName, char *handlerData)
{
    assert(functionName && targetName);

    uint32_t index;
    for (index = 0; index < gNbCustomClickHandlers; index++) {
        if (strncmp(gCustomClickHandlers[index].name, functionName, sizeof(gCustomClickHandlers[index].name)) == 0) {
            break;
        }
    }

    if (index >= gNbCustomClickHandlers) {
        Loge("Method \"%s\" not found", functionName);
        return SPECIFIC_ERROR_PARAMS;
    }

    if (!gCustomClickHandlers[index].fct) {
        Loge("Method \"%s\" not defined", functionName);
        return SPECIFIC_ERROR_PARAMS;
    }

    gCustomClickHandlers[index].fct(ctx, targetName, NULL, handlerData);

    return SPECIFIC_ERROR_NONE;
}

/*!
 *
 */
SPECIFIC_ERROR_E getSubstring(CONTEXT_S *ctx, const char *haystack, const char *needle, char *out, uint32_t *offset)
{
    (void)ctx;

    if (!haystack || !needle || !out || !offset) {
        Loge("Bad params");
        return SPECIFIC_ERROR_PARAMS;
    }

    char *result = strstr(haystack + *offset, needle);
    if (!result) {
        return SPECIFIC_ERROR_PARAMS;
    }

    int32_t len = strlen(haystack + *offset) - strlen(result);
    if (len > 0) {
        strncpy(out, haystack + *offset, len);
    }

    *offset += len + 1;

    return SPECIFIC_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/*                                     PRIVATE FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static SPECIFIC_ERROR_E getElementIndex(CONTEXT_S *ctx, char *elementName, uint32_t *index)
{
    assert(ctx && elementName && index);

    GRAPHICS_INFOS_S *graphicsInfos = &ctx->params.graphicsInfos;
    uint32_t nbGfxElements          = graphicsInfos->nbGfxElements;
    GFX_ELEMENT_S **gfxElements     = graphicsInfos->gfxElements;

    for (*index = 0; *index < nbGfxElements; (*index)++) {
        if (strncmp(gfxElements[*index]->name, elementName, sizeof(gfxElements[*index]->name)) == 0) {
            break;
        }
    }

    if (*index >= nbGfxElements) {
        Loge("Element \"%s\" not found", elementName);
        return SPECIFIC_ERROR_PARAMS;
    }

    return SPECIFIC_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/*                                        CLICK HANDLERS                                        */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static void updateText(CONTEXT_S *ctx, char *targetName, void *pData, char *handlerData)
{
    assert(ctx && targetName);

    (void)pData;

    if (!handlerData) {
        Loge("Handler data is expected");
        return;
    }

    uint32_t index;
    if (getElementIndex(ctx, targetName, &index) != SPECIFIC_ERROR_NONE) {
        return;
    }

    GRAPHICS_S *graphicsObj               = ctx->modules.graphicsObj;
    GRAPHICS_INFOS_S *graphicsInfos       = &ctx->params.graphicsInfos;
    GFX_ELEMENT_S *gfxElement             = graphicsInfos->gfxElements[index];
    SPECIFIC_ELEMENT_DATA_S *elementData  = (SPECIFIC_ELEMENT_DATA_S*)gfxElement->pData;

    uint32_t stringId, fontId, fontSize, colorId;
    sscanf(handlerData, "%u;%u;%u;%u", &stringId, &fontId, &fontSize, &colorId);

    Logd("Updating text of element \"%s\" / Params : %u | %u | %u | %u", gfxElement->name, stringId, fontId, fontSize, colorId);

    GFX_TEXT_S text = { 0 };
    elementData->getters.getString(elementData->getters.userData, stringId, graphicsInfos->currentLanguage, text.str);

    elementData->getters.getFont(elementData->getters.userData, fontId, text.ttfFont);
    text.ttfFontSize = fontSize;

    elementData->getters.getColor(elementData->getters.userData, colorId, &text.color);

    (void)graphicsObj->setData(graphicsObj, gfxElement->name, (void*)&text);
}

/*!
 *
 */
static void updateImage(CONTEXT_S *ctx, char *targetName, void *pData, char *handlerData)
{
    assert(ctx && targetName);

    if (!handlerData) {
        Loge("Handler data is expected");
        return;
    }

    uint32_t index;
    if (getElementIndex(ctx, targetName, &index) != SPECIFIC_ERROR_NONE) {
        return;
    }

    GRAPHICS_S *graphicsObj               = ctx->modules.graphicsObj;
    GRAPHICS_INFOS_S *graphicsInfos       = &ctx->params.graphicsInfos;
    GFX_ELEMENT_S *gfxElement             = graphicsInfos->gfxElements[index];
    SPECIFIC_ELEMENT_DATA_S *elementData  = (SPECIFIC_ELEMENT_DATA_S*)gfxElement->pData;

    uint32_t imageId;
    int32_t hiddenColorId;
    sscanf(handlerData, "%u;%d", &imageId, &hiddenColorId);

    Logd("Updating image of element \"%s\" / Params : %u | %d", gfxElement->name, imageId, hiddenColorId);

    GFX_IMAGE_S image = { 0 };
    elementData->getters.getImage(elementData->getters.userData, imageId, &image);

    if (hiddenColorId >= 0) {
        GFX_COLOR_S hiddenColor;
        image.hiddenColor = &hiddenColor;
        elementData->getters.getColor(elementData->getters.userData, hiddenColorId, &hiddenColor);
    }

    (void)graphicsObj->setData(graphicsObj, gfxElement->name, (void*)&image);
}

/*!
 *
 */
static void updateNav(CONTEXT_S *ctx, char *targetName, void *pData, char *handlerData)
{
    assert(ctx && targetName);

    if (!handlerData) {
        Loge("Handler data is expected");
        return;
    }

    uint32_t index;
    if (getElementIndex(ctx, targetName, &index) != SPECIFIC_ERROR_NONE) {
        return;
    }

    GRAPHICS_S *graphicsObj         = ctx->modules.graphicsObj;
    GRAPHICS_INFOS_S *graphicsInfos = &ctx->params.graphicsInfos;
    GFX_ELEMENT_S *gfxElement       = graphicsInfos->gfxElements[index];

    GFX_NAV_S nav   = { 0 };
    uint32_t offset = 0;

    if (getSubstring(ctx, handlerData, ";", nav.left, &offset) != SPECIFIC_ERROR_NONE) {
        Loge("Bad format. Expected: <left>;<up>;<right>;<down>");
        return;
    }

    if (getSubstring(ctx, handlerData, ";", nav.up, &offset) != SPECIFIC_ERROR_NONE) {
        Loge("Bad format. Expected: <left>;<up>;<right>;<down>");
        return;
    }

    if (getSubstring(ctx, handlerData, ";", nav.right, &offset) != SPECIFIC_ERROR_NONE) {
        Loge("Bad format. Expected: <left>;<up>;<right>;<down>");
        return;
    }

    if (getSubstring(ctx, handlerData, ";", nav.down, &offset) != SPECIFIC_ERROR_NONE) {
        strncpy(nav.down, handlerData + offset, sizeof(nav.down));
    }

    Logd("Updating nav of element \"%s\" / Params : %s | %s | %s | %s", gfxElement->name, nav.left, nav.up, nav.right, nav.down);

    (void)graphicsObj->setNav(graphicsObj, gfxElement->name, &nav);
}
