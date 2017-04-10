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
* \file   Loaders.c
* \brief  TODO
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include "core/Loaders.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                            */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "LOADERS"

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                          VARIABLES                                           */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                         PROTOTYPES                                           */
/* -------------------------------------------------------------------------------------------- */

extern LOADERS_ERROR_E loadGraphicsXml_f  (LOADERS_S *obj, CONTEXT_S *ctx, XML_GRAPHICS_S *xmlGraphics);
extern LOADERS_ERROR_E unloadGraphicsXml_f(LOADERS_S *obj, XML_GRAPHICS_S *xmlGraphics);

extern LOADERS_ERROR_E loadVideosXml_f  (LOADERS_S *obj, CONTEXT_S *ctx, XML_VIDEOS_S *xmlVideos);
extern LOADERS_ERROR_E unloadVideosXml_f(LOADERS_S *obj, XML_VIDEOS_S *xmlVideos);

extern LOADERS_ERROR_E loadServersXml_f  (LOADERS_S *obj, CONTEXT_S *ctx, XML_SERVERS_S *xmlServers);
extern LOADERS_ERROR_E unloadServersXml_f(LOADERS_S *obj, XML_SERVERS_S *xmlServers);

extern LOADERS_ERROR_E loadClientsXml_f  (LOADERS_S *obj, CONTEXT_S *ctx, XML_CLIENTS_S *xmlClients);
extern LOADERS_ERROR_E unloadClientsXml_f(LOADERS_S *obj, XML_CLIENTS_S *xmlClients);

/* -------------------------------------------------------------------------------------------- */
/*                                      PUBLIC FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
LOADERS_ERROR_E Loaders_Init(LOADERS_S **obj)
{
    assert(obj && (*obj = calloc(1, sizeof(LOADERS_S))));
    
    (*obj)->loadGraphicsXml   = loadGraphicsXml_f;
    (*obj)->unloadGraphicsXml = unloadGraphicsXml_f;
    
    (*obj)->loadVideosXml     = loadVideosXml_f;
    (*obj)->unloadVideosXml   = unloadVideosXml_f;
    
    (*obj)->loadServersXml    = loadServersXml_f;
    (*obj)->unloadServersXml  = unloadServersXml_f;
    
    (*obj)->loadClientsXml    = loadClientsXml_f;
    (*obj)->unloadClientsXml  = unloadClientsXml_f;
    
    return LOADERS_ERROR_NONE;
}

/*!
 *
 */
LOADERS_ERROR_E Loaders_UnInit(LOADERS_S **obj)
{
    assert(obj && *obj);
    
    free(*obj);
    *obj = NULL;
    
    return LOADERS_ERROR_NONE;
}
