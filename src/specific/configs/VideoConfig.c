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

#include "specific/Specific.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                            */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "SPECIFIC-VIDEOCONFIG"

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                          VARIABLES                                           */
/* -------------------------------------------------------------------------------------------- */

VIDEO_CONFIG_S gVideoConfig[] = {
    // Choice 0
	{
	    .caps        = V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING,
	    .type        = V4L2_BUF_TYPE_VIDEO_CAPTURE,
	    .pixelformat = V4L2_PIX_FMT_MJPEG,
	    .colorspace  = V4L2_COLORSPACE_JPEG,
	    .memory      = V4L2_MEMORY_MMAP,
	    .awaitMode   = VIDEO_AWAIT_MODE_BLOCKING
	},

    // Choice 1
	{
	    .caps        = V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING,
	    .type        = V4L2_BUF_TYPE_VIDEO_CAPTURE,
	    .pixelformat = V4L2_PIX_FMT_MJPEG,
	    .colorspace  = V4L2_COLORSPACE_JPEG,
	    .memory      = V4L2_MEMORY_USERPTR,
	    .awaitMode   = VIDEO_AWAIT_MODE_BLOCKING
	},

    // Choice 2
	{
	    .caps        = V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING,
	    .type        = V4L2_BUF_TYPE_VIDEO_CAPTURE,
	    .pixelformat = V4L2_PIX_FMT_YVYU,
	    .colorspace  = V4L2_COLORSPACE_SMPTE170M,
	    .memory      = V4L2_MEMORY_MMAP,
	    .awaitMode   = VIDEO_AWAIT_MODE_BLOCKING
	},

    // Choice 3
	{
	    .caps        = V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING,
	    .type        = V4L2_BUF_TYPE_VIDEO_CAPTURE,
	    .pixelformat = V4L2_PIX_FMT_YVYU,
	    .colorspace  = V4L2_COLORSPACE_SMPTE170M,
	    .memory      = V4L2_MEMORY_USERPTR,
	    .awaitMode   = VIDEO_AWAIT_MODE_BLOCKING
	}
};

uint32_t gNbVideoConfigs = (uint32_t)(sizeof(gVideoConfig) / sizeof(gVideoConfig[0]));
