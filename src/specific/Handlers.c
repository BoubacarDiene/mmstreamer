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

#include "specific/Specific.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                            */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "SPECIFIC-HANDLERS"

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

typedef void (*SPECIFIC_CLICK_HANDLER_F)(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData);

typedef struct SPECIFIC_ELEMENT_DATA_S {
    uint32_t                 index;
    
    union {
        SPECIFIC_TEXT_IDS_S  text;
        SPECIFIC_IMAGE_IDS_S image;
    } ids;
    
    char                     clickHandlerName[MAX_NAME_SIZE];
    SPECIFIC_CLICK_HANDLER_F clickHandler;
    
    SPECIFIC_GETTERS_S       getters;
} SPECIFIC_ELEMENT_DATA_S;

typedef struct SPECIFIC_CLICK_HANDLER_MAP_S {
    char                     *clickHandlerName;
    SPECIFIC_CLICK_HANDLER_F clickHandler;
} SPECIFIC_CLICK_HANDLER_MAP_S;

/* -------------------------------------------------------------------------------------------- */
/*                                         PROTOTYPES                                           */
/* -------------------------------------------------------------------------------------------- */

/* Public functions */
SPECIFIC_ERROR_E initElementData_f  (SPECIFIC_S *obj, void **data);
SPECIFIC_ERROR_E uninitElementData_f(SPECIFIC_S *obj, void **data);

SPECIFIC_ERROR_E setElementGetters_f  (SPECIFIC_S *obj, void *data, SPECIFIC_GETTERS_S *getters);
SPECIFIC_ERROR_E unsetElementGetters_f(SPECIFIC_S *obj, void *data);

SPECIFIC_ERROR_E setElementTextIds_f  (SPECIFIC_S *obj, void *data, SPECIFIC_TEXT_IDS_S *textIds);
SPECIFIC_ERROR_E unsetElementTextIds_f(SPECIFIC_S *obj, void *data);

SPECIFIC_ERROR_E setElementImageIds_f  (SPECIFIC_S *obj, void *data, SPECIFIC_IMAGE_IDS_S *imageIds);
SPECIFIC_ERROR_E unsetElementImageIds_f(SPECIFIC_S *obj, void *data);

SPECIFIC_ERROR_E setClickHandler_f  (SPECIFIC_S *obj, void *data, char *clickHandlerName, uint32_t index);
SPECIFIC_ERROR_E unsetClickHandler_f(SPECIFIC_S *obj, void *data);

SPECIFIC_ERROR_E handleClick_f(CONTEXT_S *ctx, GFX_EVENT_S *gfxEvent);

/* Click handlers */
static void closeApplication(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData);
static void changeLanguage(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData);

static void hideIcon(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData);
static void showIcon(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData);

static void stopGraphics (CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData);
static void startGraphics(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData);

static void stopVideo (CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData);
static void startVideo(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData);

static void stopServers (CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData);
static void startServers(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData);

static void suspendServers(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData);
static void resumeServers (CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData);

static void stopCients  (CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData);
static void startClients(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData);

/* -------------------------------------------------------------------------------------------- */
/*                                          VARIABLES                                           */
/* -------------------------------------------------------------------------------------------- */

static SPECIFIC_CLICK_HANDLER_MAP_S gClickHandlerMap[] = {
	{ "closeApplication",                      closeApplication },
	{ "changeLanguage",                        changeLanguage   },
	{ "hideIcon",                              hideIcon         },
	{ "showIcon",                              showIcon         },
	{ "stopGraphics",                          stopGraphics     },
	{ "startGraphics",                         startGraphics    },
	{ "stopVideo",                             stopVideo        },
	{ "startVideo",                            startVideo       },
	{ "stopServers",                           stopServers      },
	{ "startServers",                          startServers     },
	{ "suspendServers",                        suspendServers   },
	{ "resumeServers",                         resumeServers    },
	{ "stopCients",                            stopCients       },
	{ "startClients",                          startClients     },
	{ NULL,                                    NULL             }
};

