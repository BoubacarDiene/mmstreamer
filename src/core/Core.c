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
* \file   Core.c
* \brief  TODO
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include "core/Listeners.h"
#include "core/Loaders.h"
#include "core/Core.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                            */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "CORE"

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

typedef struct CORE_PRIVATE_DATA_S {
    CONTEXT_S   *ctx;
    XML_S       xml;
    
    SPECIFIC_S  *specificObj;
    LOADERS_S   *loadersObj;
    LISTENERS_S *listenersObj;
} CORE_PRIVATE_DATA_S;

/* -------------------------------------------------------------------------------------------- */
/*                                          VARIABLES                                           */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                         PROTOTYPES                                           */
/* -------------------------------------------------------------------------------------------- */

static CORE_ERROR_E loadAllParams_f  (CORE_S *obj);
static CORE_ERROR_E unloadAllParams_f(CORE_S *obj);

static CORE_ERROR_E loadGraphicsParams_f  (CORE_S *obj);
static CORE_ERROR_E unloadGraphicsParams_f(CORE_S *obj);

static CORE_ERROR_E loadVideosParams_f  (CORE_S *obj);
static CORE_ERROR_E unloadVideosParams_f(CORE_S *obj);

static CORE_ERROR_E loadServersParams_f  (CORE_S *obj);
static CORE_ERROR_E unloadServersParams_f(CORE_S *obj);

static CORE_ERROR_E loadClientsParams_f  (CORE_S *obj);
static CORE_ERROR_E unloadClientsParams_f(CORE_S *obj);

static CORE_ERROR_E keepAppRunning_f(CORE_S *obj, KEEP_ALIVE_METHOD_E method, uint32_t timeout_s);

static void getString_f  (void *userData, uint32_t stringId, char *language, char *strOut);
static void getColor_f   (void *userData, int32_t colorId, GFX_COLOR_S *colorOut);
static void getFont_f    (void *userData, uint32_t fontId, char *ttfFontOut);
static void getImage_f   (void *userData, uint32_t imageId, GFX_IMAGE_S *imageOut);
static void getLanguage_f(void *userData, char *currentIn, char *nextOut);

/* -------------------------------------------------------------------------------------------- */
/*                                      PUBLIC FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
CORE_ERROR_E Core_Init(CORE_S **obj, CONTEXT_S *ctx)
{
    assert(obj && ctx && (*obj = calloc(1, sizeof(CORE_S))));
    
    CORE_PRIVATE_DATA_S *pData;
    assert((pData = calloc(1, sizeof(CORE_PRIVATE_DATA_S))));
    
    if (Specific_Init(&pData->specificObj, ctx) != SPECIFIC_ERROR_NONE) {
        Loge("Specific_Init() failed");
        goto specific_exit;
    }
    
    if (Loaders_Init(&pData->loadersObj) != LOADERS_ERROR_NONE) {
        Loge("Loader_Init() failed");
        goto loader_exit;
    }
    
    if (Listeners_Init(&pData->listenersObj, ctx, pData->specificObj) != LISTENERS_ERROR_NONE) {
        Loge("Listeners_Init() failed");
        goto listeners_exit;
    }
    
    (*obj)->loadAllParams        = loadAllParams_f;
    (*obj)->unloadAllParams      = unloadAllParams_f;
    
    (*obj)->loadGraphicsParams   = loadGraphicsParams_f;
    (*obj)->unloadGraphicsParams = unloadGraphicsParams_f;
    
    (*obj)->loadVideosParams     = loadVideosParams_f;
    (*obj)->unloadVideosParams   = unloadVideosParams_f;
    
    (*obj)->loadServersParams    = loadServersParams_f;
    (*obj)->unloadServersParams  = unloadServersParams_f;
    
    (*obj)->loadClientsParams    = loadClientsParams_f;
    (*obj)->unloadClientsParams  = unloadClientsParams_f;
    
    (*obj)->keepAppRunning       = keepAppRunning_f;
    
    pData->ctx = ctx;

    (*obj)->pData = (void*)pData;
    
    return CORE_ERROR_NONE;

listeners_exit:
    (void)Loaders_UnInit(&pData->loadersObj);

loader_exit:
    (void)Specific_UnInit(&pData->specificObj);

specific_exit:
    free((*obj)->pData);
    (*obj)->pData = NULL;
    
    free(*obj);
    *obj = NULL;
  
    return CORE_ERROR_INIT;
}

/*!
 *
 */
