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
* \file Video.c
* \brief TODO
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// HEADERS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#include <pthread.h>
#include <sys/types.h>
#include <time.h>

#include "utils/List.h"
#include "utils/Log.h"
#include "utils/Task.h"

#include "video/Video.h"

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// MACROS ////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "Video"

#define FRAMES_HANLDER_TASK_NAME "framesHandlerTask"
#define NOTIFICATION_TASK_NAME   "notificationTask"

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// TYPES /////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

struct video_context_s {
    volatile uint8_t      quit;

    struct list_s         *listenersList;
    struct task_s         *videoTask;

    pthread_mutex_t       framesHandlerLock;
    struct task_params_s  framesHandlerParams;

    sem_t                 notificationSem;
    pthread_mutex_t       notificationLock;
    struct task_params_s  notificationParams;

    pthread_mutex_t       bufferLock;

    struct video_params_s params;

    volatile uint64_t     nbFramesLost;

    struct v4l2_s         *v4l2;

    struct video_area_s   finalVideoArea;
};

struct video_private_data_s {
    struct list_s *videosList;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////// PUBLIC FUNCTIONS PROTOTYPES //////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

static enum video_error_e registerListener_f(struct video_s *obj, struct video_params_s *params,
                                             struct video_listener_s *listener);
static enum video_error_e unregisterListener_f(struct video_s *obj, struct video_params_s *params,
                                               struct video_listener_s *listener);

static enum video_error_e getFinalVideoArea_f(struct video_s *obj, struct video_params_s *params,
                                              struct video_area_s *videoArea);
static enum video_error_e getMaxBufferSize_f(struct video_s *obj, struct video_params_s *params,
                                             size_t *size);

static enum video_error_e startDeviceCapture_f(struct video_s *obj, struct video_params_s *params);
static enum video_error_e stopDeviceCapture_f(struct video_s *obj, struct video_params_s *params);

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////// PRIVATE FUNCTIONS PROTOTYPES /////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

static enum video_error_e lockBuffer_f(struct video_s *obj, struct video_context_s *ctx);
static enum video_error_e unlockBuffer_f(struct video_s *obj, struct video_context_s *ctx);

static enum video_error_e initVideoContext_f(struct video_context_s **ctx,
                                             struct video_params_s *params);
static enum video_error_e uninitVideoContext_f(struct video_context_s **ctx);
static enum video_error_e getVideoContext_f(struct video_s *obj, char *videoName,
                                            struct video_context_s **ctxOut);

static void framesHandlerFct_f(struct task_params_s *params);
static void notificationFct_f(struct task_params_s *params);
static void framesHandlerAtExit_f(struct task_params_s *params);

static uint8_t compareVideoCb(struct list_s *obj, void *elementToCheck, void *userData);
static void releaseVideoCb(struct list_s *obj, void *element);

static uint8_t compareListenerCb(struct list_s *obj, void *elementToCheck, void *userData);
static void releaseListenerCb(struct list_s *obj, void *element);

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// INITIALIZER //////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
enum video_error_e Video_Init(struct video_s **obj)
{
    assert(obj && (*obj = calloc(1, sizeof(struct video_s))));

    struct video_private_data_s *pData;
    assert((pData = calloc(1, sizeof(struct video_private_data_s))));

    struct list_params_s listParams = {0};
    listParams.compareCb = compareVideoCb;
    listParams.releaseCb = releaseVideoCb;
    listParams.browseCb  = NULL;

    if (List_Init(&pData->videosList, &listParams) != LIST_ERROR_NONE) {
        Loge("List_Init() failed");
        goto exit;
    }

    (*obj)->registerListener   = registerListener_f;
    (*obj)->unregisterListener = unregisterListener_f;
    (*obj)->getFinalVideoArea  = getFinalVideoArea_f;
    (*obj)->getMaxBufferSize   = getMaxBufferSize_f;
    (*obj)->startDeviceCapture = startDeviceCapture_f;
    (*obj)->stopDeviceCapture  = stopDeviceCapture_f;

    (*obj)->pData = pData;

    return VIDEO_ERROR_NONE;

exit:
    free(pData);
    free(*obj);
    *obj = NULL;

    return VIDEO_ERROR_INIT;
}

/*!
 *
 */
enum video_error_e Video_UnInit(struct video_s **obj)
{
    assert(obj && *obj && (*obj)->pData);

