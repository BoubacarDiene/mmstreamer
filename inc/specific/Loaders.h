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
* \file   Loaders.h
* \author Boubacar DIENE
*/

#ifndef __LOADERS_H__
#define __LOADERS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include "specific/Common.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                             */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

typedef enum   LOADERS_ERROR_E     LOADERS_ERROR_E;

typedef struct XML_COLOR_S         XML_COLOR_S;
typedef struct XML_COLORS_S        XML_COLORS_S;

typedef struct XML_FONT_S          XML_FONT_S;
typedef struct XML_FONTS_S         XML_FONTS_S;

typedef struct XML_IMAGE_S         XML_IMAGE_S;
typedef struct XML_IMAGES_S        XML_IMAGES_S;

typedef struct XML_STRING_S        XML_STRING_S;
typedef struct XML_STRINGS_S       XML_STRINGS_S;

typedef struct XML_VIDEO_S         XML_VIDEO_S;

typedef struct XML_COMMON_S        XML_COMMON_S;
typedef struct XML_SCREEN_S        XML_SCREEN_S;
typedef struct XML_ELEMENT_TEXT_S  XML_ELEMENT_TEXT_S;
typedef struct XML_ELEMENT_IMAGE_S XML_ELEMENT_IMAGE_S;
typedef struct XML_ELEMENT_NAV_S   XML_ELEMENT_NAV_S;
typedef struct XML_ELEMENT_S       XML_ELEMENT_S;
typedef struct XML_GRAPHICS_S      XML_GRAPHICS_S;

typedef struct XML_SERVER_S        XML_SERVER_S;
typedef struct XML_SERVERS_S       XML_SERVERS_S;

typedef struct XML_CLIENT_S        XML_CLIENT_S;
typedef struct XML_CLIENTS_S       XML_CLIENTS_S;

typedef struct XML_S               XML_S;

typedef struct LOADERS_S           LOADERS_S;

typedef LOADERS_ERROR_E (*LOADERS_LOAD_GRAPHICS_XML_F  )(LOADERS_S *obj, CONTEXT_S *ctx, XML_GRAPHICS_S *xmlGraphics);
typedef LOADERS_ERROR_E (*LOADERS_UNLOAD_GRAPHICS_XML_F)(LOADERS_S *obj, XML_GRAPHICS_S *xmlGraphics);

typedef LOADERS_ERROR_E (*LOADERS_LOAD_VIDEO_XML_F  )(LOADERS_S *obj, CONTEXT_S *ctx, XML_VIDEO_S *xmlVideo);
typedef LOADERS_ERROR_E (*LOADERS_UNLOAD_VIDEO_XML_F)(LOADERS_S *obj, XML_VIDEO_S *xmlVideo);

typedef LOADERS_ERROR_E (*LOADERS_LOAD_SERVERS_XML_F  )(LOADERS_S *obj, CONTEXT_S *ctx, XML_SERVERS_S *xmlServers);
typedef LOADERS_ERROR_E (*LOADERS_UNLOAD_SERVERS_XML_F)(LOADERS_S *obj, XML_SERVERS_S *xmlServers);

typedef LOADERS_ERROR_E (*LOADERS_LOAD_CLIENTS_XML_F  )(LOADERS_S *obj, CONTEXT_S *ctx, XML_CLIENTS_S *xmlClients);
typedef LOADERS_ERROR_E (*LOADERS_UNLOAD_CLIENTS_XML_F)(LOADERS_S *obj, XML_CLIENTS_S *xmlClients);

enum LOADERS_ERROR_E {
    LOADERS_ERROR_NONE,
    LOADERS_ERROR_INIT,
    LOADERS_ERROR_UNINIT,
    LOADERS_ERROR_XML
};

struct XML_COLOR_S {
    uint32_t id;
    uint8_t  red;
    uint8_t  green;
    uint8_t  blue;
};

struct XML_COLORS_S {
    uint32_t    nbColors;
    XML_COLOR_S *colors;
};

struct XML_FONT_S {
    uint32_t id;
    char     *file;
};

struct XML_FONTS_S {
    uint32_t   nbFonts;
    XML_FONT_S *fonts;
};

struct XML_IMAGE_S {
    uint32_t id;
    char     *file;
    uint8_t  format;
};

struct XML_IMAGES_S {
    uint32_t    nbImages;
    XML_IMAGE_S *images;
};

struct XML_STRING_S {
    uint32_t id;
    char     *str;
};

struct XML_STRINGS_S {
    char         *language;
    uint32_t     nbStrings;
    XML_STRING_S *strings;
};

struct XML_VIDEO_S {
    uint8_t  priority;
    uint32_t configChoice;
    char     *graphicsDest;
    char     *serverDest;
    
    char     *deviceSrc;
    uint16_t deviceWidth;
    uint16_t deviceHeight;
    
