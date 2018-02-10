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
* \file   GraphicsLoader.c
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
#define TAG "GraphicsLoader"

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                         PROTOTYPES                                           */
/* -------------------------------------------------------------------------------------------- */

LOADERS_ERROR_E loadGraphicsXml_f  (LOADERS_S *obj, CONTEXT_S *ctx, XML_GRAPHICS_S *xmlGraphics);
LOADERS_ERROR_E unloadGraphicsXml_f(LOADERS_S *obj, XML_GRAPHICS_S *xmlGraphics);

static void onCommonCb (void *userData, const char **attrs);
static void onColorsCb (void *userData, const char **attrs);
static void onImagesCb (void *userData, const char **attrs);
static void onFontsCb  (void *userData, const char **attrs);
static void onStringsCb(void *userData, const char **attrs);

static void onScreenCb    (void *userData, const char **attrs);
static void onBackgroundCb(void *userData, const char **attrs);
static void onIconCb      (void *userData, const char **attrs);

static void onElementStartCb (void *userData, const char **attrs);
static void onElementEndCb   (void *userData);
static void onElementConfigCb(void *userData, const char **attrs);
static void onElementTextCb  (void *userData, const char **attrs);
static void onElementNavCb   (void *userData, const char **attrs);
static void onElementImageCb (void *userData, const char **attrs);

static void onElementClickStartCb(void *userData, const char **attrs);
static void onElementClickEndCb  (void *userData);
static void onElementHandlerCb   (void *userData, const char **attrs);

static void onFocusCb(void *userData, const char **attrs);
static void onBlurCb (void *userData, const char **attrs);
static void onResetCb(void *userData, const char **attrs);

static void onColorCb(void *userData, const char **attrs);
static void onImageCb(void *userData, const char **attrs);
static void onFontCb (void *userData, const char **attrs);

