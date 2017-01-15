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
* \file   Listeners.h
* \author Boubacar DIENE
*/

#ifndef __LISTENERS_H__
#define __LISTENERS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include "specific/Common.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                            */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

typedef enum   LISTENERS_ERROR_E         LISTENERS_ERROR_E;

typedef struct LISTENERS_GETTERS_S       LISTENERS_GETTERS_S;
typedef struct LISTENERS_TEXT_IDS_S      LISTENERS_TEXT_IDS_S;
typedef struct LISTENERS_IMAGE_IDS_S     LISTENERS_IMAGE_IDS_S;
typedef struct LISTENERS_ELEMENT_PDATA_S LISTENERS_ELEMENT_PDATA_S;
typedef struct LISTENERS_PDATA_S         LISTENERS_PDATA_S;
typedef struct LISTENERS_S               LISTENERS_S;

typedef void (*LISTENERS_CLICK_HANDLER_F)(CONTEXT_S *ctx, char *gfxElementName, LISTENERS_ELEMENT_PDATA_S *pData);

typedef void (*LISTENERS_GET_STRING_F  )(void *userData, uint32_t stringId, char *language, char *strOut);
typedef void (*LISTENERS_GET_COLOR_F   )(void *userData, int32_t colorId, GFX_COLOR_S *colorOut);
typedef void (*LISTENERS_GET_FONT_F    )(void *userData, uint32_t fontId, char *ttfFontOut);
typedef void (*LISTENERS_GET_IMAGE_F   )(void *userData, uint32_t imageId, GFX_IMAGE_S *imageOut);
typedef void (*LISTENERS_GET_LANGUAGE_F)(void *userData, char *currentIn, char *nextOut);

typedef LISTENERS_ERROR_E (*LISTENERS_SET_GRAPHICS_LISTENERS_F  )(LISTENERS_S *obj);
typedef LISTENERS_ERROR_E (*LISTENERS_UNSET_GRAPHICS_LISTENERS_F)(LISTENERS_S *obj);

typedef LISTENERS_ERROR_E (*LISTENERS_SET_VIDEO_LISTENERS_F  )(LISTENERS_S *obj);
typedef LISTENERS_ERROR_E (*LISTENERS_UNSET_VIDEO_LISTENERS_F)(LISTENERS_S *obj);

typedef LISTENERS_ERROR_E (*LISTENERS_SET_SERVERS_LISTENERS_F  )(LISTENERS_S *obj);
typedef LISTENERS_ERROR_E (*LISTENERS_UNSET_SERVERS_LISTENERS_F)(LISTENERS_S *obj);

typedef LISTENERS_ERROR_E (*LISTENERS_SET_CLIENTS_LISTENERS_F  )(LISTENERS_S *obj);
typedef LISTENERS_ERROR_E (*LISTENERS_UNSET_CLIENTS_LISTENERS_F)(LISTENERS_S *obj);

enum LISTENERS_ERROR_E {
    LISTENERS_ERROR_NONE,
    LISTENERS_ERROR_INIT,
    LISTENERS_ERROR_UNINIT,
    LISTENERS_ERROR_LISTENER
};

struct LISTENERS_GETTERS_S {
    LISTENERS_GET_STRING_F   getString;
    LISTENERS_GET_COLOR_F    getColor;
    LISTENERS_GET_FONT_F     getFont;
    LISTENERS_GET_IMAGE_F    getImage;
    LISTENERS_GET_LANGUAGE_F getLanguage;
    
    void                     *userData;
};

struct LISTENERS_TEXT_IDS_S {
    uint32_t stringId;
    uint32_t fontId;
    uint32_t colorId;
};

struct LISTENERS_IMAGE_IDS_S {
    uint32_t imageId;
    int32_t  hiddenColorId;
};

struct LISTENERS_ELEMENT_PDATA_S {
    uint32_t                  index;
    
    union {
        LISTENERS_TEXT_IDS_S  text;
        LISTENERS_IMAGE_IDS_S image;
    } ids;
    
    char                      clickHandlerName[MAX_NAME_SIZE];
    LISTENERS_CLICK_HANDLER_F clickHandler;
    
    LISTENERS_GETTERS_S       getters;
};

struct LISTENERS_PDATA_S {
    CONTEXT_S  *ctx;
    BUFFER_S   buffer;
};

struct LISTENERS_S {
    LISTENERS_SET_GRAPHICS_LISTENERS_F   setGraphicsListeners;
    LISTENERS_UNSET_GRAPHICS_LISTENERS_F unsetGraphicsListeners;
    
    LISTENERS_SET_VIDEO_LISTENERS_F      setVideoListeners;
    LISTENERS_UNSET_VIDEO_LISTENERS_F    unsetVideoListeners;
    
    LISTENERS_SET_SERVERS_LISTENERS_F    setServersListeners;
    LISTENERS_UNSET_SERVERS_LISTENERS_F  unsetServersListeners;
    
    LISTENERS_SET_CLIENTS_LISTENERS_F    setClientsListeners;
    LISTENERS_UNSET_CLIENTS_LISTENERS_F  unsetClientsListeners;
    
    LISTENERS_PDATA_S                    *pData;
};

/* -------------------------------------------------------------------------------------------- */
/*                                      PUBLIC FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

LISTENERS_ERROR_E Listeners_Init  (LISTENERS_S **obj, CONTEXT_S *ctx);
LISTENERS_ERROR_E Listeners_UnInit(LISTENERS_S **obj);

#ifdef __cplusplus
}
#endif

#endif //__LISTENERS_H__
