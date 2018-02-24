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
* \file Loaders.h
* \author Boubacar DIENE
*/

#ifndef __LOADERS_H__
#define __LOADERS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// HEADERS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#include "core/Common.h"

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////// TYPES DECLARATION ///////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum loaders_error_e;

struct xml_color_s;
struct xml_colors_s;
struct xml_font_s;
struct xml_fonts_s;
struct xml_image_s;
struct xml_images_s;
struct xml_string_s;
struct xml_strings_s;
struct xml_capability_s;
struct xml_config_s;
struct xml_video_area_s;
struct xml_video_s;
struct xml_videos_s;
struct xml_common_files_s;
struct xml_common_s;
struct xml_screen_s;
struct xml_element_text_s;
struct xml_element_image_s;
struct xml_element_nav_s;
struct xml_element_click_s;
struct xml_element_s;
struct xml_graphics_s;
struct xml_server_s;
struct xml_servers_s;
struct xml_client_s;
struct xml_clients_s;
struct xml_s;
struct loaders_s;

/* -------------------------------------------------------------------------------------------- */
/* ///////////////////////////////////// PUBLIC FUNCTIONS ///////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

typedef enum loaders_error_e (*loaders_load_graphics_xml_f)(struct loaders_s *obj,
                                                            struct context_s *ctx,
                                                            struct xml_graphics_s *xmlGraphics);
typedef enum loaders_error_e (*loaders_unload_graphics_xml_f)(struct loaders_s *obj,
                                                              struct xml_graphics_s *xmlGraphics);

typedef enum loaders_error_e (*loaders_load_common_xml_f)(struct loaders_s *obj,
                                                          struct context_s *ctx,
                                                          struct xml_common_s *xmlCommon);
typedef enum loaders_error_e (*loaders_unload_common_xml_f)(struct loaders_s *obj,
                                                            struct xml_common_s *xmlCommon);

typedef enum loaders_error_e (*loaders_load_videos_xml_f)(struct loaders_s *obj,
                                                          struct context_s *ctx,
                                                          struct xml_videos_s *xmlVideos);
typedef enum loaders_error_e (*loaders_unload_videos_xml_f)(struct loaders_s *obj,
                                                            struct xml_videos_s *xmlVideos);

typedef enum loaders_error_e (*loaders_load_servers_xml_f)(struct loaders_s *obj,
                                                           struct context_s *ctx,
                                                           struct xml_servers_s *xmlServers);
typedef enum loaders_error_e (*loaders_unload_servers_xml_f)(struct loaders_s *obj,
                                                             struct xml_servers_s *xmlServers);

typedef enum loaders_error_e (*loaders_load_clients_xml_f)(struct loaders_s *obj,
                                                           struct context_s *ctx,
                                                           struct xml_clients_s *xmlClients);
typedef enum loaders_error_e (*loaders_unload_clients_xml_f)(struct loaders_s *obj,
                                                             struct xml_clients_s *xmlClients);

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// TYPES /////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum loaders_error_e {
    LOADERS_ERROR_NONE,
    LOADERS_ERROR_INIT,
    LOADERS_ERROR_UNINIT,
    LOADERS_ERROR_XML
};

struct xml_color_s {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t alpha;
};

struct xml_colors_s {
    uint32_t           nbColors;
    struct xml_color_s *colors;
};

struct xml_font_s {
    char *file;
};

struct xml_fonts_s {
    uint32_t          nbFonts;
    struct xml_font_s *fonts;
};

struct xml_image_s {
    char    *file;
    uint8_t format;
};

struct xml_images_s {
    uint32_t           nbImages;
    struct xml_image_s *images;
};

struct xml_string_s {
    char *str;
};

struct xml_strings_s {
    char                *language;
    uint32_t            nbStrings;
    struct xml_string_s *strings;
};

struct xml_capability_s {
    char *item;
};

struct xml_config_s {
    uint8_t                 nbItems;
    struct xml_capability_s *capabilities;

    char                    *bufferType;
    char                    *pixelFormat;
    char                    *colorspace;
    char                    *memory;
    char                    *awaitMode;
};

struct xml_video_area_s {
    uint32_t left;
    uint32_t top;
    uint32_t width;
    uint32_t height;
};

struct xml_video_s {
    uint8_t                 priority;
    uint32_t                configChoice;
    char                    *graphicsDest;
    char                    *serverDest;

    char                    *deviceName;
    char                    *deviceSrc;

    struct xml_video_area_s deviceArea;
    struct xml_video_area_s croppingArea;
    struct xml_video_area_s composingArea;
    
    uint8_t                 nbBuffers;
    uint8_t                 desiredFps;
};

struct xml_videos_s {
    uint8_t             nbVideos;
    struct xml_video_s  *videos;

    uint8_t             nbConfigs;
    struct xml_config_s *configs;

    void                *reserved;
};

struct xml_common_files_s {
    char colors[MAX_PATH_SIZE];
    char images[MAX_PATH_SIZE];
    char fonts[MAX_PATH_SIZE];
    char strings[MAX_PATH_SIZE];
};

struct xml_common_s {
    uint32_t                  nbLanguages;
    char                      defaultLanguage[MIN_STR_SIZE];

    struct xml_common_files_s files;
    
    struct xml_colors_s       xmlColors;
    struct xml_fonts_s        xmlFonts;
    struct xml_images_s       xmlImages;
    struct xml_strings_s      *xmlStrings;

    void                      *reserved;
};

struct xml_screen_s {
    char     *name;
    uint16_t width;
    uint16_t height;
    char     *fbDeviceName;
    uint8_t  bitsPerPixel;
    uint8_t  fullscreen;
    uint8_t  showCursor;
    char     *caption;
    
    uint8_t  useColor;
    uint32_t BgColorId;
    
    uint32_t BgImageId;
    int32_t  BgHiddenColorId;
    
    uint32_t iconImageId;
    int32_t  iconHiddenColorId;
    
    uint32_t videoFormat;
};

struct xml_element_text_s {
    uint32_t stringId;
    uint32_t fontId;
    uint32_t colorId;
    uint8_t  size;
};

struct xml_element_image_s {
    uint32_t imageId;
    int32_t  hiddenColorId;
};

struct xml_element_nav_s {
    char *left;
    char *up;
    char *right;
    char *down;
};

struct xml_element_click_s {
    char *name;
    char *data;
};

struct xml_element_s {
    char                       *name;
    char                       *groupName;
    
    uint8_t                    redrawGroup;
    
    uint8_t                    type;
    
    uint32_t                   x;
    uint32_t                   y;
    uint32_t                   width;
    uint32_t                   height;
    
    uint8_t                    visible;
    uint8_t                    clickable;
    uint8_t                    focusable;
    uint8_t                    hasFocus;
    
    struct xml_element_text_s  *text;
    struct xml_element_image_s *image;
    struct xml_element_nav_s   *nav;

    uint32_t                   nbClickHandlers;
    struct xml_element_click_s *clickHandlers;
};

struct xml_graphics_s {
    char                 *defaultLanguage;

    char                 *colorsXmlFile;
    char                 *imagesXmlFile;
    char                 *fontsXmlFile;
    char                 *stringsXmlFile;
    
    uint32_t             onFocusColorId;
    uint32_t             onBlurColorId;
    uint32_t             onResetColorId;
    
    struct xml_screen_s  screen;
    
    uint32_t             nbElements;
    struct xml_element_s *elements;
    
    void                 *reserved;
};

struct xml_server_s {
    char     *name;
    uint8_t  type;
    uint8_t  link;
    uint8_t  mode;
    uint8_t  acceptMode;
    uint8_t  priority;
    uint32_t maxClients;
    char     *mime;
    
    char     *host;
    char     *service;
    char     *path;
    
    char     *socketName;
};

struct xml_servers_s {
    uint8_t             nbServers;
    struct xml_server_s *servers;
    
    void                *reserved;
};

struct xml_client_s {
    char    *name;
    uint8_t type;
    uint8_t link;
    uint8_t mode;
    uint8_t priority;
    char    *graphicsDest;
    char    *serverDest;
    
    char    *serverHost;
    char    *serverService;
    char    *serverPath;
    
    char    *serverSocketName;
};

struct xml_clients_s {
    uint8_t             nbClients;
    struct xml_client_s *clients;
    
    void                *reserved;
};

struct xml_s {
    struct xml_common_s   xmlCommon;
    struct xml_graphics_s xmlGraphics;
    struct xml_videos_s   xmlVideos;
    struct xml_servers_s  xmlServers;
    struct xml_clients_s  xmlClients;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// MAIN CONTEXT /////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

struct loaders_s {
    loaders_load_graphics_xml_f   loadGraphicsXml;
    loaders_unload_graphics_xml_f unloadGraphicsXml;
    
    loaders_load_common_xml_f     loadCommonXml;
    loaders_unload_common_xml_f   unloadCommonXml;
    
    loaders_load_videos_xml_f     loadVideosXml;
    loaders_unload_videos_xml_f   unloadVideosXml;
    
    loaders_load_servers_xml_f    loadServersXml;
    loaders_unload_servers_xml_f  unloadServersXml;
    
    loaders_load_clients_xml_f    loadClientsXml;
    loaders_unload_clients_xml_f  unloadClientsXml;
    
    void *pData;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// INITIALIZER //////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum loaders_error_e Loaders_Init(struct loaders_s **obj);
enum loaders_error_e Loaders_UnInit(struct loaders_s **obj);

#ifdef __cplusplus
}
#endif

#endif //__LOADERS_H__
