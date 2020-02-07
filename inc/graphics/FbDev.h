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
* \file FbDev.h
* \brief Framebuffer device management
*        IMPORTANT : Mmstreamer engine must be run as root to make it have rights to use fbDev
* \author Boubacar DIENE
*/

#ifndef __FBDEV_H__
#define __FBDEV_H__

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

enum fbdev_error_e;

struct fbdev_infos_s;
struct fbdev_s;

/* -------------------------------------------------------------------------------------------- */
/* ///////////////////////////////////// PUBLIC FUNCTIONS ///////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

typedef enum fbdev_error_e (*fbdev_open_f)(struct fbdev_s *obj, const char *fbName);
typedef enum fbdev_error_e (*fbdev_is_opened_f)(struct fbdev_s *obj, uint8_t *opened);
typedef enum fbdev_error_e (*fbdev_close_f)(struct fbdev_s *obj);

typedef enum fbdev_error_e (*fbdev_get_infos_f)(struct fbdev_s *obj,
                                                struct fbdev_infos_s *fbInfos);
typedef enum fbdev_error_e (*fbdev_set_depth_f)(struct fbdev_s *obj, uint32_t depth);

typedef enum fbdev_error_e (*fbdev_clear_f)(struct fbdev_s *obj);
typedef enum fbdev_error_e (*fbdev_restore_f)(struct fbdev_s *obj);

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// TYPES /////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum fbdev_error_e {
    FBDEV_ERROR_NONE,
    FBDEV_ERROR_INIT,
    FBDEV_ERROR_UNINIT,
    FBDEV_ERROR_PARAMS,
    FBDEV_ERROR_IO
};

struct fbdev_infos_s {
    uint32_t width;
    uint32_t height;

    uint32_t depth;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// MAIN CONTEXT /////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

struct fbdev_s {
    fbdev_open_f      open;
    fbdev_is_opened_f isOpened;
    fbdev_close_f     close;

    fbdev_get_infos_f getInfos;
    fbdev_set_depth_f setDepth;

    fbdev_clear_f     clear;
    fbdev_restore_f   restore;

    void *pData;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// INITIALIZER //////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum fbdev_error_e FbDev_Init(struct fbdev_s **obj);
enum fbdev_error_e FbDev_UnInit(struct fbdev_s **obj);

#ifdef __cplusplus
}
#endif

#endif //__FBDEV_H__
