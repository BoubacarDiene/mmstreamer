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
* \file FbDev.c
* \brief Framebuffer management
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// HEADERS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>

#include "utils/Log.h"

#include "graphics/FbDev.h"

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// MACROS ////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "FbDev"

#define FBDEV_MAX_PATH_SIZE 16
#define FBDEV_PATH_PREFIX   "/dev/"

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// TYPES /////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

struct fbdev_private_data_s {
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;

    char                     path[FBDEV_MAX_PATH_SIZE];
    int32_t                  fd;
    char                     *map;

    volatile uint8_t         opened;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////// PUBLIC FUNCTIONS PROTOTYPES //////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

static enum fbdev_error_e open_f(struct fbdev_s *obj, const char *fbName);
static enum fbdev_error_e isOpened_f(struct fbdev_s *obj, uint8_t *opened);
static enum fbdev_error_e close_f(struct fbdev_s *obj);

static enum fbdev_error_e getInfos_f(struct fbdev_s *obj, struct fbdev_infos_s *fbInfos);
static enum fbdev_error_e setDepth_f(struct fbdev_s *obj, uint32_t depth);

static enum fbdev_error_e clear_f(struct fbdev_s *obj);
static enum fbdev_error_e restore_f(struct fbdev_s *obj);

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// INITIALIZER //////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
enum fbdev_error_e FbDev_Init(struct fbdev_s **obj)
{
    ASSERT(obj && (*obj = calloc(1, sizeof(struct fbdev_s))));

    struct fbdev_private_data_s *pData;
    ASSERT((pData = calloc(1, sizeof(struct fbdev_private_data_s))));

    (*obj)->open     = open_f;
    (*obj)->isOpened = isOpened_f;
    (*obj)->close    = close_f;

    (*obj)->getInfos = getInfos_f;
    (*obj)->setDepth = setDepth_f;

    (*obj)->clear    = clear_f;
    (*obj)->restore  = restore_f;

    (*obj)->pData = (void*)pData;

    return FBDEV_ERROR_NONE;
}

/*!
 *
 */
enum fbdev_error_e FbDev_UnInit(struct fbdev_s **obj)
{
    ASSERT(obj && *obj && (*obj)->pData);
    
    free((*obj)->pData);
    free(*obj);
    *obj = NULL;
    
    return FBDEV_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////// PUBLIC FUNCTIONS IMPLEMENTATION ///////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static enum fbdev_error_e open_f(struct fbdev_s *obj, const char *fbName)
{
    ASSERT(obj && obj->pData && fbName);

    struct fbdev_private_data_s *pData = (struct fbdev_private_data_s*)(obj->pData);

    sprintf(pData->path, "%s%s", FBDEV_PATH_PREFIX, fbName);

    Logd("Opening framebuffer device \"%s\"", pData->path);

    pData->fd = open(pData->path, O_RDWR);
    if (pData->fd < 0) {
        Loge("Failed to open framebuffer device \"%s\" - %s", pData->path, strerror(errno));
        return FBDEV_ERROR_IO;
    }

    pData->opened = 1;

    return FBDEV_ERROR_NONE;
}

/*!
 *
 */
static enum fbdev_error_e isOpened_f(struct fbdev_s *obj, uint8_t *opened)
{
    ASSERT(obj && obj->pData && opened);

    struct fbdev_private_data_s *pData = (struct fbdev_private_data_s*)(obj->pData);

    *opened = pData->opened;

    return FBDEV_ERROR_NONE;
}

/*!
 *
 */
static enum fbdev_error_e close_f(struct fbdev_s *obj)
{
    ASSERT(obj && obj->pData);

    struct fbdev_private_data_s *pData = (struct fbdev_private_data_s*)(obj->pData);

    if (!pData->opened) {
        Loge("No framebuffer device opened yet");
        return FBDEV_ERROR_IO;
    }

    Logd("Closing framebuffer device \"%s\"", pData->path);

    close(pData->fd);
    memset(pData->path, '\0', sizeof(pData->path));

    return FBDEV_ERROR_NONE;
}

/*!
 *
 */
static enum fbdev_error_e getInfos_f(struct fbdev_s *obj, struct fbdev_infos_s *fbInfos)
{
    ASSERT(obj && obj->pData && fbInfos);

