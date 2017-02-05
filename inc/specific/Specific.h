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
* \file   Specific.h
* \author Boubacar DIENE
*/

#ifndef __SPECIFIC_H__
#define __SPECIFIC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include "core/Common.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                             */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEFS                                           */
/* -------------------------------------------------------------------------------------------- */

typedef enum SPECIFIC_ERROR_E            SPECIFIC_ERROR_E;

typedef struct SPECIFIC_GETTERS_S        SPECIFIC_GETTERS_S;
typedef struct SPECIFIC_TEXT_IDS_S       SPECIFIC_TEXT_IDS_S;
typedef struct SPECIFIC_IMAGE_IDS_S      SPECIFIC_IMAGE_IDS_S;
typedef struct SPECIFIC_HANDLERS_S       SPECIFIC_HANDLERS_S;
typedef struct SPECIFIC_CLICK_HANDLERS_S SPECIFIC_CLICK_HANDLERS_S;
typedef struct SPECIFIC_ELEMENT_DATA_S   SPECIFIC_ELEMENT_DATA_S;
typedef struct SPECIFIC_S                SPECIFIC_S;

typedef void (*SPECIFIC_CLICK_HANDLER_F)(CONTEXT_S *ctx, char *gfxElementName, void *gfxElementData, char *handlerData);

typedef void (*SPECIFIC_GET_STRING_F  )(void *userData, uint32_t stringId, char *language, char *strOut);
typedef void (*SPECIFIC_GET_COLOR_F   )(void *userData, int32_t colorId, GFX_COLOR_S *colorOut);
typedef void (*SPECIFIC_GET_FONT_F    )(void *userData, uint32_t fontId, char *ttfFontOut);
typedef void (*SPECIFIC_GET_IMAGE_F   )(void *userData, uint32_t imageId, GFX_IMAGE_S *imageOut);
typedef void (*SPECIFIC_GET_LANGUAGE_F)(void *userData, char *currentIn, char *nextOut);

typedef SPECIFIC_ERROR_E (*SPECIFIC_GET_VIDEO_CONFIG_F)(SPECIFIC_S *obj, VIDEO_CONFIG_S *config, uint32_t configChoice);

typedef SPECIFIC_ERROR_E (*SPECIFIC_INIT_ELEMENT_DATA_F  )(SPECIFIC_S *obj, void **data);
typedef SPECIFIC_ERROR_E (*SPECIFIC_UNINIT_ELEMENT_DATA_F)(SPECIFIC_S *obj, void **data);

typedef SPECIFIC_ERROR_E (*SPECIFIC_SET_ELEMENT_GETTERS_F  )(SPECIFIC_S *obj, void *data, SPECIFIC_GETTERS_S *getters);
typedef SPECIFIC_ERROR_E (*SPECIFIC_UNSET_ELEMENT_GETTERS_F)(SPECIFIC_S *obj, void *data);

typedef SPECIFIC_ERROR_E (*SPECIFIC_SET_ELEMENT_TEXT_IDS_F  )(SPECIFIC_S *obj, void *data, SPECIFIC_TEXT_IDS_S *textIds);
typedef SPECIFIC_ERROR_E (*SPECIFIC_UNSET_ELEMENT_TEXT_IDS_F)(SPECIFIC_S *obj, void *data);

typedef SPECIFIC_ERROR_E (*SPECIFIC_SET_ELEMENT_IMAGE_IDS_F  )(SPECIFIC_S *obj, void *data, SPECIFIC_IMAGE_IDS_S *imageIds);
typedef SPECIFIC_ERROR_E (*SPECIFIC_UNSET_ELEMENT_IMAGE_IDS_F)(SPECIFIC_S *obj, void *data);

typedef SPECIFIC_ERROR_E (*SPECIFIC_SET_CLICK_HANDLERS_F  )(SPECIFIC_S *obj, void *data, SPECIFIC_HANDLERS_S *handlers, uint32_t nbHandlers, uint32_t index);
typedef SPECIFIC_ERROR_E (*SPECIFIC_UNSET_CLICK_HANDLERS_F)(SPECIFIC_S *obj, void *data);

typedef SPECIFIC_ERROR_E (*SPECIFIC_HANDLE_CLICK_F)(CONTEXT_S *ctx, GFX_EVENT_S *gfxEvent);

enum SPECIFIC_ERROR_E {
    SPECIFIC_ERROR_NONE,
    SPECIFIC_ERROR_INIT,
    SPECIFIC_ERROR_UNINIT,
    SPECIFIC_ERROR_PARAMS
};

struct SPECIFIC_GETTERS_S {
    SPECIFIC_GET_STRING_F   getString;
    SPECIFIC_GET_COLOR_F    getColor;
    SPECIFIC_GET_FONT_F     getFont;
    SPECIFIC_GET_IMAGE_F    getImage;
    SPECIFIC_GET_LANGUAGE_F getLanguage;
    
    void                    *userData;
};

struct SPECIFIC_TEXT_IDS_S {
    uint32_t stringId;
    uint32_t fontId;
    uint32_t colorId;
};

struct SPECIFIC_IMAGE_IDS_S {
    uint32_t imageId;
    int32_t  hiddenColorId;
};

struct SPECIFIC_HANDLERS_S {
    char *name;
    char *data;
};

struct SPECIFIC_CLICK_HANDLERS_S {
    char                     *name;
    char                     *data;
    SPECIFIC_CLICK_HANDLER_F fct;
};

struct SPECIFIC_ELEMENT_DATA_S {
    uint32_t                  index;

    union {
        SPECIFIC_TEXT_IDS_S   text;
        SPECIFIC_IMAGE_IDS_S  image;
    } ids;

    uint32_t                  nbClickHandlers;
    SPECIFIC_CLICK_HANDLERS_S *clickHandlers;

    SPECIFIC_GETTERS_S        getters;
};

struct SPECIFIC_S {
    SPECIFIC_GET_VIDEO_CONFIG_F        getVideoConfig;

    SPECIFIC_INIT_ELEMENT_DATA_F       initElementData;
    SPECIFIC_UNINIT_ELEMENT_DATA_F     uninitElementData;

    SPECIFIC_SET_ELEMENT_GETTERS_F     setElementGetters;
    SPECIFIC_UNSET_ELEMENT_GETTERS_F   unsetElementGetters;

    SPECIFIC_SET_ELEMENT_TEXT_IDS_F    setElementTextIds;
    SPECIFIC_UNSET_ELEMENT_TEXT_IDS_F  unsetElementTextIds;

    SPECIFIC_SET_ELEMENT_IMAGE_IDS_F   setElementImageIds;
    SPECIFIC_UNSET_ELEMENT_IMAGE_IDS_F unsetElementImageIds;

    SPECIFIC_SET_CLICK_HANDLERS_F      setClickHandlers;
    SPECIFIC_UNSET_CLICK_HANDLERS_F    unsetClickHandlers;

    SPECIFIC_HANDLE_CLICK_F            handleClick;

    void                               *pData;
};

/* -------------------------------------------------------------------------------------------- */
/*                                           VARIABLES                                          */
/* -------------------------------------------------------------------------------------------- */

SPECIFIC_ERROR_E Specific_Init  (SPECIFIC_S **obj, CONTEXT_S *ctx);
SPECIFIC_ERROR_E Specific_UnInit(SPECIFIC_S **obj);

#ifdef __cplusplus
}
#endif

#endif //__SPECIFIC_H__
