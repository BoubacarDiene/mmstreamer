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
* \file V4l2.c
* \brief Implement V4L2 API
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// HEADERS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "utils/Log.h"
#include "video/V4l2.h"

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// MACROS ////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "v4l2"

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////// PUBLIC FUNCTIONS PROTOTYPES //////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

static enum v4l2_error_e openDevice_f(struct v4l2_s *obj,
                                       struct v4l2_open_device_params_s *params);
static enum v4l2_error_e closeDevice_f(struct v4l2_s *obj);

static enum v4l2_error_e configureDevice_f(struct v4l2_s *obj,
                                           struct v4l2_configure_device_params_s *params);
static enum v4l2_error_e setCroppingArea_f(struct v4l2_s *obj,
                                           struct v4l2_selection_params_s *cropRectInOut);
static enum v4l2_error_e setComposingArea_f(struct v4l2_s *obj,
                                            struct v4l2_selection_params_s *composeRectInOut);

static enum v4l2_error_e requestBuffers_f(struct v4l2_s *obj,
                                          struct v4l2_request_buffers_params_s *params);
static enum v4l2_error_e releaseBuffers_f(struct v4l2_s *obj);

static enum v4l2_error_e startCapture_f(struct v4l2_s *obj);
static enum v4l2_error_e stopCapture_f(struct v4l2_s *obj);

static enum v4l2_error_e awaitData_f(struct v4l2_s *obj, int32_t timeout_ms);
static enum v4l2_error_e stopAwaitingData_f(struct v4l2_s *obj);

static enum v4l2_error_e queueBuffer_f(struct v4l2_s *obj, uint32_t index);
static enum v4l2_error_e dequeueBuffer_f(struct v4l2_s *obj);

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////// PRIVATE FUNCTIONS PROTOTYPES /////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

static enum v4l2_error_e v4l2Ioctl_f(int32_t fd, uint64_t req, void *args);

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// INITIALIZER //////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 * \fn enum v4l2_error_e V4l2_Init(struct v4l2_s **obj)
 * \brief Create an instance of V4l2 module
 * \param[in, out] obj
 * \return V4L2_ERROR_NONE on success
 * \return V4L2_ERROR_INIT on error
 */
enum v4l2_error_e V4l2_Init(struct v4l2_s **obj)
{
    ASSERT(obj && (*obj = calloc(1, sizeof(struct v4l2_s))));
    
    (*obj)->openDevice       = openDevice_f;
    (*obj)->closeDevice      = closeDevice_f;
    
    (*obj)->configureDevice  = configureDevice_f;
    (*obj)->setCroppingArea  = setCroppingArea_f;
    (*obj)->setComposingArea = setComposingArea_f;
    
    (*obj)->requestBuffers   = requestBuffers_f;
    (*obj)->releaseBuffers   = releaseBuffers_f;
    
    (*obj)->startCapture     = startCapture_f;
    (*obj)->stopCapture      = stopCapture_f;
    
    (*obj)->awaitData        = awaitData_f;
    (*obj)->stopAwaitingData = stopAwaitingData_f;
    
    (*obj)->queueBuffer      = queueBuffer_f;
    (*obj)->dequeueBuffer    = dequeueBuffer_f;
    
    (*obj)->deviceFd = -1;
    
    return V4L2_ERROR_NONE;
}

/*!
 * \fn enum v4l2_error_e V4l2_UnInit(struct v4l2_s **obj)
 * \brief Destroy object created using V4l2_Init()
 * \param[in, out] obj
 * \return V4L2_ERROR_NONE on success
 * \return V4L2_ERROR_INIT on error
 */
enum v4l2_error_e V4l2_UnInit(struct v4l2_s **obj)
{
    ASSERT(obj && *obj);
    
    free(*obj);
    *obj = NULL;
    