static void onStrGroupStartCb(void *userData, const char **attrs);
static void onStringCb       (void *userData, const char **attrs);
static void onStrGroupEndCb  (void *userData);

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
LOADERS_ERROR_E loadGraphicsXml_f(LOADERS_S *obj, CONTEXT_S *ctx, XML_GRAPHICS_S *xmlGraphics)
{
    assert(obj && ctx && xmlGraphics);
    
    PARSER_S *parserObj = ctx->parserObj;
    INPUT_S *input      = &ctx->input;
    
    xmlGraphics->reserved = ctx;
    
    Logd("Parsing file : \"%s/%s\"", input->resRootDir, input->graphicsConfig.xml);
    
    PARSER_TAGS_HANDLER_S gfxTagsHandlers[] = {
        { XML_TAG_COMMON,      onCommonCb,             NULL,                NULL },
        { XML_TAG_COLORS,      onColorsCb,             NULL,                NULL },
        { XML_TAG_IMAGES,      onImagesCb,             NULL,                NULL },
        { XML_TAG_FONTS,       onFontsCb,              NULL,                NULL },
        { XML_TAG_STRINGS,     onStringsCb,            NULL,                NULL },
        { XML_TAG_SCREEN,      onScreenCb,             NULL,                NULL },
        { XML_TAG_BACKGROUND,  onBackgroundCb,         NULL,                NULL },
        { XML_TAG_ICON,        onIconCb,               NULL,                NULL },
        { XML_TAG_ELEMENT,     onElementStartCb,       onElementEndCb,      NULL },
        { XML_TAG_CONFIG,      onElementConfigCb,      NULL,                NULL },
        { XML_TAG_TEXT,        onElementTextCb,        NULL,                NULL },
        { XML_TAG_NAV,         onElementNavCb,         NULL,                NULL },
        { XML_TAG_IMAGE,       onElementImageCb,       NULL,                NULL },
        { XML_TAG_ON_CLICK,    onElementClickStartCb,  onElementClickEndCb, NULL },
        { XML_TAG_HANDLER,     onElementHandlerCb,     NULL,                NULL },
        { XML_TAG_FOCUS,       onFocusCb,              NULL,                NULL },
        { XML_TAG_BLUR,        onBlurCb,               NULL,                NULL },
        { XML_TAG_RESET,       onResetCb,              NULL,                NULL },
        { NULL,                NULL,                   NULL,                NULL }
    };
    
    PARSER_PARAMS_S gfxParserParams;
    snprintf(gfxParserParams.path, sizeof(gfxParserParams.path), "%s/%s", input->resRootDir, input->graphicsConfig.xml);
    gfxParserParams.encoding     = PARSER_ENCODING_UTF_8;
    gfxParserParams.tagsHandlers = gfxTagsHandlers;
    gfxParserParams.onErrorCb    = onErrorCb;
    gfxParserParams.userData     = xmlGraphics;
    
    if (parserObj->parse(parserObj, &gfxParserParams) != PARSER_ERROR_NONE) {
        Loge("Failed to parse file");
        xmlGraphics->reserved = NULL;
        return LOADERS_ERROR_XML;
    }
    
    if (xmlGraphics->common.colorsXmlFile) {
        Logd("Parsing file : \"%s/%s\"", input->resRootDir, xmlGraphics->common.colorsXmlFile);
        
        PARSER_TAGS_HANDLER_S colorsTagsHandlers[] = {
        	{ XML_TAG_COLOR,  onColorCb,  NULL,  NULL },
        	{ NULL,           NULL,       NULL,  NULL }
        };
        
        PARSER_PARAMS_S colorsParserParams;
        snprintf(colorsParserParams.path, sizeof(colorsParserParams.path), "%s/%s", input->resRootDir, xmlGraphics->common.colorsXmlFile);
        colorsParserParams.encoding     = PARSER_ENCODING_UTF_8;
        colorsParserParams.tagsHandlers = colorsTagsHandlers;
        colorsParserParams.onErrorCb    = onErrorCb;
        colorsParserParams.userData     = xmlGraphics;
        
        if (parserObj->parse(parserObj, &colorsParserParams) != PARSER_ERROR_NONE) {
            Loge("Failed to parse file");
            xmlGraphics->reserved = NULL;
            return LOADERS_ERROR_XML;
        }
    }
    
    if (xmlGraphics->common.imagesXmlFile) {
        Logd("Parsing file : \"%s/%s\"", input->resRootDir, xmlGraphics->common.imagesXmlFile);
        
        PARSER_TAGS_HANDLER_S imagesTagsHandlers[] = {
        	{ XML_TAG_IMAGE,  onImageCb,  NULL,  NULL },
        	{ NULL,           NULL,       NULL,  NULL }
        };
        
        PARSER_PARAMS_S imagesParserParams;
        snprintf(imagesParserParams.path, sizeof(imagesParserParams.path), "%s/%s", input->resRootDir, xmlGraphics->common.imagesXmlFile);
        imagesParserParams.encoding     = PARSER_ENCODING_UTF_8;
        imagesParserParams.tagsHandlers = imagesTagsHandlers;
        imagesParserParams.onErrorCb    = onErrorCb;
        imagesParserParams.userData     = xmlGraphics;
        
        if (parserObj->parse(parserObj, &imagesParserParams) != PARSER_ERROR_NONE) {
            Loge("Failed to parse file");
            xmlGraphics->reserved = NULL;
            return LOADERS_ERROR_XML;
        }
    }
    
    if (xmlGraphics->common.fontsXmlFile) {
        Logd("Parsing file : \"%s/%s\"", input->resRootDir, xmlGraphics->common.fontsXmlFile);
        
        PARSER_TAGS_HANDLER_S fontsTagsHandlers[] = {
        	{ XML_TAG_FONT,  onFontCb,  NULL,  NULL },
        	{ NULL,          NULL,      NULL,  NULL }
        };
        
        PARSER_PARAMS_S fontsParserParams;
        snprintf(fontsParserParams.path, sizeof(fontsParserParams.path), "%s/%s", input->resRootDir, xmlGraphics->common.fontsXmlFile);
        fontsParserParams.encoding     = PARSER_ENCODING_UTF_8;
        fontsParserParams.tagsHandlers = fontsTagsHandlers;
        fontsParserParams.onErrorCb    = onErrorCb;
        fontsParserParams.userData     = xmlGraphics;
        
        if (parserObj->parse(parserObj, &fontsParserParams) != PARSER_ERROR_NONE) {
            Loge("Failed to parse file");
            xmlGraphics->reserved = NULL;
            return LOADERS_ERROR_XML;
        }
    }
    
    if (xmlGraphics->common.stringsXmlFile) {
        Logd("Parsing file : \"%s/%s\"", input->resRootDir, xmlGraphics->common.stringsXmlFile);
        
        PARSER_TAGS_HANDLER_S stringsTagsHandlers[] = {
        	{ XML_TAG_STR_GROUP,  onStrGroupStartCb,  onStrGroupEndCb,  NULL },
        	{ XML_TAG_STRING,     onStringCb,         NULL,             NULL },
        	{ NULL,               NULL,               NULL,             NULL }
        };
        
        PARSER_PARAMS_S stringsParserParams;
        snprintf(stringsParserParams.path, sizeof(stringsParserParams.path), "%s/%s", input->resRootDir, xmlGraphics->common.stringsXmlFile);
        stringsParserParams.encoding     = PARSER_ENCODING_UTF_8;
        stringsParserParams.tagsHandlers = stringsTagsHandlers;
        stringsParserParams.onErrorCb    = onErrorCb;
        stringsParserParams.userData     = xmlGraphics;
        
        if (parserObj->parse(parserObj, &stringsParserParams) != PARSER_ERROR_NONE) {
            Loge("Failed to parse file");
            xmlGraphics->reserved = NULL;
            return LOADERS_ERROR_XML;
        }
    }
    
    return LOADERS_ERROR_NONE;
}

/*!
 *
 */
