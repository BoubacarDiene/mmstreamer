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
* \file   FbDev.c
* \brief  Framebuffer management
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>

#include "utils/Log.h"

#include "graphics/FbDev.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                            */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "FbDev"

#define FBDEV_MAX_PATH_SIZE 16
#define FBDEV_PATH_PREFIX   "/dev/"

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

typedef struct FBDEV_PRIVATE_DATA_S {
    FBDEV_VSCREENINFO vinfo;
    FBDEV_FSCREENINFO finfo;

    char              path[FBDEV_MAX_PATH_SIZE];
    int32_t           fd;
    char              *map;

    volatile uint8_t  opened;
} FBDEV_PRIVATE_DATA_S;

/* -------------------------------------------------------------------------------------------- */
/*                                          VARIABLES                                           */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                         PROTOTYPES                                           */
/* -------------------------------------------------------------------------------------------- */

static FBDEV_ERROR_E open_f    (FBDEV_S *obj, const char *fbName);
static FBDEV_ERROR_E isOpened_f(FBDEV_S *obj, uint8_t *opened);
static FBDEV_ERROR_E close_f   (FBDEV_S *obj);

static FBDEV_ERROR_E getInfos_f(FBDEV_S *obj, FBDEV_INFOS_S *fbInfos);
static FBDEV_ERROR_E setDepth_f(FBDEV_S *obj, uint32_t depth);

static FBDEV_ERROR_E clear_f  (FBDEV_S *obj);
static FBDEV_ERROR_E restore_f(FBDEV_S *obj);

/* -------------------------------------------------------------------------------------------- */
/*                                      PUBLIC FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
FBDEV_ERROR_E FbDev_Init(FBDEV_S **obj)
{
    assert(obj && (*obj = calloc(1, sizeof(FBDEV_S))));

    FBDEV_PRIVATE_DATA_S *pData;
    assert((pData = calloc(1, sizeof(FBDEV_PRIVATE_DATA_S))));

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
FBDEV_ERROR_E FbDev_UnInit(FBDEV_S **obj)
{
    assert(obj && *obj && (*obj)->pData);
    
    FBDEV_PRIVATE_DATA_S *pData = (FBDEV_PRIVATE_DATA_S*)((*obj)->pData);
    
    free(pData);
    pData = NULL;
    
    free(*obj);
    *obj = NULL;
    
    return FBDEV_ERROR_NONE;
}	

/* -------------------------------------------------------------------------------------------- */
/*                                     PRIVATE FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static FBDEV_ERROR_E open_f(FBDEV_S *obj, const char *fbName)
{
    assert(obj && obj->pData && fbName);

    FBDEV_PRIVATE_DATA_S *pData = (FBDEV_PRIVATE_DATA_S*)(obj->pData);

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
static FBDEV_ERROR_E isOpened_f(FBDEV_S *obj, uint8_t *opened)
{
    assert(obj && obj->pData && opened);

    FBDEV_PRIVATE_DATA_S *pData = (FBDEV_PRIVATE_DATA_S*)(obj->pData);

    *opened = pData->opened;

    return FBDEV_ERROR_NONE;
}

/*!
 *
 */
static FBDEV_ERROR_E close_f(FBDEV_S *obj)
{
    assert(obj && obj->pData);

    FBDEV_PRIVATE_DATA_S *pData = (FBDEV_PRIVATE_DATA_S*)(obj->pData);

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
static FBDEV_ERROR_E getInfos_f(FBDEV_S *obj, FBDEV_INFOS_S *fbInfos)
{
    assert(obj && obj->pData && fbInfos);

    FBDEV_PRIVATE_DATA_S *pData = (FBDEV_PRIVATE_DATA_S*)(obj->pData);

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
            pData->finfo.smem_len, pData->vinfo.xres, pData->vinfo.yres, pData->vinfo.bits_per_pixel);

    fbInfos->width  = pData->vinfo.xres;
    fbInfos->height = pData->vinfo.yres;
    fbInfos->depth  = pData->vinfo.bits_per_pixel;

    return FBDEV_ERROR_NONE;
}

/*!
 *
 */
static FBDEV_ERROR_E setDepth_f(FBDEV_S *obj, uint32_t depth)
{
    assert(obj && obj->pData);

    FBDEV_PRIVATE_DATA_S *pData = (FBDEV_PRIVATE_DATA_S*)(obj->pData);

    if (!pData->opened) {
        Loge("No framebuffer device opened yet");
        return FBDEV_ERROR_IO;
    }

    Logd("Updating depth : current = %u / new = %u", pData->vinfo.bits_per_pixel, depth);

    FBDEV_VSCREENINFO vinfo;
    memcpy(&vinfo, &pData->vinfo, sizeof(FBDEV_VSCREENINFO));

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
static FBDEV_ERROR_E clear_f(FBDEV_S *obj)
{
    assert(obj && obj->pData);

    FBDEV_PRIVATE_DATA_S *pData = (FBDEV_PRIVATE_DATA_S*)(obj->pData);

    if (!pData->opened) {
        Loge("No framebuffer device opened yet");
        return FBDEV_ERROR_IO;
    }

    Logd("Clearing framebuffer device \"%s\"", pData->path);

    pData->map = (char*)mmap(0,
                                pData->finfo.smem_len,
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
static FBDEV_ERROR_E restore_f(FBDEV_S *obj)
{
    assert(obj && obj->pData);

    FBDEV_PRIVATE_DATA_S *pData = (FBDEV_PRIVATE_DATA_S*)(obj->pData);

    if (!pData->opened) {
        Loge("No framebuffer device opened yet");
        return FBDEV_ERROR_IO;
    }

    Logd("Restoring framebuffer device \"%s\"", pData->path);

    FBDEV_VSCREENINFO vinfo;
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