    return V4L2_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////// PUBLIC FUNCTIONS IMPLEMENTATION ///////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 * \fn static enum v4l2_error_e openDevice_f(struct v4l2_s *obj,
 *                                           struct v4l2_open_device_params_s *params)
 * \brief Open video device
 * \param[in] obj
 * \param[in] params
 * \return V4L2_ERROR_NONE on success
 * \return V4L2_ERROR_INIT on error
 */
static enum v4l2_error_e openDevice_f(struct v4l2_s *obj,
                                      struct v4l2_open_device_params_s *params)
{
    ASSERT(obj && params);
    
    enum v4l2_error_e ret = V4L2_ERROR_NONE;
    
    if (access(params->path, F_OK) != 0) {
        Loge("\"%s\" does not exist (Please, disable Videos in Main.xml i.e \"enable=0\" to test other modules)", params->path);
        ret = V4L2_ERROR_UNKNOWN_DEVICE;
        goto exit;
    }
    
    if (pipe(obj->quitFd) < 0) {
        Loge("Failed to create pipe - %s", strerror(errno));
        ret = V4L2_ERROR_IO;
        goto pipe_exit;
    }
    
    if ((obj->deviceFd = open(params->path, O_RDWR)) < 0) {
        Loge("Failed to open \"%s\" - %s", params->path, strerror(errno));
        ret = V4L2_ERROR_IO;
        goto open_exit;
    }
    
    if ((ret = v4l2Ioctl_f(obj->deviceFd, VIDIOC_QUERYCAP, &obj->caps)) != V4L2_ERROR_NONE) {
        Loge("Failed to query capabilities");
        goto caps_exit;
    }
    
    if (!(obj->caps.capabilities & params->caps)) {
        Loge("Requested capapbilities not supported");
        ret = V4L2_ERROR_BAD_CAPS;
        goto caps_exit;
    }
    
    strncpy(obj->path, params->path, sizeof(obj->path));

    return ret;

caps_exit:
    close(obj->deviceFd);
    obj->deviceFd = -1;

open_exit:
    close(obj->quitFd[V4L2_PIPE_READ]);
    obj->quitFd[V4L2_PIPE_READ] = -1;
    close(obj->quitFd[V4L2_PIPE_WRITE]);
    obj->quitFd[V4L2_PIPE_WRITE] = -1;

pipe_exit:
exit:
    return ret;
}

/*!
 * \fn static enum v4l2_error_e closeDevice_f(struct v4l2_s *obj)
 * \brief Close video device
 * \param[in] obj
 * \return V4L2_ERROR_NONE on success
 * \return V4L2_ERROR_INIT on error
 */
static enum v4l2_error_e closeDevice_f(struct v4l2_s *obj)
{
    ASSERT(obj);
    
    if (obj->deviceFd != -1) {
        close(obj->deviceFd);
        obj->deviceFd = -1;
    }

    if (obj->quitFd[V4L2_PIPE_READ] != -1) {
        close(obj->quitFd[V4L2_PIPE_READ]);
        close(obj->quitFd[V4L2_PIPE_WRITE]);
    }

    return V4L2_ERROR_NONE;
}

/*!
 * \fn static enum v4l2_error_e configureDevice_f(struct v4l2_s *obj,
 *                                                struct v4l2_configure_device_params_s *params)
 * \brief Configure video device
 * \param[in] obj
 * \param[in] params
 * \return V4L2_ERROR_NONE on success
 * \return V4L2_ERROR_INIT on error
 */
static enum v4l2_error_e configureDevice_f(struct v4l2_s *obj,
                                           struct v4l2_configure_device_params_s *params)
{
    ASSERT(obj && (obj->deviceFd != -1) && params);
    
    enum v4l2_error_e ret = V4L2_ERROR_NONE;

    /* Set format */
    obj->format.type = params->type;
    if ((ret = v4l2Ioctl_f(obj->deviceFd, VIDIOC_G_FMT, &obj->format)) != V4L2_ERROR_NONE) {
        Loge("Failed to get video format");
        goto exit;
    }

    obj->format.fmt.pix.width       = params->width;
    obj->format.fmt.pix.height      = params->height;
    obj->format.fmt.pix.pixelformat = params->pixelformat;
    obj->format.fmt.pix.colorspace  = params->colorspace;

