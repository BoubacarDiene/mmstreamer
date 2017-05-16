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

#define HANDLERS_COMMAND_CLOSE_APPLICATION  "closeApplication"
#define HANDLERS_COMMAND_CHANGE_LANGUAGE    "changeLanguage"
#define HANDLERS_COMMAND_SAVE_VIDEO_ELEMENT "saveVideoElement"
#define HANDLERS_COMMAND_TAKE_SCREENSHOT    "takeScreenshot"
#define HANDLERS_COMMAND_HIDE_ELEMENT       "hideElement"
#define HANDLERS_COMMAND_SHOW_ELEMENT       "showElement"
#define HANDLERS_COMMAND_SET_FOCUS          "setFocus"
#define HANDLERS_COMMAND_HIDE_GROUP         "hideGroup"
#define HANDLERS_COMMAND_SHOW_GROUP         "showGroup"
#define HANDLERS_COMMAND_SET_CLICKABLE      "setClickable"
#define HANDLERS_COMMAND_SET_NOT_CLICKABLE  "setNotClickable"
#define HANDLERS_COMMAND_STOP_GRAPHICS      "stopGraphics"
#define HANDLERS_COMMAND_START_GRAPHICS     "startGraphics"
#define HANDLERS_COMMAND_STOP_VIDEO         "stopVideo"
#define HANDLERS_COMMAND_START_VIDEO        "startVideo"
#define HANDLERS_COMMAND_STOP_SERVER        "stopServer"
#define HANDLERS_COMMAND_START_SERVER       "startServer"
#define HANDLERS_COMMAND_SUSPEND_SERVER     "suspendServer"
#define HANDLERS_COMMAND_RESUME_SERVER      "resumeServer"
#define HANDLERS_COMMAND_STOP_CLIENT        "stopClient"
#define HANDLERS_COMMAND_START_CLIENT       "startClient"
#define HANDLERS_COMMAND_MULTI_INPUTS       "multiInputs"
#define HANDLERS_COMMAND_UPDATE_TEXT        "updateText"
#define HANDLERS_COMMAND_UPDATE_IMAGE       "updateImage"
#define HANDLERS_COMMAND_UPDATE_NAV         "updateNav"

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEFS                                           */
/* -------------------------------------------------------------------------------------------- */

typedef enum   HANDLERS_ERROR_E        HANDLERS_ERROR_E;

typedef struct COMMAND_HANDLERS_S      COMMAND_HANDLERS_S;
typedef struct HANDLERS_ID_S           HANDLERS_ID_S;
typedef struct HANDLERS_PRIVATE_DATA_S HANDLERS_PRIVATE_DATA_S;
typedef struct HANDLERS_S              HANDLERS_S;

typedef HANDLERS_ERROR_E (*COMMAND_HANDLER_F)(HANDLERS_S *obj, char *gfxElementName, void *gfxElementData, char *handlerData);

typedef HANDLERS_ERROR_E (*HANDLERS_GET_COMMAND_HANDLER_F)(HANDLERS_S *obj, const char *handlerName, COMMAND_HANDLER_F *out);
typedef HANDLERS_ERROR_E (*HANDLERS_GET_ELEMENT_INDEX_F)(HANDLERS_S *obj, char *elementName, uint32_t *index);
typedef HANDLERS_ERROR_E (*HANDLERS_GET_SUBSTRING_F    )(HANDLERS_S *obj, const char *haystack, const char *needle, char *out, uint32_t *offset);

enum HANDLERS_ERROR_E {
    HANDLERS_ERROR_NONE,
    HANDLERS_ERROR_INIT,
    HANDLERS_ERROR_UNINIT,
    HANDLERS_ERROR_PARAMS,
    HANDLERS_ERROR_STATE,
    HANDLERS_ERROR_COMMAND,
    HANDLERS_ERROR_IO
};

struct COMMAND_HANDLERS_S {
    char              *name;
    char              *data;
    COMMAND_HANDLER_F fct;
};

struct HANDLERS_ID_S {
    char *name;
    char *data;
};

struct HANDLERS_PRIVATE_DATA_S {
    CONTEXT_S          *ctx;

    uint32_t           nbSingleInputHandlers;
    COMMAND_HANDLERS_S *singleInputHandlers;

    uint32_t           nbMultiInputsHandlers;
    COMMAND_HANDLERS_S *multiInputsHandlers;
};

struct HANDLERS_S {
    HANDLERS_GET_COMMAND_HANDLER_F getCommandHandler;
    HANDLERS_GET_ELEMENT_INDEX_F   getElementIndex;
    HANDLERS_GET_SUBSTRING_F       getSubstring;

    void                           *pData;
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
