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
* \file   VideoLoader.c
* \brief  TODO
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include "specific/Loaders.h"
#include "specific/XmlDefines.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                            */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "VIDEO-LOADER"

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                         PROTOTYPES                                           */
/* -------------------------------------------------------------------------------------------- */

LOADERS_ERROR_E loadVideoXml_f  (LOADERS_S *obj, CONTEXT_S *ctx, XML_VIDEO_S *xmlVideo);
LOADERS_ERROR_E unloadVideoXml_f(LOADERS_S *obj, XML_VIDEO_S *xmlVideo);

static void onGeneralCb(void *userData, const char **attrs);
static void onDeviceCb (void *userData, const char **attrs);
static void onOutputCb (void *userData, const char **attrs);
static void onBufferCb (void *userData, const char **attrs);

static void onErrorCb(void *userData, int32_t errorCode, const char *errorStr);

/* -------------------------------------------------------------------------------------------- */
/*                                          VARIABLES                                           */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                          FUNCTIONS                                           */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
LOADERS_ERROR_E loadVideoXml_f(LOADERS_S *obj, CONTEXT_S *ctx, XML_VIDEO_S *xmlVideo)
{
    assert(obj && ctx && xmlVideo);
    
    PARSER_S *parserObj = ctx->modules.parserObj;
    INPUT_S *input      = &ctx->input;
    
    xmlVideo->reserved  = ctx;
    
    Logd("Parsing file : \"%s/%s\"", input->xmlRootDir, input->videoXml);
    
    PARSER_TAGS_HANDLER_S tagsHandlers[] = {
    	{ XML_TAG_GENERAL,  onGeneralCb,      NULL,           NULL },
    	{ XML_TAG_DEVICE,   onDeviceCb,       NULL,           NULL },
    	{ XML_TAG_OUTPUT,   onOutputCb,       NULL,           NULL },
    	{ XML_TAG_BUFFER,   onBufferCb,       NULL,           NULL },
    	{ NULL,             NULL,             NULL,           NULL }
    };
    
    PARSER_PARAMS_S parserParams;
    snprintf(parserParams.path, sizeof(parserParams.path), "%s/%s", input->xmlRootDir, input->videoXml);
    parserParams.encoding     = PARSER_ENCODING_UTF_8;
    parserParams.tagsHandlers = tagsHandlers;
    parserParams.onErrorCb    = onErrorCb;
    parserParams.userData     = xmlVideo;
    
    if (parserObj->parse(parserObj, &parserParams) != PARSER_ERROR_NONE) {
        Loge("Failed to parse file");
        xmlVideo->reserved = NULL;
        return LOADERS_ERROR_XML;
    }
    
    return LOADERS_ERROR_NONE;
}

/*!
 *
 */
LOADERS_ERROR_E unloadVideoXml_f(LOADERS_S *obj, XML_VIDEO_S *xmlVideo)
{
    assert(obj && xmlVideo);
    
    if (xmlVideo->graphicsDest) {
        free(xmlVideo->graphicsDest);
        xmlVideo->graphicsDest = NULL;
    }
    
    if (xmlVideo->serverDest) {
        free(xmlVideo->serverDest);
        xmlVideo->serverDest = NULL;
    }
    
    if (xmlVideo->deviceSrc) {
        free(xmlVideo->deviceSrc);
        xmlVideo->deviceSrc = NULL;
    }
    
    xmlVideo->reserved = NULL;
    
    return LOADERS_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/*                                           CALLBACKS                                          */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static void onGeneralCb(void *userData, const char **attrs)
{
    assert(userData);
    
    XML_VIDEO_S *xmlVideo = (XML_VIDEO_S*)userData;
    CONTEXT_S *ctx        = (CONTEXT_S*)xmlVideo->reserved;
    PARSER_S *parserObj   = ctx->modules.parserObj;
    
    PARSER_ATTR_HANDLER_S attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_PRIORITY,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&xmlVideo->priority,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_CONFIG_CHOICE,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&xmlVideo->configChoice,
    	    .attrGetter.scalar = parserObj->getUint32
        },
    	{
    	    .attrName          = XML_ATTR_GFX_DEST,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&xmlVideo->graphicsDest,
    	    .attrGetter.vector = parserObj->getString
        },
    	{
    	    .attrName          = XML_ATTR_SERVER_DEST,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&xmlVideo->serverDest,
    	    .attrGetter.vector = parserObj->getString
        },
    	{
    	    NULL,
    	    PARSER_ATTR_TYPE_NONE,
    	    NULL,
    	    NULL
        }
    };
    
    if (parserObj->getAttributes(parserObj, attrHandlers, attrs) != PARSER_ERROR_NONE) {
    	Loge("Failed to retrieve attributes in \"General\" tag");
    }
    
    if (xmlVideo->graphicsDest && (strlen(xmlVideo->graphicsDest) == 0)) {
        free(xmlVideo->graphicsDest);
        xmlVideo->graphicsDest = NULL;
    }
    
    if (xmlVideo->serverDest && (strlen(xmlVideo->serverDest) == 0)) {
        free(xmlVideo->serverDest);
        xmlVideo->serverDest = NULL;
    }
}