    if ((ret = v4l2Ioctl_f(obj->deviceFd, VIDIOC_S_FMT, &obj->format)) != V4L2_ERROR_NONE) {
        Loge("Failed to set video format");
        goto exit;
    }
    
    /* Set framerate */
    struct v4l2_streamparm streamparm = {0};
    streamparm.type = params->type;
    if ((ret = v4l2Ioctl_f(obj->deviceFd, VIDIOC_G_PARM, &streamparm)) != V4L2_ERROR_NONE) {
        Loge("Failed to get current framerate");
        goto exit;
    }

    Logd("Current framerate : %d fps", streamparm.parm.output.timeperframe.denominator);

    if (streamparm.parm.capture.capability & V4L2_CAP_TIMEPERFRAME) {
        streamparm.parm.capture.timeperframe.numerator = 1;
        streamparm.parm.capture.timeperframe.denominator = params->desiredFps;

        /* Set fps */
        if ((ret = v4l2Ioctl_f(obj->deviceFd, VIDIOC_S_PARM, &streamparm)) != V4L2_ERROR_NONE) {
            Loge("Failed to change framerate");
            goto exit;
        }

        /* Check new fps */
        if ((ret = v4l2Ioctl_f(obj->deviceFd, VIDIOC_G_PARM, &streamparm)) != V4L2_ERROR_NONE) {
             goto exit;
        }

        Logd("New framerate : %d fps / Requested : %d fps",
                streamparm.parm.output.timeperframe.denominator, params->desiredFps);
    }
    else {
        Loge("Your driver does not allow to update framerate");
    }
    
exit:
    return ret;
}

/*!
 * \fn static enum v4l2_error_e setCroppingArea_f(struct v4l2_s *obj,
 *                                                struct v4l2_selection_params_s *cropRectInOut)
 * \brief Set cropping area
 * \param[in] obj
 * \param[in] cropRectInOut
 * \return V4L2_ERROR_NONE on success
 * \return V4L2_ERROR_INIT on error
 */
static enum v4l2_error_e setCroppingArea_f(struct v4l2_s *obj,
                                           struct v4l2_selection_params_s *cropRectInOut)
{
    ASSERT(obj && (obj->deviceFd != -1) && cropRectInOut);
    
    enum v4l2_error_e ret = V4L2_ERROR_NONE;

    Logd("Requested : left = %d /  top = %d / width = %u / height = %u",
            cropRectInOut->left, cropRectInOut->top, cropRectInOut->width, cropRectInOut->height);

    struct v4l2_selection sel = {
        .type   = obj->format.type,
        .target = V4L2_SEL_TGT_CROP_DEFAULT,
    };

    if ((ret = v4l2Ioctl_f(obj->deviceFd, VIDIOC_G_SELECTION, &sel)) != V4L2_ERROR_NONE) {
        Loge("Failed to get V4L2_SEL_TGT_CROP_DEFAUL");
        return ret;
    }

    Logd("Default : left = %d / top = %d / width = %u / height = %u",
            sel.r.left, sel.r.top, sel.r.width, sel.r.height);

    struct v4l2_rect r;

    if ((cropRectInOut->left >= sel.r.left)
        && (cropRectInOut->left <= sel.r.left + (int32_t)sel.r.width)) {
        r.left = cropRectInOut->left;
    }
    else {
        r.left = sel.r.left;
    }

    if ((cropRectInOut->top >= sel.r.top)
        && (cropRectInOut->top <= sel.r.top + (int32_t)sel.r.height)) {
        r.top = cropRectInOut->top;
    }
    else {
        r.top = sel.r.top;
    }

    if (r.left + (int32_t)cropRectInOut->width <= sel.r.left + (int32_t)sel.r.width) {
        r.width = cropRectInOut->width;
    }
    else {
        r.width = (uint32_t)((int32_t)sel.r.width + sel.r.left - r.left);
    }

