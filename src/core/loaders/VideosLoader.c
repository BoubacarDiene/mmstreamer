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

#include "core/Loaders.h"
#include "core/XmlDefines.h"

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

LOADERS_ERROR_E loadVideosXml_f  (LOADERS_S *obj, CONTEXT_S *ctx, XML_VIDEOS_S *xmlVideos);
LOADERS_ERROR_E unloadVideosXml_f(LOADERS_S *obj, XML_VIDEOS_S *xmlVideos);

static void onVideoStartCb(void *userData, const char **attrs);
static void onVideoEndCb  (void *userData);

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
LOADERS_ERROR_E loadVideosXml_f(LOADERS_S *obj, CONTEXT_S *ctx, XML_VIDEOS_S *xmlVideos)
{
    assert(obj && ctx && xmlVideos);
    
    PARSER_S *parserObj = ctx->modules.parserObj;
    INPUT_S *input      = &ctx->input;
    
    xmlVideos->reserved = ctx;
    
    Logd("Parsing file : \"%s/%s\"", input->xmlRootDir, input->videosXml);
    
    PARSER_TAGS_HANDLER_S tagsHandlers[] = {
    	{ XML_TAG_VIDEO,    onVideoStartCb,   onVideoEndCb,   NULL },
    	{ XML_TAG_GENERAL,  onGeneralCb,      NULL,           NULL },
    	{ XML_TAG_DEVICE,   onDeviceCb,       NULL,           NULL },
    	{ XML_TAG_OUTPUT,   onOutputCb,       NULL,           NULL },
    	{ XML_TAG_BUFFER,   onBufferCb,       NULL,           NULL },
    	{ NULL,             NULL,             NULL,           NULL }
    };
    
    PARSER_PARAMS_S parserParams;
    snprintf(parserParams.path, sizeof(parserParams.path), "%s/%s", input->xmlRootDir, input->videosXml);
    parserParams.encoding     = PARSER_ENCODING_UTF_8;
    parserParams.tagsHandlers = tagsHandlers;
    parserParams.onErrorCb    = onErrorCb;
    parserParams.userData     = xmlVideos;
    
    if (parserObj->parse(parserObj, &parserParams) != PARSER_ERROR_NONE) {
        Loge("Failed to parse file");
        xmlVideos->reserved = NULL;
        return LOADERS_ERROR_XML;
    }
    
    return LOADERS_ERROR_NONE;
}

/*!
 *
 */
