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
* \file VideoConfig.c
* \brief TODO
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// HEADERS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#include "utils/Log.h"

#include "core/Configs.h"

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// MACROS ////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "VideoConfig"

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// TYPES /////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

struct configs_video_capability_s {
    char     *str;
    uint32_t value;
};

struct configs_video_buffer_type_s {
    char               *str;
    enum v4l2_buf_type value;
};

struct configs_video_pixel_format_s {
    char     *str;
    uint32_t value;
};

struct configs_video_colorspace_s {
    char                 *str;
    enum v4l2_colorspace value;
};

struct configs_video_memory_s {
    char             *str;
    enum v4l2_memory value;
};

struct configs_video_await_mode_s {
    char                    *str;
    enum video_await_mode_e value;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////// PUBLIC FUNCTIONS PROTOTYPES //////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum configs_error_e getVideoConfig_f(struct configs_s *obj, struct video_config_s *config,
                                      struct video_config_choice_s *configChoice);

/* -------------------------------------------------------------------------------------------- */
/* ///////////////////////////////////// GLOBAL VARIABLES ///////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

struct configs_video_capability_s gVideoCaps[] = {
	{ "V4L2_CAP_VIDEO_CAPTURE",  V4L2_CAP_VIDEO_CAPTURE },
	{ "V4L2_CAP_STREAMING",      V4L2_CAP_STREAMING     },
	{ NULL,                      V4L2_CAP_VIDEO_CAPTURE }
};

uint32_t gNbVideoCaps = (uint32_t)(sizeof(gVideoCaps) / sizeof(gVideoCaps[0]));

/* //////////////////////////////////////////////////////////////////////////////////////////// */

struct configs_video_buffer_type_s gVideoBufferTypes[] = {
	{ "V4L2_BUF_TYPE_VIDEO_CAPTURE",  V4L2_BUF_TYPE_VIDEO_CAPTURE },
	{ NULL,                           V4L2_BUF_TYPE_VIDEO_CAPTURE }
};

uint32_t gNbVideoBufferTypes = (uint32_t)(sizeof(gVideoBufferTypes)
                                        / sizeof(gVideoBufferTypes[0]));

/* //////////////////////////////////////////////////////////////////////////////////////////// */

struct configs_video_pixel_format_s gVideoPixelFormats[] = {
	{ "V4L2_PIX_FMT_MJPEG",  V4L2_PIX_FMT_MJPEG },
	{ "V4L2_PIX_FMT_YVYU",   V4L2_PIX_FMT_YVYU  },
	{ NULL,                  V4L2_PIX_FMT_MJPEG }
};

uint32_t gNbVideoPixelFormats = (uint32_t)(sizeof(gVideoPixelFormats)
                                         / sizeof(gVideoPixelFormats[0]));

/* //////////////////////////////////////////////////////////////////////////////////////////// */

struct configs_video_colorspace_s gVideoColorspaces[] = {
	{ "V4L2_COLORSPACE_JPEG",       V4L2_COLORSPACE_JPEG      },
	{ "V4L2_COLORSPACE_SMPTE170M",  V4L2_COLORSPACE_SMPTE170M },
	{ NULL,                         V4L2_COLORSPACE_JPEG      }
};

uint32_t gNbVideoColorspaces = (uint32_t)(sizeof(gVideoColorspaces)
                                        / sizeof(gVideoColorspaces[0]));

/* //////////////////////////////////////////////////////////////////////////////////////////// */

struct configs_video_memory_s gVideoMemories[] = {
	{ "V4L2_MEMORY_MMAP",     V4L2_MEMORY_MMAP    },
	{ "V4L2_MEMORY_USERPTR",  V4L2_MEMORY_USERPTR },
	{ NULL,                   V4L2_MEMORY_MMAP    }
};

uint32_t gNbVideoMemories = (uint32_t)(sizeof(gVideoMemories) / sizeof(gVideoMemories[0]));

/* //////////////////////////////////////////////////////////////////////////////////////////// */

