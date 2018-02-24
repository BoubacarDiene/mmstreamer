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
* \file VideosListeners.c
* \brief TODO
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// HEADERS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#include "core/Listeners.h"

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// MACROS ////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "VideosListeners"

#define VIDEO_LISTENER4GFX_NAME    "videoListener4Gfx"
#define VIDEO_LISTENER4SERVER_NAME "videoListener4Server"

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// TYPES /////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

struct videos_listeners_private_data_s {
    uint8_t                   videoIndex;
    struct buffer_s           buffer;
    struct listeners_params_s *listenersParams;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////// PUBLIC FUNCTIONS PROTOTYPES //////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum listeners_error_e setVideosListeners_f(struct listeners_s *obj);
enum listeners_error_e unsetVideosListeners_f(struct listeners_s *obj);

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////////// CALLBACKS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

static void onVideo4GfxCb(struct video_buffer_s *videoBuffer, void *userData);
static void onVideo4ServerCb(struct video_buffer_s *videoBuffer, void *userData);

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////// PUBLIC FUNCTIONS IMPLEMENTATION ///////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
enum listeners_error_e setVideosListeners_f(struct listeners_s *obj)
{
    assert(obj);
    
    struct listeners_params_s *listenersParams = &obj->params;
    struct input_s *input                      = &listenersParams->ctx->input;
    struct videos_infos_s *videosInfos         = &listenersParams->ctx->params.videosInfos;
    struct video_device_s ***videoDevices      = &videosInfos->devices;
    struct video_device_s *videoDevice         = NULL;
    uint8_t nbDevices                          = videosInfos->nbDevices;
    uint8_t *nbVideoListeners                  = NULL;
    struct video_listener_s  ***videoListeners = NULL;
    struct video_listener_s  *videoListener    = NULL;

    uint8_t videoIndex, listenerIndex;
    for (videoIndex = 0; videoIndex < nbDevices; videoIndex++) {
        videoDevice      = (*videoDevices)[videoIndex];
        nbVideoListeners = &videoDevice->nbVideoListeners;
        videoListeners   = &videoDevice->videoListeners;

        if (input->graphicsConfig.enable && videoDevice->graphicsDest) {
            *nbVideoListeners = 1;
        }
    
        if (input->serversConfig.enable && videoDevice->serverDest) {
            (*nbVideoListeners)++;
        }
    
        Logd("nbVideoListeners = %u", *nbVideoListeners);
    
        if (*nbVideoListeners > 0) {
            struct videos_listeners_private_data_s *pData;

            assert((*videoListeners = calloc(*nbVideoListeners,
                                             sizeof(struct video_listener_s*))));
            assert((pData = calloc(1, sizeof(struct videos_listeners_private_data_s))));

            pData->videoIndex      = videoIndex;
            pData->listenersParams = listenersParams;

            listenerIndex = 0;

            if (input->graphicsConfig.enable && videoDevice->graphicsDest) {
                (*videoListeners)[listenerIndex] = calloc(1, sizeof(struct video_listener_s));
                assert((*videoListeners)[listenerIndex]);

                videoListener = (*videoListeners)[listenerIndex];
                strcpy(videoListener->name, VIDEO_LISTENER4GFX_NAME);
                videoListener->onVideoBufferAvailableCb = onVideo4GfxCb;
                videoListener->userData                 = pData;
            
                listenerIndex++;
            }
        
            if (input->serversConfig.enable && videoDevice->serverDest) {
                (*videoListeners)[listenerIndex] = calloc(1, sizeof(struct video_listener_s));
                assert((*videoListeners)[listenerIndex]);

                videoListener = (*videoListeners)[listenerIndex];
                strcpy(videoListener->name, VIDEO_LISTENER4SERVER_NAME);
                videoListener->onVideoBufferAvailableCb = onVideo4ServerCb;
                videoListener->userData                 = pData;
            }
        }
    }
    
    return LISTENERS_ERROR_NONE;
}

/*!
 *
 */
enum listeners_error_e unsetVideosListeners_f(struct listeners_s *obj)
{
    assert(obj);
    
    struct listeners_params_s *listenersParams = &obj->params;
    struct videos_infos_s *videosInfos         = &listenersParams->ctx->params.videosInfos;
    struct video_device_s ***videoDevices      = &videosInfos->devices;
    struct video_device_s *videoDevice         = NULL;
    uint8_t nbDevices                          = videosInfos->nbDevices;
    uint8_t nbVideoListeners                   = 0;
    struct video_listener_s  ***videoListeners = NULL;
    struct video_listener_s  **videoListener   = NULL;
    struct videos_listeners_private_data_s *pData     = NULL;