CORE_ERROR_E Core_UnInit(CORE_S **obj)
{
    assert(obj && *obj && (*obj)->pData);
    
    CORE_PRIVATE_DATA_S *pData = (CORE_PRIVATE_DATA_S*)((*obj)->pData);
    
    (void)Listeners_UnInit(&pData->listenersObj);
    (void)Loaders_UnInit(&pData->loadersObj);
    (void)Specific_UnInit(&pData->specificObj);
    
    pData->ctx = NULL;
    
    free((*obj)->pData);
    (*obj)->pData = NULL;
    
    free(*obj);
    *obj = NULL;
    
    return CORE_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/*                                     PRIVATE FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static CORE_ERROR_E loadAllParams_f(CORE_S *obj)
{
    CORE_ERROR_E ret = CORE_ERROR_NONE;
    
    if ((ret = loadGraphicsParams_f(obj)) != CORE_ERROR_NONE) {
        Loge("Failed to load graphics params");
        goto graphics_exit;
    }
    
    if ((ret = loadVideosParams_f(obj)) != CORE_ERROR_NONE) {
        Loge("Failed to load videos params");
        goto video_exit;
    }
    
    if ((ret = loadServersParams_f(obj)) != CORE_ERROR_NONE) {
        Loge("Failed to load servers params");
        goto servers_exit;
    }
    
    if ((ret = loadClientsParams_f(obj)) != CORE_ERROR_NONE) {
        Loge("Failed to load clients params");
        goto clients_exit;
    }
    
    return CORE_ERROR_NONE;

clients_exit:
    (void)unloadServersParams_f(obj);

servers_exit:
    (void)unloadVideosParams_f(obj);

video_exit:
    (void)unloadGraphicsParams_f(obj);

graphics_exit:
    return ret;
}

/*!
 *
 */
static CORE_ERROR_E unloadAllParams_f(CORE_S *obj)
{
    (void)unloadClientsParams_f(obj);
    (void)unloadServersParams_f(obj);
    (void)unloadVideosParams_f(obj);
    (void)unloadGraphicsParams_f(obj);

    return CORE_ERROR_NONE;
}

/*!
 *
 */
static CORE_ERROR_E loadGraphicsParams_f(CORE_S *obj)
{
    assert(obj && obj->pData);
    
    CORE_PRIVATE_DATA_S *pData = (CORE_PRIVATE_DATA_S*)(obj->pData);

    if (pData->loadersObj->loadGraphicsXml(pData->loadersObj, pData->ctx, &pData->xml.xmlGraphics) != LOADERS_ERROR_NONE) {
        Loge("Failed to load graphics xml");
        return CORE_ERROR_XML;
    }
    
    XML_GRAPHICS_S *xmlGraphics       = &pData->xml.xmlGraphics;
    GRAPHICS_INFOS_S *graphicsInfos   = &pData->ctx->params.graphicsInfos;
    GRAPHICS_PARAMS_S *graphicsParams = &graphicsInfos->graphicsParams;
    
    assert((graphicsInfos->currentLanguage = calloc(1, MIN_STR_SIZE)));
    strncpy(graphicsInfos->currentLanguage, xmlGraphics->common.defaultLanguage, MIN_STR_SIZE);
    
    Logd("Setting main colors");

    getColor_f((void*)&xmlGraphics->common, (int32_t)xmlGraphics->onFocusColorId, &graphicsParams->colorOnFocus);
    getColor_f((void*)&xmlGraphics->common, (int32_t)xmlGraphics->onBlurColorId, &graphicsParams->colorOnBlur);
    getColor_f((void*)&xmlGraphics->common, (int32_t)xmlGraphics->onResetColorId, &graphicsParams->colorOnReset);

    Logd("Setting screen params");
    
    GFX_SCREEN_S *screenParams = &graphicsParams->screenParams;
    
    screenParams->rect.x = 0;
    screenParams->rect.y = 0;
    screenParams->rect.w = xmlGraphics->screen.width;
    screenParams->rect.h = xmlGraphics->screen.height;

    screenParams->bitsPerPixel = xmlGraphics->screen.bitsPerPixel;
    screenParams->isFullScreen = xmlGraphics->screen.fullscreen;
    
    if (strlen(xmlGraphics->screen.caption) > 0) {
        screenParams->isTitleBarUsed = 1;
        strncpy(screenParams->caption, xmlGraphics->screen.caption, sizeof(screenParams->caption));
    }
    else {
        screenParams->isTitleBarUsed = 0;
    }
    
    getImage_f((void*)&xmlGraphics->common, xmlGraphics->screen.iconImageId, &screenParams->icon);
    
    if (xmlGraphics->screen.iconHiddenColorId >= 0) {
        assert((screenParams->icon.hiddenColor = calloc(1, sizeof(GFX_COLOR_S))));
        getColor_f((void*)&xmlGraphics->common, xmlGraphics->screen.iconHiddenColorId, screenParams->icon.hiddenColor);
    }

    if (!(screenParams->isBgImageUsed = !xmlGraphics->screen.useColor)) {
        getColor_f((void*)&xmlGraphics->common, (int32_t)xmlGraphics->screen.BgColorId, &screenParams->background.color);
    }
    else {
        getImage_f((void*)&xmlGraphics->common, xmlGraphics->screen.BgImageId, &screenParams->background.image);
        
        int32_t BgHiddenColorId = xmlGraphics->screen.BgHiddenColorId;
        if (BgHiddenColorId >= 0) {
            assert((screenParams->background.image.hiddenColor = calloc(1, sizeof(GFX_COLOR_S))));
            getColor_f((void*)&xmlGraphics->common, BgHiddenColorId, screenParams->background.image.hiddenColor);
        }
    }

    screenParams->videoFormat = xmlGraphics->screen.videoFormat;
    
    Logd("Setting graphics elements");
    
    uint32_t index;
    
    GRAPHICS_S *graphicsObj      = pData->ctx->modules.graphicsObj;
    uint32_t *nbGfxElements      = &graphicsInfos->nbGfxElements;
    GFX_ELEMENT_S ***gfxElements = &graphicsInfos->gfxElements;
    
    *nbGfxElements = xmlGraphics->nbElements;
    
    if (*nbGfxElements == 0) {
        Logw("No graphics element defined");
    }
    else {
        assert((*gfxElements = (GFX_ELEMENT_S**)calloc(*nbGfxElements, sizeof(GFX_ELEMENT_S*))));
        
        for (index = 0; index < *nbGfxElements; index++) {
            if (graphicsObj->createElement(graphicsObj, &(*gfxElements)[index]) != GRAPHICS_ERROR_NONE) {
                Loge("Failed to create graphics element");
                goto badConfig_exit;
            }
            
            (void)pData->specificObj->initElementData(pData->specificObj, &(*gfxElements)[index]->pData);
            
            strncpy((*gfxElements)[index]->name, xmlGraphics->elements[index].name, sizeof((*gfxElements)[index]->name));
            strncpy((*gfxElements)[index]->groupName, xmlGraphics->elements[index].groupName, sizeof((*gfxElements)[index]->groupName));

            (*gfxElements)[index]->redrawGroup = xmlGraphics->elements[index].redrawGroup;
            (*gfxElements)[index]->type        = xmlGraphics->elements[index].type;
	        
            switch ((*gfxElements)[index]->type) {
                case GFX_ELEMENT_TYPE_VIDEO:
                    memset(&(*gfxElements)[index]->data.buffer, '\0', sizeof(BUFFER_S));
                    break;
	                
                case GFX_ELEMENT_TYPE_IMAGE:
                    if (!xmlGraphics->elements[index].image) {
                        Loge("Bad config - image is expected");
                        goto badConfig_exit;
                    }

                    SPECIFIC_IMAGE_IDS_S imageIds;
                    imageIds.imageId       = xmlGraphics->elements[index].image->imageId;
                    imageIds.hiddenColorId = xmlGraphics->elements[index].image->hiddenColorId;
	                
                    (void)pData->specificObj->setElementImageIds(pData->specificObj, (*gfxElements)[index]->pData, &imageIds);
	                
                    getImage_f((void*)&xmlGraphics->common, imageIds.imageId, &(*gfxElements)[index]->data.image);
	                
                    if (imageIds.hiddenColorId >= 0) {
                        assert(((*gfxElements)[index]->data.image.hiddenColor = calloc(1, sizeof(GFX_COLOR_S))));
                        getColor_f((void*)&xmlGraphics->common, imageIds.hiddenColorId, (*gfxElements)[index]->data.image.hiddenColor);
                    }
                    break;
	                
                case GFX_ELEMENT_TYPE_TEXT:
                    if (!xmlGraphics->elements[index].text) {
                        Loge("Bad config - text is expected");
                        goto badConfig_exit;
                    }
                    
                    SPECIFIC_TEXT_IDS_S textIds;
                    textIds.stringId = xmlGraphics->elements[index].text->stringId;
                    textIds.fontId   = xmlGraphics->elements[index].text->fontId;
                    textIds.colorId  = xmlGraphics->elements[index].text->colorId;
	                
                    (void)pData->specificObj->setElementTextIds(pData->specificObj, (*gfxElements)[index]->pData, &textIds);
                    
                    getString_f((void*)&xmlGraphics->common, textIds.stringId, graphicsInfos->currentLanguage, (*gfxElements)[index]->data.text.str);
                    getFont_f((void*)&xmlGraphics->common, textIds.fontId, (*gfxElements)[index]->data.text.ttfFont);
	                
                    (*gfxElements)[index]->data.text.ttfFontSize = xmlGraphics->elements[index].text->size;
                    getColor_f((void*)&xmlGraphics->common, (int32_t)textIds.colorId, &(*gfxElements)[index]->data.text.color);
                    break;
	                
                default:
                    ;
            }
	        
            (*gfxElements)[index]->rect.x = xmlGraphics->elements[index].x;
            (*gfxElements)[index]->rect.y = xmlGraphics->elements[index].y;
            (*gfxElements)[index]->rect.w = xmlGraphics->elements[index].width;
            (*gfxElements)[index]->rect.h = xmlGraphics->elements[index].height;

            (*gfxElements)[index]->isVisible   = xmlGraphics->elements[index].visible;
            (*gfxElements)[index]->isClickable = xmlGraphics->elements[index].clickable;
            (*gfxElements)[index]->isFocusable = xmlGraphics->elements[index].focusable;
            (*gfxElements)[index]->hasFocus    = xmlGraphics->elements[index].hasFocus;
            
            if (xmlGraphics->elements[index].nav) {
                strncpy((*gfxElements)[index]->navLeft,  xmlGraphics->elements[index].nav->left,  sizeof((*gfxElements)[index]->navLeft));
                strncpy((*gfxElements)[index]->navUp,    xmlGraphics->elements[index].nav->up,    sizeof((*gfxElements)[index]->navUp));
                strncpy((*gfxElements)[index]->navRight, xmlGraphics->elements[index].nav->right, sizeof((*gfxElements)[index]->navRight));
                strncpy((*gfxElements)[index]->navDown,  xmlGraphics->elements[index].nav->down,  sizeof((*gfxElements)[index]->navDown));
            }
            
            SPECIFIC_GETTERS_S getters;
            getters.getString   = getString_f;
            getters.getColor    = getColor_f;
            getters.getFont     = getFont_f;
            getters.getImage    = getImage_f;
            getters.getLanguage = getLanguage_f;
            
            getters.userData    = (void*)&xmlGraphics->common;
	                
            (void)pData->specificObj->setElementGetters(pData->specificObj, (*gfxElements)[index]->pData, &getters);
            
            (void)pData->specificObj->setClickHandlers(pData->specificObj, (*gfxElements)[index]->pData,
	                                            (SPECIFIC_HANDLERS_S*)xmlGraphics->elements[index].clickHandlers,
	                                            xmlGraphics->elements[index].nbClickHandlers, index);
            
            if (graphicsObj->pushElement(graphicsObj, (*gfxElements)[index]) != GRAPHICS_ERROR_NONE) {
                Loge("Failed to push element \"%s\"", (*gfxElements)[index]->name);
                goto badConfig_exit;
            }
        }
    }
    
    Logd("Setting graphics listeners");
    
    if (pData->listenersObj->setGraphicsListeners(pData->listenersObj) != LISTENERS_ERROR_NONE) {
        Loge("Failed to set graphics listeners");
        goto badConfig_exit;
    }
    
    return CORE_ERROR_NONE;
    
badConfig_exit:
    if ((*gfxElements)) {
        for (index = 0; index < *nbGfxElements; index++) {
            if ((*gfxElements)[index]) {
                if ((*gfxElements)[index]->pData) {
                    (void)pData->specificObj->unsetClickHandlers(pData->specificObj, (*gfxElements)[index]->pData);
                    (void)pData->specificObj->unsetElementGetters(pData->specificObj, (*gfxElements)[index]->pData);
                    (void)pData->specificObj->unsetElementTextIds(pData->specificObj, (*gfxElements)[index]->pData);
                    (void)pData->specificObj->unsetElementImageIds(pData->specificObj, (*gfxElements)[index]->pData);
                    (void)pData->specificObj->uninitElementData(pData->specificObj, &(*gfxElements)[index]->pData);
                }
                if (((*gfxElements)[index]->type == GFX_ELEMENT_TYPE_IMAGE) && (*gfxElements)[index]->data.image.hiddenColor) {
                    free((*gfxElements)[index]->data.image.hiddenColor);
                    (*gfxElements)[index]->data.image.hiddenColor = NULL;
                }
            }
        }
        (void)graphicsObj->removeAll(graphicsObj);
        free((*gfxElements));
        (*gfxElements) = NULL;
    }
    
    if (screenParams->background.image.hiddenColor) {
        free(screenParams->background.image.hiddenColor);
        screenParams->background.image.hiddenColor = NULL;
    }
    
    if (screenParams->icon.hiddenColor) {
        free(screenParams->icon.hiddenColor);
        screenParams->icon.hiddenColor = NULL;
    }
    
    (void)pData->loadersObj->unloadGraphicsXml(pData->loadersObj, xmlGraphics);
    
    return CORE_ERROR_PARAMS;
}

/*!
 *
 */
static CORE_ERROR_E unloadGraphicsParams_f(CORE_S *obj)
{
    assert(obj && obj->pData);
    
    CORE_PRIVATE_DATA_S *pData      = (CORE_PRIVATE_DATA_S*)(obj->pData);
    XML_GRAPHICS_S *xmlGraphics     = &pData->xml.xmlGraphics;
    GRAPHICS_S *graphicsObj         = pData->ctx->modules.graphicsObj;
    GRAPHICS_INFOS_S *graphicsInfos = &pData->ctx->params.graphicsInfos;
    GFX_SCREEN_S *screenParams      = &graphicsInfos->graphicsParams.screenParams;
    GFX_ELEMENT_S ***gfxElements    = &graphicsInfos->gfxElements;
    uint32_t nbGfxElements          = graphicsInfos->nbGfxElements;
    
    (void)pData->listenersObj->unsetGraphicsListeners(pData->listenersObj);
    
    if (gfxElements && *gfxElements) {
        uint32_t index;
        for (index = 0; index < nbGfxElements; index++) {
            if ((*gfxElements)[index]) {
                if ((*gfxElements)[index]->pData) {
                    (void)pData->specificObj->unsetClickHandlers(pData->specificObj, (*gfxElements)[index]->pData);
                    (void)pData->specificObj->unsetElementGetters(pData->specificObj, (*gfxElements)[index]->pData);
                    (void)pData->specificObj->unsetElementTextIds(pData->specificObj, (*gfxElements)[index]->pData);
                    (void)pData->specificObj->unsetElementImageIds(pData->specificObj, (*gfxElements)[index]->pData);
                    (void)pData->specificObj->uninitElementData(pData->specificObj, &(*gfxElements)[index]->pData);
                }
                if (((*gfxElements)[index]->type == GFX_ELEMENT_TYPE_IMAGE) && (*gfxElements)[index]->data.image.hiddenColor) {
                    free((*gfxElements)[index]->data.image.hiddenColor);
                    (*gfxElements)[index]->data.image.hiddenColor = NULL;
                }
            }
        }
    
        (void)graphicsObj->removeAll(graphicsObj);
        free((*gfxElements));
        (*gfxElements) = NULL;
    }
    
    if (screenParams->background.image.hiddenColor) {
        free(screenParams->background.image.hiddenColor);
        screenParams->background.image.hiddenColor = NULL;
    }
    
    if (screenParams->icon.hiddenColor) {
        free(screenParams->icon.hiddenColor);
        screenParams->icon.hiddenColor = NULL;
    }
    
    free(graphicsInfos->currentLanguage);
    graphicsInfos->currentLanguage = NULL;
    
    (void)pData->loadersObj->unloadGraphicsXml(pData->loadersObj, xmlGraphics);
    
    return CORE_ERROR_NONE;
}

/*!
 *
 */
static CORE_ERROR_E loadVideosParams_f(CORE_S *obj)
{
    assert(obj && obj->pData);
    
    CORE_PRIVATE_DATA_S *pData = (CORE_PRIVATE_DATA_S*)(obj->pData);

    if (pData->loadersObj->loadVideosXml(pData->loadersObj, pData->ctx, &pData->xml.xmlVideos) != LOADERS_ERROR_NONE) {
        Loge("Failed to load videos xml");
        return CORE_ERROR_XML;
    }
    
    XML_VIDEOS_S *xmlVideos        = &pData->xml.xmlVideos;
    VIDEOS_INFOS_S *videosInfos    = &pData->ctx->params.videosInfos;
    VIDEO_DEVICE_S ***videoDevices = &videosInfos->devices;
    VIDEO_DEVICE_S *videoDevice    = NULL;
    uint8_t *nbDevices             = &videosInfos->nbDevices;
    VIDEO_CONFIG_S videoConfig     = { 0 };

    *nbDevices = xmlVideos->nbVideos;
    
    Logd("Setting videos params");

    assert((*videoDevices = (VIDEO_DEVICE_S**)calloc(*nbDevices, sizeof(VIDEO_DEVICE_S*))));

    uint8_t index;
    for (index = 0; index < *nbDevices; index++) {
        assert(((*videoDevices)[index] = calloc(1, sizeof(VIDEO_DEVICE_S))));

        if (pData->specificObj->getVideoConfig(pData->specificObj,
                                                &videoConfig,
                                                xmlVideos->videos[index].configChoice) != SPECIFIC_ERROR_NONE) {
            Loge("getVideoConfig() failed");
            goto badConfig_exit;
        }

        videoDevice = (*videoDevices)[index];

        strncpy(videoDevice->videoParams.name, xmlVideos->videos[index].deviceName, sizeof(videoDevice->videoParams.name));
        strncpy(videoDevice->videoParams.path, xmlVideos->videos[index].deviceSrc, sizeof(videoDevice->videoParams.path));

        videoDevice->videoParams.caps        = videoConfig.caps;
        videoDevice->videoParams.type        = videoConfig.type;
        videoDevice->videoParams.pixelformat = videoConfig.pixelformat;
        videoDevice->videoParams.colorspace  = videoConfig.colorspace;
        videoDevice->videoParams.memory      = videoConfig.memory;
        videoDevice->videoParams.awaitMode   = videoConfig.awaitMode;

        videoDevice->videoParams.priority    = xmlVideos->videos[index].priority;
        videoDevice->videoParams.desiredFps  = xmlVideos->videos[index].desiredFps;
        videoDevice->videoParams.count       = xmlVideos->videos[index].nbBuffers;

        videoDevice->videoParams.captureResolution.width  = xmlVideos->videos[index].deviceWidth;
        videoDevice->videoParams.captureResolution.height = xmlVideos->videos[index].deviceHeight;

        videoDevice->videoParams.outputResolution.width   = xmlVideos->videos[index].outputWidth;
        videoDevice->videoParams.outputResolution.height  = xmlVideos->videos[index].outputHeight;

        videoDevice->graphicsDest  = xmlVideos->videos[index].graphicsDest ? strdup(xmlVideos->videos[index].graphicsDest) : NULL;
        videoDevice->graphicsIndex = -1;
        videoDevice->serverDest    = xmlVideos->videos[index].serverDest ? strdup(xmlVideos->videos[index].serverDest) : NULL;
        videoDevice->serverIndex   = -1;
    }

    Logd("Setting video listeners");
    
    if (pData->listenersObj->setVideosListeners(pData->listenersObj) != LISTENERS_ERROR_NONE) {
        Loge("Failed to set video listeners");
        goto badConfig_exit;
    }
    
    return CORE_ERROR_NONE;
    
badConfig_exit:
    for (index = 0; index < *nbDevices; index++) {
        videoDevice = (*videoDevices)[index];

        if (videoDevice->serverDest) {
            free(videoDevice->serverDest);
            videoDevice->serverDest = NULL;
        }
    
        if (videoDevice->graphicsDest) {
            free(videoDevice->graphicsDest);
            videoDevice->graphicsDest = NULL;
        }
    }

    if (*videoDevices) {
        free(*videoDevices);
        *videoDevices = NULL;
    }

    (void)pData->loadersObj->unloadVideosXml(pData->loadersObj, xmlVideos);

    return CORE_ERROR_PARAMS;
}

/*!
 *
 */
static CORE_ERROR_E unloadVideosParams_f(CORE_S *obj)
{
    assert(obj && obj->pData);
    
    CORE_PRIVATE_DATA_S *pData     = (CORE_PRIVATE_DATA_S*)(obj->pData);
    VIDEOS_INFOS_S *videosInfos    = &pData->ctx->params.videosInfos;
    VIDEO_DEVICE_S ***videoDevices = &videosInfos->devices;
    VIDEO_DEVICE_S *videoDevice    = NULL;
    uint8_t nbDevices              = videosInfos->nbDevices;
    
    (void)pData->listenersObj->unsetVideosListeners(pData->listenersObj);
    
    uint8_t index;
    for (index = 0; index < nbDevices; index++) {
        videoDevice = (*videoDevices)[index];

        if (videoDevice->serverDest) {
            free(videoDevice->serverDest);
            videoDevice->serverDest = NULL;
        }
    
        if (videoDevice->graphicsDest) {
            free(videoDevice->graphicsDest);
            videoDevice->graphicsDest = NULL;
        }
    }

    if (*videoDevices) {
        free(*videoDevices);
        *videoDevices = NULL;
    }
    
    (void)pData->loadersObj->unloadVideosXml(pData->loadersObj, &pData->xml.xmlVideos);
    
    return CORE_ERROR_NONE;
}

/*!
 *
 */
static CORE_ERROR_E loadServersParams_f(CORE_S *obj)
{
    assert(obj && obj->pData);
    
    CORE_PRIVATE_DATA_S *pData = (CORE_PRIVATE_DATA_S*)(obj->pData);

    if (pData->loadersObj->loadServersXml(pData->loadersObj, pData->ctx, &pData->xml.xmlServers) != LOADERS_ERROR_NONE) {
        Loge("Failed to load servers xml");
        return CORE_ERROR_XML;
    }
    
    XML_SERVERS_S *xmlServers       = &pData->xml.xmlServers;
    SERVERS_INFOS_S *serversInfos   = &pData->ctx->params.serversInfos;
    SERVER_PARAMS_S ***serverParams = &serversInfos->serverParams;
    uint8_t *nbServers              = &serversInfos->nbServers;
    
    *nbServers = xmlServers->nbServers;
    
    Logd("Setting servers params");
    
    assert((*serverParams = (SERVER_PARAMS_S**)calloc(*nbServers, sizeof(SERVER_PARAMS_S*))));
    
    uint8_t index;
    for (index = 0; index < *nbServers; index++) {
        assert(((*serverParams)[index] = calloc(1, sizeof(SERVER_PARAMS_S))));
        
        strncpy((*serverParams)[index]->name,
                xmlServers->servers[index].name,
                sizeof((*serverParams)[index]->name));
        
        (*serverParams)[index]->type          = xmlServers->servers[index].type;
        (*serverParams)[index]->link          = xmlServers->servers[index].link;
        (*serverParams)[index]->mode          = xmlServers->servers[index].mode;
        (*serverParams)[index]->acceptMode    = xmlServers->servers[index].acceptMode;
        (*serverParams)[index]->priority      = xmlServers->servers[index].priority;
        (*serverParams)[index]->maxClients    = xmlServers->servers[index].maxClients;
        (*serverParams)[index]->maxBufferSize = xmlServers->servers[index].maxBufferSize;
        
        strncpy((*serverParams)[index]->mime, xmlServers->servers[index].mime, sizeof((*serverParams)[index]->mime));
        
        if (xmlServers->servers[index].host
                && xmlServers->servers[index].service
                && xmlServers->servers[index].path) {
            strncpy((*serverParams)[index]->recipient.server.host,
                    xmlServers->servers[index].host,
                    sizeof((*serverParams)[index]->recipient.server.host));

            strncpy((*serverParams)[index]->recipient.server.service,
                    xmlServers->servers[index].service,
                    sizeof((*serverParams)[index]->recipient.server.service));

            strncpy((*serverParams)[index]->recipient.server.path,
                    xmlServers->servers[index].path,
                    sizeof((*serverParams)[index]->recipient.server.path));
        }
        else if (xmlServers->servers[index].socketName) {
            strncpy((*serverParams)[index]->recipient.serverSocketName,
                    xmlServers->servers[index].socketName,
                    sizeof((*serverParams)[index]->recipient.serverSocketName));
        }
        else {
            Loge("Bad server config. Neighteer socketName nor host/service/path specified");
            goto badConfig_exit;
        }
    }
    
    Logd("Setting servers listeners");
    
    if (pData->listenersObj->setServersListeners(pData->listenersObj) != LISTENERS_ERROR_NONE) {
        Loge("Failed to set servers listeners");
        goto badConfig_exit;
    }
    
    return CORE_ERROR_NONE;
    
badConfig_exit:
    for (index = 0; index < *nbServers; index++) {
        if ((*serverParams)[index]) {
            free((*serverParams)[index]);
            (*serverParams)[index] = NULL;
        }
    }
    
    if (*serverParams) {
        free(*serverParams);
        *serverParams = NULL;
    }
    
    (void)pData->loadersObj->unloadServersXml(pData->loadersObj, xmlServers);

    return CORE_ERROR_PARAMS;
}

/*!
 *
 */
static CORE_ERROR_E unloadServersParams_f(CORE_S *obj)
{
    assert(obj && obj->pData);
    
    CORE_PRIVATE_DATA_S *pData = (CORE_PRIVATE_DATA_S*)(obj->pData);
    
    SERVERS_INFOS_S *serversInfos    = &pData->ctx->params.serversInfos;
    SERVER_PARAMS_S ***serverParams  = &serversInfos->serverParams;
    uint8_t nbServers                = serversInfos->nbServers;
    
    (void)pData->listenersObj->unsetServersListeners(pData->listenersObj);
    
    uint8_t index;
    for (index = 0; index < nbServers; index++) {
        if ((*serverParams)[index]) {
            free((*serverParams)[index]);
            (*serverParams)[index] = NULL;
        }
    }
    
    if (*serverParams) {
        free(*serverParams);
        *serverParams = NULL;
    }
    
    (void)pData->loadersObj->unloadServersXml(pData->loadersObj, &pData->xml.xmlServers);
    
    return CORE_ERROR_NONE;
}

/*!
 *
 */
static CORE_ERROR_E loadClientsParams_f(CORE_S *obj)
{
    assert(obj && obj->pData);
    
    CORE_PRIVATE_DATA_S *pData = (CORE_PRIVATE_DATA_S*)(obj->pData);

    if (pData->loadersObj->loadClientsXml(pData->loadersObj, pData->ctx, &pData->xml.xmlClients) != LOADERS_ERROR_NONE) {
        Loge("Failed to load clients xml");
        return CORE_ERROR_XML;
    }
    
    XML_CLIENTS_S *xmlClients       = &pData->xml.xmlClients;
    CLIENTS_INFOS_S *clientsInfos   = &pData->ctx->params.clientsInfos;
    CLIENT_PARAMS_S ***clientParams = &clientsInfos->clientParams;
    
    char ***graphicsDest            = &clientsInfos->graphicsDest;
    int8_t **graphicsIndex          = &clientsInfos->graphicsIndex;
    
    char ***serverDest              = &clientsInfos->serverDest;
    int8_t **serverIndex            = &clientsInfos->serverIndex;
    
    uint8_t *nbClients              = &clientsInfos->nbClients;
    
    *nbClients = xmlClients->nbClients;
    
    Logd("Setting clients params");
    
    assert((*clientParams = (CLIENT_PARAMS_S**)calloc(*nbClients, sizeof(CLIENT_PARAMS_S*))));
    assert((*graphicsDest = (char**)calloc(*nbClients, sizeof(char*))));
    assert((*serverDest   = (char**)calloc(*nbClients, sizeof(char*))));
    
    assert((*graphicsIndex = (int8_t*)calloc(*nbClients, sizeof(int8_t))));
    assert((*serverIndex   = (int8_t*)calloc(*nbClients, sizeof(int8_t))));
    
    uint8_t index;
    for (index = 0; index < *nbClients; index++) {
        assert(((*clientParams)[index] = calloc(1, sizeof(CLIENT_PARAMS_S))));
        
        strncpy((*clientParams)[index]->name,
                xmlClients->clients[index].name,
                sizeof((*clientParams)[index]->name));
        
        (*clientParams)[index]->type     = xmlClients->clients[index].type;
        (*clientParams)[index]->link     = xmlClients->clients[index].link;
        (*clientParams)[index]->mode     = xmlClients->clients[index].mode;
        (*clientParams)[index]->priority = xmlClients->clients[index].priority;
        
        if (xmlClients->clients[index].graphicsDest) {
            (*graphicsDest)[index]  = strdup(xmlClients->clients[index].graphicsDest);
            (*graphicsIndex)[index] = -1;
        }
        
        if (xmlClients->clients[index].serverDest) {
            (*serverDest)[index]    = strdup(xmlClients->clients[index].serverDest);
            (*serverIndex)[index]   = -1;
        }
        
        if (xmlClients->clients[index].serverHost
                && xmlClients->clients[index].serverService
                && xmlClients->clients[index].serverPath) {
            strncpy((*clientParams)[index]->recipient.server.host,
                    xmlClients->clients[index].serverHost,
                    sizeof((*clientParams)[index]->recipient.server.host));

            strncpy((*clientParams)[index]->recipient.server.service,
                    xmlClients->clients[index].serverService,
                    sizeof((*clientParams)[index]->recipient.server.service));

            strncpy((*clientParams)[index]->recipient.server.path,
                    xmlClients->clients[index].serverPath,
                    sizeof((*clientParams)[index]->recipient.server.path));
        }
        else if (xmlClients->clients[index].serverSocketName) {
            strncpy((*clientParams)[index]->recipient.serverSocketName,
                    xmlClients->clients[index].serverSocketName,
                    sizeof((*clientParams)[index]->recipient.serverSocketName));
        }
        else {
            Loge("Bad client config. Neighteer socketName nor host/service/path specified");
            goto badConfig_exit;
        }
    }
    
    Logd("Setting clients listeners");
    
    if (pData->listenersObj->setClientsListeners(pData->listenersObj) != LISTENERS_ERROR_NONE) {
        Loge("Failed to set clients listeners");
        goto badConfig_exit;
    }
    
    return CORE_ERROR_NONE;
    
badConfig_exit:
    for (index = 0; index < *nbClients; index++) {
        if ((*clientParams)[index]) {
            free((*clientParams)[index]);
            (*clientParams)[index] = NULL;
        }
        
        if ((*graphicsDest)[index]) {
            free((*graphicsDest)[index]);
            (*graphicsDest)[index] = NULL;
        }
        
        if ((*serverDest)[index]) {
            free((*serverDest)[index]);
            (*serverDest)[index] = NULL;
        }
    }
    
    if (*clientParams) {
        free(*clientParams);
        *clientParams = NULL;
    }
    
    if (*graphicsDest) {
        free(*graphicsDest);
        *graphicsDest = NULL;
    }
    
    if (*graphicsIndex) {
        free(*graphicsIndex);
        *graphicsIndex = NULL;
    }
    
    if (*serverDest) {
        free(*serverDest);
        *serverDest = NULL;
    }
    
    if (*serverIndex) {
        free(*serverIndex);
        *serverIndex = NULL;
    }
    
    (void)pData->loadersObj->unloadClientsXml(pData->loadersObj, xmlClients);

    return CORE_ERROR_PARAMS;
}

/*!
 *
 */
static CORE_ERROR_E unloadClientsParams_f(CORE_S *obj)
{
    assert(obj && obj->pData);
    
    CORE_PRIVATE_DATA_S *pData  = (CORE_PRIVATE_DATA_S*)(obj->pData);
    
    CLIENTS_INFOS_S *clientsInfos    = &pData->ctx->params.clientsInfos;
    CLIENT_PARAMS_S ***clientParams  = &clientsInfos->clientParams;
    uint8_t nbClients                = clientsInfos->nbClients;
    
    (void)pData->listenersObj->unsetClientsListeners(pData->listenersObj);
    
    uint8_t index;
    for (index = 0; index < nbClients; index++) {
        if ((*clientParams)[index]) {
            free((*clientParams)[index]);
            (*clientParams)[index] = NULL;
        }
        
        if (clientsInfos->graphicsDest[index]) {
            free(clientsInfos->graphicsDest[index]);
            clientsInfos->graphicsDest[index] = NULL;
        }
        
        if (clientsInfos->serverDest[index]) {
            free(clientsInfos->serverDest[index]);
            clientsInfos->serverDest[index] = NULL;
        }
    }
    
    if (*clientParams) {
        free(*clientParams);
        *clientParams = NULL;
    }
    
    if (clientsInfos->graphicsDest) {
        free(clientsInfos->graphicsDest);
        clientsInfos->graphicsDest = NULL;
    }
    
    if (clientsInfos->graphicsIndex) {
        free(clientsInfos->graphicsIndex);
        clientsInfos->graphicsIndex = NULL;
    }
    
    if (clientsInfos->serverDest) {
        free(clientsInfos->serverDest);
        clientsInfos->serverDest = NULL;
    }
    
    if (clientsInfos->serverIndex) {
        free(clientsInfos->serverIndex);
        clientsInfos->serverIndex = NULL;
    }
    
    (void)pData->loadersObj->unloadClientsXml(pData->loadersObj, &pData->xml.xmlClients);
    
    return CORE_ERROR_NONE;
}

/*!
 *
 */
static CORE_ERROR_E keepAppRunning_f(CORE_S *obj, KEEP_ALIVE_METHOD_E method, uint32_t timeout_s)
{
    assert(obj && obj->pData);
    
    CORE_ERROR_E ret           = CORE_ERROR_NONE;
    CORE_PRIVATE_DATA_S *pData = (CORE_PRIVATE_DATA_S*)(obj->pData);
    CONTEXT_S *ctx             = pData->ctx;
    GRAPHICS_S *graphicsObj    = ctx->modules.graphicsObj;

    switch (method) {
        case KEEP_ALIVE_EVENTS_BASED:
            if (graphicsObj && graphicsObj->handleEvents(graphicsObj) != GRAPHICS_ERROR_NONE) {
                Loge("Error occurred while handling events");
                ret = CORE_ERROR_KEEP_ALIVE;
            }
            break;
            
        case KEEP_ALIVE_SEMAPHORE_BASED:
            if (sem_init(&ctx->keepAliveSem, 0, 0) != 0) {
                Loge("sem_init() failed");
                ret = CORE_ERROR_KEEP_ALIVE;
                break;
            }
            sem_wait(&ctx->keepAliveSem);
            (void)sem_destroy(&ctx->keepAliveSem);
            break;
            
        case KEEP_ALIVE_TIMER_BASED:
            sleep(timeout_s);
            break;
            
        default:
            Loge("Error occurred while handling events");
            ret = CORE_ERROR_KEEP_ALIVE;
    }
    
    return ret;
}

/*!
 *
 */
static void getString_f(void *userData, uint32_t stringId, char *language, char *strOut)
{
    assert(userData && language && strOut);
    
    XML_COMMON_S *common = (XML_COMMON_S*)userData;
    uint32_t defaultLanguageIndex = 0;
    
    uint32_t index;
    for (index = 0; index < common->nbLanguages; index++) {
        if (!strcmp(common->xmlStrings[index].language, language)) {
            strcpy(strOut, common->xmlStrings[index].strings[stringId].str);
            break;
        }
        else if (!strcmp(common->xmlStrings[index].language, common->defaultLanguage)) {
            defaultLanguageIndex = index;
        }
    }
    
    if (index == common->nbLanguages) {
        Loge("Language \"%s\" not found - Using default language ( \"%s\" )", language, common->defaultLanguage);
        strcpy(strOut, common->xmlStrings[defaultLanguageIndex].strings[stringId].str);
    }
}

/*!
 *
 */
static void getColor_f(void *userData, int32_t colorId, GFX_COLOR_S *colorOut)
{
    assert(userData && colorOut);
    
    XML_COMMON_S *common = (XML_COMMON_S*)userData;
    
    colorOut->red   = common->xmlColors.colors[colorId].red;
	colorOut->green = common->xmlColors.colors[colorId].green;
	colorOut->blue  = common->xmlColors.colors[colorId].blue;
}

/*!
 *
 */
static void getFont_f(void *userData, uint32_t fontId, char *ttfFontOut)
{
    assert(userData && ttfFontOut);
    
    XML_COMMON_S *common = (XML_COMMON_S*)userData;
    
    strcpy(ttfFontOut, common->xmlFonts.fonts[fontId].file);
}

/*!
 *
 */
static void getImage_f(void *userData, uint32_t imageId, GFX_IMAGE_S *imageOut)
{
    assert(userData && imageOut);
    
    XML_COMMON_S *common = (XML_COMMON_S*)userData;
    
    strncpy(imageOut->path, common->xmlImages.images[imageId].file, sizeof(imageOut->path));
	imageOut->format = common->xmlImages.images[imageId].format;
}

/*!
 *
 */
static void getLanguage_f(void *userData, char *currentIn, char *nextOut)
{
    assert(userData && currentIn && nextOut);
    
    XML_COMMON_S *common = (XML_COMMON_S*)userData;
    
    uint32_t index;
    for (index = 0; index < common->nbLanguages; index++) {
        if (!strcmp(currentIn, common->xmlStrings[index].language)) {
            break;
        }
    }
    
    strcpy(nextOut, common->xmlStrings[(index + 1) % common->nbLanguages].language);
}
