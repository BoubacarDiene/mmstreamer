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
* \file Control.h
* \author Boubacar DIENE
*/

#ifndef __CONTROL_H__
#define __CONTROL_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// HEADERS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#include "control/Controllers.h"
#include "control/Handlers.h"

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////// TYPES DECLARATION ///////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum control_error_e;

struct control_getters_s;
struct control_text_ids_s;
struct control_image_ids_s;
struct control_element_data_s;
struct control_s;

/* -------------------------------------------------------------------------------------------- */
/* ///////////////////////////////////// PUBLIC FUNCTIONS ///////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

typedef void (*control_get_string_f)(void *userData, uint32_t stringId, char *language,
                                     char *strOut);
typedef void (*control_get_color_f)(void *userData, int32_t colorId,
                                    struct gfx_color_s *colorOut);
typedef void (*control_get_font_f)(void *userData, uint32_t fontId, char *ttfFontOut);
typedef void (*control_get_image_f)(void *userData, uint32_t imageId,
                                    struct gfx_image_s *imageOut);
typedef void (*control_get_language_f)(void *userData, char *currentIn, char *nextOut);

typedef enum control_error_e (*control_init_element_data_f)(struct control_s *obj, void **data);
typedef enum control_error_e (*control_uninit_element_data_f)(struct control_s *obj, void **data);

typedef enum control_error_e (*control_set_element_getters_f)(struct control_s *obj, void *data,
                                                              struct control_getters_s *getters);
typedef enum control_error_e (*control_unset_element_getters_f)(struct control_s *obj, void *data);

typedef enum control_error_e (*control_set_element_text_ids_f)(struct control_s *obj, void *data,
                                                               struct control_text_ids_s *textIds);
typedef enum control_error_e (*control_unset_element_text_ids_f)(struct control_s *obj,
                                                                 void *data);

typedef enum control_error_e (*control_set_element_image_ids_f)(struct control_s *obj, void *data,
                                                            struct control_image_ids_s *imageIds);
typedef enum control_error_e (*control_unset_element_image_ids_f)(struct control_s *obj,
                                                                  void *data);

typedef enum control_error_e (*control_set_command_handlers_f)(struct control_s *obj, void *data,
                                                               struct handlers_id_s *handlers,
                                                               uint32_t nbHandlers,
                                                               uint32_t index);
typedef enum control_error_e (*control_unset_command_handlers_f)(struct control_s *obj,
                                                                 void *data);

typedef enum control_error_e (*control_load_controllers_f)(struct control_s *obj);
typedef enum control_error_e (*control_unload_controllers_f)(struct control_s *obj);

typedef enum control_error_e (*control_handle_click_f)(struct control_s *obj,
                                                       struct gfx_event_s *gfxEvent);
typedef enum control_error_e (*control_handle_command_f)(struct control_s *obj,
                                                         struct controllers_command_s *command);

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// TYPES /////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum control_error_e {
    CONTROL_ERROR_NONE,
    CONTROL_ERROR_INIT,
    CONTROL_ERROR_UNINIT,
    CONTROL_ERROR_PARAMS,
    CONTROL_ERROR_LOCK,
    CONTROL_ERROR_UNKNOWN
};

struct control_getters_s {
    control_get_string_f   getString;
    control_get_color_f    getColor;
    control_get_font_f     getFont;
    control_get_image_f    getImage;
    control_get_language_f getLanguage;
    
    void *userData;
};

struct control_text_ids_s {
    uint32_t stringId;
    uint32_t fontId;
    uint32_t colorId;
};

struct control_image_ids_s {
    uint32_t imageId;
    int32_t  hiddenColorId;
};

struct control_element_data_s {
    uint32_t                       index;

    union {
        struct control_text_ids_s  text;
        struct control_image_ids_s image;
    } ids;

    uint32_t                       nbCommandHandlers;
    struct handlers_commands_s     *commandHandlers;

    struct control_getters_s       getters;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// MAIN CONTEXT /////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

struct control_s {
    control_init_element_data_f       initElementData;
    control_uninit_element_data_f     uninitElementData;

    control_set_element_getters_f     setElementGetters;
    control_unset_element_getters_f   unsetElementGetters;

    control_set_element_text_ids_f    setElementTextIds;
    control_unset_element_text_ids_f  unsetElementTextIds;

    control_set_element_image_ids_f   setElementImageIds;
    control_unset_element_image_ids_f unsetElementImageIds;

    control_set_command_handlers_f    setCommandHandlers;
    control_unset_command_handlers_f  unsetCommandHandlers;

    control_load_controllers_f        loadControllers;
    control_unload_controllers_f      unloadControllers;

    control_handle_click_f            handleClick;
    control_handle_command_f          handleCommand;

    void *pData;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// INITIALIZER //////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum control_error_e Control_Init(struct control_s **obj, struct context_s *ctx);
enum control_error_e Control_UnInit(struct control_s **obj);

#ifdef __cplusplus
}
#endif

#endif //__CONTROL_H__
