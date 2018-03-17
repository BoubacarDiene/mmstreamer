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
* \file Task.h
* \author Boubacar DIENE
*/

#ifndef __TASK_H__
#define __TASK_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// HEADERS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#include "utils/Common.h"

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////// TYPES DECLARATION ///////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum task_error_e;

struct task_params_s;
struct task_s;

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////////// CALLBACKS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

typedef void (*task_function_f)(struct task_params_s *params);
typedef void (*task_atExit_f)(struct task_params_s *params);

/* -------------------------------------------------------------------------------------------- */
/* ///////////////////////////////////// PUBLIC FUNCTIONS ///////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

typedef enum task_error_e (*task_create_f)(struct task_s *obj, struct task_params_s *params);
typedef enum task_error_e (*task_destroy_f)(struct task_s *obj, struct task_params_s *params);

typedef enum task_error_e (*task_start_f)(struct task_s *obj, struct task_params_s *params);
typedef enum task_error_e (*task_stop_f)(struct task_s *obj, struct task_params_s *params);

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// TYPES /////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum task_error_e {
    TASK_ERROR_NONE,
    TASK_ERROR_INIT,
    TASK_ERROR_UNINIT,
    TASK_ERROR_CREATE,
    TASK_ERROR_DESTROY,
    TASK_ERROR_START,
    TASK_ERROR_STOP
};

struct task_params_s {
    char            name[MAX_NAME_SIZE];
    enum priority_e priority;

    task_function_f fct;
    void            *fctData;
    void            *userData;
    task_atExit_f   atExit;
    
    void            *reserved;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// MAIN CONTEXT /////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

struct task_s {
    task_create_f  create;
    task_destroy_f destroy;

    task_start_f   start;
    task_stop_f    stop;
          
    void *pData;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// INITIALIZER //////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum task_error_e Task_Init(struct task_s **obj);
enum task_error_e Task_UnInit(struct task_s **obj);

#ifdef __cplusplus
}
#endif

#endif //__TASK_H__
