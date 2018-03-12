//////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                              //
//              Copyright © 2016, 2018 Boubacar DIENE                                           //
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
* \file Handlers.h
* \author Boubacar DIENE
*/

#ifndef __HANDLERS_H__
#define __HANDLERS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// HEADERS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#include "core/Common.h"

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// MACROS ////////////////////////////////////////// */
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
#define HANDLERS_COMMAND_SEND_GFX_EVENT     "sendGfxEvent"

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////// TYPES DECLARATION ///////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum handlers_error_e;

struct handlers_commands_s;
struct handlers_id_s;
struct handlers_params_s;
struct handlers_s;

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////////// CALLBACKS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

typedef void (*handlers_on_module_state_changed_cb)(void *userData, char *name,
                                                    enum module_state_e state);

/* -------------------------------------------------------------------------------------------- */
/* ///////////////////////////////////// PUBLIC FUNCTIONS ///////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

typedef enum handlers_error_e (*handlers_command_f)(struct handlers_s *obj, char *gfxElementName,
                                                    void *gfxElementData, char *handlerData);

typedef enum handlers_error_e (*handlers_get_command_handler_f)(struct handlers_s *obj,
                                                                const char *handlerName,
                                                                handlers_command_f *out);
typedef enum handlers_error_e (*handlers_get_element_index_f)(struct handlers_s *obj,
                                                              char *elementName, uint32_t *index);
typedef enum handlers_error_e (*handlers_get_substring_f)(struct handlers_s *obj,
                                                          const char * const haystack,
                                                          const char * const needle,
                                                          char *out, uint32_t *offset);

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// TYPES /////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum handlers_error_e {
    HANDLERS_ERROR_NONE,
    HANDLERS_ERROR_INIT,
    HANDLERS_ERROR_UNINIT,
    HANDLERS_ERROR_PARAMS,
    HANDLERS_ERROR_STATE,
    HANDLERS_ERROR_COMMAND,
    HANDLERS_ERROR_IO
};

struct handlers_commands_s {
    char               *name;
    char               *data;
    handlers_command_f fct;
};

struct handlers_id_s {
    char *name;
    char *data;
};

struct handlers_params_s {
    struct context_s                    *ctx;
    handlers_on_module_state_changed_cb onModuleStateChangedCb;
    void                                *userData;
};

struct handlers_private_data_s {
    struct handlers_params_s   handlersParams;

    uint32_t                   nbSingleInputHandlers;
    struct handlers_commands_s *singleInputHandlers;

    uint32_t                   nbMultiInputsHandlers;
    struct handlers_commands_s *multiInputsHandlers;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// MAIN CONTEXT /////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

struct handlers_s {
    handlers_get_command_handler_f getCommandHandler;
    handlers_get_element_index_f   getElementIndex;
    handlers_get_substring_f       getSubstring;

    void *pData;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// INITIALIZER //////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum handlers_error_e Handlers_Init(struct handlers_s **obj,
                                    struct handlers_params_s *handlersParams);
enum handlers_error_e Handlers_UnInit(struct handlers_s **obj);

#ifdef __cplusplus
}
#endif

#endif //__HANDLERS_H__
