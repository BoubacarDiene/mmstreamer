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
* \file   Video.c
* \brief  Configure camera devices and forward captured video frames to listeners
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>

#include "utils/List.h"
#include "utils/Log.h"
#include "utils/Task.h"

#include "video/Video.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                            */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "VIDEO"

#define FRAMES_HANLDER_TASK_NAME "framesHandlerTask"
#define NOTIFICATION_TASK_NAME   "notificationTask"

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

typedef struct VIDEO_PRIVATE_DATA_S {
    volatile uint8_t   quit;
    
    LIST_S             *listenersList;
    TASK_S             *videoTask;
    
    pthread_mutex_t    framesHandlerLock;
    TASK_PARAMS_S      framesHandlerParams;
    
    sem_t              notificationSem;
    pthread_mutex_t    notificationLock;
    TASK_PARAMS_S      notificationParams;
    
    pthread_mutex_t    bufferLock;
    
    VIDEO_PARAMS_S     videoParams;
    
    volatile uint64_t  nbFramesLost;
    
    V4L2_S             *v4l2;
    
    VIDEO_RESOLUTION_S finalResolution;
} VIDEO_PRIVATE_DATA_S;

/* -------------------------------------------------------------------------------------------- */
/*                                          VARIABLES                                           */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                         PROTOTYPES                                           */
/* -------------------------------------------------------------------------------------------- */

static VIDEO_ERROR_E registerListener_f  (VIDEO_S *obj, VIDEO_LISTENER_S *listener);
static VIDEO_ERROR_E unregisterListener_f(VIDEO_S *obj, VIDEO_LISTENER_S *listener);

static VIDEO_ERROR_E getFinalResolution_f(VIDEO_S *obj, VIDEO_RESOLUTION_S *resolution);
static VIDEO_ERROR_E getMaxBufferSize_f  (VIDEO_S *obj, size_t *size);

static VIDEO_ERROR_E startDeviceCapture_f(VIDEO_S *obj, VIDEO_PARAMS_S *params);
static VIDEO_ERROR_E stopDeviceCapture_f (VIDEO_S *obj);

static VIDEO_ERROR_E lockBuffer_f  (VIDEO_S *obj);
static VIDEO_ERROR_E unlockBuffer_f(VIDEO_S *obj);

static void framesHandlerFct_f   (TASK_PARAMS_S *params);
static void notificationFct_f    (TASK_PARAMS_S *params);
static void framesHandlerAtExit_f(TASK_PARAMS_S *params);

static uint8_t compareCb(LIST_S *obj, void *elementToCheck, void *userData);
static void    releaseCb(LIST_S *obj, void *element);

/* -------------------------------------------------------------------------------------------- */
/*                                      PUBLIC FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

/*!
 * \fn             VIDEO_ERROR_E Video_Init(VIDEO_S **obj)
 * \brief          Create an instance of Video module
 * \param[in, out] obj
 * \return         VIDEO_ERROR_NONE on success
 *                 VIDEO_ERROR_INIT on error
 */
