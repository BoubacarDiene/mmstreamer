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
* \file MultiInputs.c
* \brief TODO
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// HEADERS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#include "control/Control.h"

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// MACROS ////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "MultiInputs"

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////// PRIVATE FUNCTIONS PROTOTYPES /////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

static enum handlers_error_e updateText(struct handlers_s *obj, char *targetName, void *pData,
                                        char *handlerData);
static enum handlers_error_e updateImage(struct handlers_s *obj, char *targetName, void *pData,
                                         char *handlerData);
static enum handlers_error_e updateNav(struct handlers_s *obj, char *targetName, void *pData,
                                       char *handlerData);
static enum handlers_error_e sendGfxEvent(struct handlers_s *obj, char *targetName, void *pData,
                                          char *handlerData);

/* -------------------------------------------------------------------------------------------- */
/* ///////////////////////////////////// GLOBAL VARIABLES ///////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

struct handlers_commands_s gMultiInputsHandlers[] = {
	{ HANDLERS_COMMAND_UPDATE_TEXT,     NULL,  updateText   },
	{ HANDLERS_COMMAND_UPDATE_IMAGE,    NULL,  updateImage  },
	{ HANDLERS_COMMAND_UPDATE_NAV,      NULL,  updateNav    },
	{ HANDLERS_COMMAND_SEND_GFX_EVENT,  NULL,  sendGfxEvent },
	{ NULL,                             NULL,  NULL         }
};

uint32_t gNbMultiInputsHandlers = (uint32_t)(sizeof(gMultiInputsHandlers)
                                           / sizeof(gMultiInputsHandlers[0]));

/* -------------------------------------------------------------------------------------------- */
/* ///////////////////////////// PRIVATE FUNCTIONS IMPLEMENTATION ///////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static enum handlers_error_e updateText(struct handlers_s *obj, char *targetName, void *pData,
                                        char *handlerData)
{
    assert(obj && obj->pData && targetName);

    (void)pData;

    if (!handlerData) {
        Loge("Handler data is expected");
        return HANDLERS_ERROR_PARAMS;
    }

    struct handlers_private_data_s *pvData = (struct handlers_private_data_s*)(obj->pData);
    struct context_s *ctx                  = pvData->handlersParams.ctx;
    struct graphics_infos_s *graphicsInfos = &ctx->params.graphicsInfos;

    if (graphicsInfos->state != MODULE_STATE_STARTED) {
        Loge("Graphics module not started - current state : %u", graphicsInfos->state);
        return HANDLERS_ERROR_STATE;
    }

    enum handlers_error_e ret = HANDLERS_ERROR_NONE;

    uint32_t index;
    if ((ret = obj->getElementIndex(obj, targetName, &index)) != HANDLERS_ERROR_NONE) {
        return ret;
    }

    struct graphics_s *graphicsObj             = ctx->modules.graphicsObj;
    struct gfx_element_s *gfxElement           = graphicsInfos->gfxElements[index];
    struct control_element_data_s *elementData = (struct control_element_data_s*)gfxElement->pData;

    uint32_t stringId, fontId, fontSize, colorId;
    sscanf(handlerData, "%u;%u;%u;%u", &stringId, &fontId, &fontSize, &colorId);

    Logd("Updating text of element \"%s\" / Params : %u | %u | %u | %u",
            gfxElement->name, stringId, fontId, fontSize, colorId);

    struct gfx_text_s text = {0};
    elementData->getters.getString(elementData->getters.userData,
                                   stringId,
                                   graphicsInfos->currentLanguage,
                                   text.str);

    elementData->getters.getFont(elementData->getters.userData, fontId, text.ttfFont);
    text.ttfFontSize = fontSize;

    elementData->getters.getColor(elementData->getters.userData, colorId, &text.color);

    if (graphicsObj->setData(graphicsObj, gfxElement->name, (void*)&text) != GRAPHICS_ERROR_NONE) {
        Loge("setData() failed for element \"%s\"", gfxElement->name);
        ret = HANDLERS_ERROR_COMMAND;
    }

    return ret;
}

/*!
 *
 */
static enum handlers_error_e updateImage(struct handlers_s *obj, char *targetName, void *pData,
                                         char *handlerData)
{
    assert(obj && obj->pData && targetName);

    (void)pData;

    if (!handlerData) {
        Loge("Handler data is expected");
        return HANDLERS_ERROR_PARAMS;
    }

    struct handlers_private_data_s *pvData = (struct handlers_private_data_s*)(obj->pData);
    struct context_s *ctx                  = pvData->handlersParams.ctx;
    struct graphics_infos_s *graphicsInfos = &ctx->params.graphicsInfos;

    if (graphicsInfos->state != MODULE_STATE_STARTED) {
        Loge("Graphics module not started - current state : %u", graphicsInfos->state);
        return HANDLERS_ERROR_STATE;
    }

    enum handlers_error_e ret = HANDLERS_ERROR_NONE;

    uint32_t index;
    if ((ret = obj->getElementIndex(obj, targetName, &index)) != HANDLERS_ERROR_NONE) {
        return ret;
    }

    struct graphics_s *graphicsObj             = ctx->modules.graphicsObj;
    struct gfx_element_s *gfxElement           = graphicsInfos->gfxElements[index];
    struct control_element_data_s *elementData = (struct control_element_data_s*)gfxElement->pData;

    uint32_t imageId;
    int32_t hiddenColorId;
    sscanf(handlerData, "%u;%d", &imageId, &hiddenColorId);

    Logd("Updating image of element \"%s\" / Params : %u | %d",
            gfxElement->name, imageId, hiddenColorId);

    struct gfx_image_s image = {0};
    elementData->getters.getImage(elementData->getters.userData, imageId, &image);

    if (hiddenColorId >= 0) {
        struct gfx_color_s hiddenColor;
        image.hiddenColor = &hiddenColor;
        elementData->getters.getColor(elementData->getters.userData, hiddenColorId, &hiddenColor);
    }

    if (graphicsObj->setData(graphicsObj,
                             gfxElement->name, (void*)&image) != GRAPHICS_ERROR_NONE) {
        Loge("setData() failed for element \"%s\"", gfxElement->name);
        ret = HANDLERS_ERROR_COMMAND;
    }

    return ret;
}