struct configs_video_await_mode_s gVideoAwaitModes[] = {
	{ "VIDEO_AWAIT_MODE_BLOCKING",      VIDEO_AWAIT_MODE_BLOCKING     },
	{ "VIDEO_AWAIT_MODE_NON_BLOCKING",  VIDEO_AWAIT_MODE_NON_BLOCKING },
	{ NULL,                             VIDEO_AWAIT_MODE_BLOCKING     }
};

uint32_t gNbVideoAwaitModes = (uint32_t)(sizeof(gVideoAwaitModes) / sizeof(gVideoAwaitModes[0]));

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////// PUBLIC FUNCTIONS IMPLEMENTATION ///////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum configs_error_e getVideoConfig_f(struct configs_s *obj, struct video_config_s *config,
                                      struct video_config_choice_s *configChoice)
{
    ASSERT(obj && config && configChoice);

    uint32_t i, j;

    /* Capabilities */
    if ((configChoice->nbItems == 0) || !configChoice->capabilities) {
        Loge("No video capability specified");
        return CONFIGS_ERROR_PARAMS;
    }

    for (i = 0; i < configChoice->nbItems; i++) {
        j = 0;
        while ((j < gNbVideoCaps)
                && gVideoCaps[j].str
                && (strcmp(gVideoCaps[j].str, configChoice->capabilities[i].item) != 0)) {
            j++;
        }

        if (!gVideoCaps[j].str) {
            Loge("Video capability \"%s\" not handled", configChoice->capabilities[i].item);
            return CONFIGS_ERROR_PARAMS;
        }
        
        config->caps |= gVideoCaps[j].value;
    }

    /* Buffer type */
    j = 0;
    while ((j < gNbVideoBufferTypes)
            && gVideoBufferTypes[j].str
            && (strcmp(gVideoBufferTypes[j].str, configChoice->bufferType) != 0)) {
        j++;
    }
    
    if (!gVideoBufferTypes[j].str) {
        Loge("Video bufferType \"%s\" not handled", configChoice->bufferType);
        return CONFIGS_ERROR_PARAMS;
    }
    
    config->type = gVideoBufferTypes[j].value;

    /* Pixel format */
    j = 0;
    while ((j < gNbVideoPixelFormats)
            && gVideoPixelFormats[j].str
            && (strcmp(gVideoPixelFormats[j].str, configChoice->pixelFormat) != 0)) {
        j++;
    }
    
    if (!gVideoPixelFormats[j].str) {
        Loge("Video pixelFormat \"%s\" not handled", configChoice->pixelFormat);
        return CONFIGS_ERROR_PARAMS;
    }
    
    config->pixelformat = gVideoPixelFormats[j].value;

    /* Colorspace */
    j = 0;
    while ((j < gNbVideoColorspaces)
            && gVideoColorspaces[j].str
            && (strcmp(gVideoColorspaces[j].str, configChoice->colorspace) != 0)) {
        j++;
    }
    
    if (!gVideoColorspaces[j].str) {
        Loge("Video colorspace \"%s\" not handled", configChoice->colorspace);
        return CONFIGS_ERROR_PARAMS;
    }
    
    config->colorspace  = gVideoColorspaces[j].value;

    /* Memory */
    j = 0;
    while ((j < gNbVideoMemories)
            && gVideoMemories[j].str
            && (strcmp(gVideoMemories[j].str, configChoice->memory) != 0)) {
        j++;
    }
    
    if (!gVideoMemories[j].str) {
        Loge("Video memory \"%s\" not handled", configChoice->memory);
        return CONFIGS_ERROR_PARAMS;
    }
    
    config->memory = gVideoMemories[j].value;

    /* Await mode */
    j = 0;
    while ((j < gNbVideoAwaitModes)
            && gVideoAwaitModes[j].str
            && (strcmp(gVideoAwaitModes[j].str, configChoice->awaitMode) != 0)) {
        j++;
    }
    
    if (!gVideoAwaitModes[j].str) {
        Loge("Video awaitMode \"%s\" not handled", configChoice->awaitMode);
        return CONFIGS_ERROR_PARAMS;
    }
    
    config->awaitMode = gVideoAwaitModes[j].value;

    return CONFIGS_ERROR_NONE;
}