    struct video_private_data_s *pData = (*obj)->pData;

    (void)List_UnInit(&pData->videosList);

    free(pData);

    free(*obj);
    *obj = NULL;

    return VIDEO_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////// PUBLIC FUNCTIONS IMPLEMENTATION ///////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static enum video_error_e registerListener_f(struct video_s *obj, struct video_params_s *params,
                                             struct video_listener_s *listener)
{
    assert(obj && obj->pData && params);

    if (!listener || !listener->name || !listener->onVideoBufferAvailableCb) {
        Loge("Bad params");
        return VIDEO_ERROR_PARAMS;
    }

    struct video_context_s *ctx = NULL;
    enum video_error_e ret      = VIDEO_ERROR_NONE;

    if ((ret = getVideoContext_f(obj, params->name, &ctx)) != VIDEO_ERROR_NONE) {
        Loge("Failed to retrieve %s's context", params->name);
        goto exit;
    }

    struct list_s *list = ctx->listenersList;

    if (!list) {
        Loge("listeners' list not initialized yet");
        ret = VIDEO_ERROR_INIT;
        goto exit;
    }

    if (list->lock(list) != LIST_ERROR_NONE) {
        Loge("Failed to lock list");
        ret = VIDEO_ERROR_LOCK;
        goto exit;
    }

    struct video_listener_s *newListener;
    assert((newListener = calloc(1, sizeof(struct video_listener_s))));

    strncpy(newListener->name, listener->name, sizeof(newListener->name));
    newListener->onVideoBufferAvailableCb = listener->onVideoBufferAvailableCb;
    newListener->userData                 = listener->userData;

    list->add(list, (void*)newListener);

    (void)list->unlock(list);

    ret = VIDEO_ERROR_NONE;

exit:
    return ret;
}

/*!
 *
 */
static enum video_error_e unregisterListener_f(struct video_s *obj, struct video_params_s *params,
                                               struct video_listener_s *listener)
{
    assert(obj && obj->pData && params);

    if (!listener || !listener->name) {
        Loge("Bad params");
        return VIDEO_ERROR_PARAMS;
    }

    struct video_context_s *ctx = NULL;
    enum video_error_e ret      = VIDEO_ERROR_NONE;

    if ((ret = getVideoContext_f(obj, params->name, &ctx)) != VIDEO_ERROR_NONE) {
        Loge("Failed to retrieve %s's context", params->name);
        goto exit;
    }

    struct list_s *list = ctx->listenersList;

    if (!list) {
        Loge("listeners' list not initialized yet");
        ret = VIDEO_ERROR_INIT;
        goto exit;
    }

    if (list->lock(list) != LIST_ERROR_NONE) {
        Loge("Failed to lock list");
        ret = VIDEO_ERROR_LOCK;
        goto exit;
    }

    list->remove(list, (void*)listener->name);

    (void)list->unlock(list);

    ret = VIDEO_ERROR_NONE;

exit:
    return ret;
}

/*!
 *
 */
static enum video_error_e getFinalVideoArea_f(struct video_s *obj, struct video_params_s *params,
                                              struct video_area_s *videoArea)
{
    assert(obj && obj->pData && params && videoArea);

    struct video_context_s *ctx = NULL;
    enum video_error_e ret      = VIDEO_ERROR_NONE;

    if ((ret = getVideoContext_f(obj, params->name, &ctx)) != VIDEO_ERROR_NONE) {
        Loge("Failed to retrieve %s's context", params->name);
        goto exit;
    }

    memcpy(videoArea, &ctx->finalVideoArea, sizeof(struct video_area_s));

    ret = VIDEO_ERROR_NONE;

exit:
    return ret;
}

/*!
 *
 */
static enum video_error_e getMaxBufferSize_f(struct video_s *obj, struct video_params_s *params,
                                             size_t *size)
{
    assert(obj && obj->pData && params && size);

    struct video_context_s *ctx = NULL;
    enum video_error_e ret      = VIDEO_ERROR_NONE;

    if ((ret = getVideoContext_f(obj, params->name, &ctx)) != VIDEO_ERROR_NONE) {
        Loge("Failed to retrieve %s's context", params->name);
        goto exit;
    }

    if (!ctx->v4l2) {
        Loge("v4l2 object not initialized");
        ret = VIDEO_ERROR_START;
        goto exit;
    }

    *size = ctx->v4l2->maxBufferSize;

    ret = VIDEO_ERROR_NONE;

exit:
    return ret;
}

/*!
 *
 */
static enum video_error_e startDeviceCapture_f(struct video_s *obj, struct video_params_s *params)
{
    assert(obj && obj->pData && params);

