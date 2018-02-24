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
* \file GraphicsListeners.c
* \brief TODO
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// HEADERS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#include "core/Listeners.h"

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// MACROS ////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "GraphicsListeners"

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////// PUBLIC FUNCTIONS PROTOTYPES //////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum listeners_error_e setGraphicsListeners_f(struct listeners_s *obj);
enum listeners_error_e unsetGraphicsListeners_f(struct listeners_s *obj);

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////////// CALLBACKS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

static void onGfxEventCb(struct gfx_event_s *gfxEvent, void *userData);

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////// PUBLIC FUNCTIONS IMPLEMENTATION ///////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
enum listeners_error_e setGraphicsListeners_f(struct listeners_s *obj)
{
    assert(obj);
    
    struct listeners_params_s *listenersParams = &obj->params;
    struct graphics_infos_s *graphicsInfos     = &listenersParams->ctx->params.graphicsInfos;
    
    graphicsInfos->graphicsParams.onGfxEventCb = onGfxEventCb;
    graphicsInfos->graphicsParams.userData     = listenersParams;
    
    return LISTENERS_ERROR_NONE;
}

/*!
 *
 */
enum listeners_error_e unsetGraphicsListeners_f(struct listeners_s *obj)
{
    assert(obj);
    
    struct listeners_params_s *listenersParams = &obj->params;
    struct graphics_infos_s *graphicsInfos     = &listenersParams->ctx->params.graphicsInfos;
    
    graphicsInfos->graphicsParams.onGfxEventCb = NULL;
    graphicsInfos->graphicsParams.userData     = NULL;
    
    return LISTENERS_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////////// CALLBACKS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static void onGfxEventCb(struct gfx_event_s *gfxEvent, void *userData)
{
    assert(gfxEvent && userData);
    
    struct listeners_params_s *listenersParams = (struct listeners_params_s*)userData;
    struct control_s *controlObj               = listenersParams->controlObj;
    struct graphics_s *graphicsObj             = listenersParams->ctx->modules.graphicsObj;
    
    switch (gfxEvent->type) {
        case GFX_EVENT_TYPE_QUIT:
        case GFX_EVENT_TYPE_ESC:
            graphicsObj->quit(graphicsObj);
            break;
            
        case GFX_EVENT_TYPE_FOCUS:
            break;
            
        case GFX_EVENT_TYPE_CLICK:
        case GFX_EVENT_TYPE_ENTER:
            controlObj->handleClick(controlObj, gfxEvent);
            break;
            
        default:
            ;
    }
}
