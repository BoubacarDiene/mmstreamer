//////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                              //
//              Copyright © 2016, 2018 Boubacar DIENE                                           //
//                                                                                              //
//              This file is part of mmstreamer project.                                        //
//                                                                                              //
//              mmstreamer is free software: you can redistribute it and/or modify              //
//              it under the terms of the GNU General Public License as published by            //
//              the Free Software Foundation, either version 2 of the License, or               //
//              (at your option) any later version.                                             //
//                                                                                              //
//              mmstreamer is distributed in the hope that it will be useful,                   //
//              but WITHOUT ANY WARRANTY; without even the implied warranty of                  //
//              MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                   //
//              GNU General Public License for more details.                                    //
//                                                                                              //
//              You should have received a copy of the GNU General Public License               //
//              along with mmstreamer. If not, see <http://www.gnu.org/licenses/>               //
//              or write to the Free Software Foundation, Inc., 51 Franklin Street,             //
//              51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.                   //
//                                                                                              //
//////////////////////////////////////////////////////////////////////////////////////////////////

/*!
* \file Parser.c
* \brief xml parser based on expat library
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// HEADERS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#include "expat.h"
#include "utils/Log.h"
#include "utils/Parser.h"

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// MACROS ////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "Parser"

#define ENCODING_US_ASCII   "US-ASCII"
#define ENCODING_UTF_8      "UTF-8"
#define ENCODING_UTF_16     "UTF-16"
#define ENCODING_ISO_8859_1 "ISO-8859-1"

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// TYPES /////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

struct parser_private_data_s {
    struct parser_params_s       params;
	
    XML_Parser                   parser;
    char                         *xml;
    FILE                         *fd;
    struct stat                  st;
    
    size_t                       xmlBufferSize;
    size_t                       nbRead;
    uint8_t                      isFinal;
    
    struct parser_tags_handler_s *openedTag;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////// PUBLIC FUNCTIONS PROTOTYPES //////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

static enum parser_error_e parse_f(struct parser_s *obj, struct parser_params_s *params);
static enum parser_error_e getAttributes_f(struct parser_s *obj,
                                           struct parser_attr_handler_s *attrHandlers,
                                           const char **attrs);

static enum parser_error_e getString_f(struct parser_s *obj, void **attrValueOut,
                                       const char *attrValueIn);

static enum parser_error_e getInt8_f(struct parser_s *obj, void *attrValueOut,
                                     const char *attrValueIn);
static enum parser_error_e getUint8_f(struct parser_s *obj, void *attrValueOut,
                                      const char *attrValueIn);

static enum parser_error_e getInt16_f(struct parser_s *obj, void *attrValueOut,
                                      const char *attrValueIn);
static enum parser_error_e getUint16_f(struct parser_s *obj, void *attrValueOut,
                                       const char *attrValueIn);

static enum parser_error_e getInt32_f(struct parser_s *obj, void *attrValueOut,
                                      const char *attrValueIn);
static enum parser_error_e getUint32_f(struct parser_s *obj, void *attrValueOut,
                                       const char *attrValueIn);

static enum parser_error_e getInt64_f(struct parser_s *obj, void *attrValueOut,
                                      const char *attrValueIn);
static enum parser_error_e getUint64_f(struct parser_s *obj, void *attrValueOut,
                                       const char *attrValueIn);

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////////// CALLBACKS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

static void XMLCALL startElementCb(void *userData, const char *name, const char **attrs);
static void XMLCALL endElementCb(void *userData, const char *name);
static void XMLCALL dataCb(void *userData, const char *value, int32_t len);

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// INITIALIZER //////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
enum parser_error_e Parser_Init(struct parser_s **obj)
{
    assert(obj && (*obj = calloc(1, sizeof(struct parser_s))));
    
    struct parser_private_data_s *pData;
    assert((pData = calloc(1, sizeof(struct parser_private_data_s))));
    
    (*obj)->parse         = parse_f;
    (*obj)->getAttributes = getAttributes_f;
    
    (*obj)->getString     = getString_f;
    
    (*obj)->getInt8       = getInt8_f;
    (*obj)->getUint8      = getUint8_f;
    
    (*obj)->getInt16      = getInt16_f;
    (*obj)->getUint16     = getUint16_f;
    
    (*obj)->getInt32      = getInt32_f;
    (*obj)->getUint32     = getUint32_f;
    
    (*obj)->getInt64      = getInt64_f;
    (*obj)->getUint64     = getUint64_f;
    
    (*obj)->pData         = (void*)pData;
    
    return PARSER_ERROR_NONE;
}

/*!
 *
 */