    if (r.top + (int32_t)cropRectInOut->height <= sel.r.top + (int32_t)sel.r.height) {
        r.height = cropRectInOut->height;
    }
    else {
        r.height = (uint32_t)((int32_t)sel.r.height + sel.r.top - r.top);
    }

    Logd("Setting active cropping area to : left = %d /  top = %d / width = %u / height = %u",
            r.left, r.top, r.width, r.height);

    sel.r      = r;
    sel.target = V4L2_SEL_TGT_CROP_ACTIVE;

    if ((ret = v4l2Ioctl_f(obj->deviceFd, VIDIOC_S_SELECTION, &sel)) != V4L2_ERROR_NONE) {
        Loge("Failed to set V4L2_SEL_TGT_CROP_ACTIVE");
        return ret;
    }
	
    cropRectInOut->left   = r.left;
    cropRectInOut->top    = r.top;
    cropRectInOut->width  = r.width;
    cropRectInOut->height = r.height;

    Logd("Active cropping area updated : left = %d /  top = %d / width = %u / height = %u",
            sel.r.left, sel.r.top, sel.r.width, sel.r.height);
	
    return ret;
}

/*!
 * \fn static enum v4l2_error_e setComposingArea_f(struct v4l2_s *obj,
 *                                                 struct v4l2_selection_params_s *composeRectInOut)
 * \brief Set composing area
 * \param[in] obj
 * \param[in] composeRectInOut
 * \return V4L2_ERROR_NONE on success
 * \return V4L2_ERROR_INIT on error
 */
static enum v4l2_error_e setComposingArea_f(struct v4l2_s *obj,
                                            struct v4l2_selection_params_s *composeRectInOut)
{
    ASSERT(obj && (obj->deviceFd != -1) && composeRectInOut);
    
    enum v4l2_error_e ret = V4L2_ERROR_NONE;

    Logd("Requested : left = %d /  top = %d / width = %u / height = %u",
            composeRectInOut->left, composeRectInOut->top,
            composeRectInOut->width, composeRectInOut->height);

    struct v4l2_selection sel = {
        .type   = obj->format.type,
        .target = V4L2_SEL_TGT_COMPOSE_DEFAULT,
    };

    if ((ret = v4l2Ioctl_f(obj->deviceFd, VIDIOC_G_SELECTION, &sel)) != V4L2_ERROR_NONE) {
        Loge("Failed to get V4L2_SEL_TGT_COMPOSE_DEFAULT)");
        return ret;
    }

    Logd("Default : left = %d / top = %d / width = %u / height = %u",
            sel.r.left, sel.r.top, sel.r.width, sel.r.height);

    struct v4l2_rect r;

    if ((composeRectInOut->left >= sel.r.left)
        && (composeRectInOut->left <= sel.r.left + (int32_t)sel.r.width)) {
        r.left = composeRectInOut->left;
    }
    else {
        r.left = sel.r.left;
    }

    if ((composeRectInOut->top >= sel.r.top)
        && (composeRectInOut->top <= sel.r.top + (int32_t)sel.r.height)) {
        r.top = composeRectInOut->top;
    }
    else {
        r.top = sel.r.top;
    }

    if (r.left + (int32_t)composeRectInOut->width <= sel.r.left + (int32_t)sel.r.width) {
        r.width = composeRectInOut->width;
    }
    else {
        r.width = (uint32_t)((int32_t)sel.r.width + sel.r.left - r.left);
    }

    if (r.top + (int32_t)composeRectInOut->height <= sel.r.top + (int32_t)sel.r.height) {
        r.height = composeRectInOut->height;
    }
    else {
        r.height = (uint32_t)((int32_t)sel.r.height + sel.r.top - r.top);
    }

    Logd("Setting active composing area to : left = %d /  top = %d / width = %u / height = %u",
            r.left, r.top, r.width, r.height);

    sel.r      = r;
    sel.target = V4L2_SEL_TGT_COMPOSE_ACTIVE;
    sel.flags  = V4L2_SEL_FLAG_LE;

