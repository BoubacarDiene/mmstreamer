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
* \file   Parser.h
* \author Boubacar DIENE
*/

#ifndef __PARSER_H__
#define __PARSER_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include "utils/Common.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                            */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

typedef enum PARSER_ERROR_E          PARSER_ERROR_E;
typedef enum PARSER_ENCODING_E       PARSER_ENCODING_E;
typedef enum PARSER_ATTR_TYPE_E      PARSER_ATTR_TYPE_E;

typedef struct PARSER_TAGS_HANDLER_S PARSER_TAGS_HANDLER_S;
typedef struct PARSER_ATTR_HANDLER_S PARSER_ATTR_HANDLER_S;

typedef struct PARSER_PARAMS_S       PARSER_PARAMS_S;
typedef struct PARSER_S              PARSER_S;

typedef void (*ON_PARSER_START_CB)(void *userData, const char **attrs);
typedef void (*ON_PARSER_END_CB  )(void *userData);
typedef void (*ON_PARSER_DATA_CB )(void *userData, const char *value, int32_t len);
typedef void (*ON_PARSER_ERROR_CB)(void *userData, int32_t errorCode, const char *errorStr);

typedef PARSER_ERROR_E (*PARSER_PARSE_F           )(PARSER_S *obj, PARSER_PARAMS_S *params);
typedef PARSER_ERROR_E (*PARSER_GET_ATTRIBUTES_F  )(PARSER_S *obj, PARSER_ATTR_HANDLER_S *attrHandlers, const char **attrs);

typedef PARSER_ERROR_E (*PARSER_GET_SCALAR_F)(PARSER_S *obj, void *attrValueOut, const char *attrValueIn);
typedef PARSER_ERROR_E (*PARSER_GET_VECTOR_F)(PARSER_S *obj, void **attrValueOut, const char *attrValueIn);

enum PARSER_ERROR_E {
    PARSER_ERROR_NONE,
    PARSER_ERROR_INIT,
    PARSER_ERROR_UNINIT,
    PARSER_ERROR_PARSE,
    PARSER_ERROR_ATTR,
    PARSER_ERROR_FILE
};

enum PARSER_ENCODING_E {
    PARSER_ENCODING_US_ASCII,
    PARSER_ENCODING_UTF_8,
    PARSER_ENCODING_UTF_16,
    PARSER_ENCODING_ISO_8859_1
};

enum PARSER_ATTR_TYPE_E {
    PARSER_ATTR_TYPE_NONE,
    PARSER_ATTR_TYPE_SCALAR,
    PARSER_ATTR_TYPE_VECTOR
};

struct PARSER_TAGS_HANDLER_S {
    char               *tagName;
    
    ON_PARSER_START_CB onStartCb;
    ON_PARSER_END_CB   onEndCb;
    ON_PARSER_DATA_CB  onDataCb;
};

struct PARSER_ATTR_HANDLER_S {
    char                    *attrName;
    PARSER_ATTR_TYPE_E      attrType;
    
    union {
        void                *scalar;
        void                **vector;
    } attrValue;
    
    union {
        PARSER_GET_SCALAR_F scalar;
        PARSER_GET_VECTOR_F vector;
    } attrGetter;
};

struct PARSER_PARAMS_S {
    char                  path[MAX_PATH_SIZE];
    
    PARSER_ENCODING_E     encoding;
    
    PARSER_TAGS_HANDLER_S *tagsHandlers;
    
    ON_PARSER_ERROR_CB    onErrorCb;
    
    void                  *userData;
};

struct PARSER_S {
    PARSER_PARSE_F          parse;
    PARSER_GET_ATTRIBUTES_F getAttributes;
    
    PARSER_GET_VECTOR_F     getString;

    PARSER_GET_SCALAR_F     getInt8;
    PARSER_GET_SCALAR_F     getUint8;
    
    PARSER_GET_SCALAR_F     getInt16;
    PARSER_GET_SCALAR_F     getUint16;
    
    PARSER_GET_SCALAR_F     getInt32;
    PARSER_GET_SCALAR_F     getUint32;
    
    PARSER_GET_SCALAR_F     getInt64;
    PARSER_GET_SCALAR_F     getUint64;
    
    void                    *pData;
};

/* -------------------------------------------------------------------------------------------- */
/*                                      PUBLIC FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

PARSER_ERROR_E Parser_Init  (PARSER_S **obj);
PARSER_ERROR_E Parser_UnInit(PARSER_S **obj);

#ifdef __cplusplus
}
#endif

#endif //__PARSER_H__
