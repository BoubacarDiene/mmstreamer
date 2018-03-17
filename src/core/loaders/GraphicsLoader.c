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
* \file GraphicsLoader.c
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
#define TAG "GraphicsLoader"

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////// PUBLIC FUNCTIONS PROTOTYPES //////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum loaders_error_e loadGraphicsXml_f(struct loaders_s *obj, struct context_s *ctx,
                                       struct xml_graphics_s *xmlGraphics);
enum loaders_error_e unloadGraphicsXml_f(struct loaders_s *obj,
                                         struct xml_graphics_s *xmlGraphics);

enum loaders_error_e loadCommonXml_f(struct loaders_s *obj, struct context_s *ctx,
                                     struct xml_common_s *xmlCommon);
enum loaders_error_e unloadCommonXml_f(struct loaders_s *obj, struct xml_common_s *xmlCommon);

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////////// CALLBACKS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

static void onCommonCb(void *userData, const char **attrs);
static void onColorsCb(void *userData, const char **attrs);
static void onImagesCb(void *userData, const char **attrs);
static void onFontsCb(void *userData, const char **attrs);
static void onStringsCb(void *userData, const char **attrs);

static void onScreenCb(void *userData, const char **attrs);
static void onBackgroundCb(void *userData, const char **attrs);
static void onIconCb(void *userData, const char **attrs);
static void onGfxVideoCb(void *userData, const char **attrs);

static void onElementStartCb(void *userData, const char **attrs);
static void onElementEndCb(void *userData);
static void onElementConfigCb(void *userData, const char **attrs);
static void onElementTextCb(void *userData, const char **attrs);
static void onElementNavCb(void *userData, const char **attrs);
static void onElementImageCb(void *userData, const char **attrs);

static void onElementClickStartCb(void *userData, const char **attrs);
static void onElementClickEndCb(void *userData);
static void onElementHandlerCb(void *userData, const char **attrs);

static void onFocusCb(void *userData, const char **attrs);
static void onBlurCb(void *userData, const char **attrs);
static void onResetCb(void *userData, const char **attrs);

static void onColorCb(void *userData, const char **attrs);
static void onImageCb(void *userData, const char **attrs);
static void onFontCb(void *userData, const char **attrs);

static void onStrGroupStartCb(void *userData, const char **attrs);
static void onStringCb(void *userData, const char **attrs);
static void onStrGroupEndCb(void *userData);

static void onErrorCb(void *userData, int32_t errorCode, const char *errorStr);

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////// PUBLIC FUNCTIONS IMPLEMENTATION ///////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
enum loaders_error_e loadGraphicsXml_f(struct loaders_s *obj, struct context_s *ctx,
                                       struct xml_graphics_s *xmlGraphics)
{
    assert(obj && ctx && xmlGraphics);
    
    struct parser_s *parserObj = ctx->parserObj;
    struct input_s *input      = &ctx->input;
    
    xmlGraphics->reserved = ctx;
    
    Logd("Parsing file : \"%s/%s\"", input->resRootDir, input->graphicsConfig.xml);
    
    struct parser_tags_handler_s gfxTagsHandlers[] = {
        { XML_TAG_COMMON,      onCommonCb,             NULL,                 NULL },
        { XML_TAG_COLORS,      onColorsCb,             NULL,                 NULL },
        { XML_TAG_IMAGES,      onImagesCb,             NULL,                 NULL },
        { XML_TAG_FONTS,       onFontsCb,              NULL,                 NULL },
        { XML_TAG_STRINGS,     onStringsCb,            NULL,                 NULL },
        { XML_TAG_SCREEN,      onScreenCb,             NULL,                 NULL },
        { XML_TAG_BACKGROUND,  onBackgroundCb,         NULL,                 NULL },
        { XML_TAG_ICON,        onIconCb,               NULL,                 NULL },
        { XML_TAG_GFX_VIDEO,   onGfxVideoCb,           NULL,                 NULL },
        { XML_TAG_ELEMENT,     onElementStartCb,       onElementEndCb,       NULL },
        { XML_TAG_CONFIG,      onElementConfigCb,      NULL,                 NULL },
        { XML_TAG_TEXT,        onElementTextCb,        NULL,                 NULL },
        { XML_TAG_NAV,         onElementNavCb,         NULL,                 NULL },
        { XML_TAG_IMAGE,       onElementImageCb,       NULL,                 NULL },
        { XML_TAG_ON_CLICK,    onElementClickStartCb,  onElementClickEndCb,  NULL },
        { XML_TAG_HANDLER,     onElementHandlerCb,     NULL,                 NULL },
        { XML_TAG_FOCUS,       onFocusCb,              NULL,                 NULL },
        { XML_TAG_BLUR,        onBlurCb,               NULL,                 NULL },
        { XML_TAG_RESET,       onResetCb,              NULL,                 NULL },
        { NULL,                NULL,                   NULL,                 NULL }
    };
    
