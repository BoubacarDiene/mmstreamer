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
* \file   Handlers.h
* \author Boubacar DIENE
*/

#ifndef __HANDLERS_H__
#define __HANDLERS_H__

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

typedef enum   HANDLERS_ERROR_E        HANDLERS_ERROR_E;

typedef struct CLICK_HANDLERS_S        CLICK_HANDLERS_S;
typedef struct HANDLERS_ID_S           HANDLERS_ID_S;
typedef struct HANDLERS_PRIVATE_DATA_S HANDLERS_PRIVATE_DATA_S;
typedef struct HANDLERS_S              HANDLERS_S;

typedef void (*CLICK_HANDLER_F)(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData);

typedef HANDLERS_ERROR_E (*HANDLERS_GET_CLICK_HANDLER_F)(HANDLERS_S *obj, const char *handlerName, CLICK_HANDLER_F *out);
typedef HANDLERS_ERROR_E (*HANDLERS_GET_ELEMENT_INDEX_F)(HANDLERS_S *obj, char *elementName, uint32_t *index);
typedef HANDLERS_ERROR_E (*HANDLERS_GET_SUBSTRING_F    )(HANDLERS_S *obj, const char *haystack, const char *needle, char *out, uint32_t *offset);

enum HANDLERS_ERROR_E {
    HANDLERS_ERROR_NONE,
    HANDLERS_ERROR_INIT,
    HANDLERS_ERROR_UNINIT,
    HANDLERS_ERROR_PARAMS
};

struct CLICK_HANDLERS_S {
    char            *name;
    char            *data;
    CLICK_HANDLER_F fct;
};

struct HANDLERS_ID_S {
    char *name;
    char *data;
};

struct HANDLERS_PRIVATE_DATA_S {
    CONTEXT_S        *ctx;

    uint32_t         nbSingleInputClickHandlers;
    CLICK_HANDLERS_S *singleInputClickHandlers;

    uint32_t         nbMultiInputsClickHandlers;
    CLICK_HANDLERS_S *multiInputsClickHandlers;
};

struct HANDLERS_S {
    HANDLERS_GET_CLICK_HANDLER_F getClickHandler;
    HANDLERS_GET_ELEMENT_INDEX_F getElementIndex;
    HANDLERS_GET_SUBSTRING_F     getSubstring;

    void                         *pData;
};

/* -------------------------------------------------------------------------------------------- */
/*                                           VARIABLES                                          */
/* -------------------------------------------------------------------------------------------- */

HANDLERS_ERROR_E Handlers_Init  (HANDLERS_S **obj, CONTEXT_S *ctx);
HANDLERS_ERROR_E Handlers_UnInit(HANDLERS_S **obj);

#ifdef __cplusplus
}
#endif

#endif //__HANDLERS_H__
