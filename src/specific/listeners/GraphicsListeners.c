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
* \file   GraphicsListeners.c
* \brief  TODO
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include "specific/Listeners.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                            */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "GRAPHICS-LISTENERS"

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

typedef struct CLICK_FCT_MAP_S {
    char                      *clickHandlerName;
    LISTENERS_CLICK_HANDLER_F clickHandler;
} LISTENERS_CLICK_FCT_MAP_S;

/* -------------------------------------------------------------------------------------------- */
/*                                         PROTOTYPES                                           */
/* -------------------------------------------------------------------------------------------- */

LISTENERS_ERROR_E setGraphicsListeners_f  (LISTENERS_S *obj);
LISTENERS_ERROR_E unsetGraphicsListeners_f(LISTENERS_S *obj);

static void onGfxEventCb(GFX_EVENT_S *gfxEvent, void *userData);

static void clickHandler0(CONTEXT_S *ctx, char *gfxElementName, LISTENERS_ELEMENT_PDATA_S *elementPData);
static void clickHandler1(CONTEXT_S *ctx, char *gfxElementName, LISTENERS_ELEMENT_PDATA_S *elementPData);
static void clickHandler2(CONTEXT_S *ctx, char *gfxElementName, LISTENERS_ELEMENT_PDATA_S *elementPData);
static void clickHandler3(CONTEXT_S *ctx, char *gfxElementName, LISTENERS_ELEMENT_PDATA_S *elementPData);
static void clickHandler4(CONTEXT_S *ctx, char *gfxElementName, LISTENERS_ELEMENT_PDATA_S *elementPData);

/* -------------------------------------------------------------------------------------------- */
/*                                          VARIABLES                                           */
/* -------------------------------------------------------------------------------------------- */

LISTENERS_CLICK_FCT_MAP_S gClickFctMap[] = {
	{ "clickHandler0",   clickHandler0 },
	{ "clickHandler1",   clickHandler1 },
	{ "clickHandler2",   clickHandler2 },
	{ "clickHandler3",   clickHandler3 },
	{ "clickHandler4",   clickHandler4 },
	{ NULL,              NULL          }
};

/* -------------------------------------------------------------------------------------------- */
/*                                          FUNCTIONS                                           */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
LISTENERS_ERROR_E setGraphicsListeners_f(LISTENERS_S *obj)
{
    assert(obj && obj->pData);
    
    LISTENERS_PDATA_S *pData        = (LISTENERS_PDATA_S*)(obj->pData);
    GRAPHICS_INFOS_S *graphicsInfos = &pData->ctx->params.graphicsInfos;
    
    graphicsInfos->graphicsParams.onGfxEventCb = onGfxEventCb;
    graphicsInfos->graphicsParams.userData     = pData;
    
    uint32_t i, j;
    LISTENERS_ELEMENT_PDATA_S *elementPData;
    for (i = 0; i < graphicsInfos->nbGfxElements; i++) {
        elementPData = (LISTENERS_ELEMENT_PDATA_S*)graphicsInfos->gfxElements[i]->pData;
        j = 0;
        while (gClickFctMap[j].clickHandlerName
                && (strcmp(gClickFctMap[j].clickHandlerName, elementPData->clickHandlerName) != 0)) {
            j++;
        }
        if (gClickFctMap[j].clickHandlerName) {
            elementPData->clickHandler = gClickFctMap[j].clickHandler;
        }
    }
    
    return LISTENERS_ERROR_NONE;
}

/*!
 *
 */