    uint16_t outputWidth;
    uint16_t outputHeight;
    
    uint8_t  nbBuffers;
    uint8_t  desiredFps;
    
    void     *reserved;
};

struct XML_COMMON_S {
    uint32_t      nbLanguages;
    char          *defaultLanguage;
    
    char          *colorsXmlFile;
    char          *imagesXmlFile;
    char          *fontsXmlFile;
    char          *stringsXmlFile;
    
    XML_COLORS_S  xmlColors;
    XML_FONTS_S   xmlFonts;
    XML_IMAGES_S  xmlImages;
    XML_STRINGS_S *xmlStrings;
};

struct XML_SCREEN_S {
    uint16_t width;
    uint16_t height;
    uint8_t  bitsPerPixel;
    uint8_t  fullscreen;
    char     *caption;
    
    uint8_t  useColor;
    uint32_t BgColorId;
    
    uint32_t BgImageId;
    int32_t  BgHiddenColorId;
    
    uint32_t iconImageId;
    int32_t  iconHiddenColorId;
    
    uint32_t videoFormat;
};

struct XML_ELEMENT_TEXT_S {
    uint32_t stringId;
    uint32_t fontId;
    uint32_t colorId;
    uint8_t  size;
};

struct XML_ELEMENT_IMAGE_S {
    uint32_t imageId;
    int32_t  hiddenColorId;
};

struct XML_ELEMENT_NAV_S {
    char *left;
    char *up;
    char *right;
    char *down;
};

struct XML_ELEMENT_S {
    char                *name;
    char                *groupName;
    
    uint8_t             redrawGroup;
    
    uint8_t             type;
    
    uint32_t            x;
    uint32_t            y;
    uint32_t            width;
    uint32_t            height;
    
    uint8_t             visible;
    uint8_t             clickable;
    uint8_t             focusable;
    uint8_t             hasFocus;
    
    XML_ELEMENT_TEXT_S  *text;
    XML_ELEMENT_IMAGE_S *image;
    XML_ELEMENT_NAV_S   *nav;
    
    char                *clickHandlerName;
};

struct XML_GRAPHICS_S {
    XML_COMMON_S  common;
    
    uint32_t      onFocusColorId;
    uint32_t      onBlurColorId;
    uint32_t      onResetColorId;
    
    XML_SCREEN_S  screen;
    
    uint32_t      nbElements;
    XML_ELEMENT_S *elements;
    
    void         *reserved;
};

struct XML_SERVER_S {
    char     *name;
    uint8_t  type;
    uint8_t  link;
    uint8_t  mode;
    uint8_t  acceptMode;
    uint8_t  priority;
    uint32_t maxClients;
    char     *mime;
    int32_t  maxBufferSize;
    
    char     *host;
    char     *service;
    char     *path;
    
    char     *socketName;
};

struct XML_SERVERS_S {
    uint8_t      nbServers;
    XML_SERVER_S *servers;
    
    void         *reserved;
};

struct XML_CLIENT_S {
    char     *name;
    uint8_t  type;
    uint8_t  link;
    uint8_t  mode;
    uint8_t  priority;
    char     *graphicsDest;
    char     *serverDest;
    
    char     *serverHost;
    char     *serverService;
    char     *serverPath;
    
    char     *serverSocketName;
};

struct XML_CLIENTS_S {
    uint8_t      nbClients;
    XML_CLIENT_S *clients;
    
    void         *reserved;
};

struct XML_S {
    XML_GRAPHICS_S xmlGraphics;
    XML_VIDEO_S    xmlVideo;
    XML_SERVERS_S  xmlServers;
    XML_CLIENTS_S  xmlClients;
};

struct LOADERS_S {
    LOADERS_LOAD_GRAPHICS_XML_F   loadGraphicsXml;
    LOADERS_UNLOAD_GRAPHICS_XML_F unloadGraphicsXml;
    
    LOADERS_LOAD_VIDEO_XML_F      loadVideoXml;
    LOADERS_UNLOAD_VIDEO_XML_F    unloadVideoXml;
    
    LOADERS_LOAD_SERVERS_XML_F    loadServersXml;
    LOADERS_UNLOAD_SERVERS_XML_F  unloadServersXml;
    
    LOADERS_LOAD_CLIENTS_XML_F    loadClientsXml;
    LOADERS_UNLOAD_CLIENTS_XML_F  unloadClientsXml;
    
    void                         *pData;
};

/* -------------------------------------------------------------------------------------------- */
/*                                      PUBLIC FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

LOADERS_ERROR_E Loaders_Init  (LOADERS_S **obj);
LOADERS_ERROR_E Loaders_UnInit(LOADERS_S **obj);

#ifdef __cplusplus
}
#endif

#endif //__LOADERS_H__