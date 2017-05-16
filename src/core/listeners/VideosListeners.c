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
* \file   VideosListeners.c
* \brief  TODO
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include "core/Listeners.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                            */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "VIDEOS-LISTENERS"

#define VIDEO_LISTENER4GFX_NAME    "videoListener4Gfx"
#define VIDEO_LISTENER4SERVER_NAME "videoListener4Server"

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

typedef struct VIDEOS_LISTENERS_INTERNAL_PDATA_S {
    uint8_t           videoIndex;
    LISTENERS_PDATA_S *listenersPdata;
} VIDEOS_LISTENERS_INTERNAL_PDATA_S;

/* -------------------------------------------------------------------------------------------- */
/*                                         PROTOTYPES                                           */
/* -------------------------------------------------------------------------------------------- */

LISTENERS_ERROR_E setVideosListeners_f  (LISTENERS_S *obj);
LISTENERS_ERROR_E unsetVideosListeners_f(LISTENERS_S *obj);

static void onVideo4GfxCb   (VIDEO_BUFFER_S *videoBuffer, void *userData);
static void onVideo4ServerCb(VIDEO_BUFFER_S *videoBuffer, void *userData);

/* -------------------------------------------------------------------------------------------- */
/*                                          VARIABLES                                           */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                          FUNCTIONS                                           */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
LISTENERS_ERROR_E setVideosListeners_f(LISTENERS_S *obj)
{
    assert(obj && obj->pData);
    
    LISTENERS_PDATA_S *pData            = (LISTENERS_PDATA_S*)(obj->pData);
    INPUT_S *input                      = &pData->ctx->input;
    VIDEOS_INFOS_S *videosInfos         = &pData->ctx->params.videosInfos;
    VIDEO_DEVICE_S ***videoDevices      = &videosInfos->devices;
    VIDEO_DEVICE_S *videoDevice         = NULL;
    uint8_t nbDevices                   = videosInfos->nbDevices;
    uint8_t *nbVideoListeners           = NULL;
    VIDEO_LISTENER_S  ***videoListeners = NULL;
    VIDEO_LISTENER_S  *videoListener    = NULL;

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
            VIDEOS_LISTENERS_INTERNAL_PDATA_S *internalPdata;

            assert((*videoListeners = (VIDEO_LISTENER_S**)calloc(*nbVideoListeners, sizeof(VIDEO_LISTENER_S*))));
            assert((internalPdata = (VIDEOS_LISTENERS_INTERNAL_PDATA_S*)calloc(1, sizeof(VIDEOS_LISTENERS_INTERNAL_PDATA_S))));

            internalPdata->videoIndex     = videoIndex;
            internalPdata->listenersPdata = pData;

            listenerIndex = 0;

            if (input->graphicsConfig.enable && videoDevice->graphicsDest) {
                assert(((*videoListeners)[listenerIndex] = calloc(1, sizeof(VIDEO_LISTENER_S))));
                videoListener = (*videoListeners)[listenerIndex];

                strcpy(videoListener->name, VIDEO_LISTENER4GFX_NAME);
                videoListener->onVideoBufferAvailableCb = onVideo4GfxCb;
                videoListener->userData                 = internalPdata;
            
                listenerIndex++;
            }
        
            if (input->serversConfig.enable && videoDevice->serverDest) {
                assert(((*videoListeners)[listenerIndex] = calloc(1, sizeof(VIDEO_LISTENER_S))));
                videoListener = (*videoListeners)[listenerIndex];

                strcpy(videoListener->name, VIDEO_LISTENER4SERVER_NAME);
                videoListener->onVideoBufferAvailableCb = onVideo4ServerCb;
                videoListener->userData                 = internalPdata;
            }
        }
    }
    
    return LISTENERS_ERROR_NONE;
}

/*!
 *
 */