    struct video_context_s *ctx = NULL;

    /* Check if video capture has already been started */
    if (getVideoContext_f(obj, params->name, &ctx) == VIDEO_ERROR_NONE) {
        Loge("%s's context exists", params->name);
        return VIDEO_ERROR_START;
    }

    /* Initialize video context */
    if (initVideoContext_f(&ctx, params) != VIDEO_ERROR_NONE) {
        Loge("Failed to init video context");
        goto exit;
    }

    /* Add video ctx to list */
    struct video_private_data_s *pData = (struct video_private_data_s*)(obj->pData);

    if (!pData->videosList || (pData->videosList->lock(pData->videosList) != LIST_ERROR_NONE)) {
        Loge("Failed to lock videosList");
        goto list_exit;
    }
    pData->videosList->add(pData->videosList, (void*)ctx);
    (void)pData->videosList->unlock(pData->videosList);

    /* Open device */
    struct v4l2_open_device_params_s openDeviceParams = {0};
    strncpy(openDeviceParams.path, params->path, sizeof(params->path));
    openDeviceParams.caps = params->caps;

    if (ctx->v4l2->openDevice(ctx->v4l2, &openDeviceParams) != V4L2_ERROR_NONE) {
        Loge("openDevice() failed");
        goto open_exit;
    }

    /* Configure device */
    struct v4l2_configure_device_params_s configureDeviceParams = {0};
    configureDeviceParams.type        = params->type;
    configureDeviceParams.pixelformat = params->pixelformat;
    configureDeviceParams.colorspace  = params->colorspace;
    configureDeviceParams.width       = params->captureArea.width;
    configureDeviceParams.height      = params->captureArea.height;
    configureDeviceParams.desiredFps  = params->desiredFps;

    if (ctx->v4l2->configureDevice(ctx->v4l2, &configureDeviceParams) != V4L2_ERROR_NONE) {
        Loge("configureDevice() failed");
        goto configure_exit;
    }

    ctx->finalVideoArea.width  = ctx->v4l2->format.fmt.pix.width;
    ctx->finalVideoArea.height = ctx->v4l2->format.fmt.pix.height;

    uint8_t selectionApiSupported              = 0;
    struct v4l2_selection_params_s cropRect    = {0};
    struct v4l2_selection_params_s composeRect = {0};

    memcpy(&cropRect, &params->croppingArea, sizeof(struct v4l2_selection_params_s));

    if (ctx->v4l2->setCroppingArea(ctx->v4l2, &cropRect) == V4L2_ERROR_NONE) {
        memcpy(&composeRect, &params->composingArea, sizeof(struct v4l2_selection_params_s));

        if (ctx->v4l2->setComposingArea(ctx->v4l2, &composeRect) != V4L2_ERROR_NONE) {
            Loge("Failed to set composing area");
        }
        else {
            ctx->finalVideoArea.width  = composeRect.width;
            ctx->finalVideoArea.height = composeRect.height;

            selectionApiSupported = 1;
        }
    }
    else {
        Loge("Failed to set cropping area");
    }

    if (!selectionApiSupported) {
        Logw("V4L2 selection API is not supported by your driver");

        configureDeviceParams.width  = params->composingArea.width;
        configureDeviceParams.height = params->composingArea.height;

        if (ctx->v4l2->configureDevice(ctx->v4l2, &configureDeviceParams) == V4L2_ERROR_NONE) {
            ctx->finalVideoArea.width  = ctx->v4l2->format.fmt.pix.width;
            ctx->finalVideoArea.height = ctx->v4l2->format.fmt.pix.height;
        }
    }

    /* Request buffers */
    struct v4l2_request_buffers_params_s requestBuffersParams = {0};
    requestBuffersParams.count  = params->count;
    requestBuffersParams.memory = params->memory;

