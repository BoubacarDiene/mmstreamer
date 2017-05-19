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

#include <pthread.h>

#include "utils/Log.h"
#include "utils/List.h"

#include "graphics/FbDev.h"
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

    pthread_mutex_t    lock;
    
    GRAPHICS_PARAMS_S  params;
    FBDEV_INFOS_S      fbInfos;
    
    LIST_S             *gfxElementsList;
    
    GFX_ELEMENT_S      *focusedElement;
    GFX_ELEMENT_S      *lastDrawnElement;
    GFX_ELEMENT_S      *videoElement;

    FBDEV_S            *fbDevObj;
    DRAWER_S           *drawerObj;
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

    if (pthread_mutex_init(&pData->lock, NULL) != 0) {
        Loge("pthread_mutex_init() failed");
        goto lockExit;
    }

    LIST_PARAMS_S listParams;
    memset(&listParams, '\0', sizeof(LIST_PARAMS_S));
    listParams.compareCb = compareCb;
    listParams.releaseCb = releaseCb;
    listParams.browseCb  = browseCb;
    
    if (List_Init(&pData->gfxElementsList, &listParams) != LIST_ERROR_NONE) {
        goto listExit;
    }
    
    if (FbDev_Init(&pData->fbDevObj) != FBDEV_ERROR_NONE) {
        goto fbDevExit;
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

fbDevExit:
    (void)List_UnInit(&pData->gfxElementsList);

listExit:
    (void)pthread_mutex_destroy(&pData->lock);

lockExit:
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

    (void)FbDev_UnInit(&pData->fbDevObj);
    (void)List_UnInit(&pData->gfxElementsList);

    (void)pthread_mutex_destroy(&pData->lock);

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

    if (pthread_mutex_lock(&pData->lock) != 0) {
        Loge("pthread_mutex_lock() failed");
        return GRAPHICS_ERROR_LOCK;
    }

    if (pData->drawerObj) {
        Loge("Drawer already initialized");
        goto drawerInitExit;
    }

    memcpy(&pData->params.screenParams, &params->screenParams, sizeof(GFX_SCREEN_S));
    memcpy(&pData->params.colorOnFocus, &params->colorOnFocus, sizeof(GFX_COLOR_S));
    memcpy(&pData->params.colorOnBlur,  &params->colorOnBlur, sizeof(GFX_COLOR_S));
    memcpy(&pData->params.colorOnReset, &params->colorOnReset, sizeof(GFX_COLOR_S));
    
    pData->params.onGfxEventCb = params->onGfxEventCb;
    pData->params.userData     = params->userData;
    
    if (Drawer_Init(&pData->drawerObj) != DRAWER_ERROR_NONE) {
        Loge("Failed to init drawer");
        goto drawerInitExit;
    }

    uint8_t opened = 0;
    if (pData->fbDevObj->open(pData->fbDevObj, params->screenParams.fbDeviceName) == FBDEV_ERROR_NONE) {
        opened = 1;

        (void)pData->fbDevObj->getInfos(pData->fbDevObj, &pData->fbInfos);

        Logd("\nFbDevice : \"%s\" / width = %u / height = %u / depth = %u\n",
                params->screenParams.fbDeviceName, pData->fbInfos.width, pData->fbInfos.height, pData->fbInfos.depth);

        if ((pData->fbInfos.width < params->screenParams.rect.w)
            || (pData->fbInfos.height < params->screenParams.rect.h)) {
            Loge("Invalid resolution - Max values : width = %u / height = %u",
                    pData->fbInfos.width, pData->fbInfos.height);
            goto fbDevExit;
        }

        if (pData->fbInfos.depth != (uint32_t)params->screenParams.bitsPerPixel) {
            (void)pData->fbDevObj->setDepth(pData->fbDevObj, (uint32_t)params->screenParams.bitsPerPixel);
        }
    }

    if (pData->drawerObj->initScreen(pData->drawerObj, &params->screenParams) != DRAWER_ERROR_NONE) {
        Loge("Failed to init screen");
        goto initScreenExit;
    }

    (void)pthread_mutex_unlock(&pData->lock);

    return GRAPHICS_ERROR_NONE;

initScreenExit:
    if (opened && (pData->fbInfos.depth != (uint32_t)params->screenParams.bitsPerPixel)) {
        (void)pData->fbDevObj->restore(pData->fbDevObj);
    }

fbDevExit:
    if (opened) {
        (void)pData->fbDevObj->close(pData->fbDevObj);
    }
    (void)Drawer_UnInit(&pData->drawerObj);

drawerInitExit:
    (void)pthread_mutex_unlock(&pData->lock);

    return GRAPHICS_ERROR_DRAWER;
}