VIDEO_ERROR_E Video_Init(VIDEO_S **obj)
{
    assert(obj && (*obj = calloc(1, sizeof(VIDEO_S))));

    VIDEO_PRIVATE_DATA_S *pData;
    assert((pData = calloc(1, sizeof(VIDEO_PRIVATE_DATA_S))));
    
    LIST_PARAMS_S listParams;
    memset(&listParams, '\0', sizeof(LIST_PARAMS_S));
    listParams.compareCb = compareCb;
    listParams.releaseCb = releaseCb;
    listParams.browseCb  = NULL;
    
    if (List_Init(&pData->listenersList, &listParams) != LIST_ERROR_NONE) {
        Loge("List_Init() failed");
        goto list_exit;
    }
    
    if (Task_Init(&pData->videoTask) != TASK_ERROR_NONE) {
        Loge("Task_Init() failed");
        goto task_exit;
    }
    
    if (pthread_mutex_init(&pData->framesHandlerLock, NULL) != 0) {
        Loge("pthread_mutex_init() failed");
        goto framesHandlerLock_exit;
    }
    
    if (sem_init(&pData->notificationSem, 0, 0) != 0) {
        Loge("sem_init() failed");
        goto notificationSem_exit;
    }
    
    if (pthread_mutex_init(&pData->notificationLock, NULL) != 0) {
        Loge("pthread_mutex_init() failed");
        goto notificationLock_exit;
    }
    
    if (pthread_mutex_init(&pData->bufferLock, NULL) != 0) {
        Loge("pthread_mutex_init() failed");
        goto bufferLock_exit;
    }
    
    /* Set public functions */
    (*obj)->registerListener   = registerListener_f;
    (*obj)->unregisterListener = unregisterListener_f;
    (*obj)->getFinalResolution = getFinalResolution_f;
    (*obj)->getMaxBufferSize   = getMaxBufferSize_f;
    (*obj)->startDeviceCapture = startDeviceCapture_f;
    (*obj)->stopDeviceCapture  = stopDeviceCapture_f;
    
    (*obj)->pData = (void*)pData;
    
    return VIDEO_ERROR_NONE;

bufferLock_exit:
    (void)pthread_mutex_destroy(&pData->notificationLock);

notificationLock_exit:
    (void)sem_destroy(&pData->notificationSem);

notificationSem_exit:
    (void)pthread_mutex_destroy(&pData->framesHandlerLock);

framesHandlerLock_exit:
    (void)Task_UnInit(&pData->videoTask);

task_exit:
    (void)List_UnInit(&pData->listenersList);

list_exit:
    if (pData) {
        free(pData);
        pData = NULL;
    }

exit:
    if (*obj) {
        free(*obj);
        *obj = NULL;
    }
    
    return VIDEO_ERROR_INIT;
}

/*!
 * \fn             VIDEO_ERROR_E Video_UnInit(VIDEO_S **obj)
 * \brief          Destroy object created using Video_Init()
 * \param[in, out] obj
 * \return         VIDEO_ERROR_NONE   on success
 *                 VIDEO_ERROR_UNINIT on error
 */
VIDEO_ERROR_E Video_UnInit(VIDEO_S **obj)
{
    assert(obj && *obj && (*obj)->pData);
    
    VIDEO_PRIVATE_DATA_S *pData = (VIDEO_PRIVATE_DATA_S*)((*obj)->pData);
    VIDEO_ERROR_E ret           = VIDEO_ERROR_NONE;
    
    if (pthread_mutex_destroy(&pData->bufferLock) != 0) {
        Loge("pthread_mutex_destroy() failed");
        ret = VIDEO_ERROR_UNINIT;
    }
    
    if (pthread_mutex_destroy(&pData->notificationLock) != 0) {
        Loge("pthread_mutex_destroy() failed");
        ret = VIDEO_ERROR_UNINIT;
    }
    
    if (sem_destroy(&pData->notificationSem) != 0) {
        Loge("sem_destroy() failed");
        ret = VIDEO_ERROR_UNINIT;
    }
    
    if (pthread_mutex_destroy(&pData->framesHandlerLock) != 0) {
        Loge("pthread_mutex_destroy() failed");
        ret = VIDEO_ERROR_UNINIT;
    }
    
    if (Task_UnInit(&pData->videoTask) != TASK_ERROR_NONE) {
        Loge("Task_UnInit() failed");
        ret = VIDEO_ERROR_UNINIT;
    }
    
    if (List_UnInit(&pData->listenersList) != LIST_ERROR_NONE) {
        Loge("List_UnInit() failed");
        ret = VIDEO_ERROR_UNINIT;
    }
    
    free(pData);
    pData = NULL;
    
    free(*obj);
    *obj = NULL;
    
    return ret;
}