    if (ctx->v4l2->requestBuffers(ctx->v4l2, &requestBuffersParams) != V4L2_ERROR_NONE) {
        Loge("requestBuffers() failed");
        goto reqBuf_exit;
    }

    /* Start capture */
    if (ctx->v4l2->startCapture(ctx->v4l2) != V4L2_ERROR_NONE) {
        Loge("startCapture() failed");
        goto start_exit;
    }

    /* Start tasks */
    strcpy(ctx->framesHandlerParams.name, FRAMES_HANLDER_TASK_NAME);
    ctx->framesHandlerParams.priority = params->priority;
    ctx->framesHandlerParams.fct      = framesHandlerFct_f;
    ctx->framesHandlerParams.fctData  = obj;
    ctx->framesHandlerParams.userData = ctx;
    ctx->framesHandlerParams.atExit   = framesHandlerAtExit_f;

    if (ctx->videoTask->create(ctx->videoTask, &ctx->framesHandlerParams) != TASK_ERROR_NONE) {
        Loge("Failed to create framesHandler task");
        goto framesCreate_exit;
    }

    strcpy(ctx->notificationParams.name, NOTIFICATION_TASK_NAME);
    ctx->notificationParams.priority = params->priority;
    ctx->notificationParams.fct      = notificationFct_f;
    ctx->notificationParams.fctData  = obj;
    ctx->notificationParams.userData = ctx;
    ctx->notificationParams.atExit   = NULL;

    if (ctx->videoTask->create(ctx->videoTask, &ctx->notificationParams) != TASK_ERROR_NONE) {
        Loge("Failed to create notification task");
        goto notificationCreate_exit;
    }

    (void)ctx->videoTask->start(ctx->videoTask, &ctx->framesHandlerParams);
    (void)ctx->videoTask->start(ctx->videoTask, &ctx->notificationParams);

    return VIDEO_ERROR_NONE;

notificationCreate_exit:
    (void)ctx->videoTask->destroy(ctx->videoTask, &ctx->framesHandlerParams);

framesCreate_exit:
    (void)ctx->v4l2->stopCapture(ctx->v4l2);
    
start_exit:
    (void)ctx->v4l2->releaseBuffers(ctx->v4l2);

reqBuf_exit:
configure_exit:
    (void)ctx->v4l2->closeDevice(ctx->v4l2);

open_exit:
    if (!pData->videosList || (pData->videosList->lock(pData->videosList) != LIST_ERROR_NONE)) {
        Loge("Failed to lock videosList");
    }
    else {
        pData->videosList->remove(pData->videosList, (void*)ctx->params.name);
        ctx = NULL;
        (void)pData->videosList->unlock(pData->videosList);
    }

list_exit:
    if (ctx) {
        (void)uninitVideoContext_f(&ctx);
    }

exit:
    if (ctx) {
        free(ctx);
    }

    return VIDEO_ERROR_START;
}

/*!
 *
 */
static enum video_error_e stopDeviceCapture_f(struct video_s *obj, struct video_params_s *params)
{
    assert(obj && obj->pData && params);

    struct video_context_s *ctx = NULL;
    enum video_error_e ret      = VIDEO_ERROR_NONE;

    /* Check if video context exists */
    if ((ret = getVideoContext_f(obj, params->name, &ctx)) != VIDEO_ERROR_NONE) {
        Loge("%s's context does not exist", params->name);
        goto exit;
    }

    /* Stop tasks */
    ctx->quit = 1;

    (void)ctx->v4l2->stopAwaitingData(ctx->v4l2);
    sem_post(&ctx->notificationSem);

    (void)ctx->videoTask->stop(ctx->videoTask, &ctx->framesHandlerParams);
    (void)ctx->videoTask->stop(ctx->videoTask, &ctx->notificationParams);

    (void)ctx->videoTask->destroy(ctx->videoTask, &ctx->framesHandlerParams);
    (void)ctx->videoTask->destroy(ctx->videoTask, &ctx->notificationParams);

    /* Stop capture */
    if (ctx->v4l2->stopCapture(ctx->v4l2) != V4L2_ERROR_NONE) {
        Loge("stopCapture() failed");
        ret = VIDEO_ERROR_STOP;
    }