LISTENERS_ERROR_E unsetVideosListeners_f(LISTENERS_S *obj)
{
    assert(obj && obj->pData);
    
    LISTENERS_PDATA_S *pData                          = (LISTENERS_PDATA_S*)(obj->pData);
    VIDEOS_INFOS_S *videosInfos                       = &pData->ctx->params.videosInfos;
    VIDEO_DEVICE_S ***videoDevices                    = &videosInfos->devices;
    VIDEO_DEVICE_S *videoDevice                       = NULL;
    uint8_t nbDevices                                 = videosInfos->nbDevices;
    uint8_t nbVideoListeners                          = 0;
    VIDEO_LISTENER_S  ***videoListeners               = NULL;
    VIDEO_LISTENER_S  **videoListener                 = NULL;
    VIDEOS_LISTENERS_INTERNAL_PDATA_S **internalPdata = NULL;

    uint8_t videoIndex, listenerIndex;
    for (videoIndex = 0; videoIndex < nbDevices; videoIndex++) {
        videoDevice      = (*videoDevices)[videoIndex];
        nbVideoListeners = videoDevice->nbVideoListeners;
        videoListeners   = &videoDevice->videoListeners;

        for (listenerIndex = 0; listenerIndex < nbVideoListeners; listenerIndex++) {
            videoListener              = &(*videoListeners)[listenerIndex];
            internalPdata              = (VIDEOS_LISTENERS_INTERNAL_PDATA_S**)&((*videoListener)->userData);
            (*videoListener)->userData = NULL;

            if (internalPdata && *internalPdata) {
                (*internalPdata)->listenersPdata = NULL;
                free(*internalPdata);
                *internalPdata = NULL;
            }

            free(*videoListener);
            *videoListener = NULL;
        }

        free(*videoListeners);
        *videoListeners = NULL;
    }

    return LISTENERS_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/*                                          CALLBACKS                                           */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static void onVideo4GfxCb(VIDEO_BUFFER_S *videoBuffer, void *userData)
{
    assert(videoBuffer && userData);
    
    VIDEOS_LISTENERS_INTERNAL_PDATA_S *internalPdata = (VIDEOS_LISTENERS_INTERNAL_PDATA_S*)userData;
    LISTENERS_PDATA_S *listenersPdata                = (LISTENERS_PDATA_S*)internalPdata->listenersPdata;
    GRAPHICS_S *graphicsObj                          = listenersPdata->ctx->modules.graphicsObj;
    GRAPHICS_INFOS_S *graphicsInfos                  = &listenersPdata->ctx->params.graphicsInfos;
    VIDEOS_INFOS_S *videosInfos                      = &listenersPdata->ctx->params.videosInfos;
    VIDEO_DEVICE_S *videoDevice                      = videosInfos->devices[internalPdata->videoIndex];
    
    if (graphicsObj
        && videoDevice->graphicsDest
        && (graphicsInfos->state == MODULE_STATE_STARTED)) {

        listenersPdata->buffer.data   = videoBuffer->data;
        listenersPdata->buffer.length = videoBuffer->length;
        
        if (videoDevice->graphicsIndex == -1) {
            uint32_t index;
            for (index = 0; index < graphicsInfos->nbGfxElements; index++) {
                if (strcmp(graphicsInfos->gfxElements[index]->name, videoDevice->graphicsDest) == 0) {
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
        
        (void)graphicsObj->setData(graphicsObj, videoDevice->graphicsDest, &listenersPdata->buffer);
    }
}

/*!
 *
 */
static void onVideo4ServerCb(VIDEO_BUFFER_S *videoBuffer, void *userData)
{
    assert(videoBuffer && userData);
    
    VIDEOS_LISTENERS_INTERNAL_PDATA_S *internalPdata = (VIDEOS_LISTENERS_INTERNAL_PDATA_S*)userData;
    LISTENERS_PDATA_S *listenersPdata                = (LISTENERS_PDATA_S*)internalPdata->listenersPdata;
    GRAPHICS_S *graphicsObj                          = listenersPdata->ctx->modules.graphicsObj;
    SERVER_S *serverObj                              = listenersPdata->ctx->modules.serverObj;
    SERVERS_INFOS_S *serversInfos                    = &listenersPdata->ctx->params.serversInfos;
    SERVER_INFOS_S *serverInfos                      = NULL;
    VIDEOS_INFOS_S *videosInfos                      = &listenersPdata->ctx->params.videosInfos;
    VIDEO_DEVICE_S *videoDevice                      = videosInfos->devices[internalPdata->videoIndex];

    listenersPdata->buffer.data   = videoBuffer->data;
    listenersPdata->buffer.length = videoBuffer->length;
    
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
            (void)serverObj->sendData(serverObj, &serverInfos->serverParams, &listenersPdata->buffer);
        }
    }
}
