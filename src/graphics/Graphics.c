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
* \file Graphics.c
* \brief Graphics elements management
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// HEADERS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#include <pthread.h>
#include <time.h>

#include "utils/Log.h"
#include "utils/List.h"
#include "utils/Task.h"

#include "graphics/FbDev.h"
#include "graphics/Drawer.h"
#include "graphics/Graphics.h"

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// MACROS ////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "Graphics"

#define SIMULATE_EVENT_TASK_NAME "gfx-simulateEvtTask"

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// TYPES /////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

struct gfx_element_reserved_s {
    enum gfx_target_e target;
    uint8_t           surfaceUpdated;
};

struct graphics_list_element_s {
    time_t             seconds;
    struct gfx_event_s event;
};

struct graphics_task_s {
    volatile uint8_t     quit;

    struct list_s        *list;

    struct task_s        *task;
    struct task_params_s taskParams;

    sem_t                sem;
};

struct graphics_private_data_s {
    volatile uint8_t         quit;

    pthread_mutex_t          gfxLock;
    pthread_mutex_t          evtLock;

    struct graphics_params_s params;
    struct fbdev_infos_s     fbInfos;

    struct list_s            *gfxElementsList;

    struct gfx_element_s     *focusedElement;
    struct gfx_element_s     *lastDrawnElement;
    struct gfx_element_s     *videoElement;

    struct graphics_task_s   simulateEvtTask;

    struct fbdev_s           *fbDevObj;
    struct drawer_s          *drawerObj;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////// PUBLIC FUNCTIONS PROTOTYPES //////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */


static enum graphics_error_e createDrawer_f(struct graphics_s *obj,
                                            struct graphics_params_s *params);
static enum graphics_error_e destroyDrawer_f(struct graphics_s *obj);

static enum graphics_error_e createElement_f(struct graphics_s *obj,
                                             struct gfx_element_s **newGfxElement);
static enum graphics_error_e pushElement_f(struct graphics_s *obj,
                                           struct gfx_element_s *gfxElement);
static enum graphics_error_e removeElement_f(struct graphics_s *obj, char *gfxElementName);
static enum graphics_error_e removeAll_f(struct graphics_s *obj);

static enum graphics_error_e setVisible_f(struct graphics_s *obj, char *gfxElementName,
                                          uint8_t isVisible);
static enum graphics_error_e setFocus_f(struct graphics_s *obj, char *gfxElementName);
static enum graphics_error_e setClickable_f(struct graphics_s *obj, char *gfxElementName,
                                            uint8_t isClickable);
static enum graphics_error_e setNav_f(struct graphics_s *obj, char *gfxElementName,
                                      struct gfx_nav_s *nav);
static enum graphics_error_e setData_f(struct graphics_s *obj, char *gfxElementName, void *data);

static enum graphics_error_e saveVideoFrame_f(struct graphics_s *obj, struct buffer_s *buffer,
                                              struct gfx_image_s *inOut);
static enum graphics_error_e saveVideoElement_f(struct graphics_s *obj, char *gfxElementName,
                                                struct gfx_image_s *inOut);
static enum graphics_error_e takeScreenshot_f(struct graphics_s *obj, struct gfx_image_s *inOut);

static enum graphics_error_e drawAllElements_f(struct graphics_s *obj);

static enum graphics_error_e simulateGfxEvent_f(struct graphics_s *obj,
                                                struct gfx_event_s *gfxEvent);
static enum graphics_error_e handleGfxEvents_f(struct graphics_s *obj);

static enum graphics_error_e quit_f(struct graphics_s *obj);

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////// PRIVATE FUNCTIONS PROTOTYPES /////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

static enum graphics_error_e updateGroup_f(struct graphics_s *obj, char *groupName,
                                           char *gfxElementToIgnore);
static enum graphics_error_e updateElement_f(struct graphics_s *obj,
                                             struct gfx_element_s *gfxElement);
static enum graphics_error_e drawElement_f(struct graphics_s *obj,
                                           struct gfx_element_s *gfxElement);
static enum graphics_error_e getElement_f(struct graphics_s *obj, char *gfxElementName,
                                          struct gfx_element_s **gfxElement);
static enum graphics_error_e getClickedElement_f(struct graphics_s *obj,
                                                 struct gfx_event_s *gfxEvent);
static enum graphics_error_e handleGfxEvent_f(struct graphics_s *obj,
                                              struct gfx_event_s *gfxEvent);

static void taskFct_f(struct task_params_s *params);

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////////// CALLBACKS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

static uint8_t compareElementsCb(struct list_s *obj, void *elementToCheck, void *userData);
static void releaseElementCb(struct list_s *obj, void *element);
static void browseElementsCb(struct list_s *obj, void *element, void *userData);

static uint8_t compareEventsCb(struct list_s *obj, void *elementToCheck, void *userData);
static void releaseEventCb(struct list_s *obj, void *element);

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// INITIALIZER //////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
enum graphics_error_e Graphics_Init(struct graphics_s **obj)
{
    ASSERT(obj && (*obj = calloc(1, sizeof(struct graphics_s))));
    
    struct graphics_private_data_s *pData;
    ASSERT((pData = calloc(1, sizeof(struct graphics_private_data_s))));

    if (pthread_mutex_init(&pData->gfxLock, NULL) != 0) {
        Loge("pthread_mutex_init() failed");
        goto gfxLockExit;
    }

    if (pthread_mutex_init(&pData->evtLock, NULL) != 0) {
        Loge("pthread_mutex_init() failed");
        goto evtLockExit;
    }

    struct graphics_task_s *simulateEvtTask = &pData->simulateEvtTask;

    struct list_params_s elementsListParams = {0};
    elementsListParams.compareCb = compareElementsCb;
    elementsListParams.releaseCb = releaseElementCb;
    elementsListParams.browseCb  = browseElementsCb;
    
    if (List_Init(&pData->gfxElementsList, &elementsListParams) != LIST_ERROR_NONE) {
        goto listExit;
    }

    struct list_params_s eventsListParams = {0};
    eventsListParams.compareCb = compareEventsCb;
    eventsListParams.releaseCb = releaseEventCb;
    eventsListParams.browseCb  = NULL;

    if (List_Init(&simulateEvtTask->list, &eventsListParams) != LIST_ERROR_NONE) {
        Loge("List_Init() failed");
        goto simulateEvtListExit;
    }

