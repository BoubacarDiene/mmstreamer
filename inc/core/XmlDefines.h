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
* \file XmlDefines.h
* \author Boubacar DIENE
*/

#ifndef __CORE_XML_DEFINES_H__
#define __CORE_XML_DEFINES_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////////// TAGS /////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#define XML_TAG_MAIN                     "Main"
#define XML_TAG_GENERAL                  "General"
#define XML_TAG_CONTROLLERS              "Controllers"
#define XML_TAG_GRAPHICS                 "Graphics"
#define XML_TAG_VIDEOS                   "Videos"
#define XML_TAG_VIDEO                    "Video"
#define XML_TAG_SERVERS                  "Servers"
#define XML_TAG_SERVER                   "Server"
#define XML_TAG_CLIENTS                  "Clients"
#define XML_TAG_CLIENT                   "Client"
#define XML_TAG_COLORS                   "Colors"
#define XML_TAG_COLOR                    "Color"
#define XML_TAG_FONTS                    "Fonts"
#define XML_TAG_FONT                     "Font"
#define XML_TAG_IMAGES                   "Images"
#define XML_TAG_IMAGE                    "Image"
#define XML_TAG_STRINGS                  "Strings"
#define XML_TAG_STRING                   "String"
#define XML_TAG_STR_GROUP                "StrGroup"
#define XML_TAG_COMMON                   "Common"
#define XML_TAG_SCREEN                   "Screen"
#define XML_TAG_BACKGROUND               "Background"
#define XML_TAG_ICON                     "Icon"
#define XML_TAG_GFX_VIDEO                "GfxVideo"
#define XML_TAG_ELEMENTS                 "Elements"
#define XML_TAG_ELEMENT                  "Element"
#define XML_TAG_CONFIGS                  "Configs"
#define XML_TAG_CONFIG                   "Config"
#define XML_TAG_CAPABILITIES             "Capabilities"
#define XML_TAG_ITEM                     "Item"
#define XML_TAG_BUFFER_TYPE              "BufferType"
#define XML_TAG_PIXEL_FORMAT             "PixelFormat"
#define XML_TAG_COLORSPACE               "Colorspace"
#define XML_TAG_MEMORY                   "Memory"
#define XML_TAG_AWAIT_MODE               "AwaitMode"
#define XML_TAG_TEXT                     "Text"
#define XML_TAG_NAV                      "Nav"
#define XML_TAG_ON_CLICK                 "OnClick"
#define XML_TAG_HANDLER                  "Handler"
#define XML_TAG_FOCUS                    "Focus"
#define XML_TAG_BLUR                     "Blur"
#define XML_TAG_RESET                    "Reset"
#define XML_TAG_DEVICE                   "Device"
#define XML_TAG_CROPPING_AREA            "CroppingArea"
#define XML_TAG_COMPOSING_AREA           "ComposingArea"
#define XML_TAG_BUFFER                   "Buffer"
#define XML_TAG_INET                     "Inet"
#define XML_TAG_UNIX                     "Unix"

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////////// ATTRIBUTES //////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#define XML_ATTR_APP_DATA_DIR            "appDataDir"
#define XML_ATTR_RES_ROOT_DIR            "resRootDir"
#define XML_ATTR_LIB_ROOT_DIR            "libRootDir"
#define XML_ATTR_KEEP_ALIVE_METHOD       "keepAliveMethod"
#define XML_ATTR_TIMEOUT_S               "timeout_s"
#define XML_ATTR_LIB_NAME                "libName"
#define XML_ATTR_INIT_FN                 "initFn"
#define XML_ATTR_UNINIT_FN               "uninitFn"
#define XML_ATTR_ON_COMMAND_CB           "onCommandCb"
#define XML_ATTR_ON_EVENT_CB             "onEventCb"
#define XML_ATTR_ENABLE                  "enable"
#define XML_ATTR_XML_FILE                "xmlFile"
#define XML_ATTR_AUTO_START              "autoStart"
#define XML_ATTR_RED                     "red"
#define XML_ATTR_GREEN                   "green"
#define XML_ATTR_BLUE                    "blue"
#define XML_ATTR_ALPHA                   "alpha"
#define XML_ATTR_FILE                    "file"
#define XML_ATTR_FORMAT                  "format"
#define XML_ATTR_LANGUAGE                "language"
#define XML_ATTR_DEFAULT_LANGUAGE        "defaultLanguage"
#define XML_ATTR_STR                     "str"
#define XML_ATTR_X                       "x"
#define XML_ATTR_Y                       "y"
#define XML_ATTR_WIDTH                   "width"
#define XML_ATTR_HEIGHT                  "height"
#define XML_ATTR_FRAMEBUFFER_DEVICE_NAME "fbDeviceName"
#define XML_ATTR_BITS_PER_PIXEL          "bitsPerPixel"
#define XML_ATTR_FULLSCREEN              "fullscreen"
#define XML_ATTR_SHOW_CURSOR             "showCursor"
#define XML_ATTR_CAPTION                 "caption"
#define XML_ATTR_PIXEL_FORMAT            "pixelFormat"
#define XML_ATTR_USE_COLOR               "useColor"
#define XML_ATTR_STRING_ID               "stringId"
#define XML_ATTR_FONT_ID                 "fontId"
#define XML_ATTR_COLOR_ID                "colorId"
#define XML_ATTR_IMAGE_ID                "imageId"
#define XML_ATTR_HIDDEN_COLOR_ID         "hiddenColorId"
#define XML_ATTR_NAME                    "name"
#define XML_ATTR_GROUP                   "group"
#define XML_ATTR_REDRAW_GROUP            "redrawGroup"
#define XML_ATTR_VISIBLE                 "visible"
#define XML_ATTR_CLICKABLE               "clickable"
#define XML_ATTR_FOCUSABLE               "focusable"
#define XML_ATTR_HAS_FOCUS               "hasFocus"
#define XML_ATTR_DATA                    "data"
#define XML_ATTR_SIZE                    "size"
#define XML_ATTR_TOP                     "top"
#define XML_ATTR_LEFT                    "left"
#define XML_ATTR_UP                      "up"
#define XML_ATTR_RIGHT                   "right"
#define XML_ATTR_DOWN                    "down"
#define XML_ATTR_PRIORITY                "priority"
#define XML_ATTR_CONFIG_CHOICE           "configChoice"
#define XML_ATTR_GFX_DEST                "gfxDest"
#define XML_ATTR_SERVER_DEST             "serverDest"
#define XML_ATTR_SRC                     "src"
#define XML_ATTR_NB_BUFFERS              "nbBuffers"
#define XML_ATTR_DESIRED_FPS             "desiredFps"
#define XML_ATTR_VALUE                   "value"
#define XML_ATTR_MAX_CLIENTS             "maxClients"
#define XML_ATTR_TYPE                    "type"
#define XML_ATTR_LINK                    "link"
#define XML_ATTR_MODE                    "mode"
#define XML_ATTR_ACCEPT_MODE             "acceptMode"
#define XML_ATTR_MIME                    "mime"
#define XML_ATTR_MAX_BUFFER_SIZE         "maxBufferSize"
#define XML_ATTR_HOST                    "host"
#define XML_ATTR_SERVICE                 "service"
#define XML_ATTR_PATH                    "path"
#define XML_ATTR_SOCKET_NAME             "socketName"
#define XML_ATTR_SERVER_SOCKET_NAME      "serverSocketName"

#ifdef __cplusplus
}
#endif

#endif //__CORE_XML_DEFINES_H__