/*!
 *
 */
static GRAPHICS_ERROR_E destroyDrawer_f(GRAPHICS_S *obj)
{
    assert(obj && obj->pData);
    
    GRAPHICS_PRIVATE_DATA_S *pData = (GRAPHICS_PRIVATE_DATA_S*)(obj->pData);
    GRAPHICS_ERROR_E ret           = GRAPHICS_ERROR_NONE;

    if (pthread_mutex_lock(&pData->lock) != 0) {
        Loge("pthread_mutex_lock() failed");
        return GRAPHICS_ERROR_LOCK;
    }

    if (!pData->drawerObj) {
        Loge("Drawer not initialized yet");
        ret = GRAPHICS_ERROR_DRAWER;
        goto exit;
    }
    
    pData->params.userData = NULL;
    
    (void)pData->drawerObj->unInitScreen(pData->drawerObj);

    uint8_t opened = 0;
    (void)pData->fbDevObj->isOpened(pData->fbDevObj, &opened);

    if (opened) {
        if (pData->fbInfos.depth != (uint32_t)pData->params.screenParams.bitsPerPixel) {
            (void)pData->fbDevObj->restore(pData->fbDevObj);
        }
        (void)pData->fbDevObj->close(pData->fbDevObj);
    }

    (void)Drawer_UnInit(&pData->drawerObj);

exit:
    (void)pthread_mutex_unlock(&pData->lock);

    return ret;
}

/*!
 *
 */
static GRAPHICS_ERROR_E createElement_f(GRAPHICS_S *obj, GFX_ELEMENT_S **newGfxElement)
{
    assert(obj && obj->pData && newGfxElement);

    GRAPHICS_PRIVATE_DATA_S *pData = (GRAPHICS_PRIVATE_DATA_S*)(obj->pData);

    if (pthread_mutex_lock(&pData->lock) != 0) {
        Loge("pthread_mutex_lock() failed");
        return GRAPHICS_ERROR_LOCK;
    }
    
    assert((*newGfxElement = calloc(1, sizeof(GFX_ELEMENT_S))));

    (void)pthread_mutex_unlock(&pData->lock);
    
    return GRAPHICS_ERROR_NONE;
}

/*!
 *
 */
static GRAPHICS_ERROR_E pushElement_f(GRAPHICS_S *obj, GFX_ELEMENT_S *gfxElement)
{
    assert(obj && obj->pData && gfxElement);

    GRAPHICS_PRIVATE_DATA_S *pData = (GRAPHICS_PRIVATE_DATA_S*)(obj->pData);
    GRAPHICS_ERROR_E ret           = GRAPHICS_ERROR_NONE;

    if (pthread_mutex_lock(&pData->lock) != 0) {
        Loge("pthread_mutex_lock() failed");
        return GRAPHICS_ERROR_LOCK;
    }
    
    if (pData->gfxElementsList->lock(pData->gfxElementsList) != LIST_ERROR_NONE) {
        ret = GRAPHICS_ERROR_LOCK;
        goto exit;
    }
    
    pData->gfxElementsList->add(pData->gfxElementsList, (void*)gfxElement);
    
    (void)pData->gfxElementsList->unlock(pData->gfxElementsList);

exit:
    (void)pthread_mutex_unlock(&pData->lock);

    return ret;
}

/*!
 *
 */