    if (Task_Init(&simulateEvtTask->task) != TASK_ERROR_NONE) {
        Loge("Task_Init() failed");
        goto simulateEvtTaskExit;
    }

    if (sem_init(&simulateEvtTask->sem, 0, 0) != 0) {
        Loge("sem_init() failed");
        goto simulateEvtSemExit;
    }

    strcpy(simulateEvtTask->taskParams.name, SIMULATE_EVENT_TASK_NAME);
    simulateEvtTask->taskParams.priority = PRIORITY_DEFAULT;
    simulateEvtTask->taskParams.fct      = taskFct_f;
    simulateEvtTask->taskParams.fctData  = *obj;
    simulateEvtTask->taskParams.userData = NULL;
    simulateEvtTask->taskParams.atExit   = NULL;

    if (simulateEvtTask->task->create(simulateEvtTask->task,
                                      &simulateEvtTask->taskParams) != TASK_ERROR_NONE) {
        Loge("Failed to create simulateEvt task");
        goto simulateEvtTaskCreateExit;
    }

    if (simulateEvtTask->task->start(simulateEvtTask->task,
                                     &simulateEvtTask->taskParams) != TASK_ERROR_NONE) {
        Loge("Failed to start simulateEvtTask");
        goto simulateEvtTaskStart;
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
    (void)simulateEvtTask->task->stop(simulateEvtTask->task, &simulateEvtTask->taskParams);

simulateEvtTaskStart:
    (void)simulateEvtTask->task->destroy(simulateEvtTask->task, &simulateEvtTask->taskParams);

simulateEvtTaskCreateExit:
    (void)sem_destroy(&simulateEvtTask->sem);

simulateEvtSemExit:
    (void)Task_UnInit(&simulateEvtTask->task);

simulateEvtTaskExit:
    (void)List_UnInit(&simulateEvtTask->list);

simulateEvtListExit:
    (void)List_UnInit(&pData->gfxElementsList);

listExit:
    (void)pthread_mutex_destroy(&pData->evtLock);

evtLockExit:
    (void)pthread_mutex_destroy(&pData->gfxLock);

gfxLockExit:
    free(pData);
    free(*obj);
    *obj = NULL;

    return GRAPHICS_ERROR_INIT;
}

/*!
 *
 */
enum graphics_error_e Graphics_UnInit(struct graphics_s **obj)
{
    ASSERT(obj && *obj && (*obj)->pData);
    
    struct graphics_private_data_s *pData   = (struct graphics_private_data_s*)((*obj)->pData);
    struct graphics_task_s *simulateEvtTask = &pData->simulateEvtTask;

    (void)FbDev_UnInit(&pData->fbDevObj);

    simulateEvtTask->quit = 1;
    sem_post(&simulateEvtTask->sem);

    (void)simulateEvtTask->list->lock(simulateEvtTask->list);
    (void)simulateEvtTask->list->removeAll(simulateEvtTask->list);
    (void)simulateEvtTask->list->unlock(simulateEvtTask->list);

    (void)simulateEvtTask->task->stop(simulateEvtTask->task, &simulateEvtTask->taskParams);
    (void)simulateEvtTask->task->destroy(simulateEvtTask->task, &simulateEvtTask->taskParams);
    (void)sem_destroy(&simulateEvtTask->sem);
    (void)Task_UnInit(&simulateEvtTask->task);
    (void)List_UnInit(&simulateEvtTask->list);

    (void)List_UnInit(&pData->gfxElementsList);

    (void)pthread_mutex_destroy(&pData->evtLock);
    (void)pthread_mutex_destroy(&pData->gfxLock);

    free(pData);
    free(*obj);
    *obj = NULL;
    
    return GRAPHICS_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////// PUBLIC FUNCTIONS IMPLEMENTATION ///////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static enum graphics_error_e createDrawer_f(struct graphics_s *obj,
                                            struct graphics_params_s *params)
{
    ASSERT(obj && obj->pData && params);

    struct graphics_private_data_s *pData = (struct graphics_private_data_s*)(obj->pData);

    if (pthread_mutex_lock(&pData->gfxLock) != 0) {
        Loge("pthread_mutex_lock() failed");
        return GRAPHICS_ERROR_LOCK;
    }

    if (pData->drawerObj) {
        Loge("Drawer already initialized");
        goto drawerInitExit;
    }

    pData->params = *params;

    if (Drawer_Init(&pData->drawerObj) != DRAWER_ERROR_NONE) {
        Loge("Failed to init drawer");
        goto drawerInitExit;
    }

    uint8_t opened = 0;
    if (pData->fbDevObj->open(pData->fbDevObj,
                              params->screenParams.fbDeviceName) == FBDEV_ERROR_NONE) {
        opened = 1;

        (void)pData->fbDevObj->getInfos(pData->fbDevObj, &pData->fbInfos);

        Logd("\nFbDevice : \"%s\" / width = %u / height = %u / depth = %u\n",
                params->screenParams.fbDeviceName, pData->fbInfos.width,
                pData->fbInfos.height, pData->fbInfos.depth);

        if ((pData->fbInfos.width < params->screenParams.rect.w)
            || (pData->fbInfos.height < params->screenParams.rect.h)) {
            Loge("Invalid resolution - Max values : width = %u / height = %u",
                    pData->fbInfos.width, pData->fbInfos.height);
            goto fbDevExit;
        }

        if (pData->fbInfos.depth != (uint32_t)params->screenParams.bitsPerPixel) {
            (void)pData->fbDevObj->setDepth(pData->fbDevObj,
                                            (uint32_t)params->screenParams.bitsPerPixel);
        }
    }

    if (pData->drawerObj->initScreen(pData->drawerObj,
                                     &params->screenParams) != DRAWER_ERROR_NONE) {
        Loge("Failed to init screen");
        goto initScreenExit;
    }

    (void)pthread_mutex_unlock(&pData->gfxLock);

    return GRAPHICS_ERROR_NONE;

initScreenExit:
    if (opened
        && (pData->fbInfos.depth != (uint32_t)params->screenParams.bitsPerPixel)) {
        (void)pData->fbDevObj->restore(pData->fbDevObj);
    }

fbDevExit:
    if (opened) {
        (void)pData->fbDevObj->close(pData->fbDevObj);
    }
    (void)Drawer_UnInit(&pData->drawerObj);

drawerInitExit:
    (void)pthread_mutex_unlock(&pData->gfxLock);

