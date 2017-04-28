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

typedef struct CONFIGS_VIDEO_CAPABILITY_S {
    char     *str;
    uint32_t value;
} CONFIGS_VIDEO_CAPABILITY_S;

typedef struct CONFIGS_VIDEO_BUFFER_TYPE_S {
    char               *str;
    enum v4l2_buf_type value;
} CONFIGS_VIDEO_BUFFER_TYPE_S;

typedef struct CONFIGS_VIDEO_PIXEL_FORMAT_S {
    char     *str;
    uint32_t value;
} CONFIGS_VIDEO_PIXEL_FORMAT_S;

typedef struct CONFIGS_VIDEO_COLORSPACE_S {
    char                 *str;
    enum v4l2_colorspace value;
} CONFIGS_VIDEO_COLORSPACE_S;

typedef struct CONFIGS_VIDEO_MEMORY_S {
    char             *str;
    enum v4l2_memory value;
} CONFIGS_VIDEO_MEMORY_S;

typedef struct CONFIGS_VIDEO_AWAIT_MODE_S {
    char               *str;
    VIDEO_AWAIT_MODE_E value;
} CONFIGS_VIDEO_AWAIT_MODE_S;

typedef struct CONFIGS_PRIVATE_DATA_S {
    uint32_t                     nbVideoCaps;
    CONFIGS_VIDEO_CAPABILITY_S   *videoCaps;

    uint32_t                     nbVideoBufferTypes;
    CONFIGS_VIDEO_BUFFER_TYPE_S  *videoBufferTypes;

    uint32_t                     nbVideoPixelFormats;
    CONFIGS_VIDEO_PIXEL_FORMAT_S *videoPixelFormats;

    uint32_t                     nbVideoColorspaces;
    CONFIGS_VIDEO_COLORSPACE_S   *videoColorspaces;

    uint32_t                     nbVideoMemories;
    CONFIGS_VIDEO_MEMORY_S       *videoMemories;

    uint32_t                     nbVideoAwaitModes;
    CONFIGS_VIDEO_AWAIT_MODE_S   *videoAwaitModes;
} CONFIGS_PRIVATE_DATA_S;

/* -------------------------------------------------------------------------------------------- */
/*                                          VARIABLES                                           */
/* -------------------------------------------------------------------------------------------- */

CONFIGS_VIDEO_CAPABILITY_S gVideoCaps[] = {
	{ "V4L2_CAP_VIDEO_CAPTURE",             V4L2_CAP_VIDEO_CAPTURE },
	{ "V4L2_CAP_STREAMING",                 V4L2_CAP_STREAMING     },
	{ NULL,                                 V4L2_CAP_VIDEO_CAPTURE }
};

uint32_t gNbVideoCaps = (uint32_t)(sizeof(gVideoCaps) / sizeof(gVideoCaps[0]));

/* -------------------------------------------------------------------------------------------- */

CONFIGS_VIDEO_BUFFER_TYPE_S gVideoBufferTypes[] = {
	{ "V4L2_BUF_TYPE_VIDEO_CAPTURE",        V4L2_BUF_TYPE_VIDEO_CAPTURE },
	{ NULL,                                 V4L2_BUF_TYPE_VIDEO_CAPTURE }
};

uint32_t gNbVideoBufferTypes = (uint32_t)(sizeof(gVideoBufferTypes) / sizeof(gVideoBufferTypes[0]));

/* -------------------------------------------------------------------------------------------- */

CONFIGS_VIDEO_PIXEL_FORMAT_S gVideoPixelFormats[] = {
	{ "V4L2_PIX_FMT_MJPEG",                 V4L2_PIX_FMT_MJPEG },
	{ "V4L2_PIX_FMT_YVYU",                  V4L2_PIX_FMT_YVYU  },
	{ NULL,                                 V4L2_PIX_FMT_MJPEG }
};

uint32_t gNbVideoPixelFormats = (uint32_t)(sizeof(gVideoPixelFormats) / sizeof(gVideoPixelFormats[0]));

/* -------------------------------------------------------------------------------------------- */

CONFIGS_VIDEO_COLORSPACE_S gVideoColorspaces[] = {
	{ "V4L2_COLORSPACE_JPEG",               V4L2_COLORSPACE_JPEG      },
	{ "V4L2_COLORSPACE_SMPTE170M",          V4L2_COLORSPACE_SMPTE170M },
	{ NULL,                                 V4L2_COLORSPACE_JPEG      }
};

uint32_t gNbVideoColorspaces = (uint32_t)(sizeof(gVideoColorspaces) / sizeof(gVideoColorspaces[0]));

/* -------------------------------------------------------------------------------------------- */

CONFIGS_VIDEO_MEMORY_S gVideoMemories[] = {
	{ "V4L2_MEMORY_MMAP",                   V4L2_MEMORY_MMAP    },
	{ "V4L2_MEMORY_USERPTR",                V4L2_MEMORY_USERPTR },
	{ NULL,                                 V4L2_MEMORY_MMAP    }
};

uint32_t gNbVideoMemories = (uint32_t)(sizeof(gVideoMemories) / sizeof(gVideoMemories[0]));

/* -------------------------------------------------------------------------------------------- */

CONFIGS_VIDEO_AWAIT_MODE_S gVideoAwaitModes[] = {
	{ "VIDEO_AWAIT_MODE_BLOCKING",          VIDEO_AWAIT_MODE_BLOCKING     },
	{ "VIDEO_AWAIT_MODE_NON_BLOCKING",      VIDEO_AWAIT_MODE_NON_BLOCKING },
	{ NULL,                                 VIDEO_AWAIT_MODE_BLOCKING     }
};