    uint8_t videoIndex, listenerIndex;
    for (videoIndex = 0; videoIndex < nbDevices; videoIndex++) {
        videoDevice      = (*videoDevices)[videoIndex];
        nbVideoListeners = videoDevice->nbVideoListeners;
        videoListeners   = &videoDevice->videoListeners;

        for (listenerIndex = 0; listenerIndex < nbVideoListeners; listenerIndex++) {
            videoListener              = &(*videoListeners)[listenerIndex];
            pData                      = (struct videos_listeners_private_data_s*)((*videoListener)->userData);
            (*videoListener)->userData = NULL;

            free(*videoListener);
            *videoListener = NULL;
        }

        if (pData) {
            pData->listenersParams = NULL;
            pData->buffer.data     = NULL;
            free(pData);
        }

        free(*videoListeners);
        *videoListeners = NULL;
    }

    return LISTENERS_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////////// CALLBACKS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static void onVideo4GfxCb(struct video_buffer_s *videoBuffer, void *userData)
{
    assert(videoBuffer && userData);
    
    struct videos_listeners_private_data_s *pData = (struct videos_listeners_private_data_s*)userData;
    struct context_s *ctx                         = pData->listenersParams->ctx;
    struct graphics_s *graphicsObj                = ctx->modules.graphicsObj;
    struct graphics_infos_s *graphicsInfos        = &ctx->params.graphicsInfos;
    struct videos_infos_s *videosInfos            = &ctx->params.videosInfos;
    struct video_device_s *videoDevice            = videosInfos->devices[pData->videoIndex];
    
    if (graphicsObj
        && videoDevice->graphicsDest
        && (graphicsInfos->state == MODULE_STATE_STARTED)) {

        pData->buffer.data   = videoBuffer->data;
        pData->buffer.length = videoBuffer->length;
        
        if (videoDevice->graphicsIndex == -1) {
            uint32_t index;
            for (index = 0; index < graphicsInfos->nbGfxElements; index++) {
                if (strcmp(graphicsInfos->gfxElements[index]->name,
                           videoDevice->graphicsDest) == 0) {
                    Logd("Element \"%s\" found at index \"%u\"", videoDevice->graphicsDest, index);
                    break;
                }
            }
            if (index < graphicsInfos->nbGfxElements) {
                videoDevice->graphicsIndex = index;
            }
            else {
                Loge("Element \"%s\" does not exist", videoDevice->graphicsDest);
                return;
            }
        }
        
        (void)graphicsObj->setData(graphicsObj, videoDevice->graphicsDest, &pData->buffer);
    }
}

/*!
 *
 */
static void onVideo4ServerCb(struct video_buffer_s *videoBuffer, void *userData)
{
    assert(videoBuffer && userData);
    
    struct videos_listeners_private_data_s *pData = (struct videos_listeners_private_data_s*)userData;
    struct context_s *ctx                         = pData->listenersParams->ctx;
    struct graphics_s *graphicsObj                = ctx->modules.graphicsObj;
    struct server_s *serverObj                    = ctx->modules.serverObj;
    struct servers_infos_s *serversInfos          = &ctx->params.serversInfos;
    struct server_infos_s *serverInfos            = NULL;
    struct videos_infos_s *videosInfos            = &ctx->params.videosInfos;
    struct video_device_s *videoDevice            = videosInfos->devices[pData->videoIndex];

    pData->buffer.data   = videoBuffer->data;
    pData->buffer.length = videoBuffer->length;
    
    if (serverObj && videoDevice->serverDest) {
        if (videoDevice->serverIndex == -1) {
            uint8_t index;
            for (index = 0; index < serversInfos->nbServers; index++) {
                serverInfos = serversInfos->serverInfos[index];

                if (strcmp(serverInfos->serverParams.name, videoDevice->serverDest) == 0) {
                    Logd("Server \"%s\" found at index \"%u\"", videoDevice->serverDest, index);
                    break;
                }
            }

            if (index == serversInfos->nbServers) {
                Loge("Server \"%s\" does not exist", videoDevice->serverDest);
                return;
            }

            videoDevice->serverIndex = index;
        }
        else {
            serverInfos = serversInfos->serverInfos[videoDevice->serverIndex];
        }

        if (serverInfos->state == MODULE_STATE_STARTED) {
            (void)serverObj->sendData(serverObj, &serverInfos->serverParams, &pData->buffer);
        }
    }
}
