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
* \file   Task.h
* \author Boubacar DIENE
*/

#ifndef __TASK_H__
#define __TASK_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include "utils/Common.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                            */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

typedef enum TASK_ERROR_E    TASK_ERROR_E;

typedef struct TASK_PARAMS_S TASK_PARAMS_S;
typedef struct TASK_S        TASK_S;

typedef void (*TASK_FUNCTION_F)(TASK_PARAMS_S *params);
typedef void (*TASK_ATEXIT_F  )(TASK_PARAMS_S *params);

typedef TASK_ERROR_E (*TASK_CREATE_F )(TASK_S *obj, TASK_PARAMS_S *params);
typedef TASK_ERROR_E (*TASK_DESTROY_F)(TASK_S *obj, TASK_PARAMS_S *params);

typedef TASK_ERROR_E (*TASK_START_F)(TASK_S *obj, TASK_PARAMS_S *params);
typedef TASK_ERROR_E (*TASK_STOP_F )(TASK_S *obj, TASK_PARAMS_S *params);

enum TASK_ERROR_E {
    TASK_ERROR_NONE,
    TASK_ERROR_INIT,
    TASK_ERROR_UNINIT,
    TASK_ERROR_CREATE,
    TASK_ERROR_DESTROY,
    TASK_ERROR_START,
    TASK_ERROR_STOP
};

struct TASK_PARAMS_S {
    char            name[MAX_NAME_SIZE];
    PRIORITY_E      priority;
    TASK_FUNCTION_F fct;
    void            *fctData;
    void            *userData;
    TASK_ATEXIT_F   atExit;
    
    void            *reserved;
};

struct TASK_S {
    TASK_CREATE_F  create;
    TASK_DESTROY_F destroy;
    
    TASK_START_F   start;
    TASK_STOP_F    stop;
          
    void           *pData;
};

/* -------------------------------------------------------------------------------------------- */
/*                                      PUBLIC FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

TASK_ERROR_E Task_Init  (TASK_S **obj);
TASK_ERROR_E Task_UnInit(TASK_S **obj);

#ifdef __cplusplus
}
#endif

#endif //__TASK_H__
