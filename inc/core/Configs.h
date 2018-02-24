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
* \file Configs.h
* \author Boubacar DIENE
*/

#ifndef __CORE_CONFIGS_H__
#define __CORE_CONFIGS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// HEADERS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#include "video/Video.h"

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////// TYPES DECLARATION ///////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum configs_error_e;

struct video_capability_s;
struct video_config_choice_s;
struct video_config_s;
struct configs_s;

/* -------------------------------------------------------------------------------------------- */
/* ///////////////////////////////////// PUBLIC FUNCTIONS ///////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

typedef enum configs_error_e (*configs_get_video_config_f)(struct configs_s *obj,
                                                      struct video_config_s *config,
                                                      struct video_config_choice_s *configChoice);

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// TYPES /////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum configs_error_e {
    CONFIGS_ERROR_NONE,
    CONFIGS_ERROR_INIT,
    CONFIGS_ERROR_UNINIT,
    CONFIGS_ERROR_PARAMS
};

struct video_capability_s {
    char *item;
};

struct video_config_choice_s {
    uint8_t                   nbItems;
    struct video_capability_s *capabilities;

    char                      *bufferType;
    char                      *pixelFormat;
    char                      *colorspace;
    char                      *memory;
    char                      *awaitMode;
};

struct video_config_s {
    uint32_t                caps;
    enum v4l2_buf_type      type;
    uint32_t                pixelformat;
    enum v4l2_colorspace    colorspace;
    enum v4l2_memory        memory;
    enum video_await_mode_e awaitMode;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// MAIN CONTEXT /////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

struct configs_s {
    configs_get_video_config_f getVideoConfig;

    void *pData;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// INITIALIZER //////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum configs_error_e Configs_Init(struct configs_s **obj);
enum configs_error_e Configs_UnInit(struct configs_s **obj);

#ifdef __cplusplus
}
#endif

#endif //__CORE_CONFIGS_H__