enum parser_error_e Parser_UnInit(struct parser_s **obj)
{
    assert(obj && *obj && (*obj)->pData);
    
    free((*obj)->pData);
    free(*obj);
    *obj = NULL;
    
    return PARSER_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////// PUBLIC FUNCTIONS IMPLEMENTATION ///////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static enum parser_error_e parse_f(struct parser_s *obj, struct parser_params_s *params)
{
    assert(obj && obj->pData && params);
    
    if (access(params->path, F_OK) != 0) {
        Loge("File \"%s\" not found", params->path);
        return PARSER_ERROR_FILE;
    }
    
    enum parser_error_e ret = PARSER_ERROR_PARSE;
    
    struct parser_private_data_s *pData = (struct parser_private_data_s*)(obj->pData);
    pData->params = *params;
    
    XML_Char *encoding = NULL;
    switch (pData->params.encoding) {
        case PARSER_ENCODING_US_ASCII:
            encoding = strdup(ENCODING_US_ASCII);
            break;
        
        case PARSER_ENCODING_UTF_8:
            encoding = strdup(ENCODING_UTF_8);
            break;
            
        case PARSER_ENCODING_UTF_16:
            encoding = strdup(ENCODING_UTF_16);
            break;
            
        case PARSER_ENCODING_ISO_8859_1:
            encoding = strdup(ENCODING_ISO_8859_1);
            break;
        
        default:
            ;
    }
    
    if (!(pData->parser = XML_ParserCreate(encoding))) {
        Loge("Failed to create parser object");
        goto exit;
    }
    
    XML_SetUserData(pData->parser, pData);
    XML_SetElementHandler(pData->parser, startElementCb, endElementCb);
    XML_SetCharacterDataHandler(pData->parser, dataCb);
    
    if (stat(pData->params.path, &pData->st) != 0) {
        Loge("Failed to retrieve file size");
        goto exit;
    }
    
    Logd("Parsing file \"%s\" having size : %lu bytes", pData->params.path, pData->st.st_size);
    
    pData->xmlBufferSize = (pData->st.st_size > MAX_XML_SIZE ? MAX_XML_SIZE : pData->st.st_size);
    
    if (!(pData->xml = calloc(1, pData->xmlBufferSize))) {
        Loge("Failed to allocate memory");
        goto exit;
    }
    
    if (!(pData->fd = fopen(pData->params.path, "r"))) {
        Loge("Failed to open file");
        goto exit;
    }
    
    do {
        pData->nbRead = fread(pData->xml, 1, pData->xmlBufferSize, pData->fd);
        
        pData->isFinal = (pData->nbRead < pData->xmlBufferSize);
        
        if (!XML_Parse(pData->parser, pData->xml, pData->nbRead, pData->isFinal)) {
            Loge("Failed to parse file");
            if (pData->params.onErrorCb) {
                int errorCode        = XML_GetErrorCode(pData->parser);
                const char *errorStr = (const char*)XML_ErrorString((enum XML_Error)errorCode);
                Loge("errorStr = %s", errorStr);
                pData->params.onErrorCb(pData->params.userData, errorCode, errorStr);
            }
            goto exit;
        }
    }
    while (!pData->isFinal);
    
    Logd("Parsing done!");
    ret = PARSER_ERROR_NONE;
    
exit:
    if (pData->fd) {
        fclose(pData->fd);
        pData->fd = NULL;
    }
    
    if (pData->xml) {
        free(pData->xml);
        pData->xml = NULL;
    }
    
    if (pData->parser) {
        XML_ParserFree(pData->parser);
        pData->parser = NULL;
    }
    
    if (encoding) {
        free(encoding);
    }

    return ret;
}

/*!
 *
 */
static enum parser_error_e getAttributes_f(struct parser_s *obj,
                                           struct parser_attr_handler_s *attrHandlers,
                                           const char **attrs)
{
    assert(obj);

    if (!attrHandlers || !attrs) {
        Loge("Bad arguments");
        return PARSER_ERROR_ATTR;
    }
    
    struct parser_private_data_s *pData = (struct parser_private_data_s*)(obj->pData);
    uint32_t i, j;
	
    if (pData->openedTag && pData->openedTag->tagName) {
        Logd("Retrieving attributes of tag \"%s\"", pData->openedTag->tagName);
    }
	
    i = 0;
    while (attrs[i]) {
        j = 0;
        while (attrHandlers[j].attrName
               && (strcmp(attrHandlers[j].attrName, attrs[i]) != 0)) {
            j++;
        }

        if (attrHandlers[j].attrName) {
            if ((attrHandlers[j].attrType == PARSER_ATTR_TYPE_SCALAR)
                 && attrHandlers[j].attrGetter.scalar) {
                attrHandlers[j].attrGetter.scalar(obj,
                                                  attrHandlers[j].attrValue.scalar,
                                                  attrs[i + 1]);
            }
            else if ((attrHandlers[j].attrType == PARSER_ATTR_TYPE_VECTOR)
                      && attrHandlers[j].attrGetter.vector) {
                attrHandlers[j].attrGetter.vector(obj,
                                                  attrHandlers[j].attrValue.vector,
                                                  attrs[i + 1]);
            }
            Logd("%s = \"%s\"", attrs[i], attrs[i + 1]);
        }
		
        i += 2;
    }
	
    return PARSER_ERROR_NONE;
}

/*!
 *
 */
static enum parser_error_e getString_f(struct parser_s *obj, void **attrValueOut,
                                       const char *attrValueIn)
{
    assert(obj);

    if (!attrValueOut || !attrValueIn) {
        return PARSER_ERROR_ATTR;
    }

    if (*attrValueOut) {
        Loge("Memory is already allocated");
        return PARSER_ERROR_ATTR;
    }
	
    *attrValueOut = strdup(attrValueIn);
	
    return PARSER_ERROR_NONE;
}

/*!
 *
 */
static enum parser_error_e getInt8_f(struct parser_s *obj, void *attrValueOut,
                                     const char *attrValueIn)
{
    assert(obj);

    if (!attrValueOut || !attrValueIn) {
        return PARSER_ERROR_ATTR;
    }
	
    *((int8_t*)attrValueOut) = (int8_t)atoi(attrValueIn);
	
    return PARSER_ERROR_NONE;
}

/*!
 *
 */
static enum parser_error_e getUint8_f(struct parser_s *obj, void *attrValueOut,
                                      const char *attrValueIn)
{
    assert(obj);

    if (!attrValueOut || !attrValueIn) {
        return PARSER_ERROR_ATTR;
    }
	
    *((uint8_t*)attrValueOut) = (uint8_t)atoi(attrValueIn);
	
    return PARSER_ERROR_NONE;
}

/*!
 *
 */
static enum parser_error_e getInt16_f(struct parser_s *obj, void *attrValueOut,
                                      const char *attrValueIn)
{
    assert(obj);

    if (!attrValueOut || !attrValueIn) {
        return PARSER_ERROR_ATTR;
    }
	
    *((int16_t*)attrValueOut) = (int16_t)atoi(attrValueIn);
	
    return PARSER_ERROR_NONE;
}

/*!
 *
 */
static enum parser_error_e getUint16_f(struct parser_s *obj, void *attrValueOut,
                                       const char *attrValueIn)
{
    assert(obj);

    if (!attrValueOut || !attrValueIn) {
        return PARSER_ERROR_ATTR;
    }
	
    *((uint16_t*)attrValueOut) = (uint16_t)atoi(attrValueIn);
	
    return PARSER_ERROR_NONE;
}

/*!
 *
 */
static enum parser_error_e getInt32_f(struct parser_s *obj, void *attrValueOut,
                                      const char *attrValueIn)
{
    assert(obj);

    if (!attrValueOut || !attrValueIn) {
        return PARSER_ERROR_ATTR;
    }
	
    *((int32_t*)attrValueOut) = (int32_t)atoi(attrValueIn);
	
    return PARSER_ERROR_NONE;
}

/*!
 *
 */
static enum parser_error_e getUint32_f(struct parser_s *obj, void *attrValueOut,
                                       const char *attrValueIn)
{
    assert(obj);

    if (!attrValueOut || !attrValueIn) {
        return PARSER_ERROR_ATTR;
    }
	
    *((uint32_t*)attrValueOut) = (uint32_t)atoi(attrValueIn);
	
    return PARSER_ERROR_NONE;
}

/*!
 *
 */
static enum parser_error_e getInt64_f(struct parser_s *obj, void *attrValueOut,
                                      const char *attrValueIn)
{
    assert(obj);

    if (!attrValueOut || !attrValueIn) {
        return PARSER_ERROR_ATTR;
    }
	
    *((int64_t*)attrValueOut) = (int64_t)atol(attrValueIn);
	
    return PARSER_ERROR_NONE;
}

/*!
 *
 */
static enum parser_error_e getUint64_f(struct parser_s *obj, void *attrValueOut,
                                       const char *attrValueIn)
{
    assert(obj);

    if (!attrValueOut || !attrValueIn) {
        return PARSER_ERROR_ATTR;
    }
	
    *((uint64_t*)attrValueOut) = (uint64_t)atol(attrValueIn);
	
    return PARSER_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////////// CALLBACKS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static void XMLCALL startElementCb(void *userData, const char *name, const char **attrs)
{
    assert(userData && name);
	
    struct parser_private_data_s *pData = (struct parser_private_data_s*)userData;
	
    Logd("Start tag : \"%s\"", name);
	
    uint32_t index = 0;
    while (pData->params.tagsHandlers[index].tagName
           && (strcmp(pData->params.tagsHandlers[index].tagName, name) != 0)) {
        index++;
    }
	
    if (pData->params.tagsHandlers[index].tagName) {
        Logd("Start tag handler found ( tag : \"%s\" )", name);
        pData->openedTag = &pData->params.tagsHandlers[index];
    }
    else {
        Logw("Start tag handler not found ( tag : \"%s\" )", name);
    }
	
    if (pData->openedTag && pData->openedTag->onStartCb) {
        pData->openedTag->onStartCb(pData->params.userData, attrs);
    }
}

/*!
 *
 */
static void XMLCALL endElementCb(void *userData, const char *name)
{
    assert(userData && name);
	
    struct parser_private_data_s *pData = (struct parser_private_data_s*)userData;
	
    Logd("End tag : \"%s\"", name);
	
    if (!pData->openedTag || (strcmp(pData->openedTag->tagName, name) != 0)) {
        uint32_t index = 0;
        while (pData->params.tagsHandlers[index].tagName
               && (strcmp(pData->params.tagsHandlers[index].tagName, name) != 0)) {
            index++;
        }

        if (pData->params.tagsHandlers[index].tagName) {
            Logd("End tag handler found ( tag : \"%s\" )", name);
            pData->openedTag = &pData->params.tagsHandlers[index];
        }
        else {
            Logw("End tag handler not found ( tag : \"%s\" )", name);
        }
    }
	
    if (pData->openedTag && pData->openedTag->onEndCb) {
        pData->openedTag->onEndCb(pData->params.userData);
    }
	
    pData->openedTag = NULL;
}

/*!
 *
 */
static void XMLCALL dataCb(void *userData, const char *value, int32_t len)
{
    assert(userData);
	
    if ((len == 0) || !value) {
        return;
    }
	
    struct parser_private_data_s *pData = (struct parser_private_data_s*)userData;
	
    if (pData->openedTag) {
        if (pData->openedTag->tagName) {
            Logd("Value for tag : \"%s\"", pData->openedTag->tagName);
        }
	
        if (pData->openedTag->onDataCb) {
            pData->openedTag->onDataCb(pData->params.userData, value, len);
        }
    }
}