static GRAPHICS_ERROR_E removeElement_f(GRAPHICS_S *obj, char *gfxElementName)
{
    assert(obj && obj->pData && gfxElementName);
    
    GRAPHICS_PRIVATE_DATA_S *pData = (GRAPHICS_PRIVATE_DATA_S*)(obj->pData);
    GRAPHICS_ERROR_E ret           = GRAPHICS_ERROR_NONE;

    if (pthread_mutex_lock(&pData->lock) != 0) {
        Loge("pthread_mutex_lock() failed");
        return GRAPHICS_ERROR_LOCK;
    }
    
    if (pData->gfxElementsList->lock(pData->gfxElementsList) != LIST_ERROR_NONE) {
        ret = GRAPHICS_ERROR_LOCK;
        goto exit;
    }
    
    pData->gfxElementsList->remove(pData->gfxElementsList, (void*)gfxElementName);
    
    (void)pData->gfxElementsList->unlock(pData->gfxElementsList);

exit:
    (void)pthread_mutex_unlock(&pData->lock);

    return ret;
}

/*!
 *
 */
static GRAPHICS_ERROR_E removeAll_f(GRAPHICS_S *obj)
{
    assert(obj && obj->pData);
    
    GRAPHICS_PRIVATE_DATA_S *pData = (GRAPHICS_PRIVATE_DATA_S*)(obj->pData);
    GRAPHICS_ERROR_E ret           = GRAPHICS_ERROR_NONE;

    if (pthread_mutex_lock(&pData->lock) != 0) {
        Loge("pthread_mutex_lock() failed");
        return GRAPHICS_ERROR_LOCK;
    }
    
    if (pData->gfxElementsList->lock(pData->gfxElementsList) != LIST_ERROR_NONE) {
        ret = GRAPHICS_ERROR_LOCK;
        goto exit;
    }
    
    pData->gfxElementsList->removeAll(pData->gfxElementsList);
    
    (void)pData->gfxElementsList->unlock(pData->gfxElementsList);

exit:
    (void)pthread_mutex_unlock(&pData->lock);
    
    return ret;
}

/*!
 *
 */
static GRAPHICS_ERROR_E setVisible_f(GRAPHICS_S *obj, char *gfxElementName, uint8_t isVisible)
{
    assert(obj && obj->pData && gfxElementName);
    
    GRAPHICS_PRIVATE_DATA_S *pData = (GRAPHICS_PRIVATE_DATA_S*)(obj->pData);
    GRAPHICS_ERROR_E ret           = GRAPHICS_ERROR_NONE;

    if (pthread_mutex_lock(&pData->lock) != 0) {
        Loge("pthread_mutex_lock() failed");
        return GRAPHICS_ERROR_LOCK;
    }
    
    if (pData->gfxElementsList->lock(pData->gfxElementsList) != LIST_ERROR_NONE) {
        ret = GRAPHICS_ERROR_LOCK;
        goto lockExit;
    }
    
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
    else if ((ret = getElement_f(obj, gfxElementName, &gfxElement)) != GRAPHICS_ERROR_NONE) {
        goto elementExit;
    }
    
    if (gfxElement->isVisible == isVisible) {
        Logd("Same visibility => not changed");
        goto elementExit;
    }
    
    gfxElement->isVisible      = isVisible;
    gfxElement->surfaceUpdated = 0;
 
    if ((ret = updateGroup_f(obj, gfxElement->groupName, NULL)) != GRAPHICS_ERROR_NONE) {
        Loge("Failed to update group : \"%s\"", gfxElement->groupName);
        gfxElement->isVisible = !isVisible;
    }

    if (!gfxElement->isVisible && gfxElement->hasFocus) {
        Logw("Invisible element \"%s\" still has focus - Please, call setFocus on a visible element", gfxElement->name);
    }

elementExit:    
    (void)pData->gfxElementsList->unlock(pData->gfxElementsList);

lockExit:
    (void)pthread_mutex_unlock(&pData->lock);

    return ret;
}

/*!
 *
 */