/* -------------------------------------------------------------------------------------------- */
/*                                     PRIVATE FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

/*!
 * \fn        static VIDEO_ERROR_E registerListener_f(VIDEO_S *obj, VIDEO_LISTENER_S *listener)
 * \brief     Register a video listener to get captured video frames
 * \param[in] obj
 * \param[in] listener
 * \return    VIDEO_ERROR_NONE  on success
 *            VIDEO_ERROR_<XXX> on error
 */
static VIDEO_ERROR_E registerListener_f(VIDEO_S *obj, VIDEO_LISTENER_S *listener)
{
    assert(obj && obj->pData);
    
    if (!listener || !listener->name || !listener->onVideoBufferAvailableCb) {
        Loge("Bad params");
        return VIDEO_ERROR_PARAMS;
    }
    
    VIDEO_PRIVATE_DATA_S *pData = (VIDEO_PRIVATE_DATA_S*)(obj->pData);
    LIST_S               *list  = pData->listenersList;
    
    if (!list) {
        Loge("listeners' list not initialized yet");
        return VIDEO_ERROR_INIT;
    }
    
    if (list->lock(list) != LIST_ERROR_NONE) {
        Loge("Failed to lock list");
        return VIDEO_ERROR_LOCK;
    }
    
    VIDEO_LISTENER_S *newListener;
    assert((newListener = calloc(1, sizeof(VIDEO_LISTENER_S))));
    
    strncpy(newListener->name, listener->name, sizeof(newListener->name));
    newListener->onVideoBufferAvailableCb = listener->onVideoBufferAvailableCb;
    newListener->userData                 = listener->userData;
    
    list->add(list, (void*)newListener);
    
    (void)list->unlock(list);
    
    return VIDEO_ERROR_NONE;
}

/*!
 * \fn        static VIDEO_ERROR_E unregisterListener_f(VIDEO_S *obj, VIDEO_LISTENER_S *listener)
 * \brief     Unregister a registered video listener to stop receiving captured video frames
 * \param[in] obj
 * \param[in] listener
 * \return    VIDEO_ERROR_NONE  on success
 *            VIDEO_ERROR_<XXX> on error
 */
static VIDEO_ERROR_E unregisterListener_f(VIDEO_S *obj, VIDEO_LISTENER_S *listener)
{
    assert(obj && obj->pData);
    
    if (!listener || !listener->name) {
        Loge("Bad params");
        return VIDEO_ERROR_PARAMS;
    }
    
    VIDEO_PRIVATE_DATA_S *pData = (VIDEO_PRIVATE_DATA_S*)(obj->pData);
    LIST_S               *list  = pData->listenersList;
    
    if (!list) {
        Loge("listeners' list not initialized yet");
        return VIDEO_ERROR_INIT;
    }
    
    if (list->lock(list) != LIST_ERROR_NONE) {
        Loge("Failed to lock list");
        return VIDEO_ERROR_LOCK;
    }
    
    list->remove(list, (void*)listener->name);
    
    (void)list->unlock(list);
    
    return VIDEO_ERROR_NONE;
}

/*!
 * \fn         static VIDEO_ERROR_E getFinalResolution_f(VIDEO_S *obj, VIDEO_RESOLUTION_S *resolution)
 * \brief      Get resolution (Requested resolution might be reviewed depending on hardware constraints
 * \param[in]  obj
 * \param[out] resolution
 * \return     VIDEO_ERROR_NONE  on success
 *             VIDEO_ERROR_<XXX> on error
 */
static VIDEO_ERROR_E getFinalResolution_f(VIDEO_S *obj, VIDEO_RESOLUTION_S *resolution)
{
    assert(obj && obj->pData && resolution);
    
    VIDEO_PRIVATE_DATA_S *pData = (VIDEO_PRIVATE_DATA_S*)(obj->pData);
    
    memcpy(resolution, &pData->finalResolution, sizeof(VIDEO_RESOLUTION_S));
    
    return VIDEO_ERROR_NONE;
}