    return GRAPHICS_ERROR_DRAWER;
}

/*!
 *
 */
static enum graphics_error_e destroyDrawer_f(struct graphics_s *obj)
{
    ASSERT(obj && obj->pData);
    
    struct graphics_private_data_s *pData = (struct graphics_private_data_s*)(obj->pData);
    enum graphics_error_e ret             = GRAPHICS_ERROR_NONE;

    if (pthread_mutex_lock(&pData->gfxLock) != 0) {
        Loge("pthread_mutex_lock() failed");
        return GRAPHICS_ERROR_LOCK;
    }

    if (!pData->drawerObj) {
        Loge("Drawer not initialized yet");
        ret = GRAPHICS_ERROR_DRAWER;
        goto exit;
    }
    
    pData->params.userData = NULL;
    
    (void)pData->drawerObj->uninitScreen(pData->drawerObj);

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
    (void)pthread_mutex_unlock(&pData->gfxLock);

    return ret;
}

/*!
 *
 */
static enum graphics_error_e createElement_f(struct graphics_s *obj,
                                             struct gfx_element_s **newGfxElement)
{
    ASSERT(obj && obj->pData && newGfxElement);

    struct graphics_private_data_s *pData = (struct graphics_private_data_s*)(obj->pData);

    if (pthread_mutex_lock(&pData->gfxLock) != 0) {
        Loge("pthread_mutex_lock() failed");
        return GRAPHICS_ERROR_LOCK;
    }
    
    ASSERT((*newGfxElement = calloc(1, sizeof(struct gfx_element_s))));
    ASSERT(((*newGfxElement)->reserved = calloc(1, sizeof(struct gfx_element_reserved_s))));

    (void)pthread_mutex_unlock(&pData->gfxLock);
    
    return GRAPHICS_ERROR_NONE;
}

/*!
 *
 */
static enum graphics_error_e pushElement_f(struct graphics_s *obj,
                                           struct gfx_element_s *gfxElement)
{
    ASSERT(obj && obj->pData && gfxElement);

    struct graphics_private_data_s *pData = (struct graphics_private_data_s*)(obj->pData);
    enum graphics_error_e ret             = GRAPHICS_ERROR_NONE;

    if (pthread_mutex_lock(&pData->gfxLock) != 0) {
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
    (void)pthread_mutex_unlock(&pData->gfxLock);

    return ret;
}

/*!
 *
 */
static enum graphics_error_e removeElement_f(struct graphics_s *obj, char *gfxElementName)
{
    ASSERT(obj && obj->pData && gfxElementName);
    
    struct graphics_private_data_s *pData = (struct graphics_private_data_s*)(obj->pData);
    enum graphics_error_e ret             = GRAPHICS_ERROR_NONE;

    if (pthread_mutex_lock(&pData->gfxLock) != 0) {
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
    (void)pthread_mutex_unlock(&pData->gfxLock);

    return ret;
}

/*!
 *
 */
static enum graphics_error_e removeAll_f(struct graphics_s *obj)
{
    ASSERT(obj && obj->pData);
    
    struct graphics_private_data_s *pData = (struct graphics_private_data_s*)(obj->pData);
    enum graphics_error_e ret             = GRAPHICS_ERROR_NONE;

    if (pthread_mutex_lock(&pData->gfxLock) != 0) {
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
    (void)pthread_mutex_unlock(&pData->gfxLock);
    
    return ret;
}

/*!
 *
 */
static enum graphics_error_e setVisible_f(struct graphics_s *obj, char *gfxElementName,
                                                                  uint8_t isVisible)
{
    ASSERT(obj && obj->pData && gfxElementName);
    
    struct graphics_private_data_s *pData = (struct graphics_private_data_s*)(obj->pData);
    enum graphics_error_e ret             = GRAPHICS_ERROR_NONE;

    if (pthread_mutex_lock(&pData->gfxLock) != 0) {
        Loge("pthread_mutex_lock() failed");
        return GRAPHICS_ERROR_LOCK;
    }
    
    if (pData->gfxElementsList->lock(pData->gfxElementsList) != LIST_ERROR_NONE) {
        ret = GRAPHICS_ERROR_LOCK;
        goto lockExit;
    }
    
    struct gfx_element_s *gfxElement = NULL;
    
    if (pData->focusedElement
        && !strncmp(pData->focusedElement->name, gfxElementName, MAX_NAME_SIZE)) {
        gfxElement = pData->focusedElement;
    }
    else if (pData->videoElement
             && !strncmp(pData->videoElement->name, gfxElementName, MAX_NAME_SIZE)) {
        gfxElement = pData->videoElement;
    }
    else if (pData->lastDrawnElement
             && !strncmp(pData->lastDrawnElement->name, gfxElementName, MAX_NAME_SIZE)) {
        gfxElement = pData->lastDrawnElement;
    }
    else if ((ret = getElement_f(obj, gfxElementName, &gfxElement)) != GRAPHICS_ERROR_NONE) {
        goto elementExit;
    }
    
    if (gfxElement->isVisible == isVisible) {
        Logd("Same visibility => not changed");
        goto elementExit;
    }
    
    gfxElement->isVisible               = isVisible;
    gfxElement->reserved->surfaceUpdated = 0;
 
    if ((ret = updateGroup_f(obj, gfxElement->groupName, NULL)) != GRAPHICS_ERROR_NONE) {
        Loge("Failed to update group : \"%s\"", gfxElement->groupName);
        gfxElement->isVisible = !isVisible;
    }

    if (!gfxElement->isVisible && gfxElement->hasFocus) {
        Logw("Element \"%s\" is invisible - Please, call setFocus on a visible element",
                gfxElement->name);
    }

elementExit:    
    (void)pData->gfxElementsList->unlock(pData->gfxElementsList);

lockExit:
    (void)pthread_mutex_unlock(&pData->gfxLock);

    return ret;
}

/*!
 *
 */
static enum graphics_error_e setFocus_f(struct graphics_s *obj, char *gfxElementName)
{
    ASSERT(obj && obj->pData && gfxElementName);
    
    struct graphics_private_data_s *pData = (struct graphics_private_data_s*)(obj->pData);
    enum graphics_error_e ret             = GRAPHICS_ERROR_NONE;

    if (pthread_mutex_lock(&pData->gfxLock) != 0) {
        Loge("pthread_mutex_lock() failed");
        return GRAPHICS_ERROR_LOCK;
    }
    
