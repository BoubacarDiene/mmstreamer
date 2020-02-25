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
* \file VideoLoader.c
* \brief TODO
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// HEADERS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#include "core/Loaders.h"
#include "core/XmlDefines.h"

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// MACROS ////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "VideosLoader"

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////// PUBLIC FUNCTIONS PROTOTYPES //////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum loaders_error_e loadVideosXml_f(struct loaders_s *obj, struct context_s *ctx,
                                     struct xml_videos_s *xmlVideos);
enum loaders_error_e unloadVideosXml_f(struct loaders_s *obj, struct xml_videos_s *xmlVideos);

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////////// CALLBACKS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

static void onVideoStartCb(const void *userData, const char **attrs);
static void onVideoEndCb(const void *userData);

static void onGeneralCb(const void *userData, const char **attrs);
static void onDeviceCb(const void *userData, const char **attrs);
static void onCroppingAreaCb(const void *userData, const char **attrs);
static void onComposingAreaCb(const void *userData, const char **attrs);
static void onBufferCb(const void *userData, const char **attrs);

static void onConfigStartCb(const void *userData, const char **attrs);
static void onConfigEndCb(const void *userData);

static void onCapabilitiesStartCb(const void *userData, const char **attrs);
static void onCapabilitiesEndCb(const void *userData);

static void onItemCb(const void *userData, const char **attrs);
static void onBufferTypeCb(const void *userData, const char **attrs);
static void onPixelFormatCb(const void *userData, const char **attrs);
static void onColorspaceCb(const void *userData, const char **attrs);
static void onMemoryCb(const void *userData, const char **attrs);
static void onAwaitModeCb(const void *userData, const char **attrs);

static void onErrorCb(const void *userData, int32_t errorCode, const char *errorStr);

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////// PUBLIC FUNCTIONS IMPLEMENTATION ///////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
enum loaders_error_e loadVideosXml_f(struct loaders_s *obj, struct context_s *ctx,
                                     struct xml_videos_s *xmlVideos)
{
    ASSERT(obj && ctx && xmlVideos);
    
    struct parser_s *parserObj = ctx->parserObj;
    struct input_s *input      = &ctx->input;
    
    xmlVideos->reserved = ctx;
    
    Logd("Parsing file : \"%s/%s\"", input->resRootDir, input->videosConfig.xml);
    
    struct parser_tags_handler_s tagsHandlers[] = {
    	{ XML_TAG_VIDEO,           onVideoStartCb,         onVideoEndCb,         NULL },
    	{ XML_TAG_GENERAL,         onGeneralCb,            NULL,                 NULL },
    	{ XML_TAG_DEVICE,          onDeviceCb,             NULL,                 NULL },
    	{ XML_TAG_CROPPING_AREA,   onCroppingAreaCb,       NULL,                 NULL },
    	{ XML_TAG_COMPOSING_AREA,  onComposingAreaCb,      NULL,                 NULL },
    	{ XML_TAG_BUFFER,          onBufferCb,             NULL,                 NULL },
    	{ XML_TAG_CONFIG,          onConfigStartCb,        onConfigEndCb,        NULL },
    	{ XML_TAG_CAPABILITIES,    onCapabilitiesStartCb,  onCapabilitiesEndCb,  NULL },
    	{ XML_TAG_ITEM,            onItemCb,               NULL,                 NULL },
    	{ XML_TAG_BUFFER_TYPE,     onBufferTypeCb,         NULL,                 NULL },
    	{ XML_TAG_PIXEL_FORMAT,    onPixelFormatCb,        NULL,                 NULL },
    	{ XML_TAG_COLORSPACE,      onColorspaceCb,         NULL,                 NULL },
    	{ XML_TAG_MEMORY,          onMemoryCb,             NULL,                 NULL },
    	{ XML_TAG_AWAIT_MODE,      onAwaitModeCb,          NULL,                 NULL },
    	{ NULL,                    NULL,                   NULL,                 NULL }
    };
    
    struct parser_params_s parserParams;
    snprintf(parserParams.path, sizeof(parserParams.path), "%s/%s",
                                input->resRootDir, input->videosConfig.xml);
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
enum loaders_error_e unloadVideosXml_f(struct loaders_s *obj, struct xml_videos_s *xmlVideos)
{
    ASSERT(obj && xmlVideos);
    
    uint8_t i, j;
    struct xml_video_s *video;
    struct xml_config_s *config;
    