/*!
 * \fn         static VIDEO_ERROR_E getMaxBufferSize_f(VIDEO_S *obj, size_t *size)
 * \brief      Get the maximum size of video buffer
 * \param[in]  obj
 * \param[out] size
 * \return     VIDEO_ERROR_NONE  on success
 *             VIDEO_ERROR_<XXX> on error
 */
static VIDEO_ERROR_E getMaxBufferSize_f(VIDEO_S *obj, size_t *size)
{
    assert(obj && obj->pData && size);
    
    VIDEO_PRIVATE_DATA_S *pData = (VIDEO_PRIVATE_DATA_S*)(obj->pData);
    
    if (!pData->v4l2) {
        Loge("v4l2 object not initialized");
        return VIDEO_ERROR_START;
    }
    
    *size = pData->v4l2->maxBufferSize;
    
    return VIDEO_ERROR_NONE;
}

/*!
 * \fn        static VIDEO_ERROR_E startDeviceCapture_f(VIDEO_S *obj, VIDEO_PARAMS_S *params)
 * \brief     Start capturing video frames from camera
 * \param[in] obj
 * \param[in] params
 * \return    VIDEO_ERROR_NONE  on success
 *            VIDEO_ERROR_<XXX> on error
 */
static VIDEO_ERROR_E startDeviceCapture_f(VIDEO_S *obj, VIDEO_PARAMS_S *params)
{
    assert(obj && obj->pData && params);
    
    VIDEO_PRIVATE_DATA_S *pData = (VIDEO_PRIVATE_DATA_S*)(obj->pData);
    
    /* Set params and create context*/
    memcpy(&pData->videoParams, params, sizeof(VIDEO_PARAMS_S));
    
    /* Init v4l2 */
    if (V4l2_Init(&pData->v4l2) != V4L2_ERROR_NONE) {
        Loge("V4l2_Init() failed");
        goto init_exit;
    }

    /* Open device */
    V4L2_OPEN_DEVICE_PARAMS_S openDeviceParams;
    memset(&openDeviceParams, '\0', sizeof(V4L2_OPEN_DEVICE_PARAMS_S));
    strncpy(openDeviceParams.path, params->path, sizeof(params->path));
    openDeviceParams.caps = params->caps;
    
    if (pData->v4l2->openDevice(pData->v4l2, &openDeviceParams) != V4L2_ERROR_NONE) {
        Loge("openDevice() failed");
        goto open_exit;
    }

    /* Configure device */
    V4L2_CONFIGURE_DEVICE_PARAMS_S configureDeviceParams;
    memset(&configureDeviceParams, '\0', sizeof(V4L2_CONFIGURE_DEVICE_PARAMS_S));
    configureDeviceParams.type        = params->type;
    configureDeviceParams.pixelformat = params->pixelformat;
    configureDeviceParams.colorspace  = params->colorspace;
    configureDeviceParams.width       = params->captureResolution.width;
    configureDeviceParams.height      = params->captureResolution.height;
    configureDeviceParams.desiredFps  = params->desiredFps;
    
    if (pData->v4l2->configureDevice(pData->v4l2, &configureDeviceParams) != V4L2_ERROR_NONE) {
        Loge("configureDevice() failed");
        goto configure_exit;
    }
    
    pData->finalResolution.width  = pData->v4l2->format.fmt.pix.width;
    pData->finalResolution.height = pData->v4l2->format.fmt.pix.height;
        
    if ((params->captureResolution.width != params->outputResolution.width)
        || (params->captureResolution.height != params->outputResolution.height)) {

        uint8_t selectionApiSupported = 0;

        V4L2_SELECTION_PARAMS_S cropRect;
        cropRect.left   = 0;
        cropRect.top    = 0;
        cropRect.width  = params->captureResolution.width;
        cropRect.height = params->captureResolution.height;

        if (pData->v4l2->setCroppingArea(pData->v4l2, &cropRect) == V4L2_ERROR_NONE) {
            V4L2_SELECTION_PARAMS_S composeRect;
            composeRect.left   = 0;
            composeRect.top    = 0;
            composeRect.width  = params->outputResolution.width;
            composeRect.height = params->outputResolution.height;

            if (pData->v4l2->setComposingArea(pData->v4l2, &composeRect) != V4L2_ERROR_NONE) {
                Loge("Failed to set composing area");
            }
            else {
                pData->finalResolution.width  = composeRect.width;
                pData->finalResolution.height = composeRect.height;

                selectionApiSupported = 1;
            }
        }
        else {
            Loge("Failed to set cropping area");
        }

        if (!selectionApiSupported) {
            Logw("V4L2 selection API is not supported by your driver");

            configureDeviceParams.width  = params->outputResolution.width;
            configureDeviceParams.height = params->outputResolution.height;

            if (pData->v4l2->configureDevice(pData->v4l2, &configureDeviceParams) == V4L2_ERROR_NONE) {
                pData->finalResolution.width  = pData->v4l2->format.fmt.pix.width;
                pData->finalResolution.height = pData->v4l2->format.fmt.pix.height;
            }
        }
    }

    /* Request buffers */
    V4L2_REQUEST_BUFFERS_PARAMS_S requestBuffersParams;
    memset(&requestBuffersParams, '\0', sizeof(V4L2_REQUEST_BUFFERS_PARAMS_S));
    requestBuffersParams.count  = params->count;
    requestBuffersParams.memory = params->memory;

    if (pData->v4l2->requestBuffers(pData->v4l2, &requestBuffersParams) != V4L2_ERROR_NONE) {
        Loge("requestBuffers() failed");
        goto reqBuf_exit;
    }

    /* Start capture */
    if (pData->v4l2->startCapture(pData->v4l2) != V4L2_ERROR_NONE) {
        Loge("startCapture() failed");
        goto start_exit;
    }

    /* Start tasks */
    strcpy(pData->framesHandlerParams.name, FRAMES_HANLDER_TASK_NAME);
    pData->framesHandlerParams.priority = params->priority;
    pData->framesHandlerParams.fct      = framesHandlerFct_f;
    pData->framesHandlerParams.fctData  = obj;
    pData->framesHandlerParams.atExit   = framesHandlerAtExit_f;

    if (pData->videoTask->create(pData->videoTask, &pData->framesHandlerParams) != TASK_ERROR_NONE) {
        Loge("Failed to create framesHandler task");
        goto frames_create_exit;
    }

    strcpy(pData->notificationParams.name, NOTIFICATION_TASK_NAME);
    pData->notificationParams.priority = params->priority;
    pData->notificationParams.fct      = notificationFct_f;
    pData->notificationParams.fctData  = obj;
    pData->notificationParams.atExit   = NULL;
    
    if (pData->videoTask->create(pData->videoTask, &pData->notificationParams) != TASK_ERROR_NONE) {
        Loge("Failed to create notification task");
        goto notification_create_exit;
    }
    
    (void)pData->videoTask->start(pData->videoTask, &pData->framesHandlerParams);
    (void)pData->videoTask->start(pData->videoTask, &pData->notificationParams);
    
    return VIDEO_ERROR_NONE;

notification_create_exit:
    (void)pData->videoTask->destroy(pData->videoTask, &pData->framesHandlerParams);

frames_create_exit:
    (void)pData->v4l2->stopCapture(pData->v4l2);
    
start_exit:
    (void)pData->v4l2->releaseBuffers(pData->v4l2);

reqBuf_exit:
configure_exit:
    (void)pData->v4l2->closeDevice(pData->v4l2);

open_exit:
    (void)V4l2_UnInit(&pData->v4l2);
    
init_exit:
    return VIDEO_ERROR_START;
}

