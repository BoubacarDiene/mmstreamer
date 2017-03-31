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
* \file   Graphics.c
* \brief  Graphics elements management
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include "utils/Log.h"
#include "utils/List.h"

#include "graphics/Drawer.h"
#include "graphics/Graphics.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                            */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "GRAPHICS"

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

typedef struct GRAPHICS_PRIVATE_DATA_S {
    volatile uint8_t   quit;
    
    GRAPHICS_PARAMS_S  params;
    
    LIST_S             *gfxElementsList;
    
    GFX_ELEMENT_S      *focusedElement;
    GFX_ELEMENT_S      *lastDrawnElement;
    GFX_ELEMENT_S      *videoElement;
    
    DRAWER_S           *drawer;
} GRAPHICS_PRIVATE_DATA_S;

/* -------------------------------------------------------------------------------------------- */
/*                                          VARIABLES                                           */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                         PROTOTYPES                                           */
/* -------------------------------------------------------------------------------------------- */

static GRAPHICS_ERROR_E createDrawer_f (GRAPHICS_S *obj, GRAPHICS_PARAMS_S *params);
static GRAPHICS_ERROR_E destroyDrawer_f(GRAPHICS_S *obj);

static GRAPHICS_ERROR_E createElement_f(GRAPHICS_S *obj, GFX_ELEMENT_S **newGfxElement);
static GRAPHICS_ERROR_E pushElement_f  (GRAPHICS_S *obj, GFX_ELEMENT_S *gfxElement);
static GRAPHICS_ERROR_E removeElement_f(GRAPHICS_S *obj, char *gfxElementName);
static GRAPHICS_ERROR_E removeAll_f    (GRAPHICS_S *obj);

static GRAPHICS_ERROR_E setVisible_f  (GRAPHICS_S *obj, char *gfxElementName, uint8_t isVisible);
static GRAPHICS_ERROR_E setFocus_f    (GRAPHICS_S *obj, char *gfxElementName);
static GRAPHICS_ERROR_E setClickable_f(GRAPHICS_S *obj, char *gfxElementName, uint8_t isClickable);
static GRAPHICS_ERROR_E setNav_f      (GRAPHICS_S *obj, char *gfxElementName, GFX_NAV_S *nav);
static GRAPHICS_ERROR_E setData_f     (GRAPHICS_S *obj, char *gfxElementName, void *data);

static GRAPHICS_ERROR_E saveVideoFrame_f  (GRAPHICS_S *obj, BUFFER_S *buffer, GFX_IMAGE_S *inOut);
static GRAPHICS_ERROR_E saveVideoElement_f(GRAPHICS_S *obj, char *gfxElementName, GFX_IMAGE_S *inOut);
static GRAPHICS_ERROR_E takeScreenshot_f  (GRAPHICS_S *obj, GFX_IMAGE_S *inOut);

static GRAPHICS_ERROR_E drawAllElements_f(GRAPHICS_S *obj);

static GRAPHICS_ERROR_E simulateGfxEvent_f(GRAPHICS_S *obj, GFX_EVENT_S *gfxEvent);
static GRAPHICS_ERROR_E handleGfxEvents_f (GRAPHICS_S *obj);

static GRAPHICS_ERROR_E quit_f(GRAPHICS_S *obj);


static GRAPHICS_ERROR_E updateGroup_f      (GRAPHICS_S *obj, char *groupName, char *gfxElementToIgnore);
static GRAPHICS_ERROR_E updateElement_f    (GRAPHICS_S *obj, GFX_ELEMENT_S *gfxElement);
static GRAPHICS_ERROR_E drawElement_f      (GRAPHICS_S *obj, GFX_ELEMENT_S *gfxElement);
static GRAPHICS_ERROR_E getElement_f       (GRAPHICS_S *obj, char *gfxElementName, GFX_ELEMENT_S **gfxElement);
static GRAPHICS_ERROR_E getClickedElement_f(GRAPHICS_S *obj, GFX_EVENT_S *gfxEvent);
static GRAPHICS_ERROR_E handleGfxEvent_f   (GRAPHICS_S *obj, GFX_EVENT_S *gfxEvent);

static uint8_t compareCb(LIST_S *obj, void *elementToCheck, void *userData);
static void    releaseCb(LIST_S *obj, void *element);
static void    browseCb (LIST_S *obj, void *element, void *dataProvidedToBrowseFunction);

/* -------------------------------------------------------------------------------------------- */
/*                                      PUBLIC FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
GRAPHICS_ERROR_E Graphics_Init(GRAPHICS_S **obj)
{
    assert(obj && (*obj = calloc(1, sizeof(GRAPHICS_S))));
    
    GRAPHICS_PRIVATE_DATA_S *pData;
    assert((pData = calloc(1, sizeof(GRAPHICS_PRIVATE_DATA_S))));
    
    LIST_PARAMS_S listParams;
    memset(&listParams, '\0', sizeof(LIST_PARAMS_S));
    listParams.compareCb = compareCb;
    listParams.releaseCb = releaseCb;
    listParams.browseCb  = browseCb;
    
    if (List_Init(&pData->gfxElementsList, &listParams) != LIST_ERROR_NONE) {
        goto exit;
    }
    
    (*obj)->createDrawer     = createDrawer_f;
    (*obj)->destroyDrawer    = destroyDrawer_f;
    
    (*obj)->createElement    = createElement_f;
    (*obj)->pushElement      = pushElement_f;
    (*obj)->removeElement    = removeElement_f;
    (*obj)->removeAll        = removeAll_f;
    
    (*obj)->setVisible       = setVisible_f;
    (*obj)->setFocus         = setFocus_f;
    (*obj)->setClickable     = setClickable_f;
    (*obj)->setNav           = setNav_f;
    (*obj)->setData          = setData_f;
    
    (*obj)->saveVideoFrame   = saveVideoFrame_f;
    (*obj)->saveVideoElement = saveVideoElement_f;
    (*obj)->takeScreenshot   = takeScreenshot_f;
    
    (*obj)->drawAllElements  = drawAllElements_f;
    
    (*obj)->simulateGfxEvent = simulateGfxEvent_f;
    (*obj)->handleGfxEvents  = handleGfxEvents_f;
    
    (*obj)->quit             = quit_f;
    
    (*obj)->pData = (void*)pData;
    
    return GRAPHICS_ERROR_NONE;
    
exit:
    free(pData);
    pData = NULL;
    
    free(*obj);
    *obj = NULL;
    
    return GRAPHICS_ERROR_INIT;
}

/*!
 *
 */
