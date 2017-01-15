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
* \file   Drawer.h
* \author Boubacar DIENE
*/

#ifndef __DRAWER_H__
#define __DRAWER_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include "utils/Common.h"
#include "graphics/GfxCommon.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                            */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

typedef enum   DRAWER_ERROR_E   DRAWER_ERROR_E;

typedef struct DRAWER_S         DRAWER_S;

typedef DRAWER_ERROR_E (*DRAWER_INIT_SCREEN_F  )(DRAWER_S *obj, GFX_SCREEN_S *screenParams);
typedef DRAWER_ERROR_E (*DRAWER_UNINIT_SCREEN_F)(DRAWER_S *obj);

typedef DRAWER_ERROR_E (*DRAWER_DRAW_VIDEO_F)(DRAWER_S *obj, GFX_RECT_S *rect, BUFFER_S *buffer);
typedef DRAWER_ERROR_E (*DRAWER_DRAW_IMAGE_F)(DRAWER_S *obj, GFX_RECT_S *rect, GFX_IMAGE_S *image);
typedef DRAWER_ERROR_E (*DRAWER_DRAW_TEXT_F )(DRAWER_S *obj, GFX_RECT_S *rect, GFX_TEXT_S *text);

typedef DRAWER_ERROR_E (*DRAWER_SET_BGCOLOR_F)(DRAWER_S *obj, GFX_RECT_S *rect, GFX_COLOR_S *color);

typedef DRAWER_ERROR_E (*DRAWER_SAVE_BUFFER_F)(DRAWER_S *obj, BUFFER_S *buffer, GFX_IMAGE_S *inOut);

typedef DRAWER_ERROR_E (*DRAWER_GET_EVENT_F)(DRAWER_S *obj, GFX_EVENT_S *gfxEvent);

enum DRAWER_ERROR_E {
    DRAWER_ERROR_NONE,
    DRAWER_ERROR_INIT,
    DRAWER_ERROR_UNINIT,
    DRAWER_ERROR_PARAMS,
    DRAWER_ERROR_EVENT,
    DRAWER_ERROR_DRAW,
    DRAWER_ERROR_LOCK,
    DRAWER_ERROR_SAVE
};

struct DRAWER_S {
    DRAWER_INIT_SCREEN_F   initScreen;
    DRAWER_UNINIT_SCREEN_F unInitScreen;
    
    DRAWER_DRAW_VIDEO_F    drawVideo;
    DRAWER_DRAW_IMAGE_F    drawImage;
    DRAWER_DRAW_TEXT_F     drawText;
    
    DRAWER_SET_BGCOLOR_F   setBgColor;
    
    DRAWER_SAVE_BUFFER_F   saveBuffer;
    
    DRAWER_GET_EVENT_F     getEvent;
    
    void                   *pData;
};

/* -------------------------------------------------------------------------------------------- */
/*                                      PUBLIC FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

DRAWER_ERROR_E Drawer_Init  (DRAWER_S **obj);
DRAWER_ERROR_E Drawer_UnInit(DRAWER_S **obj);

#ifdef __cplusplus
}
#endif

#endif //__DRAWER_H__
