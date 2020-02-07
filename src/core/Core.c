//////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                              //
//              Copyright © 2016, 2018 Boubacar DIENE                                           //
//                                                                                              //
//              This file is part of mmstreamer project.                                        //
//                                                                                              //
//              mmstreamer is free software: you can redistribute it and/or modify              //
//              it under the terms of the GNU General Public License as published by            //
//              the Free Software Foundation, either version 2 of the License, or               //
//              (at your option) any later version.                                             //
//                                                                                              //
//              mmstreamer is distributed in the hope that it will be useful,                   //
//              but WITHOUT ANY WARRANTY; without even the implied warranty of                  //
//              MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                   //
//              GNU General Public License for more details.                                    //
//                                                                                              //
//              You should have received a copy of the GNU General Public License               //
//              along with mmstreamer. If not, see <http://www.gnu.org/licenses/>               //
//              or write to the Free Software Foundation, Inc., 51 Franklin Street,             //
//              51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.                   //
//                                                                                              //
//////////////////////////////////////////////////////////////////////////////////////////////////

/*!
* \file Core.c
* \brief TODO
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// HEADERS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#include "core/Configs.h"
#include "core/Listeners.h"
#include "core/Loaders.h"
#include "core/Core.h"

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// MACROS ////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "Core"

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// TYPES /////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

struct core_private_data_s {
    struct context_s   *ctx;
    struct xml_s       xml;

    struct configs_s   *configsObj;
    struct control_s   *controlObj;
    struct loaders_s   *loadersObj;
    struct listeners_s *listenersObj;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////// PUBLIC FUNCTIONS PROTOTYPES //////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

static enum core_error_e loadAllParams_f(struct core_s *obj);
static enum core_error_e unloadAllParams_f(struct core_s *obj);

static enum core_error_e loadGraphicsParams_f(struct core_s *obj);
static enum core_error_e unloadGraphicsParams_f(struct core_s *obj);

static enum core_error_e loadVideosParams_f(struct core_s *obj);
static enum core_error_e unloadVideosParams_f(struct core_s *obj);

static enum core_error_e loadServersParams_f(struct core_s *obj);
static enum core_error_e unloadServersParams_f(struct core_s *obj);

static enum core_error_e loadClientsParams_f(struct core_s *obj);
static enum core_error_e unloadClientsParams_f(struct core_s *obj);

static enum core_error_e keepAppRunning_f(struct core_s *obj, enum keep_alive_method_e method,
                                          uint32_t timeout_s);

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////// PRIVATE FUNCTIONS PROTOTYPES /////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

static void getString_f(void *userData, uint32_t stringId, char *language, char *strOut);
static void getColor_f(void *userData, int32_t colorId, struct gfx_color_s *colorOut);
static void getFont_f(void *userData, uint32_t fontId, char *ttfFontOut);
static void getImage_f(void *userData, uint32_t imageId, struct gfx_image_s *imageOut);
static void getLanguage_f(void *userData, char *currentIn, char *nextOut);

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// INITIALIZER //////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
enum core_error_e Core_Init(struct core_s **obj, struct context_s *ctx)
{
    ASSERT(obj && ctx && (*obj = calloc(1, sizeof(struct core_s))));
    
    struct core_private_data_s *pData;
    ASSERT((pData = calloc(1, sizeof(struct core_private_data_s))));
    
    if (Configs_Init(&pData->configsObj) != CONFIGS_ERROR_NONE) {
        Loge("Configs_Init() failed");
        goto configs_exit;
    }

    if (Control_Init(&pData->controlObj, ctx) != CONTROL_ERROR_NONE) {
        Loge("Control_Init() failed");
        goto control_exit;
    }
    
    if (Loaders_Init(&pData->loadersObj) != LOADERS_ERROR_NONE) {
        Loge("Loader_Init() failed");
        goto loader_exit;
    }
    
    struct listeners_params_s listenersParams = {
        .ctx        = ctx,
        .controlObj = pData->controlObj
    };

    if (Listeners_Init(&pData->listenersObj, &listenersParams) != LISTENERS_ERROR_NONE) {
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
    (void)Control_UnInit(&pData->controlObj);

control_exit:
    (void)Configs_UnInit(&pData->configsObj);

configs_exit:
    free(pData);
    free(*obj);
    *obj = NULL;
  
    return CORE_ERROR_INIT;
}

/*!
 *
 */
enum core_error_e Core_UnInit(struct core_s **obj)
{
    ASSERT(obj && *obj && (*obj)->pData);
    
    struct core_private_data_s *pData = (struct core_private_data_s*)((*obj)->pData);
    
    (void)Listeners_UnInit(&pData->listenersObj);
    (void)Loaders_UnInit(&pData->loadersObj);
    (void)Control_UnInit(&pData->controlObj);
    (void)Configs_UnInit(&pData->configsObj);
    
    free(pData);
    free(*obj);
    *obj = NULL;
    