/*!
 *
 */
static enum handlers_error_e updateNav(struct handlers_s *obj, char *targetName, void *pData,
                                       char *handlerData)
{
    assert(obj && obj->pData && targetName);

    (void)pData;

    if (!handlerData) {
        Loge("Handler data is expected");
        return HANDLERS_ERROR_PARAMS;
    }

    struct handlers_private_data_s *pvData = (struct handlers_private_data_s*)(obj->pData);
    struct context_s *ctx                  = pvData->handlersParams.ctx;
    struct graphics_infos_s *graphicsInfos = &ctx->params.graphicsInfos;

    if (graphicsInfos->state != MODULE_STATE_STARTED) {
        Loge("Graphics module not started - current state : %u", graphicsInfos->state);
        return HANDLERS_ERROR_STATE;
    }

    enum handlers_error_e ret = HANDLERS_ERROR_NONE;

    uint32_t index;
    if ((ret = obj->getElementIndex(obj, targetName, &index)) != HANDLERS_ERROR_NONE) {
        return ret;
    }

    struct graphics_s *graphicsObj   = ctx->modules.graphicsObj;
    struct gfx_element_s *gfxElement = graphicsInfos->gfxElements[index];

    struct gfx_nav_s nav   = {0};
    uint32_t offset = 0;

    ret = obj->getSubstring(obj, handlerData, ";", nav.left, &offset);
    if (ret != HANDLERS_ERROR_NONE) {
        Loge("Bad format. Expected: <left>;<up>;<right>;<down>");
        return ret;
    }

    ret = obj->getSubstring(obj, handlerData, ";", nav.up, &offset);
    if (ret != HANDLERS_ERROR_NONE) {
        Loge("Bad format. Expected: <left>;<up>;<right>;<down>");
        return ret;
    }

    ret = obj->getSubstring(obj, handlerData, ";", nav.right, &offset);
    if (ret != HANDLERS_ERROR_NONE) {
        Loge("Bad format. Expected: <left>;<up>;<right>;<down>");
        return ret;
    }

    if (obj->getSubstring(obj, handlerData, ";", nav.down, &offset) != HANDLERS_ERROR_NONE) {
        strncpy(nav.down, handlerData + offset, sizeof(nav.down));
    }

    Logd("Updating nav of element \"%s\" / Params : %s | %s | %s | %s",
            gfxElement->name, nav.left, nav.up, nav.right, nav.down);

    if (graphicsObj->setNav(graphicsObj, gfxElement->name, &nav) != GRAPHICS_ERROR_NONE) {
        Loge("setNav() failed for element \"%s\"", gfxElement->name);
        ret = HANDLERS_ERROR_COMMAND;
    }

    return ret;
}

/*!
 *
 */
static enum handlers_error_e sendGfxEvent(struct handlers_s *obj, char *targetName, void *pData,
                                          char *handlerData)
{
    assert(obj && obj->pData);

    (void)targetName;
    (void)pData;

    if (!handlerData) {
        Loge("Handler data is expected");
        return HANDLERS_ERROR_PARAMS;
    }

    struct handlers_private_data_s *pvData = (struct handlers_private_data_s*)(obj->pData);
    struct context_s *ctx                  = pvData->handlersParams.ctx;
    struct graphics_infos_s *graphicsInfos = &ctx->params.graphicsInfos;

    if (graphicsInfos->state != MODULE_STATE_STARTED) {
        Loge("Graphics module not started - current state : %u", graphicsInfos->state);
        return HANDLERS_ERROR_STATE;
    }

    struct graphics_s *graphicsObj = ctx->modules.graphicsObj;
    struct gfx_event_s event       = {0};
    enum handlers_error_e ret      = HANDLERS_ERROR_NONE;

    uint32_t id, x, y;
    sscanf(handlerData, "%u;%u;%u", &id, &x, &y);

    event.type   = (enum gfx_event_type_e)id;
    event.rect.x = (uint16_t)x;
    event.rect.y = (uint16_t)y;

    Logd("Sending gfx event / Type : %u / x : %u / y : %u", id, x, y);

    if (graphicsObj->simulateGfxEvent(graphicsObj, &event) != GRAPHICS_ERROR_NONE) {
        Loge("simulateGfxEvent() failed for type \"%u\"", id);
        ret = HANDLERS_ERROR_COMMAND;
    }

    return ret;
}
