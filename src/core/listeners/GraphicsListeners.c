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
* \file   GraphicsListeners.c
* \brief  TODO
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include "core/Listeners.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                            */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "GRAPHICS-LISTENERS"

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                         PROTOTYPES                                           */
/* -------------------------------------------------------------------------------------------- */

LISTENERS_ERROR_E setGraphicsListeners_f  (LISTENERS_S *obj);
LISTENERS_ERROR_E unsetGraphicsListeners_f(LISTENERS_S *obj);

static void onGfxEventCb(GFX_EVENT_S *gfxEvent, void *userData);

/* -------------------------------------------------------------------------------------------- */
/*                                          VARIABLES                                           */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                          FUNCTIONS                                           */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
LISTENERS_ERROR_E setGraphicsListeners_f(LISTENERS_S *obj)
{
    assert(obj && obj->pData);
    
    LISTENERS_PDATA_S *pData        = (LISTENERS_PDATA_S*)(obj->pData);
    GRAPHICS_INFOS_S *graphicsInfos = &pData->ctx->params.graphicsInfos;
    
    graphicsInfos->graphicsParams.onGfxEventCb = onGfxEventCb;
    graphicsInfos->graphicsParams.userData     = pData;
    
    return LISTENERS_ERROR_NONE;
}

/*!
 *
 */
LISTENERS_ERROR_E unsetGraphicsListeners_f(LISTENERS_S *obj)
{
    assert(obj && obj->pData);
    
    LISTENERS_PDATA_S *pData        = (LISTENERS_PDATA_S*)(obj->pData);
    GRAPHICS_INFOS_S *graphicsInfos = &pData->ctx->params.graphicsInfos;
    
    graphicsInfos->graphicsParams.onGfxEventCb = NULL;
    graphicsInfos->graphicsParams.userData     = NULL;
    
    return LISTENERS_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/*                                          CALLBACKS                                           */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static void onGfxEventCb(GFX_EVENT_S *gfxEvent, void *userData)
{
    assert(gfxEvent && userData);
    
    LISTENERS_PDATA_S *pData = (LISTENERS_PDATA_S*)userData;
    
    switch (gfxEvent->type) {
        case GFX_EVENT_TYPE_QUIT:
        case GFX_EVENT_TYPE_ESC:
            pData->ctx->modules.graphicsObj->quit(pData->ctx->modules.graphicsObj);
            break;
            
        case GFX_EVENT_TYPE_FOCUS:
            break;
            
        case GFX_EVENT_TYPE_CLICK:
        case GFX_EVENT_TYPE_ENTER:
            pData->controlObj->handleClick(pData->ctx, gfxEvent);
            break;
            
        default:
            ;
    }
}