static GRAPHICS_ERROR_E setFocus_f(GRAPHICS_S *obj, char *gfxElementName)
{
    assert(obj && obj->pData && gfxElementName);
    
    GRAPHICS_PRIVATE_DATA_S *pData = (GRAPHICS_PRIVATE_DATA_S*)(obj->pData);
    GRAPHICS_ERROR_E ret           = GRAPHICS_ERROR_NONE;

    if (pthread_mutex_lock(&pData->lock) != 0) {
        Loge("pthread_mutex_lock() failed");
        return GRAPHICS_ERROR_LOCK;
    }
    
    if (pData->gfxElementsList->lock(pData->gfxElementsList) != LIST_ERROR_NONE) {
        ret = GRAPHICS_ERROR_LOCK;
        goto lockExit;
    }
    
    GFX_ELEMENT_S *gfxElement = NULL;
    
    if (pData->focusedElement && !strncmp(pData->focusedElement->name, gfxElementName, MAX_NAME_SIZE)) {
        Logd("\"%s\" already has focus", gfxElementName);
        goto elementExit;
    }
    else if (pData->videoElement && !strncmp(pData->videoElement->name, gfxElementName, MAX_NAME_SIZE)) {
        gfxElement = pData->videoElement;
    }
    else if (pData->lastDrawnElement && !strncmp(pData->lastDrawnElement->name, gfxElementName, MAX_NAME_SIZE)) {
        gfxElement = pData->lastDrawnElement;
    }
    else if ((ret = getElement_f(obj, gfxElementName, &gfxElement)) != GRAPHICS_ERROR_NONE) {
        goto elementExit;
    }
    
    if (!gfxElement->isFocusable) {
        Logw("\"%s\" is not focusable", gfxElementName);
        goto elementExit;
    }
    
    if (!gfxElement->isVisible) {
        Logw("\"%s\" is not visible", gfxElementName);
        goto elementExit;
    }
    
    gfxElement->hasFocus = 1;
        
    if ((ret = updateElement_f(obj, gfxElement)) != GRAPHICS_ERROR_NONE) {
        Loge("Failed to update element : \"%s\"", gfxElementName);
    }

elementExit: 
    (void)pData->gfxElementsList->unlock(pData->gfxElementsList);

lockExit:
    (void)pthread_mutex_unlock(&pData->lock);
    
    return ret;
}

/*!
 *
 */
static GRAPHICS_ERROR_E setClickable_f(GRAPHICS_S *obj, char *gfxElementName, uint8_t isClickable)
{
    assert(obj && obj->pData && gfxElementName);
    
    GRAPHICS_PRIVATE_DATA_S *pData = (GRAPHICS_PRIVATE_DATA_S*)(obj->pData);
    GRAPHICS_ERROR_E ret           = GRAPHICS_ERROR_NONE;

    if (pthread_mutex_lock(&pData->lock) != 0) {
        Loge("pthread_mutex_lock() failed");
        return GRAPHICS_ERROR_LOCK;
    }
    
    if (pData->gfxElementsList->lock(pData->gfxElementsList) != LIST_ERROR_NONE) {
        ret = GRAPHICS_ERROR_LOCK;
        goto lockExit;
    }
    
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
    else if ((ret = getElement_f(obj, gfxElementName, &gfxElement)) != GRAPHICS_ERROR_NONE) {
        goto elementExit;
    }
    
    if (gfxElement->isClickable == isClickable) {
        Logd("Same value => not changed");
        goto elementExit;
    }
    
    gfxElement->isClickable = isClickable;

elementExit:
    (void)pData->gfxElementsList->unlock(pData->gfxElementsList);

lockExit:
    (void)pthread_mutex_unlock(&pData->lock);
    
    return ret;
}

/*!
 *
 */
static GRAPHICS_ERROR_E setNav_f(GRAPHICS_S *obj, char *gfxElementName, GFX_NAV_S *nav)
{
    assert(obj && obj->pData && gfxElementName && nav);

    GRAPHICS_PRIVATE_DATA_S *pData = (GRAPHICS_PRIVATE_DATA_S*)(obj->pData);
    GRAPHICS_ERROR_E ret           = GRAPHICS_ERROR_NONE;

    if (pthread_mutex_lock(&pData->lock) != 0) {
        Loge("pthread_mutex_lock() failed");
        return GRAPHICS_ERROR_LOCK;
    }

    if (pData->gfxElementsList->lock(pData->gfxElementsList) != LIST_ERROR_NONE) {
        ret = GRAPHICS_ERROR_LOCK;
        goto lockExit;
    }

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
    else if ((ret = getElement_f(obj, gfxElementName, &gfxElement)) != GRAPHICS_ERROR_NONE) {
        goto elementExit;
    }

    memcpy(&gfxElement->nav, nav, sizeof(GFX_NAV_S));

elementExit:
    (void)pData->gfxElementsList->unlock(pData->gfxElementsList);

lockExit:
    (void)pthread_mutex_unlock(&pData->lock);

    return ret;
}