    /* Release buffers */
    if (ctx->v4l2->releaseBuffers(ctx->v4l2) != V4L2_ERROR_NONE) {
        Loge("releaseBuffers() failed");
        ret = VIDEO_ERROR_STOP;
    }

    /* Close device */
    if (ctx->v4l2->closeDevice(ctx->v4l2) != V4L2_ERROR_NONE) {
        Loge("closeDevice() failed");
        ret = VIDEO_ERROR_STOP;
    }

    /* Remove video ctx */
    struct video_private_data_s *pData = (struct video_private_data_s*)(obj->pData);

    if (!pData->videosList || (pData->videosList->lock(pData->videosList) != LIST_ERROR_NONE)) {
        Loge("Failed to lock videosList");
        ret = VIDEO_ERROR_LOCK;
    }
    pData->videosList->remove(pData->videosList, (void*)ctx->params.name);
    (void)pData->videosList->unlock(pData->videosList);

exit:
    return ret;
}

/* -------------------------------------------------------------------------------------------- */
/* ///////////////////////////// PRIVATE FUNCTIONS IMPLEMENTATION ///////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static enum video_error_e lockBuffer_f(struct video_s *obj, struct video_context_s *ctx)
{
    assert(obj && obj->pData && ctx);

    if (pthread_mutex_lock(&ctx->bufferLock) != 0) {
        Loge("pthread_mutex_lock() failed");
        return VIDEO_ERROR_LOCK;
    }
    
    return VIDEO_ERROR_NONE;
}

/*!
 *
 */
static enum video_error_e unlockBuffer_f(struct video_s *obj, struct video_context_s *ctx)
{
    assert(obj && obj->pData && ctx);

    if (pthread_mutex_unlock(&ctx->bufferLock) != 0) {
        Loge("pthread_mutex_unlock() failed");
        return VIDEO_ERROR_LOCK;
    }

    return VIDEO_ERROR_NONE;
}

/*!
 *
 */
static enum video_error_e initVideoContext_f(struct video_context_s **ctx,
                                             struct video_params_s *params)
{
    assert(ctx && params);
    assert((*ctx = calloc(1, sizeof(struct video_context_s))));

    memcpy(&(*ctx)->params, params, sizeof(struct video_params_s));

    struct list_params_s listParams = {0};
    listParams.compareCb = compareListenerCb;
    listParams.releaseCb = releaseListenerCb;
    listParams.browseCb  = NULL;

    if (List_Init(&(*ctx)->listenersList, &listParams) != LIST_ERROR_NONE) {
        Loge("List_Init() failed");
        goto list_exit;
    }

    if (Task_Init(&(*ctx)->videoTask) != TASK_ERROR_NONE) {
        Loge("Task_Init() failed");
        goto task_exit;
    }

    if (pthread_mutex_init(&(*ctx)->framesHandlerLock, NULL) != 0) {
        Loge("pthread_mutex_init() failed");
        goto framesHandlerLock_exit;
    }

    if (sem_init(&(*ctx)->notificationSem, 0, 0) != 0) {
        Loge("sem_init() failed");
        goto notificationSem_exit;
    }

    if (pthread_mutex_init(&(*ctx)->notificationLock, NULL) != 0) {
        Loge("pthread_mutex_init() failed");
        goto notificationLock_exit;
    }

    if (pthread_mutex_init(&(*ctx)->bufferLock, NULL) != 0) {
        Loge("pthread_mutex_init() failed");
        goto bufferLock_exit;
    }

    if (V4l2_Init(&(*ctx)->v4l2) != V4L2_ERROR_NONE) {
        Loge("V4l2_Init() failed");
        goto v4l2_exit;
    }

    return VIDEO_ERROR_NONE;

v4l2_exit:
    (void)pthread_mutex_destroy(&(*ctx)->bufferLock);

bufferLock_exit:
    (void)pthread_mutex_destroy(&(*ctx)->notificationLock);

notificationLock_exit:
    (void)sem_destroy(&(*ctx)->notificationSem);

notificationSem_exit:
    (void)pthread_mutex_destroy(&(*ctx)->framesHandlerLock);

framesHandlerLock_exit:
    (void)Task_UnInit(&(*ctx)->videoTask);

task_exit:
    (void)List_UnInit(&(*ctx)->listenersList);

list_exit:
    if (*ctx) {
        free(*ctx);
        *ctx = NULL;
    }