/*!
 *
 */
static void onDeviceCb(void *userData, const char **attrs)
{
    assert(userData);
    
    XML_VIDEO_S *xmlVideo = (XML_VIDEO_S*)userData;
    CONTEXT_S *ctx        = (CONTEXT_S*)xmlVideo->reserved;
    PARSER_S *parserObj   = ctx->modules.parserObj;
    
    PARSER_ATTR_HANDLER_S attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_SRC,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&xmlVideo->deviceSrc,
    	    .attrGetter.vector = parserObj->getString
        },
    	{
    	    .attrName          = XML_ATTR_WIDTH,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&xmlVideo->deviceWidth,
    	    .attrGetter.scalar = parserObj->getUint16
        },
    	{
    	    .attrName          = XML_ATTR_HEIGHT,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&xmlVideo->deviceHeight,
    	    .attrGetter.scalar = parserObj->getUint16
        },
    	{
    	    NULL,
    	    PARSER_ATTR_TYPE_NONE,
    	    NULL,
    	    NULL
        }
    };
    
    if (parserObj->getAttributes(parserObj, attrHandlers, attrs) != PARSER_ERROR_NONE) {
    	Loge("Failed to retrieve attributes in \"Device\" tag");
    }
}

/*!
 *
 */
static void onOutputCb(void *userData, const char **attrs)
{
    assert(userData);
    
    XML_VIDEO_S *xmlVideo = (XML_VIDEO_S*)userData;
    CONTEXT_S *ctx        = (CONTEXT_S*)xmlVideo->reserved;
    PARSER_S *parserObj   = ctx->modules.parserObj;
    
    PARSER_ATTR_HANDLER_S attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_WIDTH,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&xmlVideo->outputWidth,
    	    .attrGetter.scalar = parserObj->getUint16
        },
    	{
    	    .attrName          = XML_ATTR_HEIGHT,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&xmlVideo->outputHeight,
    	    .attrGetter.scalar = parserObj->getUint16
        },
    	{
    	    NULL,
    	    PARSER_ATTR_TYPE_NONE,
    	    NULL,
    	    NULL
        }
    };
    
    if (parserObj->getAttributes(parserObj, attrHandlers, attrs) != PARSER_ERROR_NONE) {
    	Loge("Failed to retrieve attributes in \"Output\" tag");
    }
}

/*!
 *
 */
static void onBufferCb(void *userData, const char **attrs)
{
    assert(userData);
    
    XML_VIDEO_S *xmlVideo = (XML_VIDEO_S*)userData;
    CONTEXT_S *ctx        = (CONTEXT_S*)xmlVideo->reserved;
    PARSER_S *parserObj   = ctx->modules.parserObj;
    
    PARSER_ATTR_HANDLER_S attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_NB_BUFFERS,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&xmlVideo->nbBuffers,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_DESIRED_FPS,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&xmlVideo->desiredFps,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    NULL,
    	    PARSER_ATTR_TYPE_NONE,
    	    NULL,
    	    NULL
        }
    };
    
    if (parserObj->getAttributes(parserObj, attrHandlers, attrs) != PARSER_ERROR_NONE) {
    	Loge("Failed to retrieve attributes in \"Buffer\" tag");
    }
}

/*!
 *
 */
static void onErrorCb(void *userData, int32_t errorCode, const char *errorStr)
{
    (void)userData;
    
    Loge("Parsing error - errorCode = %d / errorStr = \"%s\"", errorCode, errorStr);
}