/*!
 *
 */
static GRAPHICS_ERROR_E setData_f(GRAPHICS_S *obj, char *gfxElementName, void *data)
{
    assert(obj && obj->pData && gfxElementName && data);
    
    GRAPHICS_PRIVATE_DATA_S *pData = (GRAPHICS_PRIVATE_DATA_S*)(obj->pData);
    GRAPHICS_ERROR_E ret           = GRAPHICS_ERROR_NONE;

    if (pthread_mutex_lock(&pData->lock) != 0) {
        Loge("pthread_mutex_lock() failed");
        return GRAPHICS_ERROR_LOCK;
    }
    
    if (!pData->drawerObj) {
        ret = GRAPHICS_ERROR_DRAWER;
        goto lockExit;
    }
    
    if (pData->gfxElementsList->lock(pData->gfxElementsList) != LIST_ERROR_NONE) {
        ret = GRAPHICS_ERROR_LOCK;
        goto lockExit;
    }

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
    else if ((ret = getElement_f(obj, gfxElementName, &gfxElement)) != GRAPHICS_ERROR_NONE) {
        goto elementExit;
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
            if (pData->drawerObj->setBgColor(pData->drawerObj, &gfxElement->rect, &pData->params.colorOnReset) != DRAWER_ERROR_NONE) {
                Loge("Failed to set background color of element \"%s\"", gfxElement->name);
                ret = GRAPHICS_ERROR_DRAWER;
                goto elementExit;
            }
            memcpy(&gfxElement->data.text, (GFX_TEXT_S*)data, sizeof(GFX_TEXT_S));
            break;
                
        default:
            ret = GRAPHICS_ERROR_DRAWER;
            goto elementExit;
    }
    
    if (gfxElement->redrawGroup) {
        if ((ret = updateGroup_f(obj, gfxElement->groupName, NULL)) != GRAPHICS_ERROR_NONE) {
            Loge("Failed to update group : \"%s\"", gfxElement->groupName);
        }
    }
    else if ((ret = updateElement_f(obj, gfxElement)) != GRAPHICS_ERROR_NONE) {
        Loge("Failed to update element : \"%s\"", gfxElementName);
    }
    
elementExit:
    (void)pData->gfxElementsList->unlock(pData->gfxElementsList);

lockExit:
    (void)pthread_mutex_unlock(&pData->lock);
    
    return ret;
}

/*!
 *
 */
static GRAPHICS_ERROR_E saveVideoFrame_f(GRAPHICS_S *obj, BUFFER_S *buffer, GFX_IMAGE_S *inOut)
{
    assert(obj && obj->pData && inOut);
    
    GRAPHICS_PRIVATE_DATA_S *pData = (GRAPHICS_PRIVATE_DATA_S*)(obj->pData);
    GRAPHICS_ERROR_E ret           = GRAPHICS_ERROR_DRAWER;

    if (pthread_mutex_lock(&pData->lock) != 0) {
        Loge("pthread_mutex_lock() failed");
        return GRAPHICS_ERROR_LOCK;
    }

    if (!pData->drawerObj) {
        Loge("Drawer not initialized yet");
        goto lockExit;
    }

    if (pData->gfxElementsList->lock(pData->gfxElementsList) != LIST_ERROR_NONE) {
        ret = GRAPHICS_ERROR_LOCK;
        goto lockExit;
    }

    BUFFER_S videoBuffer = { 0 };

    if (!buffer) {
        if (!pData->videoElement) {
            Loge("No video frame to save");
            ret = GRAPHICS_ERROR_PARAMS;
            goto elementExit;
        }
        Logd("Creating screenshot from the last drawn video frame");
        videoBuffer.data   = pData->videoElement->data.buffer.data;
        videoBuffer.length = pData->videoElement->data.buffer.length;
    }
    else {
        videoBuffer.data   = buffer->data;
        videoBuffer.length = buffer->length;
    }
    
    if (pData->drawerObj->saveBuffer(pData->drawerObj, &videoBuffer, inOut) != DRAWER_ERROR_NONE) {
        goto elementExit;
    }

    ret = GRAPHICS_ERROR_NONE;

elementExit:
    (void)pData->gfxElementsList->unlock(pData->gfxElementsList);

lockExit:
    (void)pthread_mutex_unlock(&pData->lock);

    return ret;
}