LOADERS_ERROR_E unloadGraphicsXml_f(LOADERS_S *obj, XML_GRAPHICS_S *xmlGraphics)
{
    assert(obj && xmlGraphics);
    
    if (xmlGraphics->common.defaultLanguage) {
        free(xmlGraphics->common.defaultLanguage);
        xmlGraphics->common.defaultLanguage = NULL;
    }
    
    if (xmlGraphics->common.colorsXmlFile) {
        free(xmlGraphics->common.colorsXmlFile);
        xmlGraphics->common.colorsXmlFile = NULL;
    }
    
    if (xmlGraphics->common.imagesXmlFile) {
        free(xmlGraphics->common.imagesXmlFile);
        xmlGraphics->common.imagesXmlFile = NULL;
    }
    
    if (xmlGraphics->common.fontsXmlFile) {
        free(xmlGraphics->common.fontsXmlFile);
        xmlGraphics->common.fontsXmlFile = NULL;
    }
    
    if (xmlGraphics->common.stringsXmlFile) {
        free(xmlGraphics->common.stringsXmlFile);
        xmlGraphics->common.stringsXmlFile = NULL;
    }
    
    if (xmlGraphics->screen.name) {
        free(xmlGraphics->screen.name);
        xmlGraphics->screen.name = NULL;
    }
    
    if (xmlGraphics->screen.fbDeviceName) {
        free(xmlGraphics->screen.fbDeviceName);
        xmlGraphics->screen.fbDeviceName = NULL;
    }
    
    if (xmlGraphics->screen.caption) {
        free(xmlGraphics->screen.caption);
        xmlGraphics->screen.caption = NULL;
    }
    
    uint32_t index, handlerIndex;
    
    XML_ELEMENT_S *element;
    for (index = 0; index < xmlGraphics->nbElements; index++) {
        element = &xmlGraphics->elements[index];
        if (element->name) {
            free(element->name);
            element->name = NULL;
        }
        if (element->groupName) {
            free(element->groupName);
            element->groupName = NULL;
        }
        if (element->text) {
            free(element->text);
            element->text = NULL;
        }
        if (element->nav) {
            if (element->nav->left) {
                free(element->nav->left);
                element->nav->left = NULL;
            }
            if (element->nav->up) {
                free(element->nav->up);
                element->nav->up = NULL;
            }
            if (element->nav->right) {
                free(element->nav->right);
                element->nav->right = NULL;
            }
            if (element->nav->down) {
                free(element->nav->down);
                element->nav->down = NULL;
            }
            free(element->nav);
            element->nav = NULL;
        }
        if (element->image) {
            free(element->image);
            element->image = NULL;
        }
        if (element->clickHandlers) {
            XML_ELEMENT_CLICK_S *handler;
            for (handlerIndex = 0; handlerIndex < element->nbClickHandlers; handlerIndex++) {
                handler = &element->clickHandlers[handlerIndex];
                if (handler->name) {
                    free(handler->name);
                    handler->name = NULL;
                }
                if (handler->data) {
                    free(handler->data);
                    handler->data = NULL;
                }
            }
            free(element->clickHandlers);
            element->clickHandlers = NULL;
        }
    }
    
    free(xmlGraphics->elements);
    xmlGraphics->elements = NULL;
    
    XML_COLORS_S *xmlColors = &xmlGraphics->common.xmlColors;
    if (xmlColors->colors) {
        free(xmlColors->colors);
        xmlColors->colors = NULL;
    }
    
    XML_IMAGES_S *xmlImages = &xmlGraphics->common.xmlImages;
    if (xmlImages->images) {
        XML_IMAGE_S *image;
        for (index = 0; index < xmlImages->nbImages; index++) {
            image = &xmlImages->images[index];
            if (image->file) {
                free(image->file);
                image->file = NULL;
            }
        }
        free(xmlImages->images);
        xmlImages->images = NULL;
    }
    
    XML_FONTS_S *xmlFonts = &xmlGraphics->common.xmlFonts;
    if (xmlFonts->fonts) {
        XML_FONT_S *font;
        for (index = 0; index < xmlFonts->nbFonts; index++) {
            font = &xmlFonts->fonts[index];
            if (font->file) {
                free(font->file);
                font->file = NULL;
            }
        }
        free(xmlFonts->fonts);
        xmlFonts->fonts = NULL;
    }
    
    XML_STRINGS_S *xmlStrings = xmlGraphics->common.xmlStrings;
    uint32_t nbXmlStrings     = xmlGraphics->common.nbLanguages;
    uint32_t nbStrings, strCount;
    
    if (xmlStrings) {
        for (index = 0; index < nbXmlStrings; index++) {
            if (xmlStrings[index].language) {
                free(xmlStrings[index].language);
                xmlStrings[index].language = NULL;
            }
            if (xmlStrings[index].strings) {
                nbStrings = xmlStrings[index].nbStrings;
                for (strCount = 0; strCount < nbStrings; strCount++) {
                    if (xmlStrings[index].strings[strCount].str) {
                        free(xmlStrings[index].strings[strCount].str);
                        xmlStrings[index].strings[strCount].str = NULL;
                    }
                }
                free(xmlStrings[index].strings);
                xmlStrings[index].strings = NULL;
            }
        }
        free(xmlStrings);
        xmlStrings = NULL;
    }
    
    xmlGraphics->reserved = NULL;
    
    return LOADERS_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/*                                           CALLBACKS                                          */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static void onCommonCb(void *userData, const char **attrs)
{
    assert(userData);
    
    XML_GRAPHICS_S *xmlGraphics = (XML_GRAPHICS_S*)userData;
    CONTEXT_S *ctx              = (CONTEXT_S*)xmlGraphics->reserved;
    PARSER_S *parserObj         = ctx->parserObj;
    
    PARSER_ATTR_HANDLER_S attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_DEFAULT_LANGUAGE,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&xmlGraphics->common.defaultLanguage,
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
    	Loge("Failed to retrieve attributes in \"Common\" tag");
    }
}

