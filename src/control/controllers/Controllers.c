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
* \file Controllers.c
* \brief TODO
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// HEADERS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#include "control/Controllers.h"

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// MACROS ////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "Controllers"

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////// PUBLIC FUNCTIONS PROTOTYPES //////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

extern enum controllers_error_e loadLibs_f(struct controllers_s *obj);
extern enum controllers_error_e unloadLibs_f(struct controllers_s *obj);

extern enum controllers_error_e initCmdsTask_f(struct controllers_s *obj);
extern enum controllers_error_e uninitCmdsTask_f(struct controllers_s *obj);
extern enum controllers_error_e startCmdsTask_f(struct controllers_s *obj);
extern enum controllers_error_e stopCmdsTask_f(struct controllers_s *obj);

extern enum controllers_error_e initEvtsTask_f(struct controllers_s *obj);
extern enum controllers_error_e uninitEvtsTask_f(struct controllers_s *obj);
extern enum controllers_error_e startEvtsTask_f(struct controllers_s *obj);
extern enum controllers_error_e stopEvtsTask_f(struct controllers_s *obj);

extern enum controllers_error_e initLibsTask_f(struct controllers_s *obj);
extern enum controllers_error_e uninitLibsTask_f(struct controllers_s *obj);
extern enum controllers_error_e startLibsTask_f(struct controllers_s *obj);
extern enum controllers_error_e stopLibsTask_f(struct controllers_s *obj);

extern enum controllers_error_e notify_f(struct controllers_s *obj,
                                         struct controller_event_s *event);

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// INITIALIZER //////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
enum controllers_error_e Controllers_Init(struct controllers_s **obj,
                                          struct controllers_params_s *params)
{
    assert(obj && params && (*obj = calloc(1, sizeof(struct controllers_s))));

    (*obj)->loadLibs   = loadLibs_f;
    (*obj)->unloadLibs = unloadLibs_f;

    (*obj)->notify     = notify_f;

    (*obj)->tasksMngt[CONTROLLERS_TASK_CMDS].init   = initCmdsTask_f;
    (*obj)->tasksMngt[CONTROLLERS_TASK_CMDS].uninit = uninitCmdsTask_f;
    (*obj)->tasksMngt[CONTROLLERS_TASK_CMDS].start  = startCmdsTask_f;
    (*obj)->tasksMngt[CONTROLLERS_TASK_CMDS].stop   = stopCmdsTask_f;

    (*obj)->tasksMngt[CONTROLLERS_TASK_EVTS].init   = initEvtsTask_f;
    (*obj)->tasksMngt[CONTROLLERS_TASK_EVTS].uninit = uninitEvtsTask_f;
    (*obj)->tasksMngt[CONTROLLERS_TASK_EVTS].start  = startEvtsTask_f;
    (*obj)->tasksMngt[CONTROLLERS_TASK_EVTS].stop   = stopEvtsTask_f;

    (*obj)->tasksMngt[CONTROLLERS_TASK_LIBS].init   = initLibsTask_f;
    (*obj)->tasksMngt[CONTROLLERS_TASK_LIBS].uninit = uninitLibsTask_f;
    (*obj)->tasksMngt[CONTROLLERS_TASK_LIBS].start  = startLibsTask_f;
    (*obj)->tasksMngt[CONTROLLERS_TASK_LIBS].stop   = stopLibsTask_f;

    (*obj)->params = *params;

    return CONTROLLERS_ERROR_NONE;
}

/*!
 *
 */
enum controllers_error_e Controllers_UnInit(struct controllers_s **obj)
{
    assert(obj && *obj);

    free(*obj);
    *obj = NULL;

    return CONTROLLERS_ERROR_NONE;
}
