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
* \file   Configs.h
* \author Boubacar DIENE
*/

#ifndef __CORE_CONFIGS_H__
#define __CORE_CONFIGS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include "video/Video.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                             */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEFS                                           */
/* -------------------------------------------------------------------------------------------- */

typedef enum CONFIGS_ERROR_E  CONFIGS_ERROR_E;

typedef struct VIDEO_CONFIG_S VIDEO_CONFIG_S;
typedef struct CONFIGS_S      CONFIGS_S;

typedef CONFIGS_ERROR_E (*CONFIGS_GET_VIDEO_CONFIG_F)(CONFIGS_S *obj, VIDEO_CONFIG_S *config, uint32_t configChoice);

enum CONFIGS_ERROR_E {
    CONFIGS_ERROR_NONE,
    CONFIGS_ERROR_INIT,
    CONFIGS_ERROR_UNINIT,
    CONFIGS_ERROR_PARAMS
};

struct VIDEO_CONFIG_S {
    uint32_t             caps;
    enum v4l2_buf_type   type;
    uint32_t             pixelformat;
    enum v4l2_colorspace colorspace;
    enum v4l2_memory     memory;
    VIDEO_AWAIT_MODE_E   awaitMode;
};

struct CONFIGS_S {
    CONFIGS_GET_VIDEO_CONFIG_F getVideoConfig;

    void                       *pData;
};

/* -------------------------------------------------------------------------------------------- */
/*                                      PUBLIC FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

CONFIGS_ERROR_E Configs_Init  (CONFIGS_S **obj);
CONFIGS_ERROR_E Configs_UnInit(CONFIGS_S **obj);

#ifdef __cplusplus
}
#endif

#endif //__CORE_CONFIGS_H__