uint32_t gNbClickHandlers = (uint32_t)(sizeof(gClickHandlerMap) / sizeof(gClickHandlerMap[0]));

/* -------------------------------------------------------------------------------------------- */
/*                                          FUNCTIONS                                           */
/* -------------------------------------------------------------------------------------------- */

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
SPECIFIC_ERROR_E setClickHandler_f(SPECIFIC_S *obj, void *data, char *clickHandlerName, uint32_t index)
{
    assert(obj && data && clickHandlerName);

    SPECIFIC_ELEMENT_DATA_S *elementData = (SPECIFIC_ELEMENT_DATA_S*)data;

    elementData->index = index;
    strncpy(elementData->clickHandlerName, clickHandlerName, sizeof(elementData->clickHandlerName));

    uint32_t i = 0;
    while ((i < gNbClickHandlers)
            && gClickHandlerMap[i].clickHandlerName
            && (strcmp(gClickHandlerMap[i].clickHandlerName, clickHandlerName) != 0)) {
        i++;
    }

    if (gClickHandlerMap[i].clickHandlerName) {
        elementData->clickHandler = gClickHandlerMap[i].clickHandler;
    }

    return SPECIFIC_ERROR_NONE;
}

/*!
 *
 */
SPECIFIC_ERROR_E unsetClickHandler_f(SPECIFIC_S *obj, void *data)
{
    assert(obj && data);

    SPECIFIC_ELEMENT_DATA_S *elementData = (SPECIFIC_ELEMENT_DATA_S*)data;

    elementData->clickHandler = NULL;

    return SPECIFIC_ERROR_NONE;
}

/*!
 *
 */
