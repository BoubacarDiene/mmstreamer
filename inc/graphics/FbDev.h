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
* \file   FbDev.h
* \brief  Framebuffer device management
*         IMPORTANT : Mmstreamer engine must be run as root to make it have rights to use fbDev
* \author Boubacar DIENE
*/

#ifndef __FBDEV_H__
#define __FBDEV_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include "utils/Common.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                            */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

typedef enum   FBDEV_ERROR_E     FBDEV_ERROR_E;

typedef struct FBDEV_INFOS_S     FBDEV_INFOS_S;
typedef struct FBDEV_S           FBDEV_S;

typedef struct fb_var_screeninfo FBDEV_VSCREENINFO;
typedef struct fb_fix_screeninfo FBDEV_FSCREENINFO;

typedef FBDEV_ERROR_E (*FBDEV_OPEN_F     )(FBDEV_S *obj, const char *fbName);
typedef FBDEV_ERROR_E (*FBDEV_IS_OPENED_F)(FBDEV_S *obj, uint8_t *opened);
typedef FBDEV_ERROR_E (*FBDEV_CLOSE_F    )(FBDEV_S *obj);

typedef FBDEV_ERROR_E (*FBDEV_GET_INFOS_F)(FBDEV_S *obj, FBDEV_INFOS_S *fbInfos);
typedef FBDEV_ERROR_E (*FBDEV_SET_DEPTH_F)(FBDEV_S *obj, uint32_t depth);

typedef FBDEV_ERROR_E (*FBDEV_CLEAR_F  )(FBDEV_S *obj);
typedef FBDEV_ERROR_E (*FBDEV_RESTORE_F)(FBDEV_S *obj);

enum FBDEV_ERROR_E {
    FBDEV_ERROR_NONE,
    FBDEV_ERROR_INIT,
    FBDEV_ERROR_UNINIT,
    FBDEV_ERROR_PARAMS,
    FBDEV_ERROR_IO
};

struct FBDEV_INFOS_S {
    uint32_t width;
    uint32_t height;

    uint32_t depth;
};

struct FBDEV_S {
    FBDEV_OPEN_F      open;
    FBDEV_IS_OPENED_F isOpened;
    FBDEV_CLOSE_F     close;

    FBDEV_GET_INFOS_F getInfos;
    FBDEV_SET_DEPTH_F setDepth;

    FBDEV_CLEAR_F     clear;
    FBDEV_RESTORE_F   restore;

    void              *pData;
};

/* -------------------------------------------------------------------------------------------- */
/*                                      PUBLIC FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

FBDEV_ERROR_E FbDev_Init  (FBDEV_S **obj);
FBDEV_ERROR_E FbDev_UnInit(FBDEV_S **obj);

#ifdef __cplusplus
}
#endif

#endif //__FBDEV_H__