    struct fbdev_private_data_s *pData = (struct fbdev_private_data_s*)(obj->pData);

    if (!pData->opened) {
        Loge("No framebuffer device opened yet");
        return FBDEV_ERROR_IO;
    }

    if (ioctl(pData->fd, FBIOGET_VSCREENINFO, &pData->vinfo) < 0) {
        Loge("Failed to read framebuffer variable infos - %s", strerror(errno));
        return FBDEV_ERROR_IO;
    }

    if (ioctl(pData->fd, FBIOGET_FSCREENINFO, &pData->finfo) < 0) {
        Loge("Failed to read framebuffer fixed infos - %s", strerror(errno));
        return FBDEV_ERROR_IO;
    }

    Logd("size : %u / width : %u / height : %u / depth : %u",
            pData->finfo.smem_len, pData->vinfo.xres, pData->vinfo.yres,
            pData->vinfo.bits_per_pixel);

    fbInfos->width  = pData->vinfo.xres;
    fbInfos->height = pData->vinfo.yres;
    fbInfos->depth  = pData->vinfo.bits_per_pixel;

    return FBDEV_ERROR_NONE;
}

/*!
 *
 */
static enum fbdev_error_e setDepth_f(struct fbdev_s *obj, uint32_t depth)
{
    ASSERT(obj && obj->pData);

    struct fbdev_private_data_s *pData = (struct fbdev_private_data_s*)(obj->pData);

    if (!pData->opened) {
        Loge("No framebuffer device opened yet");
        return FBDEV_ERROR_IO;
    }

    Logd("Updating depth : current = %u / new = %u", pData->vinfo.bits_per_pixel, depth);

    struct fb_var_screeninfo vinfo;
    memcpy(&vinfo, &pData->vinfo, sizeof(struct fb_var_screeninfo));

    vinfo.bits_per_pixel = depth;
    if (ioctl(pData->fd, FBIOPUT_VSCREENINFO, &vinfo) < 0) {
        Loge("Failed to update framebuffer depth");
        return FBDEV_ERROR_IO;
    }

    return FBDEV_ERROR_NONE;
}

/*!
 *
 */
static enum fbdev_error_e clear_f(struct fbdev_s *obj)
{
    ASSERT(obj && obj->pData);

    struct fbdev_private_data_s *pData = (struct fbdev_private_data_s*)(obj->pData);

    if (!pData->opened) {
        Loge("No framebuffer device opened yet");
        return FBDEV_ERROR_IO;
    }

    Logd("Clearing framebuffer device \"%s\"", pData->path);

    pData->map = (char*)mmap(0, pData->finfo.smem_len,
                                PROT_READ | PROT_WRITE,
                                MAP_SHARED,
                                pData->fd,
                                0);

    if (pData->map == MAP_FAILED) {
        Loge("Failed to map framebuffer device to user memory");
        return FBDEV_ERROR_IO;
    }

    memset(pData->map, 0, pData->finfo.smem_len);

    (void)munmap(pData->map, pData->finfo.smem_len);

    return FBDEV_ERROR_NONE;
}

/*!
 *
 */
static enum fbdev_error_e restore_f(struct fbdev_s *obj)
{
    ASSERT(obj && obj->pData);

    struct fbdev_private_data_s *pData = (struct fbdev_private_data_s*)(obj->pData);

    if (!pData->opened) {
        Loge("No framebuffer device opened yet");
        return FBDEV_ERROR_IO;
    }

    Logd("Restoring framebuffer device \"%s\"", pData->path);

    struct fb_var_screeninfo vinfo;
    if (ioctl(pData->fd, FBIOGET_VSCREENINFO, &vinfo) < 0) {
        Loge("Failed to read framebuffer variable infos - %s", strerror(errno));
        return FBDEV_ERROR_IO;
    }

    if (vinfo.bits_per_pixel == pData->vinfo.bits_per_pixel) {
        Logd("Framebuffer infos did not changed");
        return FBDEV_ERROR_NONE;
    }

    if (ioctl(pData->fd, FBIOPUT_VSCREENINFO, &vinfo) < 0) {
        Loge("Failed to restore framebuffer infos");
        return FBDEV_ERROR_IO;
    }

    return FBDEV_ERROR_NONE;
}