SPECIFIC_ERROR_E handleClick_f(CONTEXT_S *ctx, GFX_EVENT_S *gfxEvent)
{
    assert(ctx && gfxEvent);

    SPECIFIC_ELEMENT_DATA_S *elementData = (SPECIFIC_ELEMENT_DATA_S*)gfxEvent->gfxElementPData;

    if (elementData && elementData->clickHandler) {
        elementData->clickHandler(ctx, gfxEvent->gfxElementName, elementData);
    }

    return SPECIFIC_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/*                                        CLICK HANDLERS                                        */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static void closeApplication(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData)
{
    assert(ctx && gfxElementName && gfxElementData);
    
    Logd("Handling click on element \"%s\"", gfxElementName);

    ctx->modules.graphicsObj->quit(ctx->modules.graphicsObj);
}

/*!
 *
 */
static void changeLanguage(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData)
{
    assert(ctx && gfxElementName && gfxElementData);
    
    SPECIFIC_ELEMENT_DATA_S *elementData = (SPECIFIC_ELEMENT_DATA_S*)gfxElementData;
    
    GRAPHICS_S *graphicsObj         = ctx->modules.graphicsObj;
    GRAPHICS_INFOS_S *graphicsInfos = &ctx->params.graphicsInfos;
    uint32_t nbGfxElements          = graphicsInfos->nbGfxElements;
    GFX_ELEMENT_S **gfxElements     = graphicsInfos->gfxElements;
    
    Logd("Handling click on element \"%s\"", gfxElementName);
    
    char nextLanguage[MIN_STR_SIZE];
    memset(nextLanguage, '\0', MIN_STR_SIZE);
    
    elementData->getters.getLanguage(elementData->getters.userData, graphicsInfos->currentLanguage, nextLanguage);
    
    Logd("Changing language from \"%s\" to \"%s\"", graphicsInfos->currentLanguage, nextLanguage);
    
    uint32_t index;
    GFX_TEXT_S text;
    SPECIFIC_ELEMENT_DATA_S *data;
    
    for (index = 0; index < nbGfxElements; index++) {
        if (gfxElements[index]->type != GFX_ELEMENT_TYPE_TEXT) {
            continue;
        }
        
        data = (SPECIFIC_ELEMENT_DATA_S*)gfxElements[index]->pData;
        
        memcpy(&text, &gfxElements[index]->data.text, sizeof(GFX_TEXT_S));
        memset(text.str, '\0', sizeof(text.str));
        elementData->getters.getString(elementData->getters.userData, data->ids.text.stringId, nextLanguage, text.str);
        
        graphicsObj->setData(graphicsObj, gfxElements[index]->name, (void*)&text);
    }
    
    memset(graphicsInfos->currentLanguage, '\0', MIN_STR_SIZE);
    strcpy(graphicsInfos->currentLanguage, nextLanguage);
}

/*!
 *
 */
static void hideIcon(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData)
{
    assert(ctx && gfxElementName && gfxElementData);
    
    SPECIFIC_ELEMENT_DATA_S *elementData = (SPECIFIC_ELEMENT_DATA_S*)gfxElementData;
    
    Logd("Handling click on element \"%s\"", gfxElementName);

    ctx->modules.graphicsObj->setVisible(ctx->modules.graphicsObj, "imageBtn", 0);
}

/*!
 *
 */
static void showIcon(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData)
{
    assert(ctx && gfxElementName && gfxElementData);
    
    SPECIFIC_ELEMENT_DATA_S *elementData = (SPECIFIC_ELEMENT_DATA_S*)gfxElementData;
    
    Logd("Handling click on element \"%s\"", gfxElementName);

    ctx->modules.graphicsObj->setVisible(ctx->modules.graphicsObj, "imageBtn", 1);
}

/*!
 *
 */
static void stopGraphics(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData)
{
    assert(ctx && gfxElementName && gfxElementData);
    
    Logd("Handling click on element \"%s\"", gfxElementName);
}

/*!
 *
 */
static void startGraphics(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData)
{
    assert(ctx && gfxElementName && gfxElementData);
    
    Logd("Handling click on element \"%s\"", gfxElementName);
}

/*!
 *
 */
static void stopVideo(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData)
{
    assert(ctx && gfxElementName && gfxElementData);
    
    Logd("Handling click on element \"%s\"", gfxElementName);
}

/*!
 *
 */
static void startVideo(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData)
{
    assert(ctx && gfxElementName && gfxElementData);
    
    Logd("Handling click on element \"%s\"", gfxElementName);
}

/*!
 *
 */
static void stopServers(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData)
{
    assert(ctx && gfxElementName && gfxElementData);
    
    Logd("Handling click on element \"%s\"", gfxElementName);
}

/*!
 *
 */
static void startServers(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData)
{
    assert(ctx && gfxElementName && gfxElementData);
    
    Logd("Handling click on element \"%s\"", gfxElementName);
}

/*!
 *
 */
static void suspendServers(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData)
{
    assert(ctx && gfxElementName && gfxElementData);
    
    SERVER_S *serverObj           = ctx->modules.serverObj;
    SERVERS_INFOS_S *serversInfos = &ctx->params.serversInfos;

    Logd("Handling click on element \"%s\"", gfxElementName);

    uint32_t index;
    for (index = 0; index < serversInfos->nbServers; index++) {
        serverObj->suspendSender(serverObj, serversInfos->serverParams[index]);
    }
}

/*!
 *
 */
static void resumeServers(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData)
{
    assert(ctx && gfxElementName && gfxElementData);
    
    SERVER_S *serverObj           = ctx->modules.serverObj;
    SERVERS_INFOS_S *serversInfos = &ctx->params.serversInfos;

    Logd("Handling click on element \"%s\"", gfxElementName);

    uint32_t index;
    for (index = 0; index < serversInfos->nbServers; index++) {
        serverObj->resumeSender(serverObj, serversInfos->serverParams[index]);
    }
}

/*!
 *
 */
static void stopCients(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData)
{
    assert(ctx && gfxElementName && gfxElementData);
    
    Logd("Handling click on element \"%s\"", gfxElementName);
}

/*!
 *
 */
static void startClients(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData)
{
    assert(ctx && gfxElementName && gfxElementData);
    
    Logd("Handling click on element \"%s\"", gfxElementName);
}