    return VIDEO_ERROR_INIT;
}

/*!
 *
 */
static enum video_error_e uninitVideoContext_f(struct video_context_s **ctx)
{
    assert(ctx && *ctx);

    enum video_error_e ret = VIDEO_ERROR_NONE;

    if (V4l2_UnInit(&(*ctx)->v4l2) != V4L2_ERROR_NONE) {
        Loge("V4l2_UnInit() failed");
        ret = VIDEO_ERROR_UNINIT;
    }

    if (pthread_mutex_destroy(&(*ctx)->bufferLock) != 0) {
        Loge("pthread_mutex_destroy() failed");
        ret = VIDEO_ERROR_UNINIT;
    }

    if (pthread_mutex_destroy(&(*ctx)->notificationLock) != 0) {
        Loge("pthread_mutex_destroy() failed");
        ret = VIDEO_ERROR_UNINIT;
    }

    if (sem_destroy(&(*ctx)->notificationSem) != 0) {
        Loge("sem_destroy() failed");
        ret = VIDEO_ERROR_UNINIT;
    }

    if (pthread_mutex_destroy(&(*ctx)->framesHandlerLock) != 0) {
        Loge("pthread_mutex_destroy() failed");
        ret = VIDEO_ERROR_UNINIT;
    }

    if (Task_UnInit(&(*ctx)->videoTask) != TASK_ERROR_NONE) {
        Loge("Task_UnInit() failed");
        ret = VIDEO_ERROR_UNINIT;
    }

    if (List_UnInit(&(*ctx)->listenersList) != LIST_ERROR_NONE) {
        Loge("List_UnInit() failed");
        ret = VIDEO_ERROR_UNINIT;
    }

    free(*ctx);
    *ctx = NULL;

    return ret;
}

/*!
 *
 */
static enum video_error_e getVideoContext_f(struct video_s *obj, char *videoName,
                                            struct video_context_s **ctxOut)
{
    assert(obj && obj->pData && videoName && ctxOut);

    struct video_private_data_s *pData = (struct video_private_data_s*)(obj->pData);

    if (!pData->videosList) {
        Loge("There is currently no element in list");
        return VIDEO_ERROR_LIST;
    }

    enum video_error_e ret = VIDEO_ERROR_NONE;

    if (pData->videosList->lock(pData->videosList) != LIST_ERROR_NONE) {
        Loge("Failed to lock videosList");
        return VIDEO_ERROR_LIST;
    }

    uint32_t nbElements;
    if (pData->videosList->getNbElements(pData->videosList, &nbElements) != LIST_ERROR_NONE) {
        Loge("Failed to get number of elements");
        ret = VIDEO_ERROR_LIST;
        goto exit;
    }

    while (nbElements > 0) {
        if (pData->videosList->getElement(pData->videosList, (void**)ctxOut) != LIST_ERROR_NONE) {
            Loge("Failed to retrieve element");
            ret = VIDEO_ERROR_LIST;
            goto exit;
        }

        if (!strcmp((*ctxOut)->params.name, videoName)) {
            break;
        }

        nbElements--;
    }

    if (nbElements == 0) {
        //Loge("Element %s not found", videoName);
        ret = VIDEO_ERROR_LIST;
        goto exit;
    }

exit:
    (void)pData->videosList->unlock(pData->videosList);