/*!
 *
 */
static void onColorsCb(void *userData, const char **attrs)
{
    assert(userData);
    
    XML_GRAPHICS_S *xmlGraphics = (XML_GRAPHICS_S*)userData;
    CONTEXT_S *ctx              = (CONTEXT_S*)xmlGraphics->reserved;
    PARSER_S *parserObj         = ctx->parserObj;
    
    PARSER_ATTR_HANDLER_S attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_XML_FILE,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&xmlGraphics->common.colorsXmlFile,
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
    	Loge("Failed to retrieve attributes in \"Colors\" tag");
    }
}

/*!
 *
 */
static void onImagesCb(void *userData, const char **attrs)
{
    assert(userData);
    
    XML_GRAPHICS_S *xmlGraphics = (XML_GRAPHICS_S*)userData;
    CONTEXT_S *ctx              = (CONTEXT_S*)xmlGraphics->reserved;
    PARSER_S *parserObj         = ctx->parserObj;
    
    PARSER_ATTR_HANDLER_S attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_XML_FILE,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&xmlGraphics->common.imagesXmlFile,
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
    	Loge("Failed to retrieve attributes in \"Images\" tag");
    }
}

/*!
 *
 */
static void onFontsCb(void *userData, const char **attrs)
{
    assert(userData);
    
    XML_GRAPHICS_S *xmlGraphics = (XML_GRAPHICS_S*)userData;
    CONTEXT_S *ctx              = (CONTEXT_S*)xmlGraphics->reserved;
    PARSER_S *parserObj         = ctx->parserObj;
    
    PARSER_ATTR_HANDLER_S attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_XML_FILE,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&xmlGraphics->common.fontsXmlFile,
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
    	Loge("Failed to retrieve attributes in \"Fonts\" tag");
    }
}

/*!
 *
 */
static void onStringsCb(void *userData, const char **attrs)
{
    assert(userData);
    
    XML_GRAPHICS_S *xmlGraphics = (XML_GRAPHICS_S*)userData;
    CONTEXT_S *ctx              = (CONTEXT_S*)xmlGraphics->reserved;
    PARSER_S *parserObj         = ctx->parserObj;
    
    PARSER_ATTR_HANDLER_S attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_XML_FILE,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&xmlGraphics->common.stringsXmlFile,
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
    	Loge("Failed to retrieve attributes in \"Strings\" tag");
    }
}

/*!
 *
 */
static void onScreenCb(void *userData, const char **attrs)
{
    assert(userData);
    
    XML_GRAPHICS_S *xmlGraphics = (XML_GRAPHICS_S*)userData;
    XML_SCREEN_S *screen        = &xmlGraphics->screen;
    CONTEXT_S *ctx              = (CONTEXT_S*)xmlGraphics->reserved;
    PARSER_S *parserObj         = ctx->parserObj;
    
    PARSER_ATTR_HANDLER_S attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_NAME,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void*)&screen->name,
    	    .attrGetter.vector = parserObj->getString
        },
    	{
    	    .attrName          = XML_ATTR_WIDTH,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&screen->width,
    	    .attrGetter.scalar = parserObj->getUint16
        },
    	{
    	    .attrName          = XML_ATTR_HEIGHT,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&screen->height,
    	    .attrGetter.scalar = parserObj->getUint16
        },
    	{
    	    .attrName          = XML_ATTR_FRAMEBUFFER_DEVICE_NAME,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void*)&screen->fbDeviceName,
    	    .attrGetter.vector = parserObj->getString
        },
    	{
    	    .attrName          = XML_ATTR_BITS_PER_PIXEL,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&screen->bitsPerPixel,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_FULLSCREEN,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&screen->fullscreen,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_SHOW_CURSOR,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&screen->showCursor,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_CAPTION,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&screen->caption,
    	    .attrGetter.vector = parserObj->getString
        },
    	{
    	    .attrName          = XML_ATTR_VIDEOFORMAT,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&screen->videoFormat,
    	    .attrGetter.scalar = parserObj->getUint32
        },
    	{
    	    NULL,
    	    PARSER_ATTR_TYPE_NONE,
    	    NULL,
    	    NULL
        }
    };
    
    if (parserObj->getAttributes(parserObj, attrHandlers, attrs) != PARSER_ERROR_NONE) {
    	Loge("Failed to retrieve attributes in \"Screen\" tag");
    }
}

/*!
 *
 */