/*!
 *
 */
static GRAPHICS_ERROR_E saveVideoElement_f(GRAPHICS_S *obj, char *gfxElementName, GFX_IMAGE_S *inOut)
{
    assert(obj && obj->pData && gfxElementName && inOut);

    GRAPHICS_PRIVATE_DATA_S *pData = (GRAPHICS_PRIVATE_DATA_S*)(obj->pData);
    GRAPHICS_ERROR_E ret           = GRAPHICS_ERROR_PARAMS;

    if (pthread_mutex_lock(&pData->lock) != 0) {
        Loge("pthread_mutex_lock() failed");
        return GRAPHICS_ERROR_LOCK;
    }

    if (!pData->drawerObj) {
        Loge("Drawer not initialized yet");
        ret = GRAPHICS_ERROR_LOCK;
        goto lockExit;
    }

    if (pData->gfxElementsList->lock(pData->gfxElementsList) != LIST_ERROR_NONE) {
        ret = GRAPHICS_ERROR_LOCK;
        goto lockExit;
    }

    BUFFER_S videoBuffer = { 0 };

    if (!pData->videoElement && (strncmp(pData->videoElement->name, gfxElementName, MAX_NAME_SIZE) == 0)) {
        videoBuffer.data   = pData->videoElement->data.buffer.data;
        videoBuffer.length = pData->videoElement->data.buffer.length;
    }
    else {
        GFX_ELEMENT_S *gfxElement = NULL;

        if (getElement_f(obj, gfxElementName, &gfxElement) != GRAPHICS_ERROR_NONE) {
            Loge("Element \"%s\" not found", gfxElementName);
            goto elementExit;
        }

        if (gfxElement->type != GFX_ELEMENT_TYPE_VIDEO) {
            Loge("\"%s\" is not a video element", gfxElementName);
            goto elementExit;
        }

        if (!gfxElement->data.buffer.data || (gfxElement->data.buffer.length == 0)) {
            Logw("No video frame to save");
            goto elementExit;
        }

        videoBuffer.data   = gfxElement->data.buffer.data;
        videoBuffer.length = gfxElement->data.buffer.length;
    }

    if (pData->drawerObj->saveBuffer(pData->drawerObj, &videoBuffer, inOut) != DRAWER_ERROR_NONE) {
        ret = GRAPHICS_ERROR_DRAWER;
        goto elementExit;
    }

    ret = GRAPHICS_ERROR_NONE;

elementExit:
    (void)pData->gfxElementsList->unlock(pData->gfxElementsList);

lockExit:
    (void)pthread_mutex_unlock(&pData->lock);

    return ret;
}

/*!
 *
 */
static GRAPHICS_ERROR_E takeScreenshot_f(GRAPHICS_S *obj, GFX_IMAGE_S *inOut)
{
    assert(obj && obj->pData && inOut);

    GRAPHICS_PRIVATE_DATA_S *pData = (GRAPHICS_PRIVATE_DATA_S*)(obj->pData);
    GRAPHICS_ERROR_E ret           = GRAPHICS_ERROR_NONE;

    if (pthread_mutex_lock(&pData->lock) != 0) {
        Loge("pthread_mutex_lock() failed");
        return GRAPHICS_ERROR_LOCK;
    }

    if (!pData->drawerObj
        || (pData->drawerObj->saveScreen(pData->drawerObj, inOut) != DRAWER_ERROR_NONE)) {
        ret = GRAPHICS_ERROR_DRAWER;
    }

    (void)pthread_mutex_unlock(&pData->lock);

    return ret;
}

/*!
 *
 */
