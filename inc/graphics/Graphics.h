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
* \file   Graphics.h
* \author Boubacar DIENE
*/

#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

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

typedef enum   GRAPHICS_ERROR_E  GRAPHICS_ERROR_E;

typedef struct GRAPHICS_PARAMS_S GRAPHICS_PARAMS_S;
typedef struct GRAPHICS_S        GRAPHICS_S;

typedef void (*ON_GFX_EVENT_CB)(GFX_EVENT_S *gfxEvent, void *userData);

typedef GRAPHICS_ERROR_E (*GRAPHICS_CREATE_DRAWER_F )(GRAPHICS_S *obj, GRAPHICS_PARAMS_S *params);
typedef GRAPHICS_ERROR_E (*GRAPHICS_DESTROY_DRAWER_F)(GRAPHICS_S *obj);

typedef GRAPHICS_ERROR_E (*GRAPHICS_CREATE_ELEMENT_F)(GRAPHICS_S *obj, GFX_ELEMENT_S **newGfxElement);
typedef GRAPHICS_ERROR_E (*GRAPHICS_PUSH_ELEMENT_F  )(GRAPHICS_S *obj, GFX_ELEMENT_S *gfxElement);
typedef GRAPHICS_ERROR_E (*GRAPHICS_REMOVE_ELEMENT_F)(GRAPHICS_S *obj, char *gfxElementName);
typedef GRAPHICS_ERROR_E (*GRAPHICS_REMOVE_ALL_F    )(GRAPHICS_S *obj);

typedef GRAPHICS_ERROR_E (*GRAPHICS_SET_VISIBLE_F  )(GRAPHICS_S *obj, char *gfxElementName, uint8_t isVisible);
typedef GRAPHICS_ERROR_E (*GRAPHICS_SET_FOCUS_F    )(GRAPHICS_S *obj, char *gfxElementName);
typedef GRAPHICS_ERROR_E (*GRAPHICS_SET_CLICKABLE_F)(GRAPHICS_S *obj, char *gfxElementName, uint8_t isClickable);
typedef GRAPHICS_ERROR_E (*GRAPHICS_SET_DATA_F     )(GRAPHICS_S *obj, char *gfxElementName, void *data);

typedef GRAPHICS_ERROR_E (*GRAPHICS_CREATE_IMAGE_F)(GRAPHICS_S *obj, BUFFER_S *buffer, GFX_IMAGE_S *inOut);

typedef GRAPHICS_ERROR_E (*GRAPHICS_DRAW_ALL_ELEMENTS_F)(GRAPHICS_S *obj);

typedef GRAPHICS_ERROR_E (*GRAPHICS_HANDLE_EVENTS_F)(GRAPHICS_S *obj);

typedef GRAPHICS_ERROR_E (*GRAPHICS_QUIT_F)(GRAPHICS_S *obj);

enum GRAPHICS_ERROR_E {
    GRAPHICS_ERROR_NONE,
    GRAPHICS_ERROR_INIT,
    GRAPHICS_ERROR_UNINIT,
    GRAPHICS_ERROR_LOCK,
    GRAPHICS_ERROR_LIST,
    GRAPHICS_ERROR_DRAWER
};

struct GRAPHICS_PARAMS_S {
    GFX_SCREEN_S    screenParams;
    
    GFX_COLOR_S     colorOnFocus;
    GFX_COLOR_S     colorOnBlur;
    GFX_COLOR_S     colorOnReset;
    
    ON_GFX_EVENT_CB onGfxEventCb;
    
    void            *userData;
};

struct GRAPHICS_S {
    GRAPHICS_CREATE_DRAWER_F     createDrawer;
    GRAPHICS_DESTROY_DRAWER_F    destroyDrawer;
    
    GRAPHICS_CREATE_ELEMENT_F    createElement;
    GRAPHICS_PUSH_ELEMENT_F      pushElement;
    GRAPHICS_REMOVE_ELEMENT_F    removeElement;
    GRAPHICS_REMOVE_ALL_F        removeAll;
    
    GRAPHICS_SET_VISIBLE_F       setVisible;
    GRAPHICS_SET_FOCUS_F         setFocus;
    GRAPHICS_SET_CLICKABLE_F     setClickable;
    GRAPHICS_SET_DATA_F          setData;
    
    GRAPHICS_CREATE_IMAGE_F      createImage;
    
    GRAPHICS_DRAW_ALL_ELEMENTS_F drawAllElements;
    
    GRAPHICS_HANDLE_EVENTS_F     handleEvents;
    
    GRAPHICS_QUIT_F              quit;
    
    void                         *pData;
};

/* -------------------------------------------------------------------------------------------- */
/*                                      PUBLIC FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

GRAPHICS_ERROR_E Graphics_Init  (GRAPHICS_S **obj);
GRAPHICS_ERROR_E Graphics_UnInit(GRAPHICS_S **obj);

#ifdef __cplusplus
}
#endif

#endif //__GRAPHICS_H__