    return CORE_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////// PUBLIC FUNCTIONS IMPLEMENTATION ///////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static enum core_error_e loadAllParams_f(struct core_s *obj)
{
    enum core_error_e ret = CORE_ERROR_NONE;
    
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
static enum core_error_e unloadAllParams_f(struct core_s *obj)
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
static enum core_error_e loadGraphicsParams_f(struct core_s *obj)
{
    ASSERT(obj && obj->pData);
    
    struct core_private_data_s *pData = (struct core_private_data_s*)(obj->pData);
    enum core_error_e ret             = CORE_ERROR_PARAMS;

    if (pData->loadersObj->loadGraphicsXml(pData->loadersObj,
                                           pData->ctx,
                                           &pData->xml.xmlGraphics) != LOADERS_ERROR_NONE) {
        Loge("Failed to load graphics xml");
        return CORE_ERROR_XML;
    }
    
    struct xml_common_s *xmlCommon     = &pData->xml.xmlCommon;
    struct xml_graphics_s *xmlGraphics = &pData->xml.xmlGraphics;

    snprintf(xmlCommon->defaultLanguage,
             sizeof(xmlCommon->defaultLanguage), "%s", xmlGraphics->defaultLanguage);

    const char *colorsXmlFile = (xmlGraphics->colorsXmlFile ? xmlGraphics->colorsXmlFile : "");
    snprintf(xmlCommon->files.colors, sizeof(xmlCommon->files.colors), "%s", colorsXmlFile);

    const char *imagesXmlFile = (xmlGraphics->imagesXmlFile ? xmlGraphics->imagesXmlFile : "");
    snprintf(xmlCommon->files.images, sizeof(xmlCommon->files.images), "%s", imagesXmlFile);

    const char *fontsXmlFile = (xmlGraphics->fontsXmlFile ? xmlGraphics->fontsXmlFile : "");
    snprintf(xmlCommon->files.fonts, sizeof(xmlCommon->files.fonts), "%s", fontsXmlFile);

    const char *stringsXmlFile = (xmlGraphics->stringsXmlFile ? xmlGraphics->stringsXmlFile : "");
    snprintf(xmlCommon->files.strings, sizeof(xmlCommon->files.strings), "%s", stringsXmlFile);

    if (pData->loadersObj->loadCommonXml(pData->loadersObj,
                                         pData->ctx, xmlCommon) != LOADERS_ERROR_NONE) {
        Loge("Failed to load common xml");
        ret = CORE_ERROR_XML;
        goto unloadXml_exit;
    }

    struct graphics_infos_s *graphicsInfos   = &pData->ctx->params.graphicsInfos;
    struct graphics_params_s *graphicsParams = &graphicsInfos->graphicsParams;
    
    ASSERT((graphicsInfos->currentLanguage = calloc(1, MIN_STR_SIZE)));
    strncpy(graphicsInfos->currentLanguage, xmlCommon->defaultLanguage, MIN_STR_SIZE);
    
    Logd("Setting main colors");

    getColor_f((void*)xmlCommon, (int32_t)xmlGraphics->onFocusColorId,
                                 &graphicsParams->colorOnFocus);
    getColor_f((void*)xmlCommon, (int32_t)xmlGraphics->onBlurColorId,
                                 &graphicsParams->colorOnBlur);
    getColor_f((void*)xmlCommon, (int32_t)xmlGraphics->onResetColorId,
                                 &graphicsParams->colorOnReset);

    Logd("Setting screen params");
    
    struct gfx_screen_s *screenParams = &graphicsParams->screenParams;

    // Screen
    if (xmlGraphics->screen.name) {
        strncpy(screenParams->name, xmlGraphics->screen.name, sizeof(screenParams->name));
    }

    screenParams->rect.x = 0;
    screenParams->rect.y = 0;
    screenParams->rect.w = xmlGraphics->screen.width;
    screenParams->rect.h = xmlGraphics->screen.height;

    if (xmlGraphics->screen.fbDeviceName && ((xmlGraphics->screen.fbDeviceName)[0] != '\0')) {
        strncpy(screenParams->fbDeviceName,
                xmlGraphics->screen.fbDeviceName, sizeof(screenParams->fbDeviceName));
    }

    screenParams->bitsPerPixel = xmlGraphics->screen.bitsPerPixel;
    screenParams->isFullScreen = xmlGraphics->screen.fullscreen;
    screenParams->showCursor   = xmlGraphics->screen.showCursor;
    
    if (xmlGraphics->screen.caption && ((xmlGraphics->screen.caption)[0] != '\0')) {
        screenParams->isTitleBarUsed = 1;
        strncpy(screenParams->caption, xmlGraphics->screen.caption, sizeof(screenParams->caption));
    }
    else {
        screenParams->isTitleBarUsed = 0;
    }
    
    // Icon
    getImage_f((void*)xmlCommon, xmlGraphics->screen.iconImageId, &screenParams->icon);
    
    if (xmlGraphics->screen.iconHiddenColorId >= 0) {
        ASSERT((screenParams->icon.hiddenColor = calloc(1, sizeof(struct gfx_color_s))));
        getColor_f((void*)xmlCommon, xmlGraphics->screen.iconHiddenColorId,
                                     screenParams->icon.hiddenColor);
    }

    // Background
    if (!(screenParams->isBgImageUsed = !xmlGraphics->screen.useColor)) {
        getColor_f((void*)xmlCommon, (int32_t)xmlGraphics->screen.bgColorId,
                                     &screenParams->background.color);
    }
    else {
        getImage_f((void*)xmlCommon, xmlGraphics->screen.bgImageId,
                                     &screenParams->background.image);
        
        int32_t bgHiddenColorId = xmlGraphics->screen.bgHiddenColorId;
        if (bgHiddenColorId >= 0) {
            screenParams->background.image.hiddenColor = calloc(1, sizeof(struct gfx_color_s));
            ASSERT(screenParams->background.image.hiddenColor);
            getColor_f((void*)xmlCommon,
                       bgHiddenColorId, screenParams->background.image.hiddenColor);
        }
    }

    // GfxVideo
    struct gfx_video_s *gfxVideo = &screenParams->video;

    strncpy(gfxVideo->name, xmlGraphics->screen.gfxVideo.name, sizeof(gfxVideo->name));
    gfxVideo->pixelFormat = xmlGraphics->screen.gfxVideo.pixelFormat;

    if (!(gfxVideo->isBgImageUsed = !xmlGraphics->screen.gfxVideo.useColor)) {
        getColor_f((void*)xmlCommon, (int32_t)xmlGraphics->screen.gfxVideo.bgColorId,
                                     &gfxVideo->background.color);
    }
    else {
        getImage_f((void*)xmlCommon, xmlGraphics->screen.gfxVideo.bgImageId,
                                     &gfxVideo->background.image);
        
        int32_t gfxVideoBgHiddenColorId = xmlGraphics->screen.gfxVideo.bgHiddenColorId;
        if (gfxVideoBgHiddenColorId >= 0) {
            gfxVideo->background.image.hiddenColor = calloc(1, sizeof(struct gfx_color_s));
            ASSERT(gfxVideo->background.image.hiddenColor);
            getColor_f((void*)xmlCommon,
                       gfxVideoBgHiddenColorId, gfxVideo->background.image.hiddenColor);
        }
    }
    
    gfxVideo->rect.x = xmlGraphics->screen.gfxVideo.x;
    gfxVideo->rect.y = xmlGraphics->screen.gfxVideo.y;
    gfxVideo->rect.w = xmlGraphics->screen.gfxVideo.width;
    gfxVideo->rect.h = xmlGraphics->screen.gfxVideo.height;
    
    Logd("Setting graphics elements");
    
    uint32_t index;
    
    struct graphics_s *graphicsObj      = pData->ctx->modules.graphicsObj;
    uint32_t *nbGfxElements             = &graphicsInfos->nbGfxElements;
    struct gfx_element_s ***gfxElements = &graphicsInfos->gfxElements;
    
    *nbGfxElements = xmlGraphics->nbElements;
    
    if (*nbGfxElements == 0) {
        Logw("No graphics element defined");
    }
    else {
        ASSERT((*gfxElements = calloc(*nbGfxElements, sizeof(struct gfx_element_s*))));
        
        for (index = 0; index < *nbGfxElements; index++) {
            if (graphicsObj->createElement(graphicsObj,
                                           &(*gfxElements)[index]) != GRAPHICS_ERROR_NONE) {
                Loge("Failed to create graphics element");
                goto badConfig_exit;
            }
            
            (void)pData->controlObj->initElementData(pData->controlObj,
                                                     &(*gfxElements)[index]->pData);
            
            strncpy((*gfxElements)[index]->name, xmlGraphics->elements[index].name,
                                                 sizeof((*gfxElements)[index]->name));
            strncpy((*gfxElements)[index]->groupName, xmlGraphics->elements[index].groupName,
                                                      sizeof((*gfxElements)[index]->groupName));

            (*gfxElements)[index]->redrawGroup = xmlGraphics->elements[index].redrawGroup;
            (*gfxElements)[index]->type        = xmlGraphics->elements[index].type;
	        
            switch ((*gfxElements)[index]->type) {
                case GFX_ELEMENT_TYPE_IMAGE:
                    if (!xmlGraphics->elements[index].image) {
                        Loge("Bad config - image is expected");
                        goto badConfig_exit;
                    }

                    struct control_image_ids_s imageIds;
                    imageIds.imageId       = xmlGraphics->elements[index].image->imageId;
                    imageIds.hiddenColorId = xmlGraphics->elements[index].image->hiddenColorId;
	                
                    (void)pData->controlObj->setElementImageIds(pData->controlObj,
                                                                (*gfxElements)[index]->pData,
                                                                &imageIds);
	                
                    getImage_f((void*)xmlCommon, imageIds.imageId,
                                                 &(*gfxElements)[index]->data.image);
	                
                    if (imageIds.hiddenColorId >= 0) {
                        (*gfxElements)[index]->data.image.hiddenColor = calloc(1, sizeof(struct gfx_color_s));
                        ASSERT((*gfxElements)[index]->data.image.hiddenColor);
                        getColor_f((void*)xmlCommon, imageIds.hiddenColorId,
                                                     (*gfxElements)[index]->data.image.hiddenColor);
                    }
                    break;
	                
                case GFX_ELEMENT_TYPE_TEXT:
                    if (!xmlGraphics->elements[index].text) {
                        Loge("Bad config - text is expected");
                        goto badConfig_exit;
                    }
                    
                    struct control_text_ids_s textIds;
                    textIds.stringId = xmlGraphics->elements[index].text->stringId;
                    textIds.fontId   = xmlGraphics->elements[index].text->fontId;
                    textIds.colorId  = xmlGraphics->elements[index].text->colorId;
	                
                    (void)pData->controlObj->setElementTextIds(pData->controlObj,
                                                               (*gfxElements)[index]->pData,
                                                               &textIds);
                    
                    getString_f((void*)xmlCommon, textIds.stringId,
                                                  graphicsInfos->currentLanguage,
                                                  (*gfxElements)[index]->data.text.str);
                    getFont_f((void*)xmlCommon, textIds.fontId,
                                                (*gfxElements)[index]->data.text.ttfFont);
	                
                    (*gfxElements)[index]->data.text.ttfFontSize = xmlGraphics->elements[index].text->size;
                    getColor_f((void*)xmlCommon, (int32_t)textIds.colorId,
                                                 &(*gfxElements)[index]->data.text.color);
                    break;

                case GFX_ELEMENT_TYPE_VIDEO:
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
                strncpy((*gfxElements)[index]->nav.left, xmlGraphics->elements[index].nav->left,
                                                         sizeof((*gfxElements)[index]->nav.left));
                strncpy((*gfxElements)[index]->nav.up, xmlGraphics->elements[index].nav->up,
                                                       sizeof((*gfxElements)[index]->nav.up));
                strncpy((*gfxElements)[index]->nav.right, xmlGraphics->elements[index].nav->right,
                                                          sizeof((*gfxElements)[index]->nav.right));
                strncpy((*gfxElements)[index]->nav.down, xmlGraphics->elements[index].nav->down,
                                                         sizeof((*gfxElements)[index]->nav.down));
            }

            struct control_getters_s getters;
            getters.getString   = getString_f;
            getters.getColor    = getColor_f;
            getters.getFont     = getFont_f;
            getters.getImage    = getImage_f;
            getters.getLanguage = getLanguage_f;
            getters.userData    = (void*)xmlCommon;
	                
            (void)pData->controlObj->setElementGetters(pData->controlObj,
                                                       (*gfxElements)[index]->pData, &getters);
            (void)pData->controlObj->setCommandHandlers(pData->controlObj,
                                (*gfxElements)[index]->pData,
                                (struct handlers_id_s*)xmlGraphics->elements[index].clickHandlers,
	                            xmlGraphics->elements[index].nbClickHandlers, index);
            
            if (graphicsObj->pushElement(graphicsObj,
                                         (*gfxElements)[index]) != GRAPHICS_ERROR_NONE) {
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
    
    ret = CORE_ERROR_NONE;
    goto unloadXml_exit;

badConfig_exit:
    if ((*gfxElements)) {
        for (index = 0; index < *nbGfxElements; index++) {
            if ((*gfxElements)[index]) {
                if ((*gfxElements)[index]->pData) {
                    (void)pData->controlObj->unsetCommandHandlers(pData->controlObj,
                                                                  (*gfxElements)[index]->pData);
                    (void)pData->controlObj->unsetElementGetters(pData->controlObj,
                                                                 (*gfxElements)[index]->pData);
                    (void)pData->controlObj->unsetElementTextIds(pData->controlObj,
                                                                 (*gfxElements)[index]->pData);
                    (void)pData->controlObj->unsetElementImageIds(pData->controlObj,
                                                                  (*gfxElements)[index]->pData);
                    (void)pData->controlObj->uninitElementData(pData->controlObj,
                                                               &(*gfxElements)[index]->pData);
                }
                if (((*gfxElements)[index]->type == GFX_ELEMENT_TYPE_IMAGE)
                    && (*gfxElements)[index]->data.image.hiddenColor) {
                    free((*gfxElements)[index]->data.image.hiddenColor);
                }
            }
        }
        (void)graphicsObj->removeAll(graphicsObj);
        free((*gfxElements));
        (*gfxElements) = NULL;
    }
    
    if (gfxVideo->background.image.hiddenColor) {
        free(gfxVideo->background.image.hiddenColor);
    }
    
    if (screenParams->background.image.hiddenColor) {
        free(screenParams->background.image.hiddenColor);
        screenParams->background.image.hiddenColor = NULL;
    }
    
    if (screenParams->icon.hiddenColor) {
        free(screenParams->icon.hiddenColor);
        screenParams->icon.hiddenColor = NULL;
    }

unloadXml_exit:
    (void)pData->loadersObj->unloadGraphicsXml(pData->loadersObj, xmlGraphics);
    
    return ret;
}

/*!
 *
 */
static enum core_error_e unloadGraphicsParams_f(struct core_s *obj)
{
    ASSERT(obj && obj->pData);
    
    struct core_private_data_s *pData      = (struct core_private_data_s*)(obj->pData);
    struct xml_common_s *xmlCommon         = &pData->xml.xmlCommon;
    struct graphics_s *graphicsObj         = pData->ctx->modules.graphicsObj;
    struct graphics_infos_s *graphicsInfos = &pData->ctx->params.graphicsInfos;
    struct gfx_screen_s *screenParams      = &graphicsInfos->graphicsParams.screenParams;
    struct gfx_element_s ***gfxElements    = &graphicsInfos->gfxElements;
    uint32_t nbGfxElements                 = graphicsInfos->nbGfxElements;
    
    (void)pData->listenersObj->unsetGraphicsListeners(pData->listenersObj);

    if (gfxElements && *gfxElements) {
        uint32_t index;
        for (index = 0; index < nbGfxElements; index++) {
            if ((*gfxElements)[index]) {
                if ((*gfxElements)[index]->pData) {
                    (void)pData->controlObj->unsetCommandHandlers(pData->controlObj,
                                                                  (*gfxElements)[index]->pData);
                    (void)pData->controlObj->unsetElementGetters(pData->controlObj,
                                                                 (*gfxElements)[index]->pData);
                    (void)pData->controlObj->unsetElementTextIds(pData->controlObj,
                                                                 (*gfxElements)[index]->pData);
                    (void)pData->controlObj->unsetElementImageIds(pData->controlObj,
                                                                  (*gfxElements)[index]->pData);
                    (void)pData->controlObj->uninitElementData(pData->controlObj,
                                                               &(*gfxElements)[index]->pData);
                }
                if (((*gfxElements)[index]->type == GFX_ELEMENT_TYPE_IMAGE)
                    && (*gfxElements)[index]->data.image.hiddenColor) {
                    free((*gfxElements)[index]->data.image.hiddenColor);
                }
            }
        }
    
        (void)graphicsObj->removeAll(graphicsObj);
        free((*gfxElements));
        (*gfxElements) = NULL;
    }
    
    if (screenParams->video.background.image.hiddenColor) {
        free(screenParams->video.background.image.hiddenColor);
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

    (void)pData->loadersObj->unloadCommonXml(pData->loadersObj, xmlCommon);

    return CORE_ERROR_NONE;
}

/*!
 *
 */
static enum core_error_e loadVideosParams_f(struct core_s *obj)
{
    ASSERT(obj && obj->pData);
    
    struct core_private_data_s *pData = (struct core_private_data_s*)(obj->pData);
    enum core_error_e ret             = CORE_ERROR_NONE;

    if (pData->loadersObj->loadVideosXml(pData->loadersObj, pData->ctx,
                                         &pData->xml.xmlVideos) != LOADERS_ERROR_NONE) {
        Loge("Failed to load videos xml");
        return CORE_ERROR_XML;
    }
    
    struct xml_videos_s *xmlVideos        = &pData->xml.xmlVideos;
    struct videos_infos_s *videosInfos    = &pData->ctx->params.videosInfos;
    struct video_device_s ***videoDevices = &videosInfos->devices;
    struct video_device_s *videoDevice    = NULL;
    uint8_t *nbDevices                    = &videosInfos->nbDevices;
    struct video_config_s videoConfig     = {0};

    *nbDevices = xmlVideos->nbVideos;
    
    Logd("Setting videos params");

    ASSERT((*videoDevices = calloc(*nbDevices, sizeof(struct video_device_s*))));

    uint32_t index, configId;
    struct video_config_choice_s *configChoice;
    for (index = 0; index < *nbDevices; index++) {
        ASSERT(((*videoDevices)[index] = calloc(1, sizeof(struct video_device_s))));

        if (xmlVideos->videos[index].configChoice >= xmlVideos->nbConfigs) {
            Loge("Bad video config choice - Choice : %u / Max : %u",
                    xmlVideos->videos[index].configChoice, xmlVideos->nbConfigs - 1);
            goto badConfig_exit;
        }

        configId     = xmlVideos->videos[index].configChoice;
        configChoice = (struct video_config_choice_s*)&xmlVideos->configs[configId];

        if (pData->configsObj->getVideoConfig(pData->configsObj, &videoConfig,
                                              configChoice) != CONFIGS_ERROR_NONE) {
            Loge("getVideoConfig() failed");
            goto badConfig_exit;
        }

        videoDevice = (*videoDevices)[index];

        strncpy(videoDevice->videoParams.name, xmlVideos->videos[index].deviceName,
                                               sizeof(videoDevice->videoParams.name));
        strncpy(videoDevice->videoParams.path, xmlVideos->videos[index].deviceSrc,
                                               sizeof(videoDevice->videoParams.path));

        videoDevice->videoParams.caps        = videoConfig.caps;
        videoDevice->videoParams.type        = videoConfig.type;
        videoDevice->videoParams.pixelformat = videoConfig.pixelformat;
        videoDevice->videoParams.colorspace  = videoConfig.colorspace;
        videoDevice->videoParams.memory      = videoConfig.memory;
        videoDevice->videoParams.awaitMode   = videoConfig.awaitMode;

        videoDevice->videoParams.priority    = xmlVideos->videos[index].priority;
        videoDevice->videoParams.desiredFps  = xmlVideos->videos[index].desiredFps;
        videoDevice->videoParams.count       = xmlVideos->videos[index].nbBuffers;

        memcpy(&videoDevice->videoParams.captureArea,
                    &xmlVideos->videos[index].deviceArea,
                    sizeof(videoDevice->videoParams.captureArea));

        memcpy(&videoDevice->videoParams.croppingArea,
                    &xmlVideos->videos[index].croppingArea,
                    sizeof(videoDevice->videoParams.croppingArea));

        memcpy(&videoDevice->videoParams.composingArea,
                    &xmlVideos->videos[index].composingArea,
                    sizeof(videoDevice->videoParams.composingArea));

        videoDevice->graphicsDest  = NULL;
        videoDevice->graphicsIndex = -1;
        if (xmlVideos->videos[index].graphicsDest) {
            videoDevice->graphicsDest = strdup(xmlVideos->videos[index].graphicsDest);
        }
        
        videoDevice->serverDest  = NULL;
        videoDevice->serverIndex = -1;
        if (xmlVideos->videos[index].serverDest) {
            videoDevice->serverDest = strdup(xmlVideos->videos[index].serverDest);
        }
    }

    Logd("Setting video listeners");
    
    if (pData->listenersObj->setVideosListeners(pData->listenersObj) != LISTENERS_ERROR_NONE) {
        Loge("Failed to set video listeners");
        goto badConfig_exit;
    }

    ret = CORE_ERROR_NONE;
    goto unloadXml_exit;
    
badConfig_exit:
    for (index = 0; index < *nbDevices; index++) {
        if (((*videoDevices)[index])->serverDest) {
            free(((*videoDevices)[index])->serverDest);
        }
    
        if (((*videoDevices)[index])->graphicsDest) {
            free(((*videoDevices)[index])->graphicsDest);
        }

        free(((*videoDevices)[index]));
    }

    if (*videoDevices) {
        free(*videoDevices);
        *videoDevices = NULL;
    }

unloadXml_exit:
    (void)pData->loadersObj->unloadVideosXml(pData->loadersObj, xmlVideos);

    return ret;
}

/*!
 *
 */
static enum core_error_e unloadVideosParams_f(struct core_s *obj)
{
    ASSERT(obj && obj->pData);
    
    struct core_private_data_s *pData     = (struct core_private_data_s*)(obj->pData);
    struct videos_infos_s *videosInfos    = &pData->ctx->params.videosInfos;
    struct video_device_s ***videoDevices = &videosInfos->devices;
    uint8_t nbDevices                     = videosInfos->nbDevices;
    
    (void)pData->listenersObj->unsetVideosListeners(pData->listenersObj);
    
    uint8_t index;
    for (index = 0; index < nbDevices; index++) {
        if (((*videoDevices)[index])->serverDest) {
            free(((*videoDevices)[index])->serverDest);
        }
    
        if (((*videoDevices)[index])->graphicsDest) {
            free(((*videoDevices)[index])->graphicsDest);
        }

        free(((*videoDevices)[index]));
    }

    if (*videoDevices) {
        free(*videoDevices);
        *videoDevices = NULL;
    }
    
    return CORE_ERROR_NONE;
}

/*!
 *
 */
static enum core_error_e loadServersParams_f(struct core_s *obj)
{
    ASSERT(obj && obj->pData);
    
    struct core_private_data_s *pData = (struct core_private_data_s*)(obj->pData);
    enum core_error_e ret             = CORE_ERROR_NONE;

    if (pData->loadersObj->loadServersXml(pData->loadersObj,
                                          pData->ctx,
                                          &pData->xml.xmlServers) != LOADERS_ERROR_NONE) {
        Loge("Failed to load servers xml");
        return CORE_ERROR_XML;
    }
    
    struct xml_servers_s *xmlServers     = &pData->xml.xmlServers;
    struct servers_infos_s *serversInfos = &pData->ctx->params.serversInfos;
    struct server_infos_s ***serverInfos = &serversInfos->serverInfos;
    struct server_params_s *serverParams = NULL;
    uint8_t *nbServers                   = &serversInfos->nbServers;
    
    *nbServers = xmlServers->nbServers;
    
    Logd("Setting servers params");
    
    ASSERT((*serverInfos = calloc(*nbServers, sizeof(struct server_infos_s*))));
    
    uint8_t index;
    for (index = 0; index < *nbServers; index++) {
        ASSERT(((*serverInfos)[index] = calloc(1, sizeof(struct server_infos_s))));

        serverParams = &((*serverInfos)[index])->serverParams;

        strncpy(serverParams->name, xmlServers->servers[index].name, sizeof(serverParams->name));
        
        serverParams->type       = xmlServers->servers[index].type;
        serverParams->link       = xmlServers->servers[index].link;
        serverParams->mode       = xmlServers->servers[index].mode;
        serverParams->acceptMode = xmlServers->servers[index].acceptMode;
        serverParams->priority   = xmlServers->servers[index].priority;
        serverParams->maxClients = xmlServers->servers[index].maxClients;
        
        strncpy(serverParams->mime, xmlServers->servers[index].mime, sizeof(serverParams->mime));
        
        if (xmlServers->servers[index].host
                && xmlServers->servers[index].service
                && xmlServers->servers[index].path) {
            strncpy(serverParams->recipient.server.host,
                    xmlServers->servers[index].host,
                    sizeof(serverParams->recipient.server.host));

            strncpy(serverParams->recipient.server.service,
                    xmlServers->servers[index].service,
                    sizeof(serverParams->recipient.server.service));

            strncpy(serverParams->recipient.server.path,
                    xmlServers->servers[index].path,
                    sizeof(serverParams->recipient.server.path));
        }
        else if (xmlServers->servers[index].socketName) {
            strncpy(serverParams->recipient.serverSocketName,
                    xmlServers->servers[index].socketName,
                    sizeof(serverParams->recipient.serverSocketName));
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

    ret = CORE_ERROR_NONE;
    goto unloadXml_exit;
    
badConfig_exit:
    for (index = 0; index < *nbServers; index++) {
        free((*serverInfos)[index]);
    }
    
    if (*serverInfos) {
        free(*serverInfos);
        *serverInfos = NULL;
    }

unloadXml_exit:
    (void)pData->loadersObj->unloadServersXml(pData->loadersObj, xmlServers);

    return ret;
}

/*!
 *
 */
static enum core_error_e unloadServersParams_f(struct core_s *obj)
{
    ASSERT(obj && obj->pData);
    
    struct core_private_data_s *pData    = (struct core_private_data_s*)(obj->pData);
    struct servers_infos_s *serversInfos = &pData->ctx->params.serversInfos;
    struct server_infos_s ***serverInfos = &serversInfos->serverInfos;
    uint8_t nbServers                    = serversInfos->nbServers;
    
    (void)pData->listenersObj->unsetServersListeners(pData->listenersObj);
    
    uint8_t index;
    for (index = 0; index < nbServers; index++) {
        free((*serverInfos)[index]);
    }
    
    if (*serverInfos) {
        free(*serverInfos);
        *serverInfos = NULL;
    }
    
    return CORE_ERROR_NONE;
}

/*!
 *
 */
static enum core_error_e loadClientsParams_f(struct core_s *obj)
{
    ASSERT(obj && obj->pData);
    
    struct core_private_data_s *pData = (struct core_private_data_s*)(obj->pData);
    enum core_error_e ret             = CORE_ERROR_NONE;

    if (pData->loadersObj->loadClientsXml(pData->loadersObj,
                                          pData->ctx,
                                          &pData->xml.xmlClients) != LOADERS_ERROR_NONE) {
        Loge("Failed to load clients xml");
        return CORE_ERROR_XML;
    }
    
    struct xml_clients_s *xmlClients     = &pData->xml.xmlClients;
    struct clients_infos_s *clientsInfos = &pData->ctx->params.clientsInfos;
    struct client_infos_s ***clientInfos = &clientsInfos->clientInfos;
    struct client_params_s *clientParams = NULL;
    uint8_t *nbClients                   = &clientsInfos->nbClients;
    
    *nbClients = xmlClients->nbClients;
    
    Logd("Setting clients params");
    
    ASSERT((*clientInfos = calloc(*nbClients, sizeof(struct client_infos_s*))));
    
    uint8_t index;
    for (index = 0; index < *nbClients; index++) {
        ASSERT(((*clientInfos)[index] = calloc(1, sizeof(struct client_infos_s))));

        clientParams = &((*clientInfos)[index])->clientParams;

        strncpy(clientParams->name, xmlClients->clients[index].name, sizeof(clientParams->name));
        
        clientParams->type     = xmlClients->clients[index].type;
        clientParams->link     = xmlClients->clients[index].link;
        clientParams->mode     = xmlClients->clients[index].mode;
        clientParams->priority = xmlClients->clients[index].priority;
        
        if (xmlClients->clients[index].graphicsDest) {
            ((*clientInfos)[index])->graphicsDest  = strdup(xmlClients->clients[index].graphicsDest);
            ((*clientInfos)[index])->graphicsIndex = -1;
        }
        
        if (xmlClients->clients[index].serverDest) {
            ((*clientInfos)[index])->serverDest  = strdup(xmlClients->clients[index].serverDest);
            ((*clientInfos)[index])->serverIndex = -1;
        }
        
        if (xmlClients->clients[index].serverHost
                && xmlClients->clients[index].serverService
                && xmlClients->clients[index].serverPath) {
            strncpy(clientParams->recipient.server.host,
                    xmlClients->clients[index].serverHost,
                    sizeof(clientParams->recipient.server.host));

            strncpy(clientParams->recipient.server.service,
                    xmlClients->clients[index].serverService,
                    sizeof(clientParams->recipient.server.service));

            strncpy(clientParams->recipient.server.path,
                    xmlClients->clients[index].serverPath,
                    sizeof(clientParams->recipient.server.path));
        }
        else if (xmlClients->clients[index].serverSocketName) {
            strncpy(clientParams->recipient.serverSocketName,
                    xmlClients->clients[index].serverSocketName,
                    sizeof(clientParams->recipient.serverSocketName));
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

    ret = CORE_ERROR_NONE;
    goto unloadXml_exit;
    
badConfig_exit:
    for (index = 0; index < *nbClients; index++) {
        if (((*clientInfos)[index])->graphicsDest) {
            free(((*clientInfos)[index])->graphicsDest);
        }
        
        if (((*clientInfos)[index])->serverDest) {
            free(((*clientInfos)[index])->serverDest);
        }

        free((*clientInfos)[index]);
    }
    
    if (*clientInfos) {
        free(*clientInfos);
        *clientInfos = NULL;
    }

unloadXml_exit:
    (void)pData->loadersObj->unloadClientsXml(pData->loadersObj, xmlClients);

    return ret;
}

/*!
 *
 */
static enum core_error_e unloadClientsParams_f(struct core_s *obj)
{
    ASSERT(obj && obj->pData);
    
    struct core_private_data_s *pData    = (struct core_private_data_s*)(obj->pData);
    struct clients_infos_s *clientsInfos = &pData->ctx->params.clientsInfos;
    struct client_infos_s ***clientInfos = &clientsInfos->clientInfos;
    uint8_t nbClients                    = clientsInfos->nbClients;
    
    (void)pData->listenersObj->unsetClientsListeners(pData->listenersObj);
    
    uint8_t index;
    for (index = 0; index < nbClients; index++) {
        if (((*clientInfos)[index])->graphicsDest) {
            free(((*clientInfos)[index])->graphicsDest);
        }
        
        if (((*clientInfos)[index])->serverDest) {
            free(((*clientInfos)[index])->serverDest);
        }

        free((*clientInfos)[index]);
    }
    
    if (*clientInfos) {
        free(*clientInfos);
        *clientInfos = NULL;
    }
    
    return CORE_ERROR_NONE;
}

/*!
 *
 */
static enum core_error_e keepAppRunning_f(struct core_s *obj, enum keep_alive_method_e method,
                                          uint32_t timeout_s)
{
    ASSERT(obj && obj->pData);
    
    struct core_private_data_s *pData = (struct core_private_data_s*)(obj->pData);
    struct context_s *ctx             = pData->ctx;
    struct graphics_s *graphicsObj    = ctx->modules.graphicsObj;
    struct control_s *controlObj      = pData->controlObj;
    enum core_error_e ret             = CORE_ERROR_NONE;

    if (method == KEEP_ALIVE_SEMAPHORE_BASED) {
        if (sem_init(&ctx->keepAliveSem, 0, 0) != 0) {
            Loge("sem_init() failed");
            return CORE_ERROR_KEEP_ALIVE;
        }
    }

    if (controlObj->loadControllers(controlObj) != CONTROL_ERROR_NONE) {
        Loge("loadControllers() failed");
        goto exit;
    }

    switch (method) {
        case KEEP_ALIVE_EVENTS_BASED:
            if (graphicsObj
                && graphicsObj->handleGfxEvents(graphicsObj) != GRAPHICS_ERROR_NONE) {
                Loge("Error occurred while handling events");
                ret = CORE_ERROR_KEEP_ALIVE;
            }
            break;
            
        case KEEP_ALIVE_SEMAPHORE_BASED:
            sem_wait(&ctx->keepAliveSem);
            break;
            
        case KEEP_ALIVE_TIMER_BASED:
            sleep(timeout_s);
            break;
            
        default:
            Loge("Unknown keepAlive method");
            ret = CORE_ERROR_KEEP_ALIVE;
    }

    if (controlObj->unloadControllers(controlObj) != CONTROL_ERROR_NONE) {
        Loge("unloadControllers() failed");
    }

exit:
    if (method == KEEP_ALIVE_SEMAPHORE_BASED) {
        (void)sem_destroy(&ctx->keepAliveSem);
    }

    return ret;
}

/* -------------------------------------------------------------------------------------------- */
/* ///////////////////////////// PRIVATE FUNCTIONS IMPLEMENTATION ///////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static void getString_f(void *userData, uint32_t stringId, char *language, char *strOut)
{
    ASSERT(userData && language && strOut);
    
    struct xml_common_s *common   = (struct xml_common_s*)userData;
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
        Loge("Language \"%s\" not found - Using default language ( \"%s\" )",
                language, common->defaultLanguage);
        strcpy(strOut, common->xmlStrings[defaultLanguageIndex].strings[stringId].str);
    }
}

/*!
 *
 */
static void getColor_f(void *userData, int32_t colorId, struct gfx_color_s *colorOut)
{
    ASSERT(userData && colorOut);
    
    struct xml_common_s *common = (struct xml_common_s*)userData;
    
    colorOut->red   = common->xmlColors.colors[colorId].red;
    colorOut->green = common->xmlColors.colors[colorId].green;
    colorOut->blue  = common->xmlColors.colors[colorId].blue;
    colorOut->alpha = common->xmlColors.colors[colorId].alpha;
}

/*!
 *
 */
static void getFont_f(void *userData, uint32_t fontId, char *ttfFontOut)
{
    ASSERT(userData && ttfFontOut);
    
    struct xml_common_s *common = (struct xml_common_s*)userData;
    
    strcpy(ttfFontOut, common->xmlFonts.fonts[fontId].file);
}

/*!
 *
 */
static void getImage_f(void *userData, uint32_t imageId, struct gfx_image_s *imageOut)
{
    ASSERT(userData && imageOut);
    
    struct xml_common_s *common = (struct xml_common_s*)userData;
    
    strncpy(imageOut->path, common->xmlImages.images[imageId].file, sizeof(imageOut->path));
	imageOut->format = common->xmlImages.images[imageId].format;
}

/*!
 *
 */
static void getLanguage_f(void *userData, char *currentIn, char *nextOut)
{
    ASSERT(userData && currentIn && nextOut);
    
    struct xml_common_s *common = (struct xml_common_s*)userData;
    
    uint32_t index;
    for (index = 0; index < common->nbLanguages; index++) {
        if (!strcmp(currentIn, common->xmlStrings[index].language)) {
            break;
        }
    }
    
    strcpy(nextOut, common->xmlStrings[(index + 1) % common->nbLanguages].language);
}