    for (i = 0; i < xmlVideos->nbVideos; i++) {
        video = &xmlVideos->videos[i];
        if (video->graphicsDest) {
            free(video->graphicsDest);
        }
    
        if (video->serverDest) {
            free(video->serverDest);
        }
    
        if (video->deviceSrc) {
            free(video->deviceSrc);
        }
    
        if (video->deviceName) {
            free(video->deviceName);
        }
    }
    
    free(xmlVideos->videos);
    xmlVideos->videos = NULL;
    
    for (i = 0; i < xmlVideos->nbConfigs; i++) {
        config = &xmlVideos->configs[i];
        for (j = 0; j < config->nbItems; j++) {
            if (config->capabilities[j].item) {
                free(config->capabilities[j].item);
            }
        }
        
        free(config->capabilities);

        if (config->bufferType) {
            free(config->bufferType);
        }
    
        if (config->pixelFormat) {
            free(config->pixelFormat);
        }
    
        if (config->colorspace) {
            free(config->colorspace);
        }
    
        if (config->memory) {
            free(config->memory);
        }
    
        if (config->awaitMode) {
            free(config->awaitMode);
        }
    }
    
    free(xmlVideos->configs);
    xmlVideos->configs = NULL;
    
    xmlVideos->reserved = NULL;
    
