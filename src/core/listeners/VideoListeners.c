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
* \file   VideoListeners.c
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
#define TAG "VIDEO-LISTENERS"

#define VIDEO_LISTENER4GFX_NAME    "videoListener4Gfx"
#define VIDEO_LISTENER4SERVER_NAME "videoListener4Server"

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                         PROTOTYPES                                           */
/* -------------------------------------------------------------------------------------------- */

LISTENERS_ERROR_E setVideoListeners_f  (LISTENERS_S *obj);
LISTENERS_ERROR_E unsetVideoListeners_f(LISTENERS_S *obj);

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
LISTENERS_ERROR_E setVideoListeners_f(LISTENERS_S *obj)
{
    assert(obj && obj->pData);
    
    LISTENERS_PDATA_S *pData            = (LISTENERS_PDATA_S*)(obj->pData);
    INPUT_S *input                      = &pData->ctx->input;
    VIDEO_INFOS_S *videoInfos           = &pData->ctx->params.videoInfos;
    uint8_t *nbVideoListeners           = &videoInfos->nbVideoListeners;
    VIDEO_LISTENER_S  ***videoListeners = &videoInfos->videoListeners;
    uint8_t index                       = 0;
    
    if (input->graphicsEnabled && videoInfos->graphicsDest) {
        *nbVideoListeners = 1;
    }
    
    if (input->serverEnabled && videoInfos->serverDest) {
        (*nbVideoListeners)++;
    }
    
    Logd("nbVideoListeners = %u", *nbVideoListeners);
    
    if (*nbVideoListeners > 0) {
        assert((*videoListeners = (VIDEO_LISTENER_S**)calloc(*nbVideoListeners, sizeof(VIDEO_LISTENER_S*))));
        
        if (input->graphicsEnabled && videoInfos->graphicsDest) {
            assert(((*videoListeners)[index] = calloc(1, sizeof(VIDEO_LISTENER_S))));
            strcpy((*videoListeners)[index]->name, VIDEO_LISTENER4GFX_NAME);
            (*videoListeners)[index]->onVideoBufferAvailableCb = onVideo4GfxCb;
            (*videoListeners)[index]->userData                 = pData;
            
            index++;
        }
        
        if (input->serverEnabled && videoInfos->serverDest) {
            assert(((*videoListeners)[index] = calloc(1, sizeof(VIDEO_LISTENER_S))));
            strcpy((*videoListeners)[index]->name, VIDEO_LISTENER4SERVER_NAME);
            (*videoListeners)[index]->onVideoBufferAvailableCb = onVideo4ServerCb;
            (*videoListeners)[index]->userData                 = pData;
        }
    }
    
    return LISTENERS_ERROR_NONE;
}

/*!
 *
 */
LISTENERS_ERROR_E unsetVideoListeners_f(LISTENERS_S *obj)
{
    assert(obj && obj->pData);
    
    LISTENERS_PDATA_S *pData            = (LISTENERS_PDATA_S*)(obj->pData);
    VIDEO_INFOS_S *videoInfos           = &pData->ctx->params.videoInfos;
    uint8_t *nbVideoListeners           = &videoInfos->nbVideoListeners;
    VIDEO_LISTENER_S  ***videoListeners = &videoInfos->videoListeners;
    
    uint8_t index;
    for (index = 0; index < *nbVideoListeners; index++) {
        (*videoListeners)[index]->userData = NULL;
        free((*videoListeners)[index]);
        (*videoListeners)[index] = NULL;
    }
    
    free((*videoListeners));
    (*videoListeners) = NULL;
    
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
    
    LISTENERS_PDATA_S *pData  = (LISTENERS_PDATA_S*)userData;
    GRAPHICS_S *graphicsObj   = pData->ctx->modules.graphicsObj;
    VIDEO_INFOS_S *videoInfos = &pData->ctx->params.videoInfos;
    
    if (graphicsObj && videoInfos->graphicsDest) {
        pData->buffer.data   = videoBuffer->data;
        pData->buffer.length = videoBuffer->length;
        
        if (videoInfos->graphicsIndex == -1) {
            GRAPHICS_INFOS_S *graphicsInfos = &pData->ctx->params.graphicsInfos;
            uint32_t index;
            for (index = 0; index < graphicsInfos->nbGfxElements; index++) {
                if (strcmp(graphicsInfos->gfxElements[index]->name, videoInfos->graphicsDest) == 0) {
                    Logd("Element \"%s\" found at index \"%u\"", videoInfos->graphicsDest, index);
                    break;
                }
            }
            if (index < graphicsInfos->nbGfxElements) {
                videoInfos->graphicsIndex = index;
            }
            else {
                Loge("Element \"%s\" does not exist", videoInfos->graphicsDest);
                return;
            }
        }
        
        graphicsObj->setData(graphicsObj, videoInfos->graphicsDest, &pData->buffer);
    }
}

/*!
 *
 */
static void onVideo4ServerCb(VIDEO_BUFFER_S *videoBuffer, void *userData)
{
    assert(videoBuffer && userData);
    
    LISTENERS_PDATA_S *pData  = (LISTENERS_PDATA_S*)userData;
    SERVER_S *serverObj       = pData->ctx->modules.serverObj;
    VIDEO_INFOS_S *videoInfos = &pData->ctx->params.videoInfos;
    
    if (serverObj && videoInfos->serverDest) {
        SERVERS_INFOS_S *serversInfos = &pData->ctx->params.serversInfos;
        if (videoInfos->serverIndex == -1) {
            uint8_t index;
            for (index = 0; index < serversInfos->nbServers; index++) {
                if (strcmp(serversInfos->serverParams[index]->name, videoInfos->serverDest) == 0) {
                    Logd("Server \"%s\" found at index \"%u\"", videoInfos->serverDest, index);
                    break;
                }
            }
            if (index < serversInfos->nbServers) {
                videoInfos->serverIndex = index;
            }
            else {
                Loge("Server \"%s\" does not exist", videoInfos->serverDest);
                return;
            }
        }
         
        pData->buffer.data   = videoBuffer->data;
        pData->buffer.length = videoBuffer->length;
            
        serverObj->sendData(serverObj, serversInfos->serverParams[videoInfos->serverIndex], &pData->buffer);
    }
}