static void onBackgroundCb(void *userData, const char **attrs)
{
    assert(userData);
    
    XML_GRAPHICS_S *xmlGraphics = (XML_GRAPHICS_S*)userData;
    XML_SCREEN_S *screen        = &xmlGraphics->screen;
    CONTEXT_S *ctx              = (CONTEXT_S*)xmlGraphics->reserved;
    PARSER_S *parserObj         = ctx->parserObj;
    
    PARSER_ATTR_HANDLER_S attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_USE_COLOR,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&screen->useColor,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_COLOR_ID,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&screen->BgColorId,
    	    .attrGetter.scalar = parserObj->getUint32
        },
    	{
    	    .attrName          = XML_ATTR_IMAGE_ID,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&screen->BgImageId,
    	    .attrGetter.scalar = parserObj->getUint32
        },
    	{
    	    .attrName          = XML_ATTR_HIDDEN_COLOR_ID,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&screen->BgHiddenColorId,
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
    	Loge("Failed to retrieve attributes in \"Background\" tag");
    }
}

/*!
 *
 */
static void onIconCb(void *userData, const char **attrs)
{
    assert(userData);
    
    XML_GRAPHICS_S *xmlGraphics = (XML_GRAPHICS_S*)userData;
    XML_SCREEN_S *screen        = &xmlGraphics->screen;
    CONTEXT_S *ctx              = (CONTEXT_S*)xmlGraphics->reserved;
    PARSER_S *parserObj         = ctx->parserObj;
    
    PARSER_ATTR_HANDLER_S attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_IMAGE_ID,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&screen->iconImageId,
    	    .attrGetter.scalar = parserObj->getUint32
        },
    	{
    	    .attrName          = XML_ATTR_HIDDEN_COLOR_ID,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&screen->iconHiddenColorId,
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
    	Loge("Failed to retrieve attributes in \"Icon\" tag");
    }
}

/*!
 *
 */
static void onElementStartCb(void *userData, const char **attrs)
{
    assert(userData);
    
    XML_GRAPHICS_S *xmlGraphics = (XML_GRAPHICS_S*)userData;
    CONTEXT_S *ctx              = (CONTEXT_S*)xmlGraphics->reserved;
    PARSER_S *parserObj         = ctx->parserObj;
    
    Logd("Adding element %u", (xmlGraphics->nbElements + 1));
    
    xmlGraphics->elements = realloc(xmlGraphics->elements, (xmlGraphics->nbElements + 1) * sizeof(XML_ELEMENT_S));
    assert(xmlGraphics->elements);
    
    memset(&xmlGraphics->elements[xmlGraphics->nbElements], '\0', sizeof(XML_ELEMENT_S));
    
    XML_ELEMENT_S *element = &xmlGraphics->elements[xmlGraphics->nbElements];
    
    PARSER_ATTR_HANDLER_S attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_NAME,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&element->name,
    	    .attrGetter.vector = parserObj->getString
        },
    	{
    	    .attrName          = XML_ATTR_GROUP,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&element->groupName,
    	    .attrGetter.vector = parserObj->getString
        },
    	{
    	    .attrName          = XML_ATTR_REDRAW_GROUP,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&element->redrawGroup,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_TYPE,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&element->type,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_X,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&element->x,
    	    .attrGetter.scalar = parserObj->getUint32
        },
    	{
    	    .attrName          = XML_ATTR_Y,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&element->y,
    	    .attrGetter.scalar = parserObj->getUint32
        },
    	{
    	    .attrName          = XML_ATTR_WIDTH,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&element->width,
    	    .attrGetter.scalar = parserObj->getUint32
        },
    	{
    	    .attrName          = XML_ATTR_HEIGHT,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&element->height,
    	    .attrGetter.scalar = parserObj->getUint32
        },
    	{
    	    NULL,
    	    PARSER_ATTR_TYPE_NONE,
    	    NULL,
    	    NULL
        }
    };
    
    if (parserObj->getAttributes(parserObj, attrHandlers, attrs) != PARSER_ERROR_NONE) {
    	Loge("Failed to retrieve attributes in \"Element main infos\" tag");
    }
}

/*!
 *
 */
static void onElementEndCb(void *userData)
{
    assert(userData);
    
    XML_GRAPHICS_S *xmlGraphics = (XML_GRAPHICS_S*)userData;

    xmlGraphics->nbElements++;
    
    Logd("Element %u added", xmlGraphics->nbElements);
}

/*!
 *
 */
static void onElementConfigCb(void *userData, const char **attrs)
{
    assert(userData);
    
    XML_GRAPHICS_S *xmlGraphics = (XML_GRAPHICS_S*)userData;
    CONTEXT_S *ctx              = (CONTEXT_S*)xmlGraphics->reserved;
    PARSER_S *parserObj         = ctx->parserObj;
    XML_ELEMENT_S *element      = &xmlGraphics->elements[xmlGraphics->nbElements];
    
    PARSER_ATTR_HANDLER_S attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_VISIBLE,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&element->visible,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_CLICKABLE,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&element->clickable,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_FOCUSABLE,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&element->focusable,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_HAS_FOCUS,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&element->hasFocus,
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
    	Loge("Failed to retrieve attributes in \"Element config\" tag");
    }
}

/*!
 *
 */