    return ret;
}

/*!
 * Capture video frames and wake up notification task
 */
static void framesHandlerFct_f(struct task_params_s *params)
{
    assert(params && params->fctData && params->userData);
    
    struct video_s *video       = (struct video_s*)params->fctData;
    struct video_context_s *ctx = (struct video_context_s*)params->userData;
    
    uint32_t i;
    int32_t timeout_ms = -1;
    
    switch (ctx->params.awaitMode) {
        case VIDEO_AWAIT_MODE_BLOCKING:
            break;
            
        case VIDEO_AWAIT_MODE_NON_BLOCKING:
            timeout_ms = (int32_t)WAIT_TIME_2S;
            break;
            
        default:
            ;
    }
    
    for (i = 0; i < ctx->v4l2->nbBuffers; i++) {
        /* Queue buffer */
        ctx->v4l2->queueBuffer(ctx->v4l2, i);

        /* Await data */
        while (!ctx->quit) {
            if (ctx->v4l2->awaitData(ctx->v4l2, timeout_ms) == V4L2_ERROR_NONE) {
                break;
            }
        }

        if (ctx->quit) {
            break;
        }

        /* Dequeue buffer */
        ctx->v4l2->dequeueBuffer(ctx->v4l2);
            
        /* Fill in listener's buffer */
        (void)lockBuffer_f(video, ctx);
        
        if (!(video->buffer.data)) {
            assert((video->buffer.data = calloc(1, ctx->v4l2->maxBufferSize)));
        }
        
        video->buffer.index  = ctx->v4l2->map[i].index;
        video->buffer.length = ctx->v4l2->map[i].length;
        video->buffer.offset = ctx->v4l2->map[i].offset;
        
        memcpy(video->buffer.data, ctx->v4l2->map[i].start, ctx->v4l2->map[i].length);
        
        ctx->nbFramesLost++;
            
        (void)unlockBuffer_f(video, ctx);
        
        /* Notify listeners */
        sem_post(&ctx->notificationSem);
    }
}

/*!
 * Wait for new video frames then notify listeners
 */
static void notificationFct_f(struct task_params_s *params)
{
    assert(params && params->fctData && params->userData);
    
    struct video_s *video       = (struct video_s*)params->fctData;
    struct video_context_s *ctx = (struct video_context_s*)params->userData;
    
    if (ctx->quit) {
        return;
    }
    
    sem_wait(&ctx->notificationSem);
    
    if (ctx->quit) {
        return;
    }
    
    struct list_s *list = ctx->listenersList;

    (void)lockBuffer_f(video, ctx);
    
    if (list->lock(list) != LIST_ERROR_NONE) {
        Loge("Failed to lock list");
        goto exit;
    }
    
    uint32_t nbClients;
    if (list->getNbElements(list, &nbClients) == LIST_ERROR_NONE) {
        struct video_listener_s *listener = NULL;
        while (nbClients > 0) {
            nbClients--;
            
            if (list->getElement(list, (void*)&listener) != LIST_ERROR_NONE) {
                break;
            }
            
            /* IMPORTANT: Buffer must be handled very quickly so no heavy operation please!! */
            listener->onVideoBufferAvailableCb(&video->buffer, listener->userData);
        }
    }
    
    (void)list->unlock(list);
    
    ctx->nbFramesLost--;

exit:
    (void)unlockBuffer_f(video, ctx);
}

/*!
 * Called when stopping framesHandler task
 */
static void framesHandlerAtExit_f(struct task_params_s *params)
{
    assert(params && params->fctData && params->userData);
    
    struct video_s *video       = (struct video_s*)params->fctData;
    struct video_context_s *ctx = (struct video_context_s*)params->userData;
    
    /* Release buffer */
    (void)lockBuffer_f(video, ctx);
    
    if (video->buffer.data) {
        free(video->buffer.data);
        video->buffer.data = NULL;
    }
    
    (void)unlockBuffer_f(video, ctx);
}

/*!
 *
 */
static uint8_t compareVideoCb(struct list_s *obj, void *elementToCheck, void *userData)
{
    assert(obj && elementToCheck && userData);

    struct video_context_s *ctx = (struct video_context_s*)elementToCheck;
    char *nameOfElementToRemove = (char*)userData;

    return (!strcmp(nameOfElementToRemove, ctx->params.name));
}

/*!
 *
 */
static void releaseVideoCb(struct list_s *obj, void *element)
{
    assert(obj && element);

    struct video_context_s **ctx = (struct video_context_s**)&element;

    (void)uninitVideoContext_f(ctx);
}

/*!
 *
 */
static uint8_t compareListenerCb(struct list_s *obj, void *elementToCheck, void *userData)
{
    assert(obj && elementToCheck && userData);
    
    struct video_listener_s *listener  = (struct video_listener_s*)elementToCheck;
    char *nameOfElementToRemove = (char*)userData;
        
    return (strncmp(listener->name, nameOfElementToRemove, strlen(listener->name)) == 0);
}

/*!
 *
 */
static void releaseListenerCb(struct list_s *obj, void *element)
{
    assert(obj && element);
    
    struct video_listener_s *listener = (struct video_listener_s*)element;
    free(listener);
}
