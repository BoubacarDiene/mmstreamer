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
* \file Common.h
* \author Boubacar DIENE
*/
#ifndef __COMMON_H__
#define __COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// HEADERS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#include <assert.h>
#include <errno.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// MACROS ////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#define TAG              "DEFAULT"

#define MIN_STR_SIZE     32
#define MAX_STR_SIZE     512
#define MAX_NAME_SIZE    128
#define MAX_PATH_SIZE    256
#define MAX_MIME_SIZE    32
#define MAX_ADDRESS_SIZE 32
#define MAX_HEADER_SIZE  512
#define MAX_XML_SIZE     256
#define MAX_BLOCK_SIZE   512
#define MAX_BUFFER_SIZE  1024 * 64

#define WAIT_TIME_1MS    1
#define WAIT_TIME_10MS   WAIT_TIME_1MS * 10
#define WAIT_TIME_1S     WAIT_TIME_10MS * 100
#define WAIT_TIME_2S     WAIT_TIME_1S  * 2
#define WAIT_TIME_5S     WAIT_TIME_1S  * 5
#define WAIT_TIME_10S    WAIT_TIME_5S  * 2

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// TYPES /////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum priority_e {
    PRIORITY_LOWEST,
    PRIORITY_DEFAULT,
    PRIORITY_HIGHEST
};

struct buffer_s {
    void    *data;
    ssize_t length;
};

#ifdef __cplusplus
}
#endif

#endif //__COMMON_H__