    struct parser_params_s gfxParserParams;
    snprintf(gfxParserParams.path, sizeof(gfxParserParams.path), "%s/%s",
                                   input->resRootDir, input->graphicsConfig.xml);
    gfxParserParams.encoding     = PARSER_ENCODING_UTF_8;
    gfxParserParams.tagsHandlers = gfxTagsHandlers;
    gfxParserParams.onErrorCb    = onErrorCb;
    gfxParserParams.userData     = xmlGraphics;
    
    if (parserObj->parse(parserObj, &gfxParserParams) != PARSER_ERROR_NONE) {
        Loge("Failed to parse file");
        xmlGraphics->reserved = NULL;
        return LOADERS_ERROR_XML;
    }

    return LOADERS_ERROR_NONE;
}

/*!
 *
 */
enum loaders_error_e unloadGraphicsXml_f(struct loaders_s *obj, struct xml_graphics_s *xmlGraphics)
{
    assert(obj && xmlGraphics);
    
    if (xmlGraphics->defaultLanguage) {
        free(xmlGraphics->defaultLanguage);
        xmlGraphics->defaultLanguage = NULL;
    }
    
    if (xmlGraphics->colorsXmlFile) {
        free(xmlGraphics->colorsXmlFile);
        xmlGraphics->colorsXmlFile = NULL;
    }
    
    if (xmlGraphics->imagesXmlFile) {
        free(xmlGraphics->imagesXmlFile);
        xmlGraphics->imagesXmlFile = NULL;
    }
    
    if (xmlGraphics->fontsXmlFile) {
        free(xmlGraphics->fontsXmlFile);
        xmlGraphics->fontsXmlFile = NULL;
    }
    
    if (xmlGraphics->stringsXmlFile) {
        free(xmlGraphics->stringsXmlFile);
        xmlGraphics->stringsXmlFile = NULL;
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
    
    if (xmlGraphics->screen.gfxVideo.name) {
        free(xmlGraphics->screen.gfxVideo.name);
        xmlGraphics->screen.gfxVideo.name = NULL;
    }
    
    uint32_t index, handlerIndex;
    
    struct xml_element_s *element;
    for (index = 0; index < xmlGraphics->nbElements; index++) {
        element = &xmlGraphics->elements[index];
        if (element->name) {
            free(element->name);
        }
        if (element->groupName) {
            free(element->groupName);
        }
        if (element->text) {
            free(element->text);
        }
        if (element->nav) {
            if (element->nav->left) {
                free(element->nav->left);
            }
            if (element->nav->up) {
                free(element->nav->up);
            }
            if (element->nav->right) {
                free(element->nav->right);
            }
            if (element->nav->down) {
                free(element->nav->down);
            }
            free(element->nav);
        }
        if (element->image) {
            free(element->image);
        }
        if (element->clickHandlers) {
            struct xml_element_click_s *handler;
            for (handlerIndex = 0; handlerIndex < element->nbClickHandlers; handlerIndex++) {
                handler = &element->clickHandlers[handlerIndex];
                if (handler->name) {
                    free(handler->name);
                }
                if (handler->data) {
                    free(handler->data);
                }
            }
            free(element->clickHandlers);
        }
    }
    
    free(xmlGraphics->elements);
    xmlGraphics->elements = NULL;
    
    xmlGraphics->reserved = NULL;
    
    return LOADERS_ERROR_NONE;
}

/*!
 *
 */
enum loaders_error_e loadCommonXml_f(struct loaders_s *obj, struct context_s *ctx,
                                     struct xml_common_s *xmlCommon)
{
    assert(obj && ctx && xmlCommon);
    