uint32_t gNbVideoAwaitModes = (uint32_t)(sizeof(gVideoAwaitModes) / sizeof(gVideoAwaitModes[0]));

/* -------------------------------------------------------------------------------------------- */
/*                                         PROTOTYPES                                           */
/* -------------------------------------------------------------------------------------------- */

static CONFIGS_ERROR_E getVideoConfig_f(CONFIGS_S *obj, VIDEO_CONFIG_S *config, VIDEO_CONFIG_CHOICE_S *configChoice);

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

    (*obj)->getVideoConfig = getVideoConfig_f;

    pData->nbVideoCaps         = gNbVideoCaps;
    pData->videoCaps           = gVideoCaps;

    pData->nbVideoBufferTypes  = gNbVideoBufferTypes;
    pData->videoBufferTypes    = gVideoBufferTypes;

    pData->nbVideoPixelFormats = gNbVideoPixelFormats;
    pData->videoPixelFormats   = gVideoPixelFormats;

    pData->nbVideoColorspaces  = gNbVideoColorspaces;
    pData->videoColorspaces    = gVideoColorspaces;

    pData->nbVideoMemories     = gNbVideoMemories;
    pData->videoMemories       = gVideoMemories;

    pData->nbVideoAwaitModes   = gNbVideoAwaitModes;
    pData->videoAwaitModes     = gVideoAwaitModes;

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

    pData->videoCaps         = NULL;
    pData->videoBufferTypes  = NULL;
    pData->videoPixelFormats = NULL;
    pData->videoColorspaces  = NULL;
    pData->videoMemories     = NULL;
    pData->videoAwaitModes   = NULL;

    free((*obj)->pData);
    (*obj)->pData = NULL;

    free(*obj);
    *obj = NULL;

    return CONFIGS_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/*                                     PRIVATE FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

static CONFIGS_ERROR_E getVideoConfig_f(CONFIGS_S *obj, VIDEO_CONFIG_S *config, VIDEO_CONFIG_CHOICE_S *configChoice)
{
    assert(obj && obj->pData && config && configChoice);

    CONFIGS_PRIVATE_DATA_S *pData = (CONFIGS_PRIVATE_DATA_S*)(obj->pData);

    uint32_t i, j;

    /* Capabilities */
    if ((configChoice->nbItems == 0) || !configChoice->capabilities) {
        Loge("No video capability specified");
        return CONFIGS_ERROR_PARAMS;
    }

    for (i = 0; i < configChoice->nbItems; i++) {
        j = 0;
        while ((j < pData->nbVideoCaps)
                && pData->videoCaps[j].str
                && (strcmp(pData->videoCaps[j].str, configChoice->capabilities[i].item) != 0)) {
            j++;
        }

        if (!pData->videoCaps[j].str) {
            Loge("Video capability \"%s\" not handled", configChoice->capabilities[i].item);
            return CONFIGS_ERROR_PARAMS;
        }
        
        config->caps |= pData->videoCaps[j].value;
    }

    /* Buffer type */
    j = 0;
    while ((j < pData->nbVideoBufferTypes)
            && pData->videoBufferTypes[j].str
            && (strcmp(pData->videoBufferTypes[j].str, configChoice->bufferType) != 0)) {
        j++;
    }
    
    if (!pData->videoBufferTypes[j].str) {
        Loge("Video bufferType \"%s\" not handled", configChoice->bufferType);
        return CONFIGS_ERROR_PARAMS;
    }
    
    config->type = pData->videoBufferTypes[j].value;

    /* Pixel format */
    j = 0;
    while ((j < pData->nbVideoPixelFormats)
            && pData->videoPixelFormats[j].str
            && (strcmp(pData->videoPixelFormats[j].str, configChoice->pixelFormat) != 0)) {
        j++;
    }
    
    if (!pData->videoPixelFormats[j].str) {
        Loge("Video pixelFormat \"%s\" not handled", configChoice->pixelFormat);
        return CONFIGS_ERROR_PARAMS;
    }
    
    config->pixelformat = pData->videoPixelFormats[j].value;

    /* Colorspace */
    j = 0;
    while ((j < pData->nbVideoColorspaces)
            && pData->videoColorspaces[j].str
            && (strcmp(pData->videoColorspaces[j].str, configChoice->colorspace) != 0)) {
        j++;
    }
    
    if (!pData->videoColorspaces[j].str) {
        Loge("Video colorspace \"%s\" not handled", configChoice->colorspace);
        return CONFIGS_ERROR_PARAMS;
    }
    
    config->colorspace  = pData->videoColorspaces[j].value;

    /* Memory */
    j = 0;
    while ((j < pData->nbVideoMemories)
            && pData->videoMemories[j].str
            && (strcmp(pData->videoMemories[j].str, configChoice->memory) != 0)) {
        j++;
    }
    
    if (!pData->videoMemories[j].str) {
        Loge("Video memory \"%s\" not handled", configChoice->memory);
        return CONFIGS_ERROR_PARAMS;
    }
    
    config->memory = pData->videoMemories[j].value;

    /* Await mode */
    j = 0;
    while ((j < pData->nbVideoAwaitModes)
            && pData->videoAwaitModes[j].str
            && (strcmp(pData->videoAwaitModes[j].str, configChoice->awaitMode) != 0)) {
        j++;
    }
    
    if (!pData->videoAwaitModes[j].str) {
        Loge("Video awaitMode \"%s\" not handled", configChoice->awaitMode);
        return CONFIGS_ERROR_PARAMS;
    }
    
    config->awaitMode = pData->videoAwaitModes[j].value;

    return CONFIGS_ERROR_NONE;
}