LOADERS_ERROR_E unloadVideosXml_f(LOADERS_S *obj, XML_VIDEOS_S *xmlVideos)
{
    assert(obj && xmlVideos);
    
    uint8_t index;
    XML_VIDEO_S *video;
    
    for (index = 0; index < xmlVideos->nbVideos; index++) {
        video = &xmlVideos->videos[index];
        if (video->graphicsDest) {
            free(video->graphicsDest);
            video->graphicsDest = NULL;
        }
    
        if (video->serverDest) {
            free(video->serverDest);
            video->serverDest = NULL;
        }
    
        if (video->deviceSrc) {
            free(video->deviceSrc);
            video->deviceSrc = NULL;
        }
    
        if (video->deviceName) {
            free(video->deviceName);
            video->deviceName = NULL;
        }
    }
    
    xmlVideos->reserved = NULL;
    
    return LOADERS_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/*                                           CALLBACKS                                          */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static void onVideoStartCb(void *userData, const char **attrs)
{
    assert(userData);
    
    (void)attrs;
    
    XML_VIDEOS_S *xmlVideos = (XML_VIDEOS_S*)userData;
    
    Logd("Adding video %u", (xmlVideos->nbVideos + 1));
    
    xmlVideos->videos = realloc(xmlVideos->videos, (xmlVideos->nbVideos + 1) * sizeof(XML_VIDEO_S));
    assert(xmlVideos->videos);
    
    memset(&xmlVideos->videos[xmlVideos->nbVideos], '\0', sizeof(XML_VIDEO_S));
}

/*!
 *
 */
static void onVideoEndCb(void *userData)
{
    assert(userData);
    
    XML_VIDEOS_S *xmlVideos = (XML_VIDEOS_S*)userData;

    xmlVideos->nbVideos++;
    
    Logd("Video %u added", xmlVideos->nbVideos);
}

/*!
 *
 */
static void onGeneralCb(void *userData, const char **attrs)
{
    assert(userData);
    
    XML_VIDEOS_S *xmlVideos = (XML_VIDEOS_S*)userData;
    XML_VIDEO_S *video      = &xmlVideos->videos[xmlVideos->nbVideos];
    CONTEXT_S *ctx          = (CONTEXT_S*)xmlVideos->reserved;
    PARSER_S *parserObj     = ctx->modules.parserObj;
    
    PARSER_ATTR_HANDLER_S attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_PRIORITY,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&video->priority,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_CONFIG_CHOICE,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&video->configChoice,
    	    .attrGetter.scalar = parserObj->getUint32
        },
    	{
    	    .attrName          = XML_ATTR_GFX_DEST,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&video->graphicsDest,
    	    .attrGetter.vector = parserObj->getString
        },
    	{
    	    .attrName          = XML_ATTR_SERVER_DEST,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&video->serverDest,
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
    
    if (video->graphicsDest && (strlen(video->graphicsDest) == 0)) {
        free(video->graphicsDest);
        video->graphicsDest = NULL;
    }
    
    if (video->serverDest && (strlen(video->serverDest) == 0)) {
        free(video->serverDest);
        video->serverDest = NULL;
    }
}

/*!
 *
 */
static void onDeviceCb(void *userData, const char **attrs)
{
    assert(userData);
    
    XML_VIDEOS_S *xmlVideos = (XML_VIDEOS_S*)userData;
    XML_VIDEO_S *video      = &xmlVideos->videos[xmlVideos->nbVideos];
    CONTEXT_S *ctx          = (CONTEXT_S*)xmlVideos->reserved;
    PARSER_S *parserObj     = ctx->modules.parserObj;
    
    PARSER_ATTR_HANDLER_S attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_NAME,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&video->deviceName,
    	    .attrGetter.vector = parserObj->getString
        },
    	{
    	    .attrName          = XML_ATTR_SRC,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&video->deviceSrc,
    	    .attrGetter.vector = parserObj->getString
        },
    	{
    	    .attrName          = XML_ATTR_WIDTH,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&video->deviceWidth,
    	    .attrGetter.scalar = parserObj->getUint16
        },
    	{
    	    .attrName          = XML_ATTR_HEIGHT,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&video->deviceHeight,
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
    
    XML_VIDEOS_S *xmlVideos = (XML_VIDEOS_S*)userData;
    XML_VIDEO_S *video      = &xmlVideos->videos[xmlVideos->nbVideos];
    CONTEXT_S *ctx          = (CONTEXT_S*)xmlVideos->reserved;
    PARSER_S *parserObj     = ctx->modules.parserObj;
    
    PARSER_ATTR_HANDLER_S attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_WIDTH,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&video->outputWidth,
    	    .attrGetter.scalar = parserObj->getUint16
        },
    	{
    	    .attrName          = XML_ATTR_HEIGHT,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&video->outputHeight,
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
    
    XML_VIDEOS_S *xmlVideos = (XML_VIDEOS_S*)userData;
    XML_VIDEO_S *video      = &xmlVideos->videos[xmlVideos->nbVideos];
    CONTEXT_S *ctx          = (CONTEXT_S*)xmlVideos->reserved;
    PARSER_S *parserObj     = ctx->modules.parserObj;
    
    PARSER_ATTR_HANDLER_S attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_NB_BUFFERS,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&video->nbBuffers,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_DESIRED_FPS,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&video->desiredFps,
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