    struct parser_s *parserObj = ctx->parserObj;
    struct input_s *input      = &ctx->input;
    
    xmlCommon->reserved = ctx;
    
    if (xmlCommon->files.colors[0] != '\0') {
        Logd("Parsing file : \"%s/%s\"", input->resRootDir, xmlCommon->files.colors);
        
        struct parser_tags_handler_s colorsTagsHandlers[] = {
        	{ XML_TAG_COLOR,  onColorCb,  NULL,  NULL },
        	{ NULL,           NULL,       NULL,  NULL }
        };
        
        struct parser_params_s colorsParserParams;
        snprintf(colorsParserParams.path, sizeof(colorsParserParams.path), "%s/%s",
                                          input->resRootDir, xmlCommon->files.colors);
        colorsParserParams.encoding     = PARSER_ENCODING_UTF_8;
        colorsParserParams.tagsHandlers = colorsTagsHandlers;
        colorsParserParams.onErrorCb    = onErrorCb;
        colorsParserParams.userData     = xmlCommon;
        
        if (parserObj->parse(parserObj, &colorsParserParams) != PARSER_ERROR_NONE) {
            Loge("Failed to parse file");
            xmlCommon->reserved = NULL;
            return LOADERS_ERROR_XML;
        }
    }
    
    if (xmlCommon->files.images[0] != '\0') {
        Logd("Parsing file : \"%s/%s\"", input->resRootDir, xmlCommon->files.images);
        
        struct parser_tags_handler_s imagesTagsHandlers[] = {
        	{ XML_TAG_IMAGE,  onImageCb,  NULL,  NULL },
        	{ NULL,           NULL,       NULL,  NULL }
        };
        
        struct parser_params_s imagesParserParams;
        snprintf(imagesParserParams.path, sizeof(imagesParserParams.path), "%s/%s",
                                          input->resRootDir, xmlCommon->files.images);
        imagesParserParams.encoding     = PARSER_ENCODING_UTF_8;
        imagesParserParams.tagsHandlers = imagesTagsHandlers;
        imagesParserParams.onErrorCb    = onErrorCb;
        imagesParserParams.userData     = xmlCommon;
        
        if (parserObj->parse(parserObj, &imagesParserParams) != PARSER_ERROR_NONE) {
            Loge("Failed to parse file");
            xmlCommon->reserved = NULL;
            return LOADERS_ERROR_XML;
        }
    }
    
    if (xmlCommon->files.fonts[0] != '\0') {
        Logd("Parsing file : \"%s/%s\"", input->resRootDir, xmlCommon->files.fonts);
        
        struct parser_tags_handler_s fontsTagsHandlers[] = {
        	{ XML_TAG_FONT,  onFontCb,  NULL,  NULL },
        	{ NULL,          NULL,      NULL,  NULL }
        };
        
        struct parser_params_s fontsParserParams;
        snprintf(fontsParserParams.path, sizeof(fontsParserParams.path), "%s/%s",
                                         input->resRootDir, xmlCommon->files.fonts);
        fontsParserParams.encoding     = PARSER_ENCODING_UTF_8;
        fontsParserParams.tagsHandlers = fontsTagsHandlers;
        fontsParserParams.onErrorCb    = onErrorCb;
        fontsParserParams.userData     = xmlCommon;
        
        if (parserObj->parse(parserObj, &fontsParserParams) != PARSER_ERROR_NONE) {
            Loge("Failed to parse file");
            xmlCommon->reserved = NULL;
            return LOADERS_ERROR_XML;
        }
    }
    
