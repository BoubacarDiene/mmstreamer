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
* \file Loaders.c
* \brief TODO
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// HEADERS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#include "core/Loaders.h"

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// MACROS ////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "Loaders"

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////// PUBLIC FUNCTIONS PROTOTYPES //////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

extern enum loaders_error_e loadGraphicsXml_f(struct loaders_s *obj, struct context_s *ctx,
                                              struct xml_graphics_s *xmlGraphics);
extern enum loaders_error_e unloadGraphicsXml_f(struct loaders_s *obj,
                                                struct xml_graphics_s *xmlGraphics);

extern enum loaders_error_e loadCommonXml_f(struct loaders_s *obj, struct context_s *ctx,
                                            struct xml_common_s *xmlCommon);
extern enum loaders_error_e unloadCommonXml_f(struct loaders_s *obj,
                                              struct xml_common_s *xmlCommon);

extern enum loaders_error_e loadVideosXml_f(struct loaders_s *obj, struct context_s *ctx,
                                            struct xml_videos_s *xmlVideos);
extern enum loaders_error_e unloadVideosXml_f(struct loaders_s *obj,
                                              struct xml_videos_s *xmlVideos);

extern enum loaders_error_e loadServersXml_f(struct loaders_s *obj, struct context_s *ctx,
                                             struct xml_servers_s *xmlServers);
extern enum loaders_error_e unloadServersXml_f(struct loaders_s *obj,
                                               struct xml_servers_s *xmlServers);

extern enum loaders_error_e loadClientsXml_f(struct loaders_s *obj, struct context_s *ctx,
                                             struct xml_clients_s *xmlClients);
extern enum loaders_error_e unloadClientsXml_f(struct loaders_s *obj,
                                               struct xml_clients_s *xmlClients);

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// INITIALIZER //////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
enum loaders_error_e Loaders_Init(struct loaders_s **obj)
{
    ASSERT(obj && (*obj = calloc(1, sizeof(struct loaders_s))));
    
    (*obj)->loadGraphicsXml   = loadGraphicsXml_f;
    (*obj)->unloadGraphicsXml = unloadGraphicsXml_f;
    
    (*obj)->loadCommonXml     = loadCommonXml_f;
    (*obj)->unloadCommonXml   = unloadCommonXml_f;
    
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
enum loaders_error_e Loaders_UnInit(struct loaders_s **obj)
{
    ASSERT(obj && *obj);
    
    free(*obj);
    *obj = NULL;
    
    return LOADERS_ERROR_NONE;
}