    return LOADERS_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////////// CALLBACKS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static void onVideoStartCb(const void *userData, const char **attrs)
{
    ASSERT(userData);
    
    (void)attrs;
    
    struct xml_videos_s *xmlVideos = (struct xml_videos_s*)userData;
    
    Logd("Adding video %u", (xmlVideos->nbVideos + 1));
    
    xmlVideos->videos = realloc(xmlVideos->videos,
                                (size_t)(xmlVideos->nbVideos + 1) * sizeof(struct xml_video_s));
    ASSERT(xmlVideos->videos);
    
    memset(&xmlVideos->videos[xmlVideos->nbVideos], 0, sizeof(struct xml_video_s));
}

/*!
 *
 */
static void onVideoEndCb(const void *userData)
{
    ASSERT(userData);
    
    struct xml_videos_s *xmlVideos = (struct xml_videos_s*)userData;

    xmlVideos->nbVideos++;
    
    Logd("Video %u added", xmlVideos->nbVideos);
}

/*!
 *
 */
static void onGeneralCb(const void *userData, const char **attrs)
{
    ASSERT(userData);
    
    struct xml_videos_s *xmlVideos = (struct xml_videos_s*)userData;
    struct xml_video_s *video      = &xmlVideos->videos[xmlVideos->nbVideos];
    struct context_s *ctx          = (struct context_s*)xmlVideos->reserved;
    struct parser_s *parserObj     = ctx->parserObj;
    
    struct parser_attr_handler_s attrHandlers[] = {
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
    
    if (video->graphicsDest && ((video->graphicsDest)[0] == '\0')) {
        free(video->graphicsDest);
        video->graphicsDest = NULL;
    }
    
    if (video->serverDest && ((video->serverDest)[0] == '\0')) {
        free(video->serverDest);
        video->serverDest = NULL;
    }
}

/*!
 *
 */
static void onDeviceCb(const void *userData, const char **attrs)
{
    ASSERT(userData);
    
    struct xml_videos_s *xmlVideos = (struct xml_videos_s*)userData;
    struct xml_video_s *video      = &xmlVideos->videos[xmlVideos->nbVideos];
    struct context_s *ctx          = (struct context_s*)xmlVideos->reserved;
    struct parser_s *parserObj     = ctx->parserObj;
    
    struct parser_attr_handler_s attrHandlers[] = {
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
    	    .attrValue.scalar  = (void*)&video->deviceArea.width,
    	    .attrGetter.scalar = parserObj->getInt32
        },
    	{
    	    .attrName          = XML_ATTR_HEIGHT,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&video->deviceArea.height,
    	    .attrGetter.scalar = parserObj->getInt32
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
static void onCroppingAreaCb(const void *userData, const char **attrs)
{
    ASSERT(userData);

    struct xml_videos_s *xmlVideos = (struct xml_videos_s*)userData;
    struct xml_video_s *video      = &xmlVideos->videos[xmlVideos->nbVideos];
    struct context_s *ctx          = (struct context_s*)xmlVideos->reserved;
    struct parser_s *parserObj     = ctx->parserObj;

    struct parser_attr_handler_s attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_LEFT,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&video->croppingArea.left,
    	    .attrGetter.scalar = parserObj->getUint32
        },
    	{
    	    .attrName          = XML_ATTR_TOP,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&video->croppingArea.top,
    	    .attrGetter.scalar = parserObj->getUint32
        },
    	{
    	    .attrName          = XML_ATTR_WIDTH,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&video->croppingArea.width,
    	    .attrGetter.scalar = parserObj->getInt32
        },
    	{
    	    .attrName          = XML_ATTR_HEIGHT,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&video->croppingArea.height,
    	    .attrGetter.scalar = parserObj->getInt32
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
static void onComposingAreaCb(const void *userData, const char **attrs)
{
    ASSERT(userData);

    struct xml_videos_s *xmlVideos = (struct xml_videos_s*)userData;
    struct xml_video_s *video      = &xmlVideos->videos[xmlVideos->nbVideos];
    struct context_s *ctx          = (struct context_s*)xmlVideos->reserved;
    struct parser_s *parserObj     = ctx->parserObj;

    struct parser_attr_handler_s attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_LEFT,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&video->composingArea.left,
    	    .attrGetter.scalar = parserObj->getInt32
        },
    	{
    	    .attrName          = XML_ATTR_TOP,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&video->composingArea.top,
    	    .attrGetter.scalar = parserObj->getInt32
        },
    	{
    	    .attrName          = XML_ATTR_WIDTH,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&video->composingArea.width,
    	    .attrGetter.scalar = parserObj->getInt32
        },
    	{
    	    .attrName          = XML_ATTR_HEIGHT,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&video->composingArea.height,
    	    .attrGetter.scalar = parserObj->getInt32
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
static void onBufferCb(const void *userData, const char **attrs)
{
    ASSERT(userData);
    
    struct xml_videos_s *xmlVideos = (struct xml_videos_s*)userData;
    struct xml_video_s *video      = &xmlVideos->videos[xmlVideos->nbVideos];
    struct context_s *ctx          = (struct context_s*)xmlVideos->reserved;
    struct parser_s *parserObj     = ctx->parserObj;
    
    struct parser_attr_handler_s attrHandlers[] = {
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
static void onConfigStartCb(const void *userData, const char **attrs)
{
    ASSERT(userData);
    
    (void)attrs;
    
    struct xml_videos_s *xmlVideos = (struct xml_videos_s*)userData;
    
    Logd("Adding config %u", (xmlVideos->nbConfigs + 1));
    
    xmlVideos->configs = realloc(xmlVideos->configs,
                                 (size_t)(xmlVideos->nbConfigs + 1) * sizeof(struct xml_config_s));
    ASSERT(xmlVideos->configs);
    
    memset(&xmlVideos->configs[xmlVideos->nbConfigs], 0, sizeof(struct xml_config_s));
}

/*!
 *
 */
static void onConfigEndCb(const void *userData)
{
    ASSERT(userData);
    
    struct xml_videos_s *xmlVideos = (struct xml_videos_s*)userData;

    xmlVideos->nbConfigs++;
    
    Logd("Config %u added", xmlVideos->nbConfigs);
}

/*!
 *
 */
static void onCapabilitiesStartCb(const void *userData, const char **attrs)
{
    ASSERT(userData);
    
    (void)attrs;

    Logd("Start parsing capabilities");
}

/*!
 *
 */
static void onCapabilitiesEndCb(const void *userData)
{
    ASSERT(userData);

    Logd("End parsing capabilities");
}

/*!
 *
 */
static void onItemCb(const void *userData, const char **attrs)
{
    ASSERT(userData);
    
    struct xml_videos_s *xmlVideos = (struct xml_videos_s*)userData;
    struct xml_config_s *config    = &xmlVideos->configs[xmlVideos->nbConfigs];
    struct context_s *ctx          = (struct context_s*)xmlVideos->reserved;
    struct parser_s *parserObj     = ctx->parserObj;

    Logd("Adding item %u", (config->nbItems + 1));
    
    config->capabilities = realloc(config->capabilities,
                                   (size_t)(config->nbItems + 1) * sizeof(struct xml_capability_s));
    ASSERT(config->capabilities);
    
    memset(&config->capabilities[config->nbItems], 0, sizeof(struct xml_capability_s));

    struct xml_capability_s *capability = &config->capabilities[config->nbItems];

    struct parser_attr_handler_s attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_VALUE,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&capability->item,
    	    .attrGetter.vector = parserObj->getString
        },
    	{
    	    NULL,
    	    PARSER_ATTR_TYPE_NONE,
    	    NULL,
    	    NULL
        }
    };
    
    ++config->nbItems;
    
    if (parserObj->getAttributes(parserObj, attrHandlers, attrs) != PARSER_ERROR_NONE) {
    	Loge("Failed to retrieve attributes in \"Item\" tag");
    	return;
    }
    
    Logd("Video capability : \"%s\"", capability->item);
}

/*!
 *
 */
static void onBufferTypeCb(const void *userData, const char **attrs)
{
    ASSERT(userData);
    
    struct xml_videos_s *xmlVideos = (struct xml_videos_s*)userData;
    struct xml_config_s *config    = &xmlVideos->configs[xmlVideos->nbConfigs];
    struct context_s *ctx          = (struct context_s*)xmlVideos->reserved;
    struct parser_s *parserObj     = ctx->parserObj;

    struct parser_attr_handler_s attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_VALUE,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&config->bufferType,
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
    	Loge("Failed to retrieve attributes in \"BufferType\" tag");
    	return;
    }
    
    Logd("Video bufferType : \"%s\"", config->bufferType);
}

/*!
 *
 */
static void onPixelFormatCb(const void *userData, const char **attrs)
{
    ASSERT(userData);
    
    struct xml_videos_s *xmlVideos = (struct xml_videos_s*)userData;
    struct xml_config_s *config    = &xmlVideos->configs[xmlVideos->nbConfigs];
    struct context_s *ctx          = (struct context_s*)xmlVideos->reserved;
    struct parser_s *parserObj     = ctx->parserObj;

    struct parser_attr_handler_s attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_VALUE,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&config->pixelFormat,
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
    	Loge("Failed to retrieve attributes in \"PixelFormat\" tag");
    	return;
    }
    
    Logd("Video pixelFormat : \"%s\"", config->pixelFormat);
}

/*!
 *
 */
static void onColorspaceCb(const void *userData, const char **attrs)
{
    ASSERT(userData);
    
    struct xml_videos_s *xmlVideos = (struct xml_videos_s*)userData;
    struct xml_config_s *config    = &xmlVideos->configs[xmlVideos->nbConfigs];
    struct context_s *ctx          = (struct context_s*)xmlVideos->reserved;
    struct parser_s *parserObj     = ctx->parserObj;

    struct parser_attr_handler_s attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_VALUE,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&config->colorspace,
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
    	Loge("Failed to retrieve attributes in \"Colorspace\" tag");
    	return;
    }
    
    Logd("Video colorspace : \"%s\"", config->colorspace);
}

/*!
 *
 */
static void onMemoryCb(const void *userData, const char **attrs)
{
    ASSERT(userData);
    
    struct xml_videos_s *xmlVideos = (struct xml_videos_s*)userData;
    struct xml_config_s *config    = &xmlVideos->configs[xmlVideos->nbConfigs];
    struct context_s *ctx          = (struct context_s*)xmlVideos->reserved;
    struct parser_s *parserObj     = ctx->parserObj;

    struct parser_attr_handler_s attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_VALUE,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&config->memory,
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
    	Loge("Failed to retrieve attributes in \"Memory\" tag");
    	return;
    }
    
    Logd("Video memory : \"%s\"", config->memory);
}

/*!
 *
 */
static void onAwaitModeCb(const void *userData, const char **attrs)
{
    ASSERT(userData);
    
    struct xml_videos_s *xmlVideos = (struct xml_videos_s*)userData;
    struct xml_config_s *config    = &xmlVideos->configs[xmlVideos->nbConfigs];
    struct context_s *ctx          = (struct context_s*)xmlVideos->reserved;
    struct parser_s *parserObj     = ctx->parserObj;

    struct parser_attr_handler_s attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_VALUE,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&config->awaitMode,
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
    	Loge("Failed to retrieve attributes in \"AwaitMode\" tag");
    	return;
    }
    
    Logd("Video awaitMode : \"%s\"", config->awaitMode);
}

/*!
 *
 */
static void onErrorCb(const void *userData, int32_t errorCode, const char *errorStr)
{
    (void)userData;
    
    Loge("Parsing error - errorCode = %d / errorStr = \"%s\"", errorCode, errorStr);
}