    if (xmlCommon->files.strings[0] != '\0') {
        Logd("Parsing file : \"%s/%s\"", input->resRootDir, xmlCommon->files.strings);
        
        struct parser_tags_handler_s stringsTagsHandlers[] = {
        	{ XML_TAG_STR_GROUP,  onStrGroupStartCb,  onStrGroupEndCb,  NULL },
        	{ XML_TAG_STRING,     onStringCb,         NULL,             NULL },
        	{ NULL,               NULL,               NULL,             NULL }
        };
        
        struct parser_params_s stringsParserParams;
        snprintf(stringsParserParams.path, sizeof(stringsParserParams.path), "%s/%s",
                                           input->resRootDir, xmlCommon->files.strings);
        stringsParserParams.encoding     = PARSER_ENCODING_UTF_8;
        stringsParserParams.tagsHandlers = stringsTagsHandlers;
        stringsParserParams.onErrorCb    = onErrorCb;
        stringsParserParams.userData     = xmlCommon;
        
        if (parserObj->parse(parserObj, &stringsParserParams) != PARSER_ERROR_NONE) {
            Loge("Failed to parse file");
            xmlCommon->reserved = NULL;
            return LOADERS_ERROR_XML;
        }
    }
    
    return LOADERS_ERROR_NONE;
}

/*!
 *
 */
enum loaders_error_e unloadCommonXml_f(struct loaders_s *obj, struct xml_common_s *xmlCommon)
{
    assert(obj && xmlCommon);

    uint32_t index;

    struct xml_colors_s *xmlColors = &xmlCommon->xmlColors;
    if (xmlColors->colors) {
        free(xmlColors->colors);
        xmlColors->colors = NULL;
    }
    
    struct xml_images_s *xmlImages = &xmlCommon->xmlImages;
    if (xmlImages->images) {
        struct xml_image_s *image;
        for (index = 0; index < xmlImages->nbImages; index++) {
            image = &xmlImages->images[index];
            if (image->file) {
                free(image->file);
            }
        }
        free(xmlImages->images);
        xmlImages->images = NULL;
    }
    
    struct xml_fonts_s *xmlFonts = &xmlCommon->xmlFonts;
    if (xmlFonts->fonts) {
        struct xml_font_s *font;
        for (index = 0; index < xmlFonts->nbFonts; index++) {
            font = &xmlFonts->fonts[index];
            if (font->file) {
                free(font->file);
            }
        }
        free(xmlFonts->fonts);
        xmlFonts->fonts = NULL;
    }
    
    struct xml_strings_s *xmlStrings = xmlCommon->xmlStrings;
    uint32_t nbXmlStrings            = xmlCommon->nbLanguages;
    uint32_t nbStrings, strCount;
    
    if (xmlStrings) {
        for (index = 0; index < nbXmlStrings; index++) {
            if (xmlStrings[index].language) {
                free(xmlStrings[index].language);
            }
            if (xmlStrings[index].strings) {
                nbStrings = xmlStrings[index].nbStrings;
                for (strCount = 0; strCount < nbStrings; strCount++) {
                    if (xmlStrings[index].strings[strCount].str) {
                        free(xmlStrings[index].strings[strCount].str);
                    }
                }
                free(xmlStrings[index].strings);
            }
        }
        free(xmlStrings);
    }
    
    xmlCommon->reserved = NULL;
    
