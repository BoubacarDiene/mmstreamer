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
* \file   MultiInputs.c
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
#define TAG "HANDLERS-MULTIINPUTS"

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                         PROTOTYPES                                           */
/* -------------------------------------------------------------------------------------------- */

static void updateText (HANDLERS_S *obj, char *targetName, void *pData, char *handlerData);
static void updateImage(HANDLERS_S *obj, char *targetName, void *pData, char *handlerData);
static void updateNav  (HANDLERS_S *obj, char *targetName, void *pData, char *handlerData);

/* -------------------------------------------------------------------------------------------- */
/*                                          VARIABLES                                           */
/* -------------------------------------------------------------------------------------------- */

CLICK_HANDLERS_S gMultiInputsClickHandlers[] = {
	{ "updateText",                 NULL,             updateText       },
	{ "updateImage",                NULL,             updateImage      },
	{ "updateNav",                  NULL,             updateNav        },
	{ NULL,                         NULL,             NULL             }
};

uint32_t gNbMultiInputsClickHandlers = (uint32_t)(sizeof(gMultiInputsClickHandlers) / sizeof(gMultiInputsClickHandlers[0]));

/* -------------------------------------------------------------------------------------------- */
/*                                        CLICK HANDLERS                                        */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static void updateText(HANDLERS_S *obj, char *targetName, void *pData, char *handlerData)
{
    assert(obj && obj->pData && targetName);

    (void)pData;

    if (!handlerData) {
        Loge("Handler data is expected");
        return;
    }

    uint32_t index;
    if (obj->getElementIndex(obj, targetName, &index) != HANDLERS_ERROR_NONE) {
        return;
    }

    HANDLERS_PRIVATE_DATA_S *pvData     = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                      = pvData->ctx;
    GRAPHICS_S *graphicsObj             = ctx->modules.graphicsObj;
    GRAPHICS_INFOS_S *graphicsInfos     = &ctx->params.graphicsInfos;
    GFX_ELEMENT_S *gfxElement           = graphicsInfos->gfxElements[index];
    CONTROL_ELEMENT_DATA_S *elementData = (CONTROL_ELEMENT_DATA_S*)gfxElement->pData;

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
static void updateImage(HANDLERS_S *obj, char *targetName, void *pData, char *handlerData)
{
    assert(obj && obj->pData && targetName);

    if (!handlerData) {
        Loge("Handler data is expected");
        return;
    }

    uint32_t index;
    if (obj->getElementIndex(obj, targetName, &index) != HANDLERS_ERROR_NONE) {
        return;
    }

    HANDLERS_PRIVATE_DATA_S *pvData     = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                      = pvData->ctx;
    GRAPHICS_S *graphicsObj             = ctx->modules.graphicsObj;
    GRAPHICS_INFOS_S *graphicsInfos     = &ctx->params.graphicsInfos;
    GFX_ELEMENT_S *gfxElement           = graphicsInfos->gfxElements[index];
    CONTROL_ELEMENT_DATA_S *elementData = (CONTROL_ELEMENT_DATA_S*)gfxElement->pData;

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
static void updateNav(HANDLERS_S *obj, char *targetName, void *pData, char *handlerData)
{
    assert(obj && obj->pData && targetName);

    if (!handlerData) {
        Loge("Handler data is expected");
        return;
    }

    uint32_t index;
    if (obj->getElementIndex(obj, targetName, &index) != HANDLERS_ERROR_NONE) {
        return;
    }

    HANDLERS_PRIVATE_DATA_S *pvData = (HANDLERS_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx                  = pvData->ctx;
    GRAPHICS_S *graphicsObj         = ctx->modules.graphicsObj;
    GRAPHICS_INFOS_S *graphicsInfos = &ctx->params.graphicsInfos;
    GFX_ELEMENT_S *gfxElement       = graphicsInfos->gfxElements[index];

    GFX_NAV_S nav   = { 0 };
    uint32_t offset = 0;

    if (obj->getSubstring(obj, handlerData, ";", nav.left, &offset) != HANDLERS_ERROR_NONE) {
        Loge("Bad format. Expected: <left>;<up>;<right>;<down>");
        return;
    }

    if (obj->getSubstring(obj, handlerData, ";", nav.up, &offset) != HANDLERS_ERROR_NONE) {
        Loge("Bad format. Expected: <left>;<up>;<right>;<down>");
        return;
    }

    if (obj->getSubstring(obj, handlerData, ";", nav.right, &offset) != HANDLERS_ERROR_NONE) {
        Loge("Bad format. Expected: <left>;<up>;<right>;<down>");
        return;
    }

    if (obj->getSubstring(obj, handlerData, ";", nav.down, &offset) != HANDLERS_ERROR_NONE) {
        strncpy(nav.down, handlerData + offset, sizeof(nav.down));
    }

    Logd("Updating nav of element \"%s\" / Params : %s | %s | %s | %s", gfxElement->name, nav.left, nav.up, nav.right, nav.down);

    (void)graphicsObj->setNav(graphicsObj, gfxElement->name, &nav);
}
