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
* \file Log.h
* \author Boubacar DIENE
*/

#ifndef __LOG_H__
#define __LOG_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// HEADERS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#include <stdio.h>
#include <string.h>

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// MACROS ////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#define ERROR_LVL   1
#define WARNING_LVL 2
#define INFO_LVL    3
#define DEBUG_LVL   4

#define BLUE    "\033[0;34m"
#define YELLOW  "\033[1;33m"
#define RED     "\033[5;31m"
#define GREEN   "\033[0;32m"

#define END     "\033[0m"

#define __file__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#if LOG_LEVEL >= DEBUG_LVL
    #define Logd(...)(printf("%s%s : %s:%d (%s) %s", BLUE, TAG, __file__, __LINE__, __func__, END),\
                        printf(__VA_ARGS__),printf("\n"))
#else
    #define Logd(...)
#endif

#if LOG_LEVEL >= INFO_LVL
    #define Logi(...)(printf("%s%s : %s:%d (%s) %s", GREEN, TAG, __file__, __LINE__, __func__, END),\
                        printf(__VA_ARGS__),printf("\n"))
#else
    #define Logi(...)
#endif

#if LOG_LEVEL >= WARNING_LVL
    #define Logw(...)(printf("%s%s : %s:%d (%s) %s", YELLOW, TAG, __file__, __LINE__, __func__, END),\
                        printf(__VA_ARGS__),printf("\n"))
#else
    #define Logw(...)
#endif

#if LOG_LEVEL >= ERROR_LVL
    #define Loge(...)(printf("%s%s : %s:%d (%s) %s", RED, TAG, __file__, __LINE__, __func__, END),\
                        printf(__VA_ARGS__),printf("\n"))
#else
    #define Loge(...)
#endif

#ifdef __cplusplus
}
#endif

#endif //__LOG_H__
