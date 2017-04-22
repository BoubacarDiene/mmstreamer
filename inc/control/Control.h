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
* \file   Control.h
* \author Boubacar DIENE
*/

#ifndef __CONTROL_H__
#define __CONTROL_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include "control/Handlers.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                             */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEFS                                           */
/* -------------------------------------------------------------------------------------------- */

typedef enum   CONTROL_ERROR_E          CONTROL_ERROR_E;

typedef struct CONTROL_GETTERS_S        CONTROL_GETTERS_S;
typedef struct CONTROL_TEXT_IDS_S       CONTROL_TEXT_IDS_S;
typedef struct CONTROL_IMAGE_IDS_S      CONTROL_IMAGE_IDS_S;
typedef struct CONTROL_ELEMENT_DATA_S   CONTROL_ELEMENT_DATA_S;
typedef struct CONTROL_S                CONTROL_S;

typedef void (*CONTROL_CLICK_HANDLER_F)(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData);

typedef void (*CONTROL_GET_STRING_F  )(void *userData, uint32_t stringId, char *language, char *strOut);
typedef void (*CONTROL_GET_COLOR_F   )(void *userData, int32_t colorId, GFX_COLOR_S *colorOut);
typedef void (*CONTROL_GET_FONT_F    )(void *userData, uint32_t fontId, char *ttfFontOut);
typedef void (*CONTROL_GET_IMAGE_F   )(void *userData, uint32_t imageId, GFX_IMAGE_S *imageOut);
typedef void (*CONTROL_GET_LANGUAGE_F)(void *userData, char *currentIn, char *nextOut);

typedef CONTROL_ERROR_E (*CONTROL_INIT_ELEMENT_DATA_F  )(CONTROL_S *obj, void **data);
typedef CONTROL_ERROR_E (*CONTROL_UNINIT_ELEMENT_DATA_F)(CONTROL_S *obj, void **data);

typedef CONTROL_ERROR_E (*CONTROL_SET_ELEMENT_GETTERS_F  )(CONTROL_S *obj, void *data, CONTROL_GETTERS_S *getters);
typedef CONTROL_ERROR_E (*CONTROL_UNSET_ELEMENT_GETTERS_F)(CONTROL_S *obj, void *data);

typedef CONTROL_ERROR_E (*CONTROL_SET_ELEMENT_TEXT_IDS_F  )(CONTROL_S *obj, void *data, CONTROL_TEXT_IDS_S *textIds);
typedef CONTROL_ERROR_E (*CONTROL_UNSET_ELEMENT_TEXT_IDS_F)(CONTROL_S *obj, void *data);

typedef CONTROL_ERROR_E (*CONTROL_SET_ELEMENT_IMAGE_IDS_F  )(CONTROL_S *obj, void *data, CONTROL_IMAGE_IDS_S *imageIds);
typedef CONTROL_ERROR_E (*CONTROL_UNSET_ELEMENT_IMAGE_IDS_F)(CONTROL_S *obj, void *data);

typedef CONTROL_ERROR_E (*CONTROL_SET_CLICK_HANDLERS_F  )(CONTROL_S *obj, void *data, HANDLERS_ID_S *handlers, uint32_t nbHandlers, uint32_t index);
typedef CONTROL_ERROR_E (*CONTROL_UNSET_CLICK_HANDLERS_F)(CONTROL_S *obj, void *data);

typedef CONTROL_ERROR_E (*CONTROL_HANDLE_CLICK_F)(CONTROL_S *obj, GFX_EVENT_S *gfxEvent);

enum CONTROL_ERROR_E {
    CONTROL_ERROR_NONE,
    CONTROL_ERROR_INIT,
    CONTROL_ERROR_UNINIT,
    CONTROL_ERROR_PARAMS
};

struct CONTROL_GETTERS_S {
    CONTROL_GET_STRING_F   getString;
    CONTROL_GET_COLOR_F    getColor;
    CONTROL_GET_FONT_F     getFont;
    CONTROL_GET_IMAGE_F    getImage;
    CONTROL_GET_LANGUAGE_F getLanguage;
    
    void                   *userData;
};

struct CONTROL_TEXT_IDS_S {
    uint32_t stringId;
    uint32_t fontId;
    uint32_t colorId;
};

struct CONTROL_IMAGE_IDS_S {
    uint32_t imageId;
    int32_t  hiddenColorId;
};

struct CONTROL_ELEMENT_DATA_S {
    uint32_t                index;

    union {
        CONTROL_TEXT_IDS_S  text;
        CONTROL_IMAGE_IDS_S image;
    } ids;

    uint32_t                nbClickHandlers;
    CLICK_HANDLERS_S        *clickHandlers;

    CONTROL_GETTERS_S       getters;
};

struct CONTROL_S {
    CONTROL_INIT_ELEMENT_DATA_F       initElementData;
    CONTROL_UNINIT_ELEMENT_DATA_F     uninitElementData;

    CONTROL_SET_ELEMENT_GETTERS_F     setElementGetters;
    CONTROL_UNSET_ELEMENT_GETTERS_F   unsetElementGetters;

    CONTROL_SET_ELEMENT_TEXT_IDS_F    setElementTextIds;
    CONTROL_UNSET_ELEMENT_TEXT_IDS_F  unsetElementTextIds;

    CONTROL_SET_ELEMENT_IMAGE_IDS_F   setElementImageIds;
    CONTROL_UNSET_ELEMENT_IMAGE_IDS_F unsetElementImageIds;

    CONTROL_SET_CLICK_HANDLERS_F      setClickHandlers;
    CONTROL_UNSET_CLICK_HANDLERS_F    unsetClickHandlers;

    CONTROL_HANDLE_CLICK_F            handleClick;

    void                              *pData;
};

/* -------------------------------------------------------------------------------------------- */
/*                                           VARIABLES                                          */
/* -------------------------------------------------------------------------------------------- */

CONTROL_ERROR_E Control_Init  (CONTROL_S **obj, CONTEXT_S *ctx);
CONTROL_ERROR_E Control_UnInit(CONTROL_S **obj);

#ifdef __cplusplus
}
#endif

#endif //__CONTROL_H__
