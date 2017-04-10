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
* \file   VideoConfig.c
* \brief  TODO
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include "utils/Log.h"

#include "core/Configs.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                            */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "CORE-VIDEOCONFIG"

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

typedef struct CONFIGS_PRIVATE_DATA_S {
    VIDEO_CONFIG_S **videoConfigs;
    uint32_t       nbVideoConfigs;
} CONFIGS_PRIVATE_DATA_S;

/* -------------------------------------------------------------------------------------------- */
/*                                          VARIABLES                                           */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                         PROTOTYPES                                           */
/* -------------------------------------------------------------------------------------------- */

static CONFIGS_ERROR_E getVideoConfig_f(CONFIGS_S *obj, VIDEO_CONFIG_S *config, uint32_t configChoice);

/* -------------------------------------------------------------------------------------------- */
/*                                      PUBLIC FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
CONFIGS_ERROR_E Configs_Init(CONFIGS_S **obj)
{
    assert(obj && (*obj = calloc(1, sizeof(CONFIGS_S))));

    CONFIGS_PRIVATE_DATA_S *pData;
    assert((pData = calloc(1, sizeof(CONFIGS_PRIVATE_DATA_S))));

    pData->nbVideoConfigs = 4;
    assert((pData->videoConfigs = calloc(pData->nbVideoConfigs, sizeof(VIDEO_CONFIG_S*))));

    // Choice 0
    assert((pData->videoConfigs[0] = calloc(1, sizeof(VIDEO_CONFIG_S))));
	pData->videoConfigs[0]->caps        = V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING;
	pData->videoConfigs[0]->type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	pData->videoConfigs[0]->pixelformat = V4L2_PIX_FMT_MJPEG;
	pData->videoConfigs[0]->colorspace  = V4L2_COLORSPACE_JPEG;
	pData->videoConfigs[0]->memory      = V4L2_MEMORY_MMAP;
	pData->videoConfigs[0]->awaitMode   = VIDEO_AWAIT_MODE_BLOCKING;

    // Choice 1
    assert((pData->videoConfigs[1] = calloc(1, sizeof(VIDEO_CONFIG_S))));
	pData->videoConfigs[1]->caps        = V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING;
	pData->videoConfigs[1]->type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	pData->videoConfigs[1]->pixelformat = V4L2_PIX_FMT_MJPEG;
	pData->videoConfigs[1]->colorspace  = V4L2_COLORSPACE_JPEG;
	pData->videoConfigs[1]->memory      = V4L2_MEMORY_USERPTR;
	pData->videoConfigs[1]->awaitMode   = VIDEO_AWAIT_MODE_BLOCKING;

    // Choice 2
    assert((pData->videoConfigs[2] = calloc(1, sizeof(VIDEO_CONFIG_S))));
	pData->videoConfigs[2]->caps        = V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING;
	pData->videoConfigs[2]->type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	pData->videoConfigs[2]->pixelformat = V4L2_PIX_FMT_YVYU;
	pData->videoConfigs[2]->colorspace  = V4L2_COLORSPACE_SMPTE170M;
	pData->videoConfigs[2]->memory      = V4L2_MEMORY_MMAP;
	pData->videoConfigs[2]->awaitMode   = VIDEO_AWAIT_MODE_BLOCKING;

    // Choice 3
    assert((pData->videoConfigs[3] = calloc(1, sizeof(VIDEO_CONFIG_S))));
	pData->videoConfigs[3]->caps        = V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING;
	pData->videoConfigs[3]->type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	pData->videoConfigs[3]->pixelformat = V4L2_PIX_FMT_YVYU;
	pData->videoConfigs[3]->colorspace  = V4L2_COLORSPACE_SMPTE170M;
	pData->videoConfigs[3]->memory      = V4L2_MEMORY_USERPTR;
	pData->videoConfigs[3]->awaitMode   = VIDEO_AWAIT_MODE_BLOCKING;

    (*obj)->getVideoConfig = getVideoConfig_f;

    (*obj)->pData = (void*)pData;

    return CONFIGS_ERROR_NONE;
}

/*!
 *
 */
CONFIGS_ERROR_E Configs_UnInit(CONFIGS_S **obj)
{
    assert(obj && *obj && (*obj)->pData);

    CONFIGS_PRIVATE_DATA_S *pData = (CONFIGS_PRIVATE_DATA_S*)((*obj)->pData);

    uint32_t i;
    for (i = 0; i < pData->nbVideoConfigs; i++) {
        free(pData->videoConfigs[i]);
        pData->videoConfigs[i] = NULL;
    }

    free(pData->videoConfigs);
    pData->videoConfigs = NULL;

    free((*obj)->pData);
    (*obj)->pData = NULL;

    free(*obj);
    *obj = NULL;

    return CONFIGS_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/*                                     PRIVATE FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

static CONFIGS_ERROR_E getVideoConfig_f(CONFIGS_S *obj, VIDEO_CONFIG_S *config, uint32_t configChoice)
{
    assert(obj && config && obj->pData);

    CONFIGS_PRIVATE_DATA_S *pData = (CONFIGS_PRIVATE_DATA_S*)(obj->pData);

    if (configChoice >= pData->nbVideoConfigs) {
        Loge("Bad choice %u / Nb video configs : %u", configChoice, pData->nbVideoConfigs);
        return CONFIGS_ERROR_PARAMS;
    }

    config->caps        = pData->videoConfigs[configChoice]->caps;
    config->type        = pData->videoConfigs[configChoice]->type;
    config->pixelformat = pData->videoConfigs[configChoice]->pixelformat;
    config->colorspace  = pData->videoConfigs[configChoice]->colorspace;
    config->memory      = pData->videoConfigs[configChoice]->memory;
    config->awaitMode   = pData->videoConfigs[configChoice]->awaitMode;

    return CONFIGS_ERROR_NONE;
}