/*!
 * \fn        static VIDEO_ERROR_E stopDeviceCapture_f(VIDEO_S *obj)
 * \brief     Stop capturing video frames from camera
 * \param[in] obj
 * \return    VIDEO_ERROR_NONE  on success
 *            VIDEO_ERROR_<XXX> on error
 */
static VIDEO_ERROR_E stopDeviceCapture_f(VIDEO_S *obj)
{
    assert(obj && obj->pData);
    
    VIDEO_PRIVATE_DATA_S *pData = (VIDEO_PRIVATE_DATA_S*)(obj->pData);
    
    /* Stop tasks */
    pData->quit = 1;
    
    sem_post(&pData->notificationSem);
    
    (void)pData->videoTask->stop(pData->videoTask, &pData->framesHandlerParams);
    (void)pData->videoTask->stop(pData->videoTask, &pData->notificationParams);
    
    (void)pData->videoTask->destroy(pData->videoTask, &pData->framesHandlerParams);
    (void)pData->videoTask->destroy(pData->videoTask, &pData->notificationParams);
    
    /* Stop capture */
    if (pData->v4l2->stopCapture(pData->v4l2) != V4L2_ERROR_NONE) {
        Loge("stopCapture() failed");
    }
    
    /* Release buffers */
    if (pData->v4l2->releaseBuffers(pData->v4l2) != V4L2_ERROR_NONE) {
        Loge("releaseBuffers() failed");
    }
    
    /* Close device */
    if (pData->v4l2->closeDevice(pData->v4l2) != V4L2_ERROR_NONE) {
        Loge("closeDevice() failed");
    }
    
    /* UnInit v4l2 */
    if (V4l2_UnInit(&pData->v4l2) != V4L2_ERROR_NONE) {
        Loge("V4l2_UnInit() failed");
    }
    
    return VIDEO_ERROR_NONE;
}

