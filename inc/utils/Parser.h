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
* \file Parser.h
* \author Boubacar DIENE
*/

#ifndef __PARSER_H__
#define __PARSER_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// HEADERS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#include "utils/Common.h"

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////// TYPES DECLARATION ///////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum parser_error_e;
enum parser_encoding_e;
enum parser_attr_type_e;

struct parser_tags_handler_s;
struct parser_attr_handler_s;
struct parser_params_s;
struct parser_s;

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////////// CALLBACKS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

typedef void (*parser_on_start_cb)(void *userData, const char **attrs);
typedef void (*parser_on_end_cb)(void *userData);
typedef void (*parser_on_data_cb)(void *userData, const char *value, int32_t len);
typedef void (*parser_on_error_cb)(void *userData, int32_t errorCode, const char *errorStr);

/* -------------------------------------------------------------------------------------------- */
/* ///////////////////////////////////// PUBLIC FUNCTIONS ///////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

typedef enum parser_error_e (*parser_parse_f)(struct parser_s *obj,
                                              struct parser_params_s *params);
typedef enum parser_error_e (*parser_get_attributes_f)(struct parser_s *obj,
                                                       struct parser_attr_handler_s *attrHandlers,
                                                       const char **attrs);
typedef enum parser_error_e (*parser_get_scalar_f)(struct parser_s *obj, void *attrValueOut,
                                                   const char *attrValueIn);
typedef enum parser_error_e (*parser_get_vector_f)(struct parser_s *obj, void **attrValueOut,
                                                   const char *attrValueIn);

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// TYPES /////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum parser_error_e {
    PARSER_ERROR_NONE,
    PARSER_ERROR_INIT,
    PARSER_ERROR_UNINIT,
    PARSER_ERROR_PARSE,
    PARSER_ERROR_ATTR,
    PARSER_ERROR_FILE
};

enum parser_encoding_e {
    PARSER_ENCODING_US_ASCII,
    PARSER_ENCODING_UTF_8,
    PARSER_ENCODING_UTF_16,
    PARSER_ENCODING_ISO_8859_1
};

enum parser_attr_type_e {
    PARSER_ATTR_TYPE_NONE,
    PARSER_ATTR_TYPE_SCALAR,
    PARSER_ATTR_TYPE_VECTOR
};

struct parser_tags_handler_s {
    char *tagName;
    
    parser_on_start_cb onStartCb;
    parser_on_end_cb   onEndCb;
    parser_on_data_cb  onDataCb;
};

struct parser_attr_handler_s {
    char                    *attrName;
    enum parser_attr_type_e attrType;
    
    union {
        void                *scalar;
        void                **vector;
    } attrValue;
    
    union {
        parser_get_scalar_f scalar;
        parser_get_vector_f vector;
    } attrGetter;
};

struct parser_params_s {
    char                         path[MAX_PATH_SIZE];
    enum parser_encoding_e       encoding;
    struct parser_tags_handler_s *tagsHandlers;

    parser_on_error_cb           onErrorCb;
    
    void                         *userData;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// MAIN CONTEXT /////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

struct parser_s {
    parser_parse_f          parse;
    parser_get_attributes_f getAttributes;
    
    parser_get_vector_f     getString;

    parser_get_scalar_f     getInt8;
    parser_get_scalar_f     getUint8;

    parser_get_scalar_f     getInt16;
    parser_get_scalar_f     getUint16;

    parser_get_scalar_f     getInt32;
    parser_get_scalar_f     getUint32;

    parser_get_scalar_f     getInt64;
    parser_get_scalar_f     getUint64;
    
    void *pData;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// INITIALIZER //////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum parser_error_e Parser_Init(struct parser_s **obj);
enum parser_error_e Parser_UnInit(struct parser_s **obj);

#ifdef __cplusplus
}
#endif

#endif //__PARSER_H__