    if ((ret = v4l2Ioctl_f(obj->deviceFd, VIDIOC_S_SELECTION, &sel)) != V4L2_ERROR_NONE) {
        Loge("Failed to set V4L2_SEL_TGT_COMPOSE_ACTIVE");
        return ret;
    }
	
    composeRectInOut->left   = r.left;
    composeRectInOut->top    = r.top;
    composeRectInOut->width  = r.width;
    composeRectInOut->height = r.height;

    Logd("Active composing area updated : left = %d /  top = %d / width = %u / height = %u",
            sel.r.left, sel.r.top, sel.r.width, sel.r.height);
	
    return ret;
}

/*!
 * \fn static enum v4l2_error_e requestBuffers_f(struct v4l2_s *obj,
 *                                               struct v4l2_request_buffers_params_s *params)
 * \brief Request video buffers
 * \param[in] obj
 * \param[in] params
 * \return V4L2_ERROR_NONE on success
 * \return V4L2_ERROR_INIT on error
 */
static enum v4l2_error_e requestBuffers_f(struct v4l2_s *obj,
                                          struct v4l2_request_buffers_params_s *params)
{
    ASSERT(obj && (obj->deviceFd != -1) && params);
    
    enum v4l2_error_e ret = V4L2_ERROR_NONE;

    struct v4l2_requestbuffers req = {0};
    struct v4l2_buffer buf         = {0};

    obj->memory        = params->memory;
    obj->maxBufferSize = 0;

    req.count  = params->count;
    req.type   = obj->format.type;
    req.memory = obj->memory;

    if (v4l2Ioctl_f(obj->deviceFd, VIDIOC_REQBUFS, &req) != V4L2_ERROR_NONE) {
        Loge("Failed to request buffers");
        return V4L2_ERROR_IO;
    }

    if (req.count < params->count) {
        Loge("Allocated number of buffers not enough");
        ret = V4L2_ERROR_MEMORY;
        goto exit;
    }
    
    obj->nbBuffers = req.count;
    ASSERT((obj->map = calloc(obj->nbBuffers, sizeof(struct v4l2_mapping_buffer_s))));

    uint32_t i;
    for (i = 0; i < obj->nbBuffers; i++) {
        buf.type   = req.type;
        buf.memory = req.memory;
        buf.index  = i;

        if (v4l2Ioctl_f(obj->deviceFd, VIDIOC_QUERYBUF, &buf) != V4L2_ERROR_NONE) {
            Loge("Failed to query buffer");
            ret = V4L2_ERROR_IO;
            goto exit;
        }

        obj->map[i].index  = buf.index;
        obj->map[i].length = buf.length;
        obj->map[i].offset = buf.m.offset;
        
        switch (params->memory) {
            case V4L2_MEMORY_MMAP:
                obj->map[i].start = mmap(NULL /* start anywhere */,
                                          buf.length,
                                          PROT_READ | PROT_WRITE,
                                          MAP_SHARED,
                                          obj->deviceFd, buf.m.offset);

                if (obj->map[i].start == MAP_FAILED) {
                    Loge("mmap() failed");
                    ret = V4L2_ERROR_MEMORY;
                    goto exit;
                }
                break;
                
            case V4L2_MEMORY_USERPTR:
                ASSERT((obj->map[i].start = calloc(1, buf.length)));
                break;
                
            default:
                ;
        }
        
        if (obj->maxBufferSize < buf.length) {
            obj->maxBufferSize = buf.length;
        }

        memset(&buf, 0, sizeof(struct v4l2_buffer));
    }
    
    return V4L2_ERROR_NONE;

exit:
    releaseBuffers_f(obj);
    
    return ret;
}

/*!
 * \fn static enum v4l2_error_e releaseBuffers_f(struct v4l2_s *obj)
 * \brief Release allocated video buffers
 * \param[in] obj
 * \return V4L2_ERROR_NONE on success
 * \return V4L2_ERROR_INIT on error
 */
