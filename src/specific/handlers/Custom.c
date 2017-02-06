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

SPECIFIC_ERROR_E callCustomHandler(CONTEXT_S *ctx, char *functionName, void *gfxElementData, char *handlerData);

static void updateText (CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData);
static void updateImage(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData);

/* -------------------------------------------------------------------------------------------- */
/*                                          VARIABLES                                           */
/* -------------------------------------------------------------------------------------------- */

SPECIFIC_CLICK_HANDLERS_S gCustomClickHandlers[] = {
	{ "updateText",                 NULL,             updateText  },
	{ "updateImage",                NULL,             updateImage },
	{ NULL,                         NULL,             NULL        }
};

uint32_t gNbCustomClickHandlers = (uint32_t)(sizeof(gCustomClickHandlers) / sizeof(gCustomClickHandlers[0]));

/* -------------------------------------------------------------------------------------------- */
/*                                          FUNCTIONS                                           */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
SPECIFIC_ERROR_E callCustomHandler(CONTEXT_S *ctx, char *functionName, void *gfxElementData, char *handlerData)
{
    assert(functionName && gfxElementData);

    SPECIFIC_ELEMENT_DATA_S *elementData = (SPECIFIC_ELEMENT_DATA_S*)gfxElementData;
    GFX_ELEMENT_S *gfxElement            = ctx->params.graphicsInfos.gfxElements[elementData->index];

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

    gCustomClickHandlers[index].fct(ctx, gfxElement->name, gfxElementData, handlerData);

    return SPECIFIC_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/*                                        CLICK HANDLERS                                        */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static void updateText(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(ctx && gfxElementName && gfxElementData);

    if (!handlerData) {
        Loge("Handler data is expected");
        return;
    }

    SPECIFIC_ELEMENT_DATA_S *elementData = (SPECIFIC_ELEMENT_DATA_S*)gfxElementData;
    GRAPHICS_S *graphicsObj              = ctx->modules.graphicsObj;
    GRAPHICS_INFOS_S *graphicsInfos      = &ctx->params.graphicsInfos;
    GFX_ELEMENT_S *gfxElement            = graphicsInfos->gfxElements[elementData->index];

    uint32_t stringId, fontId, fontSize, colorId;
    sscanf(handlerData, "%u;%u;%u;%u", &stringId, &fontId, &fontSize, &colorId);

    Logd("Updating text of element \"%s\" / Params : %u | %u | %u | %u", gfxElementName, stringId, fontId, fontSize, colorId);

    GFX_TEXT_S text;
    memset(&text, '\0', sizeof(GFX_TEXT_S));

    elementData->getters.getString(elementData->getters.userData, stringId, graphicsInfos->currentLanguage, text.str);

    elementData->getters.getFont(elementData->getters.userData, fontId, text.ttfFont);
    text.ttfFontSize = fontSize;

    elementData->getters.getColor(elementData->getters.userData, colorId, &text.color);

    (void)graphicsObj->setData(graphicsObj, gfxElementName, (void*)&text);
}

/*!
 *
 */
static void updateImage(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData)
{
    assert(ctx && gfxElementName && gfxElementData);

    if (!handlerData) {
        Loge("Handler data is expected");
        return;
    }

    SPECIFIC_ELEMENT_DATA_S *elementData = (SPECIFIC_ELEMENT_DATA_S*)gfxElementData;
    GRAPHICS_S *graphicsObj              = ctx->modules.graphicsObj;
    GRAPHICS_INFOS_S *graphicsInfos      = &ctx->params.graphicsInfos;
    GFX_ELEMENT_S *gfxElement            = graphicsInfos->gfxElements[elementData->index];

    uint32_t imageId;
    int32_t hiddenColorId;
    sscanf(handlerData, "%u;%d", &imageId, &hiddenColorId);

    Logd("Updating image of element \"%s\" / Params : %u | %d", gfxElementName, imageId, hiddenColorId);

    GFX_IMAGE_S image;
    memset(&image, '\0', sizeof(GFX_IMAGE_S));

    elementData->getters.getImage(elementData->getters.userData, imageId, &image);

    if (hiddenColorId >= 0) {
        GFX_COLOR_S hiddenColor;
        image.hiddenColor = &hiddenColor;
        elementData->getters.getColor(elementData->getters.userData, hiddenColorId, &hiddenColor);
    }

    (void)graphicsObj->setData(graphicsObj, gfxElementName, (void*)&image);
}