static void onElementTextCb(void *userData, const char **attrs)
{
    assert(userData);
    
    XML_GRAPHICS_S *xmlGraphics = (XML_GRAPHICS_S*)userData;
    CONTEXT_S *ctx              = (CONTEXT_S*)xmlGraphics->reserved;
    PARSER_S *parserObj         = ctx->parserObj;
    XML_ELEMENT_S *element      = &xmlGraphics->elements[xmlGraphics->nbElements];
    
    assert((element->text = calloc(1, sizeof(XML_ELEMENT_TEXT_S))));
    
    PARSER_ATTR_HANDLER_S attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_STRING_ID,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&element->text->stringId,
    	    .attrGetter.scalar = parserObj->getUint32
        },
    	{
    	    .attrName          = XML_ATTR_FONT_ID,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&element->text->fontId,
    	    .attrGetter.scalar = parserObj->getUint32
        },
    	{
    	    .attrName          = XML_ATTR_COLOR_ID,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&element->text->colorId,
    	    .attrGetter.scalar = parserObj->getUint32
        },
    	{
    	    .attrName          = XML_ATTR_SIZE,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&element->text->size,
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
    	Loge("Failed to retrieve attributes in \"Element text\" tag");
    }
}

/*!
 *
 */
static void onElementNavCb(void *userData, const char **attrs)
{
    assert(userData);
    
    XML_GRAPHICS_S *xmlGraphics = (XML_GRAPHICS_S*)userData;
    CONTEXT_S *ctx              = (CONTEXT_S*)xmlGraphics->reserved;
    PARSER_S *parserObj         = ctx->parserObj;
    XML_ELEMENT_S *element      = &xmlGraphics->elements[xmlGraphics->nbElements];
    
    assert((element->nav = calloc(1, sizeof(XML_ELEMENT_NAV_S))));
    
    PARSER_ATTR_HANDLER_S attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_LEFT,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&element->nav->left,
    	    .attrGetter.vector = parserObj->getString
        },
    	{
    	    .attrName          = XML_ATTR_UP,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&element->nav->up,
    	    .attrGetter.vector = parserObj->getString
        },
    	{
    	    .attrName          = XML_ATTR_RIGHT,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&element->nav->right,
    	    .attrGetter.vector = parserObj->getString
        },
    	{
    	    .attrName          = XML_ATTR_DOWN,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&element->nav->down,
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
    	Loge("Failed to retrieve attributes in \"Element nav\" tag");
    }
}

/*!
 *
 */
static void onElementImageCb(void *userData, const char **attrs)
{
    assert(userData);
    
    XML_GRAPHICS_S *xmlGraphics = (XML_GRAPHICS_S*)userData;
    CONTEXT_S *ctx              = (CONTEXT_S*)xmlGraphics->reserved;
    PARSER_S *parserObj         = ctx->parserObj;
    XML_ELEMENT_S *element      = &xmlGraphics->elements[xmlGraphics->nbElements];
    
    assert((element->image = calloc(1, sizeof(XML_ELEMENT_IMAGE_S))));
    
    PARSER_ATTR_HANDLER_S attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_IMAGE_ID,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&element->image->imageId,
    	    .attrGetter.scalar = parserObj->getUint32
        },
    	{
    	    .attrName          = XML_ATTR_HIDDEN_COLOR_ID,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&element->image->hiddenColorId,
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
    	Loge("Failed to retrieve attributes in \"Element image\" tag");
    }
}

/*!
 *
 */
static void onElementClickStartCb(void *userData, const char **attrs)
{
    assert(userData);

    (void)attrs;
}

/*!
 *
 */
static void onElementClickEndCb(void *userData)
{
    assert(userData);

    XML_GRAPHICS_S *xmlGraphics = (XML_GRAPHICS_S*)userData;
    XML_ELEMENT_S *element      = &xmlGraphics->elements[xmlGraphics->nbElements];

    Logd("%u click handlers added", element->nbClickHandlers);
}

/*!
 *
 */
static void onElementHandlerCb(void *userData, const char **attrs)
{
    assert(userData);

    XML_GRAPHICS_S *xmlGraphics  = (XML_GRAPHICS_S*)userData;
    CONTEXT_S *ctx               = (CONTEXT_S*)xmlGraphics->reserved;
    PARSER_S *parserObj          = ctx->parserObj;
    XML_ELEMENT_S *element       = &xmlGraphics->elements[xmlGraphics->nbElements];

    element->clickHandlers = realloc(element->clickHandlers, (element->nbClickHandlers + 1) * sizeof(XML_ELEMENT_CLICK_S));
    assert(element->clickHandlers);

    memset(&element->clickHandlers[element->nbClickHandlers], '\0', sizeof(XML_ELEMENT_CLICK_S));

    XML_ELEMENT_CLICK_S *handler = &element->clickHandlers[element->nbClickHandlers];

    PARSER_ATTR_HANDLER_S attrHandlers[] = {
        {
            .attrName          = XML_ATTR_NAME,
            .attrType          = PARSER_ATTR_TYPE_VECTOR,
            .attrValue.vector  = (void**)&handler->name,
            .attrGetter.vector = parserObj->getString
        },
        {
            .attrName          = XML_ATTR_DATA,
            .attrType          = PARSER_ATTR_TYPE_VECTOR,
            .attrValue.vector  = (void**)&handler->data,
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
        Loge("Failed to retrieve attributes in \"Element click handler\" tag");
    }

    element->nbClickHandlers++;
}

/*!
 *
 */
static void onFocusCb(void *userData, const char **attrs)
{
    assert(userData);
    
    XML_GRAPHICS_S *xmlGraphics = (XML_GRAPHICS_S*)userData;
    CONTEXT_S *ctx              = (CONTEXT_S*)xmlGraphics->reserved;
    PARSER_S *parserObj         = ctx->parserObj;
    
    PARSER_ATTR_HANDLER_S attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_COLOR_ID,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&xmlGraphics->onFocusColorId,
    	    .attrGetter.scalar = parserObj->getUint32
        },
    	{
    	    NULL,
    	    PARSER_ATTR_TYPE_NONE,
    	    NULL,
    	    NULL
        }
    };
    
    if (parserObj->getAttributes(parserObj, attrHandlers, attrs) != PARSER_ERROR_NONE) {
    	Loge("Failed to retrieve attributes in \"Focus\" tag");
    }
}