static enum v4l2_error_e releaseBuffers_f(struct v4l2_s *obj)
{
    ASSERT(obj && (obj->deviceFd != -1));

    if (obj->map) {
        uint32_t i;

        switch (obj->memory) {
            case V4L2_MEMORY_MMAP:
                for (i = 0; i < obj->nbBuffers; i++) {
                    if (obj->map[i].start
                        && munmap(obj->map[i].start, obj->map[i].length) < 0) {
                        Loge("munmap() failed");
                    }
                }
                break;

            case V4L2_MEMORY_USERPTR:
                for (i = 0; i < obj->nbBuffers; i++) {
                    if (obj->map[i].start) {
                        free(obj->map[i].start);
                    }
                }
                break;
                
            default:
                ;
        }

        free(obj->map);
        obj->map = NULL;
    }

    struct v4l2_requestbuffers req = {0};
    req.type   = obj->format.type;
    req.memory = obj->memory;
    
    if (v4l2Ioctl_f(obj->deviceFd, VIDIOC_REQBUFS, &req) != V4L2_ERROR_NONE) {
        Loge("Failed to release buffers");
    }
    
    return V4L2_ERROR_NONE;
}

/*!
 * \fn static enum v4l2_error_e startCapture_f(struct v4l2_s *obj)
 * \brief Start video capture
 * \param[in] obj
 * \return V4L2_ERROR_NONE on success
 * \return V4L2_ERROR_INIT on error
 */
static enum v4l2_error_e startCapture_f(struct v4l2_s *obj)
{
    ASSERT(obj && (obj->deviceFd != -1));
    
    if (v4l2Ioctl_f(obj->deviceFd, VIDIOC_STREAMON, &obj->format.type) != V4L2_ERROR_NONE) {
        Loge("Failed to start stream");
        return V4L2_ERROR_CAPTURE;
    }
    
    return V4L2_ERROR_NONE;
}

/*!
 * \fn static enum v4l2_error_e stopCapture_f(struct v4l2_s *obj)
 * \brief Stop video capture
 * \param[in] obj
 * \return V4L2_ERROR_NONE on success
 * \return V4L2_ERROR_INIT on error
 */
static enum v4l2_error_e stopCapture_f(struct v4l2_s *obj)
{
    ASSERT(obj && (obj->deviceFd != -1));
    
    if (v4l2Ioctl_f(obj->deviceFd, VIDIOC_STREAMOFF, &obj->format.type) != V4L2_ERROR_NONE) {
        Loge("Failed to stop stream");
        return V4L2_ERROR_CAPTURE;
    }
    
    return V4L2_ERROR_NONE;
}

/*!
 * \fn static enum v4l2_error_e awaitData_f(struct v4l2_s *obj, int32_t timeout_ms)
 * \brief Wait a given duration
 * \param[in] obj
 * \param[in] timeout_ms : Time in ms to wait before returning / -1 => not used
 * \return V4L2_ERROR_NONE on success
 * \return V4L2_ERROR_INIT on error
 */
static enum v4l2_error_e awaitData_f(struct v4l2_s *obj, int32_t timeout_ms)
{
    ASSERT(obj && (obj->deviceFd != -1));
    
    int32_t selectRetval;
    int32_t maxFd;
    fd_set fds;
    struct timeval *tv    = NULL;
    enum v4l2_error_e ret = V4L2_ERROR_NONE;

    FD_ZERO(&fds);
    FD_SET(obj->deviceFd, &fds);
    FD_SET(obj->quitFd[V4L2_PIPE_READ], &fds);

    maxFd = (obj->deviceFd > obj->quitFd[V4L2_PIPE_READ] ? obj->deviceFd : obj->quitFd[V4L2_PIPE_READ]);

    if (timeout_ms > 0) {
        ASSERT((tv = calloc(1, sizeof(struct timeval))));
        tv->tv_sec  = 0;
        tv->tv_usec = timeout_ms * 1000;
    }

    selectRetval = select(maxFd + 1, &fds, NULL, NULL, tv);
    