static GRAPHICS_ERROR_E drawAllElements_f(GRAPHICS_S *obj)
{
    assert(obj && obj->pData);
    
    GRAPHICS_PRIVATE_DATA_S *pData = (GRAPHICS_PRIVATE_DATA_S*)(obj->pData);
    GRAPHICS_ERROR_E ret           = GRAPHICS_ERROR_NONE;

    if (pthread_mutex_lock(&pData->lock) != 0) {
        Loge("pthread_mutex_lock() failed");
        return GRAPHICS_ERROR_LOCK;
    }
    
    if (pData->gfxElementsList->lock(pData->gfxElementsList) != LIST_ERROR_NONE) {
        ret = GRAPHICS_ERROR_LOCK;
        goto lockExit;
    }
    
    if (pData->gfxElementsList->browseElements(pData->gfxElementsList, obj) != LIST_ERROR_NONE) {
        Loge("Failed to browse elements' list");
        ret = GRAPHICS_ERROR_LIST;
    }
    
    (void)pData->gfxElementsList->unlock(pData->gfxElementsList);

lockExit:
    (void)pthread_mutex_unlock(&pData->lock);

    return ret;
}

/*!
 *
 */
static GRAPHICS_ERROR_E simulateGfxEvent_f(GRAPHICS_S *obj, GFX_EVENT_S *gfxEvent)
{
    assert(obj && obj->pData && gfxEvent);

    GRAPHICS_PRIVATE_DATA_S *pData = (GRAPHICS_PRIVATE_DATA_S*)(obj->pData);

    if (pData->quit) {
        Logw("Too late! Graphics module is sstopped or being stopped");
        return GRAPHICS_ERROR_NONE;
    }

    return handleGfxEvent_f(obj, gfxEvent);
}

/*!
 *
 */
static GRAPHICS_ERROR_E handleGfxEvents_f(GRAPHICS_S *obj)
{
    assert(obj && obj->pData);

    GRAPHICS_PRIVATE_DATA_S *pData = (GRAPHICS_PRIVATE_DATA_S*)(obj->pData);

    if (!pData->drawerObj) {
        Loge("Drawer not initialized yet");
        return GRAPHICS_ERROR_DRAWER;
    }

    GFX_EVENT_S evt;

    while (!pData->quit && pData->drawerObj) {
        if (pData->drawerObj->getEvent(pData->drawerObj, &evt) != DRAWER_ERROR_NONE) {
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

    GRAPHICS_PRIVATE_DATA_S *pData = (GRAPHICS_PRIVATE_DATA_S*)(obj->pData);

    if (pthread_mutex_lock(&pData->lock) != 0) {
        Loge("pthread_mutex_lock() failed");
        return GRAPHICS_ERROR_LOCK;
    }
    
    pData->quit = 1;

    (void)pthread_mutex_unlock(&pData->lock);

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

    if (!pData->drawerObj) {
        Loge("Drawer not initialized yet");
        return GRAPHICS_ERROR_DRAWER;
    }

    if (!gfxElement->isVisible) {
        // Clear surface
        if (!gfxElement->surfaceUpdated
            && pData->drawerObj->setBgColor(pData->drawerObj, &gfxElement->rect, &pData->params.colorOnReset) != DRAWER_ERROR_NONE) {
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
                if (pData->drawerObj->setBgColor(pData->drawerObj,
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
            
            if (pData->drawerObj->setBgColor(pData->drawerObj, &gfxElement->rect, &pData->params.colorOnFocus) != DRAWER_ERROR_NONE) {
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
                    && pData->drawerObj->setBgColor(pData->drawerObj, &gfxElement->rect, &pData->params.colorOnBlur) != DRAWER_ERROR_NONE) {
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

    if (!pData->drawerObj) {
        Loge("Drawer not initialized yet");
        return GRAPHICS_ERROR_DRAWER;
    }

    switch (gfxElement->type) {
        case GFX_ELEMENT_TYPE_VIDEO:
            if (pData->drawerObj->drawVideo(pData->drawerObj, &gfxElement->rect, &gfxElement->data.buffer) != DRAWER_ERROR_NONE) {
                ret = GRAPHICS_ERROR_DRAWER;
                goto exit;
            }
            pData->videoElement = gfxElement;
            break;
                
        case GFX_ELEMENT_TYPE_IMAGE:
            if (pData->drawerObj->drawImage(pData->drawerObj, &gfxElement->rect, &gfxElement->data.image) != DRAWER_ERROR_NONE) {
                ret = GRAPHICS_ERROR_DRAWER;
                goto exit;
            }
            break;
                
        case GFX_ELEMENT_TYPE_TEXT:
            if (pData->drawerObj->drawText(pData->drawerObj, &gfxElement->rect, &gfxElement->data.text) != DRAWER_ERROR_NONE) {
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