GRAPHICS_ERROR_E Graphics_UnInit(GRAPHICS_S **obj)
{
    assert(obj && *obj && (*obj)->pData);
    
    GRAPHICS_PRIVATE_DATA_S *pData = (GRAPHICS_PRIVATE_DATA_S*)((*obj)->pData);
    
    (void)List_UnInit(&pData->gfxElementsList);
    
    free((*obj)->pData);
    (*obj)->pData = NULL;
    
    free(*obj);
    *obj = NULL;
    
    return GRAPHICS_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/*                                     PRIVATE FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static GRAPHICS_ERROR_E createDrawer_f(GRAPHICS_S *obj, GRAPHICS_PARAMS_S *params)
{
    assert(obj && obj->pData && params);
    
    GRAPHICS_PRIVATE_DATA_S *pData = (GRAPHICS_PRIVATE_DATA_S*)(obj->pData);
    
    memcpy(&pData->params.screenParams, &params->screenParams, sizeof(GFX_SCREEN_S));
    memcpy(&pData->params.colorOnFocus, &params->colorOnFocus, sizeof(GFX_COLOR_S));
    memcpy(&pData->params.colorOnBlur,  &params->colorOnBlur, sizeof(GFX_COLOR_S));
    memcpy(&pData->params.colorOnReset, &params->colorOnReset, sizeof(GFX_COLOR_S));
    
    pData->params.onGfxEventCb = params->onGfxEventCb;
    pData->params.userData     = params->userData;
    
    if (Drawer_Init(&pData->drawer) != DRAWER_ERROR_NONE) {
        Loge("Failed to init drawer");
        return GRAPHICS_ERROR_DRAWER;
    }
    
    if (pData->drawer->initScreen(pData->drawer, &params->screenParams) != DRAWER_ERROR_NONE) {
        Loge("Failed to init screen");
        (void)Drawer_UnInit(&pData->drawer);
        return GRAPHICS_ERROR_DRAWER;
    }
    
    return GRAPHICS_ERROR_NONE;
}

/*!
 *
 */
static GRAPHICS_ERROR_E destroyDrawer_f(GRAPHICS_S *obj)
{
    assert(obj && obj->pData);
    
    GRAPHICS_PRIVATE_DATA_S *pData = (GRAPHICS_PRIVATE_DATA_S*)(obj->pData);
    
    pData->params.userData = NULL;
    
    (void)pData->drawer->unInitScreen(pData->drawer);
    (void)Drawer_UnInit(&pData->drawer);
    
    return GRAPHICS_ERROR_NONE;
}

/*!
 *
 */
static GRAPHICS_ERROR_E createElement_f(GRAPHICS_S *obj, GFX_ELEMENT_S **newGfxElement)
{
    assert(obj && obj->pData && newGfxElement);
    
    assert((*newGfxElement = calloc(1, sizeof(GFX_ELEMENT_S))));
    
    return GRAPHICS_ERROR_NONE;
}

/*!
 *
 */
static GRAPHICS_ERROR_E pushElement_f(GRAPHICS_S *obj, GFX_ELEMENT_S *gfxElement)
{
    assert(obj && obj->pData && gfxElement);
    
    GRAPHICS_PRIVATE_DATA_S *pData = (GRAPHICS_PRIVATE_DATA_S*)(obj->pData);
    
    if (pData->gfxElementsList->lock(pData->gfxElementsList) != LIST_ERROR_NONE) {
        return GRAPHICS_ERROR_LOCK;
    }
    
    pData->gfxElementsList->add(pData->gfxElementsList, (void*)gfxElement);
    
    (void)pData->gfxElementsList->unlock(pData->gfxElementsList);
    
    return GRAPHICS_ERROR_NONE;
}

/*!
 *
 */
static GRAPHICS_ERROR_E removeElement_f(GRAPHICS_S *obj, char *gfxElementName)
{
    assert(obj && obj->pData && gfxElementName);
    
    GRAPHICS_PRIVATE_DATA_S *pData = (GRAPHICS_PRIVATE_DATA_S*)(obj->pData);
    
    if (pData->gfxElementsList->lock(pData->gfxElementsList) != LIST_ERROR_NONE) {
        return GRAPHICS_ERROR_LOCK;
    }
    
    pData->gfxElementsList->remove(pData->gfxElementsList, (void*)gfxElementName);
    
    (void)pData->gfxElementsList->unlock(pData->gfxElementsList);
    
    return GRAPHICS_ERROR_NONE;
}

/*!
 *
 */
static GRAPHICS_ERROR_E removeAll_f(GRAPHICS_S *obj)
{
    assert(obj && obj->pData);
    
    GRAPHICS_PRIVATE_DATA_S *pData = (GRAPHICS_PRIVATE_DATA_S*)(obj->pData);
    
    if (pData->gfxElementsList->lock(pData->gfxElementsList) != LIST_ERROR_NONE) {
        return GRAPHICS_ERROR_LOCK;
    }
    
    pData->gfxElementsList->removeAll(pData->gfxElementsList);
    
    (void)pData->gfxElementsList->unlock(pData->gfxElementsList);
    
    return GRAPHICS_ERROR_NONE;
}

/*!
 *
 */
static GRAPHICS_ERROR_E setVisible_f(GRAPHICS_S *obj, char *gfxElementName, uint8_t isVisible)
{
    assert(obj && obj->pData && gfxElementName);
    
    GRAPHICS_PRIVATE_DATA_S *pData = (GRAPHICS_PRIVATE_DATA_S*)(obj->pData);
    
    if (pData->gfxElementsList->lock(pData->gfxElementsList) != LIST_ERROR_NONE) {
        return GRAPHICS_ERROR_LOCK;
    }
    
    GFX_ELEMENT_S *gfxElement = NULL;
    GRAPHICS_ERROR_E      ret = GRAPHICS_ERROR_NONE;
    
    if (pData->focusedElement && !strncmp(pData->focusedElement->name, gfxElementName, MAX_NAME_SIZE)) {
        gfxElement = pData->focusedElement;
    }
    else if (pData->videoElement && !strncmp(pData->videoElement->name, gfxElementName, MAX_NAME_SIZE)) {
        gfxElement = pData->videoElement;
    }
    else if (pData->lastDrawnElement && !strncmp(pData->lastDrawnElement->name, gfxElementName, MAX_NAME_SIZE)) {
        gfxElement = pData->lastDrawnElement;
    }
    else if (getElement_f(obj, gfxElementName, &gfxElement) != GRAPHICS_ERROR_NONE) {
        goto exit;
    }
    
    if (gfxElement->isVisible == isVisible) {
        Logd("Same visibility => not changed");
        ret = GRAPHICS_ERROR_NONE;
        goto exit;
    }
    
    gfxElement->isVisible      = isVisible;
    gfxElement->surfaceUpdated = 0;
 
    if (updateGroup_f(obj, gfxElement->groupName, NULL) != GRAPHICS_ERROR_NONE) {
        Loge("Failed to update group : \"%s\"", gfxElement->groupName);
        gfxElement->isVisible = !isVisible;
        ret = GRAPHICS_ERROR_DRAWER;
    }

    if (!gfxElement->isVisible && gfxElement->hasFocus) {
        Logw("Invisible element \"%s\" still has focus - Please, call setFocus on a visible element", gfxElement->name);
    }

exit:    
    (void)pData->gfxElementsList->unlock(pData->gfxElementsList);
    
    return ret;
}

/*!
 *
 */
static GRAPHICS_ERROR_E setFocus_f(GRAPHICS_S *obj, char *gfxElementName)
{
    assert(obj && obj->pData && gfxElementName);
    
    GRAPHICS_PRIVATE_DATA_S *pData = (GRAPHICS_PRIVATE_DATA_S*)(obj->pData);
    
    if (pData->gfxElementsList->lock(pData->gfxElementsList) != LIST_ERROR_NONE) {
        return GRAPHICS_ERROR_LOCK;
    }
    
    GRAPHICS_ERROR_E      ret = GRAPHICS_ERROR_NONE;
    GFX_ELEMENT_S *gfxElement = NULL;
    
    if (pData->focusedElement && !strncmp(pData->focusedElement->name, gfxElementName, MAX_NAME_SIZE)) {
        Logd("\"%s\" already has focus", gfxElementName);
        goto exit;
    }
    else if (pData->videoElement && !strncmp(pData->videoElement->name, gfxElementName, MAX_NAME_SIZE)) {
        gfxElement = pData->videoElement;
    }
    else if (pData->lastDrawnElement && !strncmp(pData->lastDrawnElement->name, gfxElementName, MAX_NAME_SIZE)) {
        gfxElement = pData->lastDrawnElement;
    }
    else if (getElement_f(obj, gfxElementName, &gfxElement) != GRAPHICS_ERROR_NONE) {
        goto exit;
    }
    
    if (!gfxElement->isFocusable) {
        Logw("\"%s\" is not focusable", gfxElementName);
        goto exit;
    }
    
    if (!gfxElement->isVisible) {
        Logw("\"%s\" is not visible", gfxElementName);
        goto exit;
    }
    
    gfxElement->hasFocus = 1;
        
    if (updateElement_f(obj, gfxElement) != GRAPHICS_ERROR_NONE) {
        Loge("Failed to update element : \"%s\"", gfxElementName);
        ret = GRAPHICS_ERROR_DRAWER;
    }

exit: 
    (void)pData->gfxElementsList->unlock(pData->gfxElementsList);
    
    return ret;
}

/*!
 *
 */
static GRAPHICS_ERROR_E setClickable_f(GRAPHICS_S *obj, char *gfxElementName, uint8_t isClickable)
{
    assert(obj && obj->pData && gfxElementName);
    
    GRAPHICS_PRIVATE_DATA_S *pData = (GRAPHICS_PRIVATE_DATA_S*)(obj->pData);
    
    if (pData->gfxElementsList->lock(pData->gfxElementsList) != LIST_ERROR_NONE) {
        return GRAPHICS_ERROR_LOCK;
    }
    
    GFX_ELEMENT_S *gfxElement = NULL;
    GRAPHICS_ERROR_E      ret = GRAPHICS_ERROR_NONE;
    
    if (pData->focusedElement && !strncmp(pData->focusedElement->name, gfxElementName, MAX_NAME_SIZE)) {
        gfxElement = pData->focusedElement;
    }
    else if (pData->videoElement && !strncmp(pData->videoElement->name, gfxElementName, MAX_NAME_SIZE)) {
        gfxElement = pData->videoElement;
    }
    else if (pData->lastDrawnElement && !strncmp(pData->lastDrawnElement->name, gfxElementName, MAX_NAME_SIZE)) {
        gfxElement = pData->lastDrawnElement;
    }
    else if (getElement_f(obj, gfxElementName, &gfxElement) != GRAPHICS_ERROR_NONE) {
        goto exit;
    }
    
    if (gfxElement->isClickable == isClickable) {
        Logd("Same value => not changed");
        ret = GRAPHICS_ERROR_NONE;
        goto exit;
    }
    
    gfxElement->isClickable = isClickable;

exit:
    (void)pData->gfxElementsList->unlock(pData->gfxElementsList);
    
    return ret;
}

/*!
 *
 */
static GRAPHICS_ERROR_E setNav_f(GRAPHICS_S *obj, char *gfxElementName, GFX_NAV_S *nav)
{
    assert(obj && obj->pData && gfxElementName && nav);

    GRAPHICS_PRIVATE_DATA_S *pData = (GRAPHICS_PRIVATE_DATA_S*)(obj->pData);

    if (pData->gfxElementsList->lock(pData->gfxElementsList) != LIST_ERROR_NONE) {
        return GRAPHICS_ERROR_LOCK;
    }

    GFX_ELEMENT_S *gfxElement = NULL;
    GRAPHICS_ERROR_E ret      = GRAPHICS_ERROR_NONE;

    if (pData->focusedElement && !strncmp(pData->focusedElement->name, gfxElementName, MAX_NAME_SIZE)) {
        gfxElement = pData->focusedElement;
    }
    else if (pData->videoElement && !strncmp(pData->videoElement->name, gfxElementName, MAX_NAME_SIZE)) {
        gfxElement = pData->videoElement;
    }
    else if (pData->lastDrawnElement && !strncmp(pData->lastDrawnElement->name, gfxElementName, MAX_NAME_SIZE)) {
        gfxElement = pData->lastDrawnElement;
    }
    else if (getElement_f(obj, gfxElementName, &gfxElement) != GRAPHICS_ERROR_NONE) {
        goto exit;
    }

    memcpy(&gfxElement->nav, nav, sizeof(GFX_NAV_S));

exit:
    (void)pData->gfxElementsList->unlock(pData->gfxElementsList);

    return ret;
}

/*!
 *
 */
static GRAPHICS_ERROR_E setData_f(GRAPHICS_S *obj, char *gfxElementName, void *data)
{
    assert(obj && obj->pData && gfxElementName && data);
    
    GRAPHICS_PRIVATE_DATA_S *pData = (GRAPHICS_PRIVATE_DATA_S*)(obj->pData);
    
    if (!pData->drawer) {
        return GRAPHICS_ERROR_DRAWER;
    }
    
    if (pData->gfxElementsList->lock(pData->gfxElementsList) != LIST_ERROR_NONE) {
        return GRAPHICS_ERROR_LOCK;
    }
    
    GRAPHICS_ERROR_E ret      = GRAPHICS_ERROR_NONE;
    GFX_ELEMENT_S *gfxElement = NULL;
    
    if (pData->focusedElement && !strncmp(pData->focusedElement->name, gfxElementName, MAX_NAME_SIZE)) {
        gfxElement = pData->focusedElement;
    }
    else if (pData->videoElement && !strncmp(pData->videoElement->name, gfxElementName, MAX_NAME_SIZE)) {
        gfxElement = pData->videoElement;
    }
    else if (pData->lastDrawnElement && !strncmp(pData->lastDrawnElement->name, gfxElementName, MAX_NAME_SIZE)) {
        gfxElement = pData->lastDrawnElement;
    }
    else if (getElement_f(obj, gfxElementName, &gfxElement) != GRAPHICS_ERROR_NONE) {
        goto exit;
    }
        
    switch (gfxElement->type) {
        case GFX_ELEMENT_TYPE_VIDEO:
            gfxElement->data.buffer.data   = ((BUFFER_S*)data)->data;
            gfxElement->data.buffer.length = ((BUFFER_S*)data)->length;
            break;
                
        case GFX_ELEMENT_TYPE_IMAGE:
            strncpy(gfxElement->data.image.path, ((GFX_IMAGE_S*)data)->path, strlen(((GFX_IMAGE_S*)data)->path));
            gfxElement->data.image.format      = ((GFX_IMAGE_S*)data)->format;
            if (!((GFX_IMAGE_S*)data)->hiddenColor) {
                if (gfxElement->data.image.hiddenColor) {
                    free(gfxElement->data.image.hiddenColor);
                    gfxElement->data.image.hiddenColor = NULL;
                }
            }
            else {
                if (!gfxElement->data.image.hiddenColor) {
                    assert((gfxElement->data.image.hiddenColor = calloc(1, sizeof(GFX_COLOR_S))));
                }
                memcpy(gfxElement->data.image.hiddenColor, ((GFX_IMAGE_S*)data)->hiddenColor, sizeof(GFX_COLOR_S));
            }
            break;
                
        case GFX_ELEMENT_TYPE_TEXT:
            if (pData->drawer->setBgColor(pData->drawer, &gfxElement->rect, &pData->params.colorOnReset) != DRAWER_ERROR_NONE) {
                Loge("Failed to set background color of element \"%s\"", gfxElement->name);
                ret = GRAPHICS_ERROR_DRAWER;
                goto exit;
            }
            memcpy(&gfxElement->data.text, (GFX_TEXT_S*)data, sizeof(GFX_TEXT_S));
            break;
                
        default:
            ret = GRAPHICS_ERROR_DRAWER;
            goto exit;
    }
    
    if (gfxElement->redrawGroup) {
        if (updateGroup_f(obj, gfxElement->groupName, NULL) != GRAPHICS_ERROR_NONE) {
            Loge("Failed to update group : \"%s\"", gfxElement->groupName);
            ret = GRAPHICS_ERROR_DRAWER;
        }
    }
    else if (updateElement_f(obj, gfxElement) != GRAPHICS_ERROR_NONE) {
        Loge("Failed to update element : \"%s\"", gfxElementName);
        ret = GRAPHICS_ERROR_DRAWER;
    }
    
exit:
    (void)pData->gfxElementsList->unlock(pData->gfxElementsList);
    
    return ret;
}

/*!
 *
 */
static GRAPHICS_ERROR_E saveVideoFrame_f(GRAPHICS_S *obj, BUFFER_S *buffer, GFX_IMAGE_S *inOut)
{
    assert(obj && obj->pData && inOut);
    
    GRAPHICS_PRIVATE_DATA_S *pData = (GRAPHICS_PRIVATE_DATA_S*)(obj->pData);

    if (pData->gfxElementsList->lock(pData->gfxElementsList) != LIST_ERROR_NONE) {
        return GRAPHICS_ERROR_LOCK;
    }

    GRAPHICS_ERROR_E ret = GRAPHICS_ERROR_PARAMS;
    BUFFER_S videoBuffer = { 0 };

    if (!buffer) {
        if (!pData->videoElement) {
            Loge("No video frame to save");
            goto exit;
        }
        Logd("Creating screenshot from the last drawn video frame");
        videoBuffer.data   = pData->videoElement->data.buffer.data;
        videoBuffer.length = pData->videoElement->data.buffer.length;
    }
    else {
        videoBuffer.data   = buffer->data;
        videoBuffer.length = buffer->length;
    }
    
    if (pData->drawer->saveBuffer(pData->drawer, &videoBuffer, inOut) != DRAWER_ERROR_NONE) {
        ret = GRAPHICS_ERROR_DRAWER;
        goto exit;
    }

    ret = GRAPHICS_ERROR_NONE;

exit:
    (void)pData->gfxElementsList->unlock(pData->gfxElementsList);

    return ret;
}

/*!
 *
 */
static GRAPHICS_ERROR_E saveVideoElement_f(GRAPHICS_S *obj, char *gfxElementName, GFX_IMAGE_S *inOut)
{
    assert(obj && obj->pData && gfxElementName && inOut);

    GRAPHICS_PRIVATE_DATA_S *pData = (GRAPHICS_PRIVATE_DATA_S*)(obj->pData);

    if (pData->gfxElementsList->lock(pData->gfxElementsList) != LIST_ERROR_NONE) {
        return GRAPHICS_ERROR_LOCK;
    }

    GRAPHICS_ERROR_E ret = GRAPHICS_ERROR_PARAMS;
    BUFFER_S videoBuffer = { 0 };

    if (!pData->videoElement && (strncmp(pData->videoElement->name, gfxElementName, MAX_NAME_SIZE) == 0)) {
        videoBuffer.data   = pData->videoElement->data.buffer.data;
        videoBuffer.length = pData->videoElement->data.buffer.length;
    }
    else {
        GFX_ELEMENT_S *gfxElement = NULL;

        if (getElement_f(obj, gfxElementName, &gfxElement) != GRAPHICS_ERROR_NONE) {
            Loge("Element \"%s\" not found", gfxElementName);
            goto exit;
        }

        if (gfxElement->type != GFX_ELEMENT_TYPE_VIDEO) {
            Loge("\"%s\" is not a video element", gfxElementName);
            goto exit;
        }

        if (!gfxElement->data.buffer.data || (gfxElement->data.buffer.length == 0)) {
            Logw("No video frame to save");
            goto exit;
        }

        videoBuffer.data   = gfxElement->data.buffer.data;
        videoBuffer.length = gfxElement->data.buffer.length;
    }

    if (pData->drawer->saveBuffer(pData->drawer, &videoBuffer, inOut) != DRAWER_ERROR_NONE) {
        ret = GRAPHICS_ERROR_DRAWER;
        goto exit;
    }

    ret = GRAPHICS_ERROR_NONE;

exit:
    (void)pData->gfxElementsList->unlock(pData->gfxElementsList);

    return ret;
}

/*!
 *
 */
static GRAPHICS_ERROR_E takeScreenshot_f(GRAPHICS_S *obj, GFX_IMAGE_S *inOut)
{
    assert(obj && obj->pData && inOut);

    GRAPHICS_PRIVATE_DATA_S *pData = (GRAPHICS_PRIVATE_DATA_S*)(obj->pData);

    if (pData->drawer->saveScreen(pData->drawer, inOut) != DRAWER_ERROR_NONE) {
        return GRAPHICS_ERROR_DRAWER;
    }

    return GRAPHICS_ERROR_NONE;
}

/*!
 *
 */
static GRAPHICS_ERROR_E drawAllElements_f(GRAPHICS_S *obj)
{
    assert(obj && obj->pData);
    
    GRAPHICS_PRIVATE_DATA_S *pData = (GRAPHICS_PRIVATE_DATA_S*)(obj->pData);
    
    if (pData->gfxElementsList->lock(pData->gfxElementsList) != LIST_ERROR_NONE) {
        return GRAPHICS_ERROR_LOCK;
    }
    
    GRAPHICS_ERROR_E ret = GRAPHICS_ERROR_NONE;
    
    if (pData->gfxElementsList->browseElements(pData->gfxElementsList, obj) != LIST_ERROR_NONE) {
        Loge("Failed to browse elements' list");
        ret = GRAPHICS_ERROR_LIST;
    }
    
    (void)pData->gfxElementsList->unlock(pData->gfxElementsList);
    
    return ret;
}

/*!
 *
 */
static GRAPHICS_ERROR_E simulateGfxEvent_f(GRAPHICS_S *obj, GFX_EVENT_S *gfxEvent)
{
    assert(obj && obj->pData && gfxEvent);

    GRAPHICS_PRIVATE_DATA_S *pData = (GRAPHICS_PRIVATE_DATA_S*)(obj->pData);

    if (!pData->quit) {
        Logw("Too late! Stopping graphics module");
        return GRAPHICS_ERROR_NONE;
    }

    return handleGfxEvent_f(obj, gfxEvent);;
}

/*!
 *
 */
static GRAPHICS_ERROR_E handleGfxEvents_f(GRAPHICS_S *obj)
{
    assert(obj && obj->pData);

    GRAPHICS_PRIVATE_DATA_S *pData = (GRAPHICS_PRIVATE_DATA_S*)(obj->pData);

    if (!pData->drawer) {
        return GRAPHICS_ERROR_DRAWER;
    }

    GFX_EVENT_S evt;

    while (!pData->quit) {
        if (pData->drawer->getEvent(pData->drawer, &evt) != DRAWER_ERROR_NONE) {
            continue;
        }

        evt.gfxElementName  = NULL;
        evt.gfxElementPData = NULL;

        (void)handleGfxEvent_f(obj, &evt);
    }

    return GRAPHICS_ERROR_NONE;
}

/*!
 *
 */
static GRAPHICS_ERROR_E quit_f(GRAPHICS_S *obj)
{
    assert(obj && obj->pData);
    
    ((GRAPHICS_PRIVATE_DATA_S*)(obj->pData))->quit = 1;
    
    return GRAPHICS_ERROR_NONE;
}

/*!
 *
 */
static GRAPHICS_ERROR_E updateGroup_f(GRAPHICS_S *obj, char *groupName, char *gfxElementToIgnore)
{
    assert(obj && obj->pData && groupName);
    
    if (strlen(groupName) == 0) {
        Loge("Invalid group name");
        return GRAPHICS_ERROR_DRAWER;
    }
    
    GRAPHICS_PRIVATE_DATA_S *pData = (GRAPHICS_PRIVATE_DATA_S*)(obj->pData);
    GRAPHICS_ERROR_E ret           = GRAPHICS_ERROR_NONE;
    GFX_ELEMENT_S *gfxElement      = NULL;
    
    uint32_t nbElements;
    if (pData->gfxElementsList->getNbElements(pData->gfxElementsList, &nbElements) != LIST_ERROR_NONE) {
        Loge("Failed to get number of elements");
        ret = GRAPHICS_ERROR_LIST;
        goto exit;
    }
        
    while (nbElements > 0) {
        if (pData->gfxElementsList->getElement(pData->gfxElementsList, (void*)&gfxElement) != LIST_ERROR_NONE) {
            Loge("Failed to retrieve element");
            ret = GRAPHICS_ERROR_LIST;
            goto exit;
        }
        
        if (!strncmp(gfxElement->groupName, groupName, MAX_NAME_SIZE)
            && (!gfxElementToIgnore || strncmp(gfxElement->name, gfxElementToIgnore, MAX_NAME_SIZE))) {
            if ((ret = updateElement_f(obj, gfxElement)) != GRAPHICS_ERROR_NONE) {
                Loge("Failed to update element : \"%s\"", gfxElement->name);
                goto exit;
            }
        }
        
        nbElements--;
    }
    
exit:

    return ret;
}

/*!
 *
 */
static GRAPHICS_ERROR_E updateElement_f(GRAPHICS_S *obj, GFX_ELEMENT_S *gfxElement)
{
    assert(obj && obj->pData && gfxElement);
    
    GRAPHICS_PRIVATE_DATA_S *pData = (GRAPHICS_PRIVATE_DATA_S*)(obj->pData);
    GRAPHICS_ERROR_E           ret = GRAPHICS_ERROR_NONE;
        
    if (!gfxElement->isVisible) {
        // Clear surface
        if (!gfxElement->surfaceUpdated
            && pData->drawer->setBgColor(pData->drawer, &gfxElement->rect, &pData->params.colorOnReset) != DRAWER_ERROR_NONE) {
            Loge("Failed to set background color of element \"%s\"", gfxElement->name);
            ret = GRAPHICS_ERROR_DRAWER;
            goto exit;
        }

        gfxElement->surfaceUpdated = 1;

        // Change focused element if required
        if (pData->focusedElement && !strncmp(pData->focusedElement->name, gfxElement->name, MAX_NAME_SIZE)) {
            Logw("Current focused element is not visible anymore => Please, set a new one");
            pData->focusedElement->hasFocus = 0;
            pData->focusedElement           = NULL;
        }
    }
    else {
        if (gfxElement->hasFocus) {
            if (pData->focusedElement && strncmp(pData->focusedElement->name, gfxElement->name, MAX_NAME_SIZE)) {
                Logd("\"%s\" was not the current focused element (Current : %s)", gfxElement->name, pData->focusedElement->name);
                pData->focusedElement->hasFocus = 0;
                if (pData->drawer->setBgColor(pData->drawer,
                                                &pData->focusedElement->rect,
                                                &pData->params.colorOnBlur) != DRAWER_ERROR_NONE) {
                    Loge("Failed to set colorOnBlur on \"%s\"", pData->focusedElement->name);
                    ret = GRAPHICS_ERROR_DRAWER;
                    goto exit;
                }
                
                if ((ret = drawElement_f(obj, pData->focusedElement)) != GRAPHICS_ERROR_NONE) {
                    Loge("Failed to update element : \"%s\"", pData->focusedElement->name);
                    goto exit;
                }
            }
            
            if (pData->drawer->setBgColor(pData->drawer, &gfxElement->rect, &pData->params.colorOnFocus) != DRAWER_ERROR_NONE) {
                Loge("Failed to set colorOnFocus on \"%s\"", gfxElement->name);
                ret = GRAPHICS_ERROR_DRAWER;
                goto exit;
            }
                    
            pData->focusedElement = gfxElement;
            Logd("Focus now given to : \"%s\"", pData->focusedElement->name);
            
            if (pData->params.onGfxEventCb) {
                GFX_EVENT_S evt;
                evt.type            = GFX_EVENT_TYPE_FOCUS;
                evt.gfxElementName  = strdup(pData->focusedElement->name);
                evt.gfxElementPData = pData->focusedElement->pData;
                pData->params.onGfxEventCb(&evt, pData->params.userData);
            }
        }
        else if (gfxElement->isFocusable
                    && pData->drawer->setBgColor(pData->drawer, &gfxElement->rect, &pData->params.colorOnBlur) != DRAWER_ERROR_NONE) {
            Loge("Failed to set colorOnBlur on \"%s\"", gfxElement->name);
            ret = GRAPHICS_ERROR_DRAWER;
            goto exit;
        }
    
        ret = drawElement_f(obj, gfxElement);
    }
    
exit:

    return ret;
}

/*!
 *
 */
static GRAPHICS_ERROR_E drawElement_f(GRAPHICS_S *obj, GFX_ELEMENT_S *gfxElement)
{
    assert(obj && obj->pData && gfxElement);
    
    GRAPHICS_PRIVATE_DATA_S *pData = (GRAPHICS_PRIVATE_DATA_S*)(obj->pData);
    GRAPHICS_ERROR_E           ret = GRAPHICS_ERROR_NONE;
    
    switch (gfxElement->type) {
        case GFX_ELEMENT_TYPE_VIDEO:
            if (pData->drawer->drawVideo(pData->drawer, &gfxElement->rect, &gfxElement->data.buffer) != DRAWER_ERROR_NONE) {
                ret = GRAPHICS_ERROR_DRAWER;
                goto exit;
            }
            pData->videoElement = gfxElement;
            break;
                
        case GFX_ELEMENT_TYPE_IMAGE:
            if (pData->drawer->drawImage(pData->drawer, &gfxElement->rect, &gfxElement->data.image) != DRAWER_ERROR_NONE) {
                ret = GRAPHICS_ERROR_DRAWER;
                goto exit;
            }
            break;
                
        case GFX_ELEMENT_TYPE_TEXT:
            if (pData->drawer->drawText(pData->drawer, &gfxElement->rect, &gfxElement->data.text) != DRAWER_ERROR_NONE) {
                ret = GRAPHICS_ERROR_DRAWER;
                goto exit;
            }
            break;
                
        default:
            ret = GRAPHICS_ERROR_DRAWER;
            goto exit;
    }
        
    pData->lastDrawnElement = gfxElement;
    
exit:

    return ret;
}

/*!
 *
 */
static GRAPHICS_ERROR_E getElement_f(GRAPHICS_S *obj, char *gfxElementName, GFX_ELEMENT_S **gfxElement)
{
    assert(obj && obj->pData && gfxElementName);
    
    GRAPHICS_PRIVATE_DATA_S *pData = (GRAPHICS_PRIVATE_DATA_S*)(obj->pData);
    GRAPHICS_ERROR_E           ret = GRAPHICS_ERROR_NONE;

    uint32_t nbElements;
    if (pData->gfxElementsList->getNbElements(pData->gfxElementsList, &nbElements) != LIST_ERROR_NONE) {
        Loge("Failed to get number of elements");
        ret = GRAPHICS_ERROR_LIST;
        goto exit;
    }
    
    while (nbElements > 0) {
        if (pData->gfxElementsList->getElement(pData->gfxElementsList, (void*)gfxElement) != LIST_ERROR_NONE) {
            Loge("Failed to retrieve element");
            ret = GRAPHICS_ERROR_LIST;
            goto exit;
        }
            
        if (!strncmp((*gfxElement)->name, gfxElementName, MAX_NAME_SIZE)) {
            break;
        }
            
        nbElements--;
    }
        
    if (nbElements == 0) {
        Loge("Element %s not found", gfxElementName);
        ret = GRAPHICS_ERROR_LIST;
    }
    
exit:

    return ret;
}

/*!
 *
 */
static GRAPHICS_ERROR_E getClickedElement_f(GRAPHICS_S *obj, GFX_EVENT_S *gfxEvent)
{
    assert(obj && obj->pData && gfxEvent);
    
    GRAPHICS_PRIVATE_DATA_S *pData = (GRAPHICS_PRIVATE_DATA_S*)(obj->pData);
    
    if (pData->gfxElementsList->lock(pData->gfxElementsList) != LIST_ERROR_NONE) {
        return GRAPHICS_ERROR_LOCK;
    }
    
    GRAPHICS_ERROR_E ret      = GRAPHICS_ERROR_NONE;
    GFX_ELEMENT_S *gfxElement = NULL;
    
    uint32_t nbElements;
    if (pData->gfxElementsList->getNbElements(pData->gfxElementsList, &nbElements) != LIST_ERROR_NONE) {
        Loge("Failed to get number of elements");
        ret = GRAPHICS_ERROR_LIST;
        goto exit;
    }
    
    while (nbElements > 0) {
        if (pData->gfxElementsList->getElement(pData->gfxElementsList, (void*)&gfxElement) != LIST_ERROR_NONE) {
            Loge("Failed to retrieve element");
            continue;
        }
        
        if (!gfxElement->isVisible) {
            continue;
        }
        
        if (gfxElement->isClickable
            && (gfxEvent->rect.x >= gfxElement->rect.x) && (gfxEvent->rect.x <= gfxElement->rect.x + gfxElement->rect.w)
            && (gfxEvent->rect.y >= gfxElement->rect.y) && (gfxEvent->rect.y <= gfxElement->rect.y + gfxElement->rect.h)) {
            gfxEvent->gfxElementName  = strdup(gfxElement->name);
            gfxEvent->gfxElementPData = gfxElement->pData;
            break;
        }
            
        nbElements--;
    }
    
    if (nbElements <= 0) {
        ret = GRAPHICS_ERROR_LIST;
    }
    
exit:
    (void)pData->gfxElementsList->unlock(pData->gfxElementsList);
    
    return ret;
}

/*!
 *
 */
static GRAPHICS_ERROR_E handleGfxEvent_f(GRAPHICS_S *obj, GFX_EVENT_S *gfxEvent)
{
    assert(obj && obj->pData && gfxEvent);

    if (gfxEvent->gfxElementName || gfxEvent->gfxElementPData) {
        Loge("Bad params - Element's name and pData should be null");
        return GRAPHICS_ERROR_PARAMS;
    }

    GRAPHICS_PRIVATE_DATA_S *pData = (GRAPHICS_PRIVATE_DATA_S*)(obj->pData);

    switch (gfxEvent->type) {
        case GFX_EVENT_TYPE_QUIT:
        case GFX_EVENT_TYPE_ESC:
            if (pData->params.onGfxEventCb) {
                pData->params.onGfxEventCb(gfxEvent, pData->params.userData);
            }
            else {
                pData->quit = 1;
            }
            break;

        case GFX_EVENT_TYPE_ENTER:
            if (pData->gfxElementsList->lock(pData->gfxElementsList) == LIST_ERROR_NONE) {
                if (pData->focusedElement && pData->focusedElement->isClickable) {
                    gfxEvent->gfxElementName  = strdup(pData->focusedElement->name);
                    gfxEvent->gfxElementPData = pData->focusedElement->pData;
                }
                (void)pData->gfxElementsList->unlock(pData->gfxElementsList);
            }

            if (gfxEvent->gfxElementName && pData->params.onGfxEventCb) {
                pData->params.onGfxEventCb(gfxEvent, pData->params.userData);
            }
            break;

        case GFX_EVENT_TYPE_CLICK:
            if (pData->params.onGfxEventCb && (getClickedElement_f(obj, gfxEvent) == GRAPHICS_ERROR_NONE)) {
                setFocus_f(obj, gfxEvent->gfxElementName);
                pData->params.onGfxEventCb(gfxEvent, pData->params.userData);
            }
            break;

        case GFX_EVENT_TYPE_MOVE_LEFT:
            if (pData->gfxElementsList->lock(pData->gfxElementsList) == LIST_ERROR_NONE) {
                if (pData->focusedElement && (strlen(pData->focusedElement->nav.left) != 0)) {
                    gfxEvent->gfxElementName = strdup(pData->focusedElement->nav.left);
                }
                (void)pData->gfxElementsList->unlock(pData->gfxElementsList);
            }

            if (gfxEvent->gfxElementName) {
                Logd("LEFT: moving to : \"%s\"", gfxEvent->gfxElementName);
                setFocus_f(obj, gfxEvent->gfxElementName);
            }
            break;

        case GFX_EVENT_TYPE_MOVE_UP:
            if (pData->gfxElementsList->lock(pData->gfxElementsList) == LIST_ERROR_NONE) {
                if (pData->focusedElement && (strlen(pData->focusedElement->nav.up) != 0)) {
                    gfxEvent->gfxElementName = strdup(pData->focusedElement->nav.up);
                }
                (void)pData->gfxElementsList->unlock(pData->gfxElementsList);
            }

            if (gfxEvent->gfxElementName) {
                Logd("UP: moving to : \"%s\"", gfxEvent->gfxElementName);
                setFocus_f(obj, gfxEvent->gfxElementName);
            }
            break;

        case GFX_EVENT_TYPE_MOVE_RIGHT:
            if (pData->gfxElementsList->lock(pData->gfxElementsList) == LIST_ERROR_NONE) {
                if (pData->focusedElement && (strlen(pData->focusedElement->nav.right) != 0)) {
                    gfxEvent->gfxElementName = strdup(pData->focusedElement->nav.right);
                }
                (void)pData->gfxElementsList->unlock(pData->gfxElementsList);
            }

            if (gfxEvent->gfxElementName) {
                Logd("RIGHT: moving to : \"%s\"", gfxEvent->gfxElementName);
                setFocus_f(obj, gfxEvent->gfxElementName);
            }
            break;

        case GFX_EVENT_TYPE_MOVE_DOWN:
            if (pData->gfxElementsList->lock(pData->gfxElementsList) == LIST_ERROR_NONE) {
                if (pData->focusedElement && (strlen(pData->focusedElement->nav.down) != 0)) {
                    gfxEvent->gfxElementName = strdup(pData->focusedElement->nav.down);
                }
                (void)pData->gfxElementsList->unlock(pData->gfxElementsList);
            }

            if (gfxEvent->gfxElementName) {
                Logd("DOWN: moving to : \"%s\"", gfxEvent->gfxElementName);
                setFocus_f(obj, gfxEvent->gfxElementName);
            }
            break;

        default:
            ;
    }

    if (gfxEvent->gfxElementName) {
        free(gfxEvent->gfxElementName);
        gfxEvent->gfxElementName  = NULL;
        gfxEvent->gfxElementPData = NULL;
    }

    return GRAPHICS_ERROR_NONE;
}

/*!
 *
 */
static uint8_t compareCb(LIST_S *obj, void *elementToCheck, void *userData)
{
    assert(obj && elementToCheck && userData);
    
    GFX_ELEMENT_S *gfxElement   = (GFX_ELEMENT_S*)elementToCheck;
    char *nameOfElementToRemove = (char*)userData;
    
    return (strncmp(nameOfElementToRemove, gfxElement->name, MAX_NAME_SIZE) == 0);
}

/*!
 *
 */
static void releaseCb(LIST_S *obj, void *element)
{
    assert(obj && element);
    
    GRAPHICS_PRIVATE_DATA_S *pData = (GRAPHICS_PRIVATE_DATA_S*)(obj->pData);
    GFX_ELEMENT_S *gfxElement      = (GFX_ELEMENT_S*)element;
    
    gfxElement->pData = NULL;
    
    free(gfxElement);
    gfxElement = NULL;
}

/*!
 *
 */
static void browseCb(LIST_S *obj, void *element, void *dataProvidedToBrowseFunction)
{
    assert(obj && element);
    
    GFX_ELEMENT_S *gfxElement = (GFX_ELEMENT_S*)element;
    GRAPHICS_S    *gfxObj     = (GRAPHICS_S*)dataProvidedToBrowseFunction;
    
    if (updateElement_f(gfxObj, gfxElement) != GRAPHICS_ERROR_NONE) {
        //Loge("Failed to update element : \"%s\"", gfxElement->name);
    }
}