    return LOADERS_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////////// CALLBACKS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static void onCommonCb(void *userData, const char **attrs)
{
    assert(userData);
    
    struct xml_graphics_s *xmlGraphics = (struct xml_graphics_s*)userData;
    struct context_s *ctx              = (struct context_s*)xmlGraphics->reserved;
    struct parser_s *parserObj         = ctx->parserObj;
    
    struct parser_attr_handler_s attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_DEFAULT_LANGUAGE,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&xmlGraphics->defaultLanguage,
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
    
    struct xml_graphics_s *xmlGraphics = (struct xml_graphics_s*)userData;
    struct context_s *ctx              = (struct context_s*)xmlGraphics->reserved;
    struct parser_s *parserObj         = ctx->parserObj;
    
    struct parser_attr_handler_s attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_XML_FILE,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&xmlGraphics->colorsXmlFile,
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
    
    struct xml_graphics_s *xmlGraphics = (struct xml_graphics_s*)userData;
    struct context_s *ctx              = (struct context_s*)xmlGraphics->reserved;
    struct parser_s *parserObj         = ctx->parserObj;
    
    struct parser_attr_handler_s attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_XML_FILE,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&xmlGraphics->imagesXmlFile,
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
    
    struct xml_graphics_s *xmlGraphics = (struct xml_graphics_s*)userData;
    struct context_s *ctx              = (struct context_s*)xmlGraphics->reserved;
    struct parser_s *parserObj         = ctx->parserObj;
    
    struct parser_attr_handler_s attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_XML_FILE,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&xmlGraphics->fontsXmlFile,
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
    
    struct xml_graphics_s *xmlGraphics = (struct xml_graphics_s*)userData;
    struct context_s *ctx              = (struct context_s*)xmlGraphics->reserved;
    struct parser_s *parserObj         = ctx->parserObj;
    
    struct parser_attr_handler_s attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_XML_FILE,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&xmlGraphics->stringsXmlFile,
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
    
    struct xml_graphics_s *xmlGraphics = (struct xml_graphics_s*)userData;
    struct xml_screen_s *screen        = &xmlGraphics->screen;
    struct context_s *ctx              = (struct context_s*)xmlGraphics->reserved;
    struct parser_s *parserObj         = ctx->parserObj;
    
    struct parser_attr_handler_s attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_NAME,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&screen->name,
    	    .attrGetter.vector = parserObj->getString
        },
    	{
    	    .attrName          = XML_ATTR_WIDTH,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&screen->width,
    	    .attrGetter.scalar = parserObj->getInt32
        },
    	{
    	    .attrName          = XML_ATTR_HEIGHT,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&screen->height,
    	    .attrGetter.scalar = parserObj->getInt32
        },
    	{
    	    .attrName          = XML_ATTR_FRAMEBUFFER_DEVICE_NAME,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&screen->fbDeviceName,
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
    
    struct xml_graphics_s *xmlGraphics = (struct xml_graphics_s*)userData;
    struct xml_screen_s *screen        = &xmlGraphics->screen;
    struct context_s *ctx              = (struct context_s*)xmlGraphics->reserved;
    struct parser_s *parserObj         = ctx->parserObj;
    
    struct parser_attr_handler_s attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_USE_COLOR,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&screen->useColor,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_COLOR_ID,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&screen->bgColorId,
    	    .attrGetter.scalar = parserObj->getUint32
        },
    	{
    	    .attrName          = XML_ATTR_IMAGE_ID,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&screen->bgImageId,
    	    .attrGetter.scalar = parserObj->getUint32
        },
    	{
    	    .attrName          = XML_ATTR_HIDDEN_COLOR_ID,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&screen->bgHiddenColorId,
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
    
    struct xml_graphics_s *xmlGraphics = (struct xml_graphics_s*)userData;
    struct xml_screen_s *screen        = &xmlGraphics->screen;
    struct context_s *ctx              = (struct context_s*)xmlGraphics->reserved;
    struct parser_s *parserObj         = ctx->parserObj;
    
    struct parser_attr_handler_s attrHandlers[] = {
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
static void onGfxVideoCb(void *userData, const char **attrs)
{
    assert(userData);
    
    struct xml_graphics_s *xmlGraphics = (struct xml_graphics_s*)userData;
    struct xml_gfx_video_s *gfxVideo   = &xmlGraphics->screen.gfxVideo;
    struct context_s *ctx              = (struct context_s*)xmlGraphics->reserved;
    struct parser_s *parserObj         = ctx->parserObj;
    
    struct parser_attr_handler_s attrHandlers[] = {
    	{
    	    .attrName          = XML_ATTR_NAME,
    	    .attrType          = PARSER_ATTR_TYPE_VECTOR,
    	    .attrValue.vector  = (void**)&gfxVideo->name,
    	    .attrGetter.vector = parserObj->getString
        },
    	{
    	    .attrName          = XML_ATTR_PIXEL_FORMAT,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&gfxVideo->pixelFormat,
    	    .attrGetter.scalar = parserObj->getUint32
        },
    	{
    	    .attrName          = XML_ATTR_USE_COLOR,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&gfxVideo->useColor,
    	    .attrGetter.scalar = parserObj->getUint8
        },
    	{
    	    .attrName          = XML_ATTR_COLOR_ID,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&gfxVideo->bgColorId,
    	    .attrGetter.scalar = parserObj->getUint32
        },
    	{
    	    .attrName          = XML_ATTR_IMAGE_ID,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&gfxVideo->bgImageId,
    	    .attrGetter.scalar = parserObj->getUint32
        },
    	{
    	    .attrName          = XML_ATTR_HIDDEN_COLOR_ID,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&gfxVideo->bgHiddenColorId,
    	    .attrGetter.scalar = parserObj->getInt32
        },
    	{
    	    .attrName          = XML_ATTR_X,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&gfxVideo->x,
    	    .attrGetter.scalar = parserObj->getInt32
        },
    	{
    	    .attrName          = XML_ATTR_Y,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&gfxVideo->y,
    	    .attrGetter.scalar = parserObj->getInt32
        },
    	{
    	    .attrName          = XML_ATTR_WIDTH,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&gfxVideo->width,
    	    .attrGetter.scalar = parserObj->getInt32
        },
    	{
    	    .attrName          = XML_ATTR_HEIGHT,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&gfxVideo->height,
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
    
    struct xml_graphics_s *xmlGraphics = (struct xml_graphics_s*)userData;
    struct context_s *ctx              = (struct context_s*)xmlGraphics->reserved;
    struct parser_s *parserObj         = ctx->parserObj;
    
    Logd("Adding element %u", (xmlGraphics->nbElements + 1));
    
    xmlGraphics->elements = realloc(xmlGraphics->elements,
                                    (xmlGraphics->nbElements + 1) * sizeof(struct xml_element_s));
    assert(xmlGraphics->elements);
    
    memset(&xmlGraphics->elements[xmlGraphics->nbElements], 0, sizeof(struct xml_element_s));
    
    struct xml_element_s *element = &xmlGraphics->elements[xmlGraphics->nbElements];
    
    struct parser_attr_handler_s attrHandlers[] = {
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
    	    .attrGetter.scalar = parserObj->getInt32
        },
    	{
    	    .attrName          = XML_ATTR_Y,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&element->y,
    	    .attrGetter.scalar = parserObj->getInt32
        },
    	{
    	    .attrName          = XML_ATTR_WIDTH,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&element->width,
    	    .attrGetter.scalar = parserObj->getInt32
        },
    	{
    	    .attrName          = XML_ATTR_HEIGHT,
    	    .attrType          = PARSER_ATTR_TYPE_SCALAR,
    	    .attrValue.scalar  = (void*)&element->height,
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
    	Loge("Failed to retrieve attributes in \"Element main infos\" tag");
    }
}

/*!
 *
 */
static void onElementEndCb(void *userData)
{
    assert(userData);
    
    struct xml_graphics_s *xmlGraphics = (struct xml_graphics_s*)userData;

    xmlGraphics->nbElements++;
    
    Logd("Element %u added", xmlGraphics->nbElements);
}

/*!
 *
 */
static void onElementConfigCb(void *userData, const char **attrs)
{
    assert(userData);
    
    struct xml_graphics_s *xmlGraphics = (struct xml_graphics_s*)userData;
    struct context_s *ctx              = (struct context_s*)xmlGraphics->reserved;
    struct parser_s *parserObj         = ctx->parserObj;
    struct xml_element_s *element      = &xmlGraphics->elements[xmlGraphics->nbElements];
    
    struct parser_attr_handler_s attrHandlers[] = {
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
    
    struct xml_graphics_s *xmlGraphics = (struct xml_graphics_s*)userData;
    struct context_s *ctx              = (struct context_s*)xmlGraphics->reserved;
    struct parser_s *parserObj         = ctx->parserObj;
    struct xml_element_s *element      = &xmlGraphics->elements[xmlGraphics->nbElements];
    
    assert((element->text = calloc(1, sizeof(struct xml_element_text_s))));
    
    struct parser_attr_handler_s attrHandlers[] = {
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
    
    struct xml_graphics_s *xmlGraphics = (struct xml_graphics_s*)userData;
    struct context_s *ctx              = (struct context_s*)xmlGraphics->reserved;
    struct parser_s *parserObj         = ctx->parserObj;
    struct xml_element_s *element      = &xmlGraphics->elements[xmlGraphics->nbElements];
    
    assert((element->nav = calloc(1, sizeof(struct xml_element_nav_s))));
    
    struct parser_attr_handler_s attrHandlers[] = {
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
    
    struct xml_graphics_s *xmlGraphics = (struct xml_graphics_s*)userData;
    struct context_s *ctx              = (struct context_s*)xmlGraphics->reserved;
    struct parser_s *parserObj         = ctx->parserObj;
    struct xml_element_s *element      = &xmlGraphics->elements[xmlGraphics->nbElements];
    
    assert((element->image = calloc(1, sizeof(struct xml_element_image_s))));
    
    struct parser_attr_handler_s attrHandlers[] = {
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

    struct xml_graphics_s *xmlGraphics = (struct xml_graphics_s*)userData;
    struct xml_element_s *element      = &xmlGraphics->elements[xmlGraphics->nbElements];

    Logd("%u click handlers added", element->nbClickHandlers);
}

/*!
 *
 */
static void onElementHandlerCb(void *userData, const char **attrs)
{
    assert(userData);

    struct xml_graphics_s *xmlGraphics = (struct xml_graphics_s*)userData;
    struct context_s *ctx              = (struct context_s*)xmlGraphics->reserved;
    struct parser_s *parserObj         = ctx->parserObj;
    struct xml_element_s *element      = &xmlGraphics->elements[xmlGraphics->nbElements];

    element->clickHandlers = realloc(element->clickHandlers,
                                     (element->nbClickHandlers + 1)
                                     * sizeof(struct xml_element_click_s));
    assert(element->clickHandlers);

    memset(&element->clickHandlers[element->nbClickHandlers],
                       0, sizeof(struct xml_element_click_s));

    struct xml_element_click_s *handler = &element->clickHandlers[element->nbClickHandlers];

    struct parser_attr_handler_s attrHandlers[] = {
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
    
    struct xml_graphics_s *xmlGraphics = (struct xml_graphics_s*)userData;
    struct context_s *ctx              = (struct context_s*)xmlGraphics->reserved;
    struct parser_s *parserObj         = ctx->parserObj;
    
    struct parser_attr_handler_s attrHandlers[] = {
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
    
    struct xml_graphics_s *xmlGraphics = (struct xml_graphics_s*)userData;
    struct context_s *ctx              = (struct context_s*)xmlGraphics->reserved;
    struct parser_s *parserObj         = ctx->parserObj;
    
    struct parser_attr_handler_s attrHandlers[] = {
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
    
    struct xml_graphics_s *xmlGraphics = (struct xml_graphics_s*)userData;
    struct context_s *ctx              = (struct context_s*)xmlGraphics->reserved;
    struct parser_s *parserObj         = ctx->parserObj;
    
    struct parser_attr_handler_s attrHandlers[] = {
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
    
    struct xml_common_s *xmlCommon = (struct xml_common_s*)userData;
    struct context_s *ctx          = (struct context_s*)xmlCommon->reserved;
    struct parser_s *parserObj     = ctx->parserObj;
    struct xml_colors_s *xmlColors = &xmlCommon->xmlColors;
    
    Logd("Adding color %u", (xmlColors->nbColors + 1));
    
    xmlColors->colors = realloc(xmlColors->colors,
                                (xmlColors->nbColors + 1) * sizeof(struct xml_color_s));
    assert(xmlColors->colors);
    
    struct xml_color_s *color = &xmlColors->colors[xmlColors->nbColors];
    memset(color, 0, sizeof(struct xml_color_s));
    
    struct parser_attr_handler_s attrHandlers[] = {
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
    
    struct xml_common_s *xmlCommon = (struct xml_common_s*)userData;
    struct context_s *ctx          = (struct context_s*)xmlCommon->reserved;
    struct input_s *input          = &ctx->input;
    struct parser_s *parserObj     = ctx->parserObj;
    struct xml_images_s *xmlImages = &xmlCommon->xmlImages;
    
    Logd("Adding image %u", (xmlImages->nbImages + 1));
    
    xmlImages->images = realloc(xmlImages->images,
                                (xmlImages->nbImages + 1) * sizeof(struct xml_image_s));
    assert(xmlImages->images);
    
    struct xml_image_s *image = &xmlImages->images[xmlImages->nbImages];
    memset(image, 0, sizeof(struct xml_image_s));
    
    struct parser_attr_handler_s attrHandlers[] = {
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
    }
    
    xmlImages->nbImages++;
}

/*!
 *
 */
static void onFontCb(void *userData, const char **attrs)
{
    assert(userData);
    
    struct xml_common_s *xmlCommon = (struct xml_common_s*)userData;
    struct context_s *ctx          = (struct context_s*)xmlCommon->reserved;
    struct input_s *input          = &ctx->input;
    struct parser_s *parserObj     = ctx->parserObj;
    struct xml_fonts_s *xmlFonts   = &xmlCommon->xmlFonts;
    
    Logd("Adding font %u", (xmlFonts->nbFonts + 1));
    
    xmlFonts->fonts = realloc(xmlFonts->fonts,
                              (xmlFonts->nbFonts + 1) * sizeof(struct xml_font_s));
    assert(xmlFonts->fonts);
    
    struct xml_font_s *font = &xmlFonts->fonts[xmlFonts->nbFonts];
    memset(font, 0, sizeof(struct xml_font_s));
    
    struct parser_attr_handler_s attrHandlers[] = {
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
    }
    
    xmlFonts->nbFonts++;
}

/*!
 *
 */
static void onStrGroupStartCb(void *userData, const char **attrs)
{
    assert(userData);
    
    struct xml_common_s *xmlCommon    = (struct xml_common_s*)userData;
    struct xml_strings_s **xmlStrings = (struct xml_strings_s**)&xmlCommon->xmlStrings;
    uint32_t nbXmlStrings             = xmlCommon->nbLanguages;
    struct context_s *ctx             = (struct context_s*)xmlCommon->reserved;
    struct parser_s *parserObj        = ctx->parserObj;
    
    Logd("Adding strGroup %u", (nbXmlStrings + 1));
    
    *xmlStrings = realloc(*xmlStrings, (nbXmlStrings + 1) * sizeof(struct xml_strings_s));
    assert(*xmlStrings);
    
    memset(&(*xmlStrings)[nbXmlStrings], 0, sizeof(struct xml_strings_s));
    
    struct parser_attr_handler_s attrHandlers[] = {
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
    
    struct xml_common_s *xmlCommon   = (struct xml_common_s*)userData;
    struct context_s *ctx            = (struct context_s*)xmlCommon->reserved;
    struct parser_s *parserObj       = ctx->parserObj;
    uint32_t nbXmlStrings            = xmlCommon->nbLanguages;
    struct xml_strings_s *xmlStrings = xmlCommon->xmlStrings;
    
    uint32_t *nbStrings           = &xmlStrings[nbXmlStrings].nbStrings;
    struct xml_string_s **strings = &xmlStrings[nbXmlStrings].strings;
    
    Logd("Adding string %u", (*nbStrings + 1));
    
    *strings = realloc(*strings, (*nbStrings + 1) * sizeof(struct xml_string_s));
    assert(*strings);
    
    memset(&(*strings)[*nbStrings], 0, sizeof(struct xml_string_s));
    
    struct parser_attr_handler_s attrHandlers[] = {
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
    
    struct xml_common_s *xmlCommon = (struct xml_common_s*)userData;
    
    (xmlCommon->nbLanguages)++;
    
    Logd("StrGroup %u added", xmlCommon->nbLanguages);
}

/*!
 *
 */
static void onErrorCb(void *userData, int32_t errorCode, const char *errorStr)
{
    (void)userData;
    
    Loge("Parsing error - errorCode = %d / errorStr = \"%s\"", errorCode, errorStr);
}