    if (tv) {
        free(tv);
    }

    if (selectRetval == -1) { /* Interrupted */
        goto exit;
    }

    if (selectRetval == 0) { /* Timeout */
        ret = V4L2_ERROR_TIMEOUT;
        goto exit;
    }
    
    return V4L2_ERROR_NONE;
    
exit:
    return ret;
}

/*!
 * \fn static enum v4l2_error_e stopAwaitingData_f(struct v4l2_s *obj)
 * \brief Avoid deadlock in awaitData()
 * \param[in] obj
 * \return V4L2_ERROR_NONE on success
 * \return V4L2_ERROR_IO on error
 */
static enum v4l2_error_e stopAwaitingData_f(struct v4l2_s *obj)
{
    ASSERT(obj && (obj->quitFd[V4L2_PIPE_WRITE] != -1));

    if (write(obj->quitFd[V4L2_PIPE_WRITE], "\n", 1) < 0) {
        Loge("Writing to pipe failed - %s", strerror(errno));
        return V4L2_ERROR_IO;
    }

    return V4L2_ERROR_NONE;
}

/*!
 * \fn static enum v4l2_error_e queueBuffer_f(struct v4l2_s *obj, uint32_t index)
 * \brief Queue video buffer
 * \param[in] obj
 * \param[in] index
 * \return V4L2_ERROR_NONE on success
 * \return V4L2_ERROR_INIT on error
 */
static enum v4l2_error_e queueBuffer_f(struct v4l2_s *obj, uint32_t index)
{
    ASSERT(obj && (obj->deviceFd != -1));
    
    struct v4l2_buffer buffer = {0};

    buffer.type   = obj->format.type;
    buffer.memory = obj->memory;
    buffer.index  = index;

    if (buffer.memory == V4L2_MEMORY_USERPTR) {
        buffer.m.userptr = (uint64_t)obj->map[index].start;
        buffer.length    = (uint32_t)obj->map[index].length;
    }

    return v4l2Ioctl_f(obj->deviceFd, VIDIOC_QBUF, &buffer);
}

/*!
 * \fn static enum v4l2_error_e dequeueBuffer_f(struct v4l2_s *obj)
 * \brief Dequeue video buffer
 * \param[in] obj
 * \return V4L2_ERROR_NONE on success
 * \return V4L2_ERROR_INIT on error
 */
static enum v4l2_error_e dequeueBuffer_f(struct v4l2_s *obj)
{
    ASSERT(obj && (obj->deviceFd != -1));
    
    struct v4l2_buffer buffer = {0};

    buffer.type   = obj->format.type;
    buffer.memory = obj->memory;

    if (v4l2Ioctl_f(obj->deviceFd, VIDIOC_DQBUF, &buffer) != V4L2_ERROR_NONE) {
        Loge("Failed to dequeue buffer");
        return V4L2_ERROR_IO;
    }

    uint32_t i = 0;
    switch (buffer.memory) {
        case V4L2_MEMORY_MMAP:
            i = buffer.index;
            break;
            
        case V4L2_MEMORY_USERPTR:
            for (i = 0; i < obj->nbBuffers; i++) {
                if ((buffer.m.userptr == (uint64_t)obj->map[i].start)
                    && (buffer.length == obj->map[i].length)) {
                    break;
                }
            }
            break;
            
        default:
            ;
    }
    
    ASSERT(i < obj->nbBuffers);

    return V4L2_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/* ///////////////////////////// PRIVATE FUNCTIONS IMPLEMENTATION ///////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 * Custom ioctl with retries when interrupted
 */
static enum v4l2_error_e v4l2Ioctl_f(int32_t fd, uint64_t req, void *args)
{
    int32_t ret;

    do {
        ret = ioctl(fd, req, args);
    }
    while ((-1 == ret) && (EINTR == errno));
    
    if (ret == -1) {
        Loge("ioctl() failed - %s", strerror(errno));
        return V4L2_ERROR_IO;
    }
    
    return V4L2_ERROR_NONE;
}
