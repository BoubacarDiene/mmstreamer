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
* \file Listeners.c
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
#define TAG "Listeners"

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////// PUBLIC FUNCTIONS PROTOTYPES //////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

extern enum listeners_error_e setGraphicsListeners_f(struct listeners_s *obj);
extern enum listeners_error_e unsetGraphicsListeners_f(struct listeners_s *obj);

extern enum listeners_error_e setVideosListeners_f(struct listeners_s *obj);
extern enum listeners_error_e unsetVideosListeners_f(struct listeners_s *obj);

extern enum listeners_error_e setServersListeners_f(struct listeners_s *obj);
extern enum listeners_error_e unsetServersListeners_f(struct listeners_s *obj);

extern enum listeners_error_e setClientsListeners_f(struct listeners_s *obj);
extern enum listeners_error_e unsetClientsListeners_f(struct listeners_s *obj);

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// INITIALIZER //////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
enum listeners_error_e Listeners_Init(struct listeners_s **obj,
                                      struct listeners_params_s *params)
{
    ASSERT(obj && params && (*obj = calloc(1, sizeof(struct listeners_s))));
    
    (*obj)->setGraphicsListeners   = setGraphicsListeners_f;
    (*obj)->unsetGraphicsListeners = unsetGraphicsListeners_f;
    
    (*obj)->setVideosListeners     = setVideosListeners_f;
    (*obj)->unsetVideosListeners   = unsetVideosListeners_f;
    
    (*obj)->setServersListeners    = setServersListeners_f;
    (*obj)->unsetServersListeners  = unsetServersListeners_f;
    
    (*obj)->setClientsListeners    = setClientsListeners_f;
    (*obj)->unsetClientsListeners  = unsetClientsListeners_f;
    
    (*obj)->params = *params;
    
    return LISTENERS_ERROR_NONE;
}

/*!
 *
 */
enum listeners_error_e Listeners_UnInit(struct listeners_s **obj)
{
    ASSERT(obj && *obj);

    free(*obj);
    *obj = NULL;
    
    return LISTENERS_ERROR_NONE;
}