/*!
 *
 */
static void onBlurCb(void *userData, const char **attrs)
{
    assert(userData);
    
    XML_GRAPHICS_S *xmlGraphics = (XML_GRAPHICS_S*)userData;
    CONTEXT_S *ctx              = (CONTEXT_S*)xmlGraphics->reserved;
    PARSER_S *parserObj         = ctx->parserObj;
    
    PARSER_ATTR_HANDLER_S attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_COLOR_ID,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&xmlGraphics->onBlurColorId,
    	    .attrGetter.scalar = parserObj->getUint32
        },
    	{
    	    NULL,
    	    PARSER_ATTR_TYPE_NONE,
    	    NULL,
    	    NULL
        }
    };
    
    if (parserObj->getAttributes(parserObj, attrHandlers, attrs) != PARSER_ERROR_NONE) {
    	Loge("Failed to retrieve attributes in \"Blur\" tag");
    }
}

/*!
 *
 */
static void onResetCb(void *userData, const char **attrs)
{
    assert(userData);
    
    XML_GRAPHICS_S *xmlGraphics = (XML_GRAPHICS_S*)userData;
    CONTEXT_S *ctx              = (CONTEXT_S*)xmlGraphics->reserved;
    PARSER_S *parserObj         = ctx->parserObj;
    
    PARSER_ATTR_HANDLER_S attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_COLOR_ID,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&xmlGraphics->onResetColorId,
    	    .attrGetter.scalar = parserObj->getUint32
        },
    	{
    	    NULL,
    	    PARSER_ATTR_TYPE_NONE,
    	    NULL,
    	    NULL
        }
    };
    
    if (parserObj->getAttributes(parserObj, attrHandlers, attrs) != PARSER_ERROR_NONE) {
    	Loge("Failed to retrieve attributes in \"Reset\" tag");
    }
}

/*!
 *
 */
static void onColorCb(void *userData, const char **attrs)
{
    assert(userData);
    
    XML_GRAPHICS_S *xmlGraphics = (XML_GRAPHICS_S*)userData;
    CONTEXT_S *ctx              = (CONTEXT_S*)xmlGraphics->reserved;
    PARSER_S *parserObj         = ctx->parserObj;
    XML_COLORS_S *xmlColors     = &xmlGraphics->common.xmlColors;
    
    Logd("Adding color %u", (xmlColors->nbColors + 1));
    
    xmlColors->colors = realloc(xmlColors->colors, (xmlColors->nbColors + 1) * sizeof(XML_COLOR_S));
    assert(xmlColors->colors);
    
    XML_COLOR_S *color = &xmlColors->colors[xmlColors->nbColors];
    memset(color, '\0', sizeof(XML_COLOR_S));
    
    PARSER_ATTR_HANDLER_S attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_RED,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&color->red,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_GREEN,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&color->green,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_BLUE,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&color->blue,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_ALPHA,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&color->alpha,
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
    	Loge("Failed to retrieve attributes in \"Color\" tag");
    }
    
    xmlColors->nbColors++;
}

/*!
 *
 */
static void onImageCb(void *userData, const char **attrs)
{
    assert(userData);
    
    XML_GRAPHICS_S *xmlGraphics = (XML_GRAPHICS_S*)userData;
    CONTEXT_S *ctx              = (CONTEXT_S*)xmlGraphics->reserved;
    INPUT_S *input              = &ctx->input;
    PARSER_S *parserObj         = ctx->parserObj;
    XML_IMAGES_S *xmlImages     = &xmlGraphics->common.xmlImages;
    
    Logd("Adding image %u", (xmlImages->nbImages + 1));
    
    xmlImages->images = realloc(xmlImages->images, (xmlImages->nbImages + 1) * sizeof(XML_IMAGE_S));
    assert(xmlImages->images);
    
    XML_IMAGE_S *image = &xmlImages->images[xmlImages->nbImages];
    memset(image, '\0', sizeof(XML_IMAGE_S));
    
    PARSER_ATTR_HANDLER_S attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_FILE,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&image->file,
    	    .attrGetter.vector = parserObj->getString
        },
    	{
    	    .attrName          = XML_ATTR_FORMAT,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&image->format,
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
    	Loge("Failed to retrieve attributes in \"Image\" tag");
    }
    
    if (image->file) {
        char *temp   = strdup(image->file);
        uint32_t len = (strlen(temp) + strlen(input->resRootDir) + 2) * sizeof(char);
        
        assert((image->file = realloc(image->file, len)));
        memset(image->file, '\0', len);
        
        snprintf(image->file, len, "%s/%s", input->resRootDir, temp);
        
        free(temp);
        temp = NULL;
    }
    
    xmlImages->nbImages++;
}