/*!
 * \fn        static VIDEO_ERROR_E lockBuffer_f(VIDEO_S *obj)
 * \brief     Require lock to have exclusive access to buffer
 * \param[in] obj
 * \return    VIDEO_ERROR_NONE  on success
 *            VIDEO_ERROR_<XXX> on error
 */
static VIDEO_ERROR_E lockBuffer_f(VIDEO_S *obj)
{
    assert(obj && obj->pData);
    
    VIDEO_PRIVATE_DATA_S *pData = (VIDEO_PRIVATE_DATA_S*)(obj->pData);
    
    if (pthread_mutex_lock(&(pData->bufferLock)) != 0) {
        Loge("pthread_mutex_lock() failed");
        return VIDEO_ERROR_LOCK;
    }
    
    return VIDEO_ERROR_NONE;
}

/*!
 * \fn        static VIDEO_ERROR_E unlockBuffer_f(VIDEO_S *obj)
 * \brief     Release lock
 * \param[in] obj
 * \return    VIDEO_ERROR_NONE  on success
 *            VIDEO_ERROR_<XXX> on error
 */
static VIDEO_ERROR_E unlockBuffer_f(VIDEO_S *obj)
{
    assert(obj && obj->pData);
    
    VIDEO_PRIVATE_DATA_S *pData = (VIDEO_PRIVATE_DATA_S*)(obj->pData);
    
    if (pthread_mutex_unlock(&(pData->bufferLock)) != 0) {
        Loge("pthread_mutex_unlock() failed");
        return VIDEO_ERROR_LOCK;
    }
    
    return VIDEO_ERROR_NONE;
}

/*!
 * Capture video frames and wake up notification task
 */