    if (pData->gfxElementsList->lock(pData->gfxElementsList) != LIST_ERROR_NONE) {
        ret = GRAPHICS_ERROR_LOCK;
        goto lockExit;
    }
    
    struct gfx_element_s *gfxElement = NULL;
    
    if (pData->focusedElement
        && !strncmp(pData->focusedElement->name, gfxElementName, MAX_NAME_SIZE)) {
        Logd("\"%s\" already has focus", gfxElementName);
        goto elementExit;
    }
    else if (pData->videoElement
             && !strncmp(pData->videoElement->name, gfxElementName, MAX_NAME_SIZE)) {
        gfxElement = pData->videoElement;
    }
    else if (pData->lastDrawnElement
             && !strncmp(pData->lastDrawnElement->name, gfxElementName, MAX_NAME_SIZE)) {
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
    (void)pthread_mutex_unlock(&pData->gfxLock);
    
    return ret;
}

/*!
 *
 */
static enum graphics_error_e setClickable_f(struct graphics_s *obj, char *gfxElementName,
                                            uint8_t isClickable)
{
    ASSERT(obj && obj->pData && gfxElementName);
    
    struct graphics_private_data_s *pData = (struct graphics_private_data_s*)(obj->pData);
    enum graphics_error_e ret             = GRAPHICS_ERROR_NONE;

    if (pthread_mutex_lock(&pData->gfxLock) != 0) {
        Loge("pthread_mutex_lock() failed");
        return GRAPHICS_ERROR_LOCK;
    }
    
    if (pData->gfxElementsList->lock(pData->gfxElementsList) != LIST_ERROR_NONE) {
        ret = GRAPHICS_ERROR_LOCK;
        goto lockExit;
    }
    
    struct gfx_element_s *gfxElement = NULL;
    
    if (pData->focusedElement
        && !strncmp(pData->focusedElement->name, gfxElementName, MAX_NAME_SIZE)) {
        gfxElement = pData->focusedElement;
    }
    else if (pData->videoElement
             && !strncmp(pData->videoElement->name, gfxElementName, MAX_NAME_SIZE)) {
        gfxElement = pData->videoElement;
    }
    else if (pData->lastDrawnElement
             && !strncmp(pData->lastDrawnElement->name, gfxElementName, MAX_NAME_SIZE)) {
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
    (void)pthread_mutex_unlock(&pData->gfxLock);
    
    return ret;
}

/*!
 *
 */
static enum graphics_error_e setNav_f(struct graphics_s *obj, char *gfxElementName,
                                      struct gfx_nav_s *nav)
{
    ASSERT(obj && obj->pData && gfxElementName && nav);

    struct graphics_private_data_s *pData = (struct graphics_private_data_s*)(obj->pData);
    enum graphics_error_e ret             = GRAPHICS_ERROR_NONE;

    if (pthread_mutex_lock(&pData->gfxLock) != 0) {
        Loge("pthread_mutex_lock() failed");
        return GRAPHICS_ERROR_LOCK;
    }

    if (pData->gfxElementsList->lock(pData->gfxElementsList) != LIST_ERROR_NONE) {
        ret = GRAPHICS_ERROR_LOCK;
        goto lockExit;
    }

    struct gfx_element_s *gfxElement = NULL;

    if (pData->focusedElement
        && !strncmp(pData->focusedElement->name, gfxElementName, MAX_NAME_SIZE)) {
        gfxElement = pData->focusedElement;
    }
    else if (pData->videoElement
             && !strncmp(pData->videoElement->name, gfxElementName, MAX_NAME_SIZE)) {
        gfxElement = pData->videoElement;
    }
    else if (pData->lastDrawnElement
             && !strncmp(pData->lastDrawnElement->name, gfxElementName, MAX_NAME_SIZE)) {
        gfxElement = pData->lastDrawnElement;
    }
    else if ((ret = getElement_f(obj, gfxElementName, &gfxElement)) != GRAPHICS_ERROR_NONE) {
        goto elementExit;
    }

    memcpy(&gfxElement->nav, nav, sizeof(struct gfx_nav_s));

elementExit:
    (void)pData->gfxElementsList->unlock(pData->gfxElementsList);

lockExit:
    (void)pthread_mutex_unlock(&pData->gfxLock);

    return ret;
}

/*!
 *
 */
static enum graphics_error_e setData_f(struct graphics_s *obj, char *gfxElementName, void *data)
{
    ASSERT(obj && obj->pData && gfxElementName && data);
    
    struct graphics_private_data_s *pData = (struct graphics_private_data_s*)(obj->pData);
    enum graphics_error_e ret             = GRAPHICS_ERROR_NONE;

    if (pthread_mutex_lock(&pData->gfxLock) != 0) {
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

    struct gfx_element_s *gfxElement = NULL;
    
    if (pData->focusedElement
        && !strncmp(pData->focusedElement->name, gfxElementName, MAX_NAME_SIZE)) {
        gfxElement = pData->focusedElement;
    }
    else if (pData->videoElement
             && !strncmp(pData->videoElement->name, gfxElementName, MAX_NAME_SIZE)) {
        gfxElement = pData->videoElement;
    }
    else if (pData->lastDrawnElement
             && !strncmp(pData->lastDrawnElement->name, gfxElementName, MAX_NAME_SIZE)) {
        gfxElement = pData->lastDrawnElement;
    }
    else if ((ret = getElement_f(obj, gfxElementName, &gfxElement)) != GRAPHICS_ERROR_NONE) {
        goto elementExit;
    }

    enum drawer_error_e drawerErr = DRAWER_ERROR_NONE;

    switch (gfxElement->type) {
        case GFX_ELEMENT_TYPE_VIDEO:
            gfxElement->data.buffer.data   = ((struct buffer_s*)data)->data;
            gfxElement->data.buffer.length = ((struct buffer_s*)data)->length;
            break;
                
        case GFX_ELEMENT_TYPE_IMAGE:
            strncpy(gfxElement->data.image.path, ((struct gfx_image_s*)data)->path,
                                                 strlen(((struct gfx_image_s*)data)->path));
            gfxElement->data.image.format = ((struct gfx_image_s*)data)->format;
            if (!((struct gfx_image_s*)data)->hiddenColor) {
                if (gfxElement->data.image.hiddenColor) {
                    free(gfxElement->data.image.hiddenColor);
                    gfxElement->data.image.hiddenColor = NULL;
                }
            }
            else {
                if (!gfxElement->data.image.hiddenColor) {
                    ASSERT((gfxElement->data.image.hiddenColor = calloc(1, sizeof(struct gfx_color_s))));
                }
                memcpy(gfxElement->data.image.hiddenColor, ((struct gfx_image_s*)data)->hiddenColor,
                                                           sizeof(struct gfx_color_s));
            }
            break;
                
        case GFX_ELEMENT_TYPE_TEXT:
            // restoreBgColor() preferred to setBgColor() because it first tries to redraw area
            // with its content (color, image) before any element is drawn. If such operation fails,
            // it then use provided fallback color
            if (pData->drawerObj->restoreBgColor) {
                drawerErr = pData->drawerObj->restoreBgColor(pData->drawerObj, &gfxElement->rect,
                                                             &pData->params.colorOnReset,
                                                             gfxElement->reserved->target);
            }
            else {
                drawerErr = pData->drawerObj->setBgColor(pData->drawerObj, &gfxElement->rect,
                                                         &pData->params.colorOnReset,
                                                         gfxElement->reserved->target);
            }

            if (drawerErr != DRAWER_ERROR_NONE) {
                Loge("Failed to restore background of element \"%s\"", gfxElement->name);
                ret = GRAPHICS_ERROR_DRAWER;
                goto elementExit;
            }

            memcpy(&gfxElement->data.text, (struct gfx_text_s*)data, sizeof(struct gfx_text_s));
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
    (void)pthread_mutex_unlock(&pData->gfxLock);
    
    return ret;
}

/*!
 *
 */
static enum graphics_error_e saveVideoFrame_f(struct graphics_s *obj, struct buffer_s *buffer,
                                              struct gfx_image_s *inOut)
{
    ASSERT(obj && obj->pData && inOut);
    
    struct graphics_private_data_s *pData = (struct graphics_private_data_s*)(obj->pData);
    enum graphics_error_e ret             = GRAPHICS_ERROR_DRAWER;

    if (pthread_mutex_lock(&pData->gfxLock) != 0) {
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

    struct buffer_s videoBuffer = {0};

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
    
    if (pData->drawerObj->saveBuffer(pData->drawerObj,
                                     &videoBuffer, inOut) != DRAWER_ERROR_NONE) {
        goto elementExit;
    }

    ret = GRAPHICS_ERROR_NONE;

elementExit:
    (void)pData->gfxElementsList->unlock(pData->gfxElementsList);

lockExit:
    (void)pthread_mutex_unlock(&pData->gfxLock);

    return ret;
}

/*!
 *
 */
static enum graphics_error_e saveVideoElement_f(struct graphics_s *obj, char *gfxElementName,
                                                struct gfx_image_s *inOut)
{
    ASSERT(obj && obj->pData && gfxElementName && inOut);

    struct graphics_private_data_s *pData = (struct graphics_private_data_s*)(obj->pData);
    enum graphics_error_e ret             = GRAPHICS_ERROR_NONE;

    if (pthread_mutex_lock(&pData->gfxLock) != 0) {
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

    struct gfx_element_s *gfxElement = NULL;

    if (pData->videoElement
        && (strncmp(pData->videoElement->name, gfxElementName, MAX_NAME_SIZE) == 0)) {
        gfxElement = pData->videoElement;
    }
    else {
        if (getElement_f(obj, gfxElementName, &gfxElement) != GRAPHICS_ERROR_NONE) {
            Loge("Element \"%s\" not found", gfxElementName);
            goto elementExit;
        }

        if (gfxElement->type != GFX_ELEMENT_TYPE_VIDEO) {
            Loge("\"%s\" is not a video element", gfxElementName);
            goto elementExit;
        }
    }

    if (pData->drawerObj->saveTarget(pData->drawerObj, inOut, &gfxElement->rect,
                                     GFX_TARGET_VIDEO) != DRAWER_ERROR_NONE) {
        ret = GRAPHICS_ERROR_DRAWER;
    }

elementExit:
    (void)pData->gfxElementsList->unlock(pData->gfxElementsList);

lockExit:
    (void)pthread_mutex_unlock(&pData->gfxLock);

    return ret;
}

/*!
 *
 */
static enum graphics_error_e takeScreenshot_f(struct graphics_s *obj, struct gfx_image_s *inOut)
{
    ASSERT(obj && obj->pData && inOut);

    struct graphics_private_data_s *pData = (struct graphics_private_data_s*)(obj->pData);
    enum graphics_error_e ret             = GRAPHICS_ERROR_NONE;

    if (pthread_mutex_lock(&pData->gfxLock) != 0) {
        Loge("pthread_mutex_lock() failed");
        return GRAPHICS_ERROR_LOCK;
    }

    if (!pData->drawerObj) {
        Loge("Drawer not initialized yet");
        ret = GRAPHICS_ERROR_DRAWER;
        goto lockExit;
    }

    if (pData->drawerObj->saveTarget(pData->drawerObj, inOut, NULL,
                                     GFX_TARGET_SCREEN) != DRAWER_ERROR_NONE) {
        ret = GRAPHICS_ERROR_DRAWER;
    }

lockExit:
    (void)pthread_mutex_unlock(&pData->gfxLock);

    return ret;
}

/*!
 *
 */
static enum graphics_error_e drawAllElements_f(struct graphics_s *obj)
{
    ASSERT(obj && obj->pData);
    
    struct graphics_private_data_s *pData = (struct graphics_private_data_s*)(obj->pData);
    enum graphics_error_e ret             = GRAPHICS_ERROR_NONE;

    if (pthread_mutex_lock(&pData->gfxLock) != 0) {
        Loge("pthread_mutex_lock() failed");
        return GRAPHICS_ERROR_LOCK;
    }

    if (!pData->drawerObj) {
        Loge("Drawer not initialized yet");
        ret = GRAPHICS_ERROR_DRAWER;
        goto lockExit;
    }

    if (pData->gfxElementsList->lock(pData->gfxElementsList) != LIST_ERROR_NONE) {
        ret = GRAPHICS_ERROR_LOCK;
        goto lockExit;
    }

    if (pData->drawerObj->startDrawingInBg) {
        (void)pData->drawerObj->startDrawingInBg(pData->drawerObj);
    }

    if (pData->gfxElementsList->browseElements(pData->gfxElementsList, obj) != LIST_ERROR_NONE) {
        Loge("Failed to browse elements' list");
        ret = GRAPHICS_ERROR_LIST;
    }

    if (pData->drawerObj->stopDrawingInBg) {
        (void)pData->drawerObj->stopDrawingInBg(pData->drawerObj);
    }

    (void)pData->gfxElementsList->unlock(pData->gfxElementsList);

lockExit:
    (void)pthread_mutex_unlock(&pData->gfxLock);

    return ret;
}

/*!
 *
 */
static enum graphics_error_e simulateGfxEvent_f(struct graphics_s *obj,
                                                struct gfx_event_s *gfxEvent)
{
    ASSERT(obj && obj->pData && gfxEvent);

    struct graphics_private_data_s *pData   = (struct graphics_private_data_s*)(obj->pData);
    struct graphics_task_s *simulateEvtTask = &pData->simulateEvtTask;
    struct list_s *evtsList                 = simulateEvtTask->list;
    struct graphics_list_element_s *element = NULL;

    if (pData->quit) {
        Logw("Too late! Graphics module is stopped or being stopped");
        return GRAPHICS_ERROR_NONE;
    }

    if (evtsList->lock(evtsList) != LIST_ERROR_NONE) {
        Loge("Failed to lock events list");
        return GRAPHICS_ERROR_LOCK;
    }

    Logd("Adding gfx event - type : \"%u\"", gfxEvent->type);

    ASSERT((element = calloc(1, sizeof(struct graphics_list_element_s))));
    element->seconds = time(NULL);
    memcpy(&element->event, gfxEvent, sizeof(struct gfx_event_s));

    (void)evtsList->add(evtsList, (void*)element);

    (void)evtsList->unlock(evtsList);

    sem_post(&simulateEvtTask->sem);

    return GRAPHICS_ERROR_NONE;
}

/*!
 *
 */
static enum graphics_error_e handleGfxEvents_f(struct graphics_s *obj)
{
    ASSERT(obj && obj->pData);

    struct graphics_private_data_s *pData = (struct graphics_private_data_s*)(obj->pData);

    if (!pData->drawerObj) {
        Loge("Drawer not initialized yet");
        return GRAPHICS_ERROR_DRAWER;
    }

    struct gfx_event_s evt;

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
static enum graphics_error_e quit_f(struct graphics_s *obj)
{
    ASSERT(obj && obj->pData);

    struct graphics_private_data_s *pData = (struct graphics_private_data_s*)(obj->pData);

    if (!pData->drawerObj) {
        Loge("Drawer not initialized yet");
        return GRAPHICS_ERROR_DRAWER;
    }

    if (pthread_mutex_lock(&pData->gfxLock) != 0) {
        Loge("pthread_mutex_lock() failed");
        return GRAPHICS_ERROR_LOCK;
    }
    
    pData->quit = 1;
    (void)pData->drawerObj->stopAwaitingEvent(pData->drawerObj);

    (void)pthread_mutex_unlock(&pData->gfxLock);

    return GRAPHICS_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/* ///////////////////////////// PRIVATE FUNCTIONS IMPLEMENTATION ///////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static enum graphics_error_e updateGroup_f(struct graphics_s *obj, char *groupName,
                                           char *gfxElementToIgnore)
{
    ASSERT(obj && obj->pData && groupName);
    
    if (groupName[0] == '\0') {
        Loge("Invalid group name");
        return GRAPHICS_ERROR_DRAWER;
    }
    
    struct graphics_private_data_s *pData = (struct graphics_private_data_s*)(obj->pData);
    enum graphics_error_e ret             = GRAPHICS_ERROR_NONE;
    struct gfx_element_s *gfxElement      = NULL;
    uint8_t drawInBg                      = 0;

    uint32_t nbElements;
    if (pData->gfxElementsList->getNbElements(pData->gfxElementsList,
                                              &nbElements) != LIST_ERROR_NONE) {
        Loge("Failed to get number of elements");
        ret = GRAPHICS_ERROR_LIST;
        goto exit;
    }

    // Background drawing is only available for screen
    // Video is always directly rendered on its dedicated window/surface
    drawInBg = (strcmp(pData->params.screenParams.video.name, groupName) != 0);
    if (drawInBg && pData->drawerObj->startDrawingInBg) {
        (void)pData->drawerObj->startDrawingInBg(pData->drawerObj);
    }

    while (nbElements > 0) {
        if (pData->gfxElementsList->getElement(pData->gfxElementsList,
                                               (void*)&gfxElement) != LIST_ERROR_NONE) {
            Loge("Failed to retrieve element");
            ret = GRAPHICS_ERROR_LIST;
            goto exit;
        }

        if (!strncmp(gfxElement->groupName, groupName, MAX_NAME_SIZE)
            && (!gfxElementToIgnore
                || strncmp(gfxElement->name, gfxElementToIgnore, MAX_NAME_SIZE))) {
            if (updateElement_f(obj, gfxElement) != GRAPHICS_ERROR_NONE) {
                //Loge("Failed to update element : \"%s\"", gfxElement->name);
                // Do not stop updating other elements of the group;
                // Needed to display gfx elements drawn in videoZone while video streaming
                // is disabled
            }
        }

        nbElements--;
    }
    
exit:
    if (drawInBg && pData->drawerObj->stopDrawingInBg) {
        (void)pData->drawerObj->stopDrawingInBg(pData->drawerObj);
    }

    return ret;
}

/*!
 *
 */
static enum graphics_error_e updateElement_f(struct graphics_s *obj,
                                             struct gfx_element_s *gfxElement)
{
    ASSERT(obj && obj->pData && gfxElement);
    
    struct graphics_private_data_s *pData = (struct graphics_private_data_s*)(obj->pData);
    enum graphics_error_e ret             = GRAPHICS_ERROR_NONE;

    if (!pData->drawerObj) {
        Loge("Drawer not initialized yet");
        return GRAPHICS_ERROR_DRAWER;
    }

    if (!gfxElement->isVisible) {
        // Reset surface
        // restoreBgColor() preferred to setBgColor() because it first tries to redraw area
        // with its content (color, image) before any element is drawn. If such operation fails,
        // it then use provided fallback color
        if (!gfxElement->reserved->surfaceUpdated) {
            enum drawer_error_e drawerErr = DRAWER_ERROR_NONE;
            if (pData->drawerObj->restoreBgColor) {
                drawerErr = pData->drawerObj->restoreBgColor(pData->drawerObj, &gfxElement->rect,
                                                             &pData->params.colorOnReset,
                                                             gfxElement->reserved->target);
            }
            else {
                drawerErr = pData->drawerObj->setBgColor(pData->drawerObj, &gfxElement->rect,
                                                         &pData->params.colorOnReset,
                                                         gfxElement->reserved->target);
            }

            if (drawerErr != DRAWER_ERROR_NONE) {
                Loge("Failed to restore background of element \"%s\"", gfxElement->name);
                ret = GRAPHICS_ERROR_DRAWER;
                goto exit;
            }
        }

        gfxElement->reserved->surfaceUpdated = 1;

        // Change focused element if required
        if (pData->focusedElement
            && !strncmp(pData->focusedElement->name, gfxElement->name, MAX_NAME_SIZE)) {
            Logw("Current focused element is not visible anymore => Please, set a new one");
            pData->focusedElement->hasFocus = 0;
            pData->focusedElement           = NULL;
        }
    }
    else {
        if (gfxElement->hasFocus) {
            if (pData->focusedElement
                && strncmp(pData->focusedElement->name, gfxElement->name, MAX_NAME_SIZE)) {
                Logd("\"%s\" was not the current focused element (Current : %s)",
                        gfxElement->name, pData->focusedElement->name);
                pData->focusedElement->hasFocus = 0;
                if (pData->drawerObj->setBgColor(pData->drawerObj,
                                                 &pData->focusedElement->rect,
                                                 &pData->params.colorOnBlur,
                                                 gfxElement->reserved->target) != DRAWER_ERROR_NONE) {
                    Loge("Failed to set colorOnBlur on \"%s\"", pData->focusedElement->name);
                    ret = GRAPHICS_ERROR_DRAWER;
                    goto exit;
                }
                
                if ((ret = drawElement_f(obj, pData->focusedElement)) != GRAPHICS_ERROR_NONE) {
                    Loge("Failed to update element : \"%s\"", pData->focusedElement->name);
                    goto exit;
                }
            }
            
            if (pData->drawerObj->setBgColor(pData->drawerObj, &gfxElement->rect,
                                             &pData->params.colorOnFocus,
                                             gfxElement->reserved->target) != DRAWER_ERROR_NONE) {
                Loge("Failed to set colorOnFocus on \"%s\"", gfxElement->name);
                ret = GRAPHICS_ERROR_DRAWER;
                goto exit;
            }
                    
            pData->focusedElement = gfxElement;
            Logd("Focus now given to : \"%s\"", pData->focusedElement->name);
            
            if (pData->params.onGfxEventCb) {
                struct gfx_event_s evt;
                evt.type            = GFX_EVENT_TYPE_FOCUS;
                evt.gfxElementName  = strdup(pData->focusedElement->name);
                evt.gfxElementPData = pData->focusedElement->pData;
                pData->params.onGfxEventCb(&evt, pData->params.userData);
                free(evt.gfxElementName);
            }
        }
        else if (gfxElement->isFocusable
                 && pData->drawerObj->setBgColor(pData->drawerObj, &gfxElement->rect,
                                                 &pData->params.colorOnBlur,
                                                 gfxElement->reserved->target) != DRAWER_ERROR_NONE) {
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
static enum graphics_error_e drawElement_f(struct graphics_s *obj,
                                           struct gfx_element_s *gfxElement)
{
    ASSERT(obj && obj->pData && gfxElement);
    
    struct graphics_private_data_s *pData = (struct graphics_private_data_s*)(obj->pData);
    enum graphics_error_e ret             = GRAPHICS_ERROR_NONE;

    if (!pData->drawerObj) {
        Loge("Drawer not initialized yet");
        return GRAPHICS_ERROR_DRAWER;
    }

    switch (gfxElement->type) {
        case GFX_ELEMENT_TYPE_VIDEO:
            if (pData->drawerObj->drawVideo(pData->drawerObj, &gfxElement->rect,
                                            &gfxElement->data.buffer) != DRAWER_ERROR_NONE) {
                ret = GRAPHICS_ERROR_DRAWER;
                goto exit;
            }
            pData->videoElement = gfxElement;
            break;

        case GFX_ELEMENT_TYPE_IMAGE:
            if (pData->drawerObj->drawImage(pData->drawerObj, &gfxElement->rect,
                                            &gfxElement->data.image,
                                            gfxElement->reserved->target) != DRAWER_ERROR_NONE) {
                ret = GRAPHICS_ERROR_DRAWER;
                goto exit;
            }
            break;

        case GFX_ELEMENT_TYPE_TEXT:
            if (pData->drawerObj->drawText(pData->drawerObj, &gfxElement->rect,
                                           &gfxElement->data.text,
                                           gfxElement->reserved->target) != DRAWER_ERROR_NONE) {
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
static enum graphics_error_e getElement_f(struct graphics_s *obj, char *gfxElementName,
                                          struct gfx_element_s **gfxElement)
{
    ASSERT(obj && obj->pData && gfxElementName);
    
    struct graphics_private_data_s *pData = (struct graphics_private_data_s*)(obj->pData);
    enum graphics_error_e ret             = GRAPHICS_ERROR_NONE;

    uint32_t nbElements;
    if (pData->gfxElementsList->getNbElements(pData->gfxElementsList,
                                              &nbElements) != LIST_ERROR_NONE) {
        Loge("Failed to get number of elements");
        ret = GRAPHICS_ERROR_LIST;
        goto exit;
    }
    
    while (nbElements > 0) {
        if (pData->gfxElementsList->getElement(pData->gfxElementsList,
                                               (void*)gfxElement) != LIST_ERROR_NONE) {
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
static enum graphics_error_e getClickedElement_f(struct graphics_s *obj,
                                                 struct gfx_event_s *gfxEvent)
{
    ASSERT(obj && obj->pData && gfxEvent);
    
    struct graphics_private_data_s *pData = (struct graphics_private_data_s*)(obj->pData);
    
    if (pData->gfxElementsList->lock(pData->gfxElementsList) != LIST_ERROR_NONE) {
        return GRAPHICS_ERROR_LOCK;
    }
    
    enum graphics_error_e ret        = GRAPHICS_ERROR_NONE;
    struct gfx_element_s *gfxElement = NULL;
    
    uint32_t nbElements;
    if (pData->gfxElementsList->getNbElements(pData->gfxElementsList,
                                              &nbElements) != LIST_ERROR_NONE) {
        Loge("Failed to get number of elements");
        ret = GRAPHICS_ERROR_LIST;
        goto exit;
    }
    
    while (nbElements > 0) {
        if (pData->gfxElementsList->getElement(pData->gfxElementsList,
                                               (void*)&gfxElement) != LIST_ERROR_NONE) {
            Loge("Failed to retrieve element");
            continue;
        }
        
        if (!gfxElement->isVisible) {
            continue;
        }
        
        if (gfxElement->isClickable
            && (gfxEvent->rect.x >= gfxElement->rect.x)
            && (gfxEvent->rect.x <= gfxElement->rect.x + (int32_t)gfxElement->rect.w)
            && (gfxEvent->rect.y >= gfxElement->rect.y)
            && (gfxEvent->rect.y <= gfxElement->rect.y + (int32_t)gfxElement->rect.h)) {
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
static enum graphics_error_e handleGfxEvent_f(struct graphics_s *obj,
                                              struct gfx_event_s *gfxEvent)
{
    ASSERT(obj && obj->pData && gfxEvent);

    if (gfxEvent->gfxElementName || gfxEvent->gfxElementPData) {
        Loge("Bad params - Element's name and pData should be null");
        return GRAPHICS_ERROR_PARAMS;
    }

    struct graphics_private_data_s *pData = (struct graphics_private_data_s*)(obj->pData);

    if (pthread_mutex_lock(&pData->evtLock) != 0) {
        Loge("pthread_mutex_lock() failed");
        return GRAPHICS_ERROR_LOCK;
    }

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
            if (pData->params.onGfxEventCb
                && (getClickedElement_f(obj, gfxEvent) == GRAPHICS_ERROR_NONE)) {
                setFocus_f(obj, gfxEvent->gfxElementName);
                pData->params.onGfxEventCb(gfxEvent, pData->params.userData);
            }
            break;

        case GFX_EVENT_TYPE_MOVE_LEFT:
            if (pData->gfxElementsList->lock(pData->gfxElementsList) == LIST_ERROR_NONE) {
                if (pData->focusedElement && ((pData->focusedElement->nav.left)[0] != '\0')) {
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
                if (pData->focusedElement && ((pData->focusedElement->nav.up)[0] != '\0')) {
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
                if (pData->focusedElement && ((pData->focusedElement->nav.right)[0] != '\0')) {
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
                if (pData->focusedElement && ((pData->focusedElement->nav.down)[0] != '\0')) {
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

    (void)pthread_mutex_unlock(&pData->evtLock);

    return GRAPHICS_ERROR_NONE;
}

static void taskFct_f(struct task_params_s *params)
{
    ASSERT(params && params->fctData);

    struct graphics_s *obj                  = (struct graphics_s*)(params->fctData);
    struct graphics_private_data_s *pData   = (struct graphics_private_data_s*)(obj->pData);
    struct graphics_task_s *simulateEvtTask = &pData->simulateEvtTask;
    struct list_s *evtsList                 = simulateEvtTask->list;
    struct graphics_list_element_s *element = NULL;

    if (simulateEvtTask->quit) {
        return;
    }

    sem_wait(&simulateEvtTask->sem);

    if (simulateEvtTask->quit) {
        return;
    }

    if (evtsList->lock(evtsList) != LIST_ERROR_NONE) {
        Loge("Failed to lock gfx events list");
        goto lockExit;
    }

    if (evtsList->getElement(evtsList, (void**)&element) != LIST_ERROR_NONE) {
        Loge("Failed to retrieve element from gfx events list");
        goto getElementExit;
    }

    (void)evtsList->unlock(evtsList);

    (void)handleGfxEvent_f(obj, &element->event);

    (void)evtsList->lock(evtsList);
    (void)evtsList->remove(evtsList, (void*)&element->seconds);
    (void)evtsList->unlock(evtsList);

    return;

getElementExit:
    (void)evtsList->unlock(evtsList);

lockExit:
    sem_post(&simulateEvtTask->sem); // Force retry
}

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////////// CALLBACKS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static uint8_t compareElementsCb(struct list_s *obj, void *elementToCheck, void *userData)
{
    ASSERT(obj && elementToCheck && userData);
    
    struct gfx_element_s *gfxElement = (struct gfx_element_s*)elementToCheck;
    char *nameOfElementToRemove      = (char*)userData;
    
    return (strncmp(nameOfElementToRemove, gfxElement->name, MAX_NAME_SIZE) == 0);
}

/*!
 *
 */
static void releaseElementCb(struct list_s *obj, void *element)
{
    ASSERT(obj && element);
    
    struct gfx_element_s *gfxElement = (struct gfx_element_s*)element;
    free(gfxElement->reserved);
    free(gfxElement);
}

/*!
 *
 */
static void browseElementsCb(struct list_s *obj, void *element, void *userData)
{
    ASSERT(obj && element);
    
    struct gfx_element_s *gfxElement      = (struct gfx_element_s*)element;
    struct graphics_s *gfxObj             = (struct graphics_s*)userData;
    struct graphics_private_data_s *pData = (struct graphics_private_data_s*)(gfxObj->pData);

    if ((gfxElement->type == GFX_ELEMENT_TYPE_VIDEO)
        || !strcmp(gfxElement->groupName, pData->params.screenParams.video.name)) {
        gfxElement->reserved->target = GFX_TARGET_VIDEO;
        Logd("Element \"%s\" will be rendered to \"Video\"", gfxElement->name);
    }
    else {
        gfxElement->reserved->target = GFX_TARGET_SCREEN;
        Logd("Element \"%s\" will be rendered to \"screen\"", gfxElement->name);
    }

    if (gfxElement->isVisible // Screen initialized with visible elements
        && updateElement_f(gfxObj, gfxElement) != GRAPHICS_ERROR_NONE) {
        //Loge("Failed to update element : \"%s\"", gfxElement->name);
    }
}

static uint8_t compareEventsCb(struct list_s *obj, void *elementToCheck, void *userData)
{
    ASSERT(obj && elementToCheck && userData);

    struct graphics_list_element_s *element = (struct graphics_list_element_s*)elementToCheck;
    time_t secondsOfElementToRemove         = *((time_t*)userData);

    return (element->seconds == secondsOfElementToRemove);
}

static void releaseEventCb(struct list_s *obj, void *element)
{
    ASSERT(obj && element);

    struct graphics_list_element_s *elementToRemove = (struct graphics_list_element_s*)element;
    free(elementToRemove);
}