/*!
 *
 */
static void onFontCb(void *userData, const char **attrs)
{
    assert(userData);
    
    XML_GRAPHICS_S *xmlGraphics = (XML_GRAPHICS_S*)userData;
    CONTEXT_S *ctx              = (CONTEXT_S*)xmlGraphics->reserved;
    INPUT_S *input              = &ctx->input;
    PARSER_S *parserObj         = ctx->parserObj;
    XML_FONTS_S *xmlFonts       = &xmlGraphics->common.xmlFonts;
    
    Logd("Adding font %u", (xmlFonts->nbFonts + 1));
    
    xmlFonts->fonts = realloc(xmlFonts->fonts, (xmlFonts->nbFonts + 1) * sizeof(XML_FONT_S));
    assert(xmlFonts->fonts);
    
    XML_FONT_S *font = &xmlFonts->fonts[xmlFonts->nbFonts];
    memset(font, '\0', sizeof(XML_FONT_S));
    
    PARSER_ATTR_HANDLER_S attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_FILE,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&font->file,
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
    	Loge("Failed to retrieve attributes in \"Font\" tag");
    }
    
    if (font->file) {
        char *temp   = strdup(font->file);
        uint32_t len = (strlen(temp) + strlen(input->resRootDir) + 2) * sizeof(char);
        
        assert((font->file = realloc(font->file, len)));
        memset(font->file, '\0', len);
        
        snprintf(font->file, len, "%s/%s", input->resRootDir, temp);
        
        free(temp);
        temp = NULL;
    }
    
    xmlFonts->nbFonts++;
}

/*!
 *
 */
static void onStrGroupStartCb(void *userData, const char **attrs)
{
    assert(userData);
    
    XML_GRAPHICS_S *xmlGraphics = (XML_GRAPHICS_S*)userData;
    XML_STRINGS_S **xmlStrings  = (XML_STRINGS_S**)&xmlGraphics->common.xmlStrings;
    uint32_t nbXmlStrings       = xmlGraphics->common.nbLanguages;
    CONTEXT_S *ctx              = (CONTEXT_S*)xmlGraphics->reserved;
    PARSER_S *parserObj         = ctx->parserObj;
    
    Logd("Adding strGroup %u", (nbXmlStrings + 1));
    
    *xmlStrings = realloc(*xmlStrings, (nbXmlStrings + 1) * sizeof(XML_STRINGS_S));
    assert(*xmlStrings);
    
    memset(&(*xmlStrings)[nbXmlStrings], '\0', sizeof(XML_STRINGS_S));
    
    PARSER_ATTR_HANDLER_S attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_LANGUAGE,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&(*xmlStrings)[nbXmlStrings].language,
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
    	Loge("Failed to retrieve attributes in \"StrGroup\" tag");
    }
}

/*!
 *
 */
static void onStringCb(void *userData, const char **attrs)
{
    assert(userData);
    
    XML_GRAPHICS_S *xmlGraphics = (XML_GRAPHICS_S*)userData;
    CONTEXT_S *ctx              = (CONTEXT_S*)xmlGraphics->reserved;
    PARSER_S *parserObj         = ctx->parserObj;
    uint32_t nbXmlStrings       = xmlGraphics->common.nbLanguages;
    XML_STRINGS_S *xmlStrings   = xmlGraphics->common.xmlStrings;
    
    uint32_t *nbStrings         = &xmlStrings[nbXmlStrings].nbStrings;
    XML_STRING_S **strings      = &xmlStrings[nbXmlStrings].strings;
    
    Logd("Adding string %u", (*nbStrings + 1));
    
    *strings = realloc(*strings, (*nbStrings + 1) * sizeof(XML_STRING_S));
    assert(*strings);
    
    memset(&(*strings)[*nbStrings], '\0', sizeof(XML_STRING_S));
    
    PARSER_ATTR_HANDLER_S attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_STR,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&(*strings)[*nbStrings].str,
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
    	Loge("Failed to retrieve attributes in \"String\" tag");
    }
    
    (*nbStrings)++;
}

/*!
 *
 */
static void onStrGroupEndCb(void *userData)
{
    assert(userData);
    
    XML_GRAPHICS_S *xmlGraphics = (XML_GRAPHICS_S*)userData;
    
    (xmlGraphics->common.nbLanguages)++;
    
    Logd("StrGroup %u added", xmlGraphics->common.nbLanguages);
}

/*!
 *
 */
static void onErrorCb(void *userData, int32_t errorCode, const char *errorStr)
{
    (void)userData;
    
    Loge("Parsing error - errorCode = %d / errorStr = \"%s\"", errorCode, errorStr);
}