LISTENERS_ERROR_E unsetGraphicsListeners_f(LISTENERS_S *obj)
{
    assert(obj && obj->pData);
    
    LISTENERS_PDATA_S *pData        = (LISTENERS_PDATA_S*)(obj->pData);
    GRAPHICS_INFOS_S *graphicsInfos = &pData->ctx->params.graphicsInfos;
    
    graphicsInfos->graphicsParams.onGfxEventCb = NULL;
    graphicsInfos->graphicsParams.userData     = NULL;
    
    uint32_t index;
    LISTENERS_ELEMENT_PDATA_S *elementPData;
    for (index = 0; index < graphicsInfos->nbGfxElements; index++) {
        elementPData = (LISTENERS_ELEMENT_PDATA_S*)graphicsInfos->gfxElements[index]->pData;
        elementPData->clickHandler = NULL;
    }
    
    return LISTENERS_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/*                                          CALLBACKS                                           */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static void onGfxEventCb(GFX_EVENT_S *gfxEvent, void *userData)
{
    assert(gfxEvent && userData);
    
    LISTENERS_PDATA_S *pData                   = (LISTENERS_PDATA_S*)userData;
    LISTENERS_ELEMENT_PDATA_S *gfxElementPData = (LISTENERS_ELEMENT_PDATA_S*)gfxEvent->gfxElementPData;
    
    switch (gfxEvent->type) {
        case GFX_EVENT_TYPE_QUIT:
        case GFX_EVENT_TYPE_ESC:
            pData->ctx->modules.graphicsObj->quit(pData->ctx->modules.graphicsObj);
            break;
            
        case GFX_EVENT_TYPE_FOCUS:
            break;
            
        case GFX_EVENT_TYPE_CLICK:
        case GFX_EVENT_TYPE_ENTER:
            if (gfxElementPData && gfxElementPData->clickHandler) {
                gfxElementPData->clickHandler(pData->ctx, gfxEvent->gfxElementName, gfxElementPData);
            }
            break;
            
        default:
            ;
    }
}

/* -------------------------------------------------------------------------------------------- */
/*                                        CLICK HANDLERS                                        */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static void clickHandler0(CONTEXT_S *ctx, char *gfxElementName, LISTENERS_ELEMENT_PDATA_S *elementPData)
{
    assert(ctx && gfxElementName && elementPData);
    
    Logd("Handling click on element \"%s\"", gfxElementName);

    ctx->modules.graphicsObj->quit(ctx->modules.graphicsObj);
}

/*!
 *
 */
static void clickHandler1(CONTEXT_S *ctx, char *gfxElementName, LISTENERS_ELEMENT_PDATA_S *elementPData)
{
    assert(ctx && gfxElementName && elementPData);
    
    GRAPHICS_S *graphicsObj         = ctx->modules.graphicsObj;
    GRAPHICS_INFOS_S *graphicsInfos = &ctx->params.graphicsInfos;
    uint32_t nbGfxElements          = graphicsInfos->nbGfxElements;
    GFX_ELEMENT_S **gfxElements     = graphicsInfos->gfxElements;
    
    Logd("Handling click on element \"%s\"", gfxElementName);
    
    char nextLanguage[MIN_STR_SIZE];
    memset(nextLanguage, '\0', MIN_STR_SIZE);
    
    elementPData->getters.getLanguage(elementPData->getters.userData, graphicsInfos->currentLanguage, nextLanguage);
    
    Logd("Changing language from \"%s\" to \"%s\"", graphicsInfos->currentLanguage, nextLanguage);
    
    uint32_t index;
    GFX_TEXT_S text;
    LISTENERS_ELEMENT_PDATA_S *data;
    
    for (index = 0; index < nbGfxElements; index++) {
        if (gfxElements[index]->type != GFX_ELEMENT_TYPE_TEXT) {
            continue;
        }
        
        data = (LISTENERS_ELEMENT_PDATA_S*)gfxElements[index]->pData;
        
        memcpy(&text, &gfxElements[index]->data.text, sizeof(GFX_TEXT_S));
        memset(text.str, '\0', sizeof(text.str));
        elementPData->getters.getString(elementPData->getters.userData, data->ids.text.stringId, nextLanguage, text.str);
        
        graphicsObj->setData(graphicsObj, gfxElements[index]->name, (void*)&text);
    }
    
    memset(graphicsInfos->currentLanguage, '\0', MIN_STR_SIZE);
    strcpy(graphicsInfos->currentLanguage, nextLanguage);
}

/*!
 *
 */
static void clickHandler2(CONTEXT_S *ctx, char *gfxElementName, LISTENERS_ELEMENT_PDATA_S *elementPData)
{
    assert(ctx && gfxElementName && elementPData);
    
    Logd("Handling click on element \"%s\"", gfxElementName);

    ctx->modules.graphicsObj->setVisible(ctx->modules.graphicsObj, "imageBtn", 0);
}

/*!
 *
 */
static void clickHandler3(CONTEXT_S *ctx, char *gfxElementName, LISTENERS_ELEMENT_PDATA_S *elementPData)
{
    assert(ctx && gfxElementName && elementPData);
    
    Logd("Handling click on element \"%s\"", gfxElementName);
}

/*!
 *
 */
static void clickHandler4(CONTEXT_S *ctx, char *gfxElementName, LISTENERS_ELEMENT_PDATA_S *elementPData)
{
    assert(ctx && gfxElementName && elementPData);
    
    Logd("Handling click on element \"%s\"", gfxElementName);
}