static void framesHandlerFct_f(TASK_PARAMS_S *params)
{
    assert(params && params->fctData);
    
    VIDEO_S *video = (VIDEO_S*)params->fctData;
    assert(video && video->pData);
    
    VIDEO_PRIVATE_DATA_S *pData = (VIDEO_PRIVATE_DATA_S*)(video->pData);
    
    uint32_t i;
    int32_t timeout_ms     = -1;
    
    switch (pData->videoParams.awaitMode) {
        case VIDEO_AWAIT_MODE_BLOCKING:
            break;
            
        case VIDEO_AWAIT_MODE_NON_BLOCKING:
            timeout_ms = (int32_t)WAIT_TIME_2S;
            break;
            
        default:
            ;
    }
    
    for (i = 0; i < pData->v4l2->nbBuffers; i++) {
        /* Queue buffer */
        pData->v4l2->queueBuffer(pData->v4l2, i);

        /* Await data */
        pData->v4l2->awaitData(pData->v4l2, timeout_ms);

        /* Dequeue buffer */
        pData->v4l2->dequeueBuffer(pData->v4l2);
            
        /* Fill in listener's buffer */
        (void)lockBuffer_f(video);
        
        if (!(video->buffer.data)) {
            assert((video->buffer.data = calloc(1, pData->v4l2->maxBufferSize)));
        }
        
        video->buffer.index  = pData->v4l2->map[i].index;
        video->buffer.length = pData->v4l2->map[i].length;
        video->buffer.offset = pData->v4l2->map[i].offset;
        
        memcpy(video->buffer.data, pData->v4l2->map[i].start, pData->v4l2->map[i].length);
        
        pData->nbFramesLost++;
            
        (void)unlockBuffer_f(video);
        
        /* Notify listeners */
        sem_post(&pData->notificationSem);
    }
}

/*!
 * Wait for new video frames then notify listeners
 */
static void notificationFct_f(TASK_PARAMS_S *params)
{
    assert(params && params->fctData);
    
    VIDEO_S *video = (VIDEO_S*)params->fctData;
    assert(video && video->pData);
    
    VIDEO_PRIVATE_DATA_S *pData = (VIDEO_PRIVATE_DATA_S*)(video->pData);
    
    if (pData->quit) {
        return;
    }
    
    sem_wait(&pData->notificationSem);
    
    if (pData->quit) {
        return;
    }
    
    LIST_S *list = pData->listenersList;

    (void)lockBuffer_f(video);
    
    if (list->lock(list) != LIST_ERROR_NONE) {
        Loge("Failed to lock list");
        goto exit;
    }
    
    uint32_t nbClients;
    if (list->getNbElements(list, &nbClients) == LIST_ERROR_NONE) {
        VIDEO_LISTENER_S *listener = NULL;
        while (nbClients > 0) {
            nbClients--;
            
            if (list->getElement(list, (void*)&listener) != LIST_ERROR_NONE) {
                break;
            }
            
            /* IMPORTANT: Listener has to handle buffer very quickly so no heavy operations please!! */
            listener->onVideoBufferAvailableCb(&video->buffer, listener->userData);
        }
    }
    
    (void)list->unlock(list);
    
    pData->nbFramesLost--;

exit:
    (void)unlockBuffer_f(video);
}

/*!
 * Called when stopping framesHandler task
 */
static void framesHandlerAtExit_f(TASK_PARAMS_S *params)
{
    assert(params && params->fctData);
    
    VIDEO_S *video = (VIDEO_S*)params->fctData;
    
    /* Release buffer */
    (void)lockBuffer_f(video);
    
    if (video->buffer.data) {
        free(video->buffer.data);
        video->buffer.data = NULL;
    }
    
    (void)unlockBuffer_f(video);
}

/*!
 * Compare two listeners
 */
static uint8_t compareCb(LIST_S *obj, void *elementToCheck, void *userData)
{
    assert(obj && elementToCheck && userData);
    
    VIDEO_LISTENER_S *listener  = (VIDEO_LISTENER_S*)elementToCheck;
    char *nameOfElementToRemove = (char*)userData;
        
    return (strncmp(listener->name, nameOfElementToRemove, strlen(listener->name)) == 0);
}

/*!
 * Release a listener
 */
static void releaseCb(LIST_S *obj, void *element)
{
    assert(obj && element);
    
    VIDEO_LISTENER_S *listener = (VIDEO_LISTENER_S*)element;
    
    listener->userData = NULL;
    
    free(listener);
    listener = NULL;
}
