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
* \file   V4l2.c
* \brief  Implement V4L2 API
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "utils/Log.h"
#include "video/V4l2.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                            */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "V4L2"

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                          VARIABLES                                           */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                         PROTOTYPES                                           */
/* -------------------------------------------------------------------------------------------- */

static V4L2_ERROR_E openDevice_f (V4L2_S *obj, V4L2_OPEN_DEVICE_PARAMS_S *params);
static V4L2_ERROR_E closeDevice_f(V4L2_S *obj);

static V4L2_ERROR_E configureDevice_f (V4L2_S *obj, V4L2_CONFIGURE_DEVICE_PARAMS_S *params);
static V4L2_ERROR_E setCroppingArea_f (V4L2_S *obj, V4L2_SELECTION_PARAMS_S *cropRectInOut);
static V4L2_ERROR_E setComposingArea_f(V4L2_S *obj, V4L2_SELECTION_PARAMS_S *composeRectInOut);
                                      
static V4L2_ERROR_E requestBuffers_f(V4L2_S *obj, V4L2_REQUEST_BUFFERS_PARAMS_S *params);
static V4L2_ERROR_E releaseBuffers_f(V4L2_S *obj);

static V4L2_ERROR_E startCapture_f(V4L2_S *obj);
static V4L2_ERROR_E stopCapture_f (V4L2_S *obj);

static V4L2_ERROR_E awaitData_f(V4L2_S *obj, int32_t timeout_ms);

static V4L2_ERROR_E queueBuffer_f  (V4L2_S *obj, uint32_t index);
static V4L2_ERROR_E dequeueBuffer_f(V4L2_S *obj);

static V4L2_ERROR_E v4l2Ioctl_f(int32_t fd, uint64_t req, void *args);

/* -------------------------------------------------------------------------------------------- */
/*                                      PUBLIC FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

/*!
 * \fn             V4L2_ERROR_E V4l2_Init(V4L2_S **obj)
 * \brief          Create an instance of V4l2 module
 * \param[in, out] obj
 * \return         V4L2_ERROR_NONE on success
 *                 V4L2_ERROR_INIT on error
 */
V4L2_ERROR_E V4l2_Init(V4L2_S **obj)
{
    assert(obj && (*obj = calloc(1, sizeof(V4L2_S))));
    
    (*obj)->fd  = -1;
    (*obj)->map = NULL;
    
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
    
    (*obj)->queueBuffer      = queueBuffer_f;
    (*obj)->dequeueBuffer    = dequeueBuffer_f;
    
    return V4L2_ERROR_NONE;
}

/*!
 * \fn             V4L2_ERROR_E V4l2_UnInit(V4L2_S **obj)
 * \brief          Destroy object created using V4l2_Init()
 * \param[in, out] obj
 * \return         V4L2_ERROR_NONE on success
 *                 V4L2_ERROR_INIT on error
 */
V4L2_ERROR_E V4l2_UnInit(V4L2_S **obj)
{
    assert(obj && *obj);
    
    free(*obj);
    *obj = NULL;
    
    return V4L2_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/*                                     PRIVATE FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

/*!
 * \fn        static V4L2_ERROR_E openDevice_f(V4L2_S *obj, V4L2_OPEN_DEVICE_PARAMS_S *params)
 * \brief     Open video device
 * \param[in] obj
 * \param[in] params
 * \return    V4L2_ERROR_NONE on success
 *            V4L2_ERROR_INIT on error
 */
static V4L2_ERROR_E openDevice_f(V4L2_S *obj, V4L2_OPEN_DEVICE_PARAMS_S *params)
{
    assert(obj && params);
    
    V4L2_ERROR_E ret = V4L2_ERROR_NONE;
    
    if (access(params->path, F_OK) != 0) {
        Loge("\"%s\" does not exist", params->path);
        ret = V4L2_ERROR_UNKNOWN_DEVICE;
        goto exit;
    }
    
    if ((obj->fd = open(params->path, O_RDWR)) < 0) {
        Loge("Failed to open \"%s\"", params->path);
        ret = V4L2_ERROR_IO;
        goto open_exit;
    }
    
    if ((ret = v4l2Ioctl_f(obj->fd, VIDIOC_QUERYCAP, &obj->caps)) != V4L2_ERROR_NONE) {
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
    close(obj->fd);
    obj->fd = -1;

open_exit:
exit:
    return ret;
}

/*!
 * \fn        static V4L2_ERROR_E closeDevice_f(V4L2_S *obj)
 * \brief     Close video device
 * \param[in] obj
 * \return    V4L2_ERROR_NONE on success
 *            V4L2_ERROR_INIT on error
 */
static V4L2_ERROR_E closeDevice_f(V4L2_S *obj)
{
    assert(obj);
    
    if (obj->fd != -1) {
        close(obj->fd);
        obj->fd = -1;
    }
    
    return V4L2_ERROR_NONE;
}

/*!
 * \fn        static V4L2_ERROR_E configureDevice_f(V4L2_S *obj, V4L2_CONFIGURE_DEVICE_PARAMS_S *params)
 * \brief     Configure video device
 * \param[in] obj
 * \param[in] params
 * \return    V4L2_ERROR_NONE on success
 *            V4L2_ERROR_INIT on error
 */
static V4L2_ERROR_E configureDevice_f(V4L2_S *obj, V4L2_CONFIGURE_DEVICE_PARAMS_S *params)
{
    assert(obj && (obj->fd != -1) && params);
    
    V4L2_ERROR_E ret = V4L2_ERROR_NONE;

    /* Set format */
    obj->format.type = params->type;
    if ((ret = v4l2Ioctl_f(obj->fd, VIDIOC_G_FMT, &obj->format)) != V4L2_ERROR_NONE) {
        Loge("Failed to get video format");
        goto exit;
    }

    obj->format.fmt.pix.width       = params->width;
    obj->format.fmt.pix.height      = params->height;
    obj->format.fmt.pix.pixelformat = params->pixelformat;
    obj->format.fmt.pix.colorspace  = params->colorspace;

    if ((ret = v4l2Ioctl_f(obj->fd, VIDIOC_S_FMT, &obj->format)) != V4L2_ERROR_NONE) {
        Loge("Failed to set video format");
        goto exit;
    }
    
    /* Set framerate */
    struct v4l2_streamparm streamparm;
    memset(&streamparm, '\0', sizeof(struct v4l2_streamparm));
    streamparm.type = params->type;
    if ((ret = v4l2Ioctl_f(obj->fd, VIDIOC_G_PARM, &streamparm)) != V4L2_ERROR_NONE) {
        Loge("Failed to get current framerate");
        goto exit;
    }

    Logd("Current framerate : %d fps", streamparm.parm.output.timeperframe.denominator);

    if (streamparm.parm.capture.capability & V4L2_CAP_TIMEPERFRAME) {
        streamparm.parm.capture.timeperframe.numerator = 1;
        streamparm.parm.capture.timeperframe.denominator = params->desiredFps;

        /* Set fps */
        if ((ret = v4l2Ioctl_f(obj->fd, VIDIOC_S_PARM, &streamparm)) != V4L2_ERROR_NONE) {
            Loge("Failed to change framerate");
            goto exit;
        }

        /* Check new fps */
        if ((ret = v4l2Ioctl_f(obj->fd, VIDIOC_G_PARM, &streamparm)) != V4L2_ERROR_NONE) {
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
 * \fn        static V4L2_ERROR_E setCroppingArea_f(V4L2_S *obj, V4L2_SELECTION_PARAMS_S *cropRectInOut)
 * \brief     Set cropping area
 * \param[in] obj
 * \param[in] cropRectInOut
 * \return    V4L2_ERROR_NONE on success
 *            V4L2_ERROR_INIT on error
 */
static V4L2_ERROR_E setCroppingArea_f(V4L2_S *obj, V4L2_SELECTION_PARAMS_S *cropRectInOut)
{
    assert(obj && (obj->fd != -1) && cropRectInOut);
    
    V4L2_ERROR_E ret = V4L2_ERROR_NONE;

    Logd("Requested : left = %d /  top = %d / width = %u / height = %u", cropRectInOut->left, cropRectInOut->top, cropRectInOut->width, cropRectInOut->height);

    struct v4l2_selection sel = {
        .type   = obj->format.type,
        .target = V4L2_SEL_TGT_CROP_DEFAULT,
    };

    if ((ret = v4l2Ioctl_f(obj->fd, VIDIOC_G_SELECTION, &sel)) != V4L2_ERROR_NONE) {
        Loge("Failed to get V4L2_SEL_TGT_CROP_DEFAUL");
        return ret;
    }

    Logd("Default : left = %d / top = %d / width = %u / height = %u", sel.r.left, sel.r.top, sel.r.width, sel.r.height);

    struct v4l2_rect r;

    if ((cropRectInOut->left >= sel.r.left) && (cropRectInOut->left <= sel.r.left + (int32_t)sel.r.width)) {
        r.left = cropRectInOut->left;
    }
    else {
        r.left = sel.r.left;
    }

    if ((cropRectInOut->top >= sel.r.top) && (cropRectInOut->top <= sel.r.top + (int32_t)sel.r.height)) {
        r.top = cropRectInOut->top;
    }
    else {
        r.top = sel.r.top;
    }

    if (r.left + (int32_t)cropRectInOut->width <= sel.r.left + (int32_t)sel.r.width) {
        r.width = cropRectInOut->width;
    }
    else {
        r.width = (int32_t)sel.r.width + sel.r.left - r.left;
    }

    if (r.top + (int32_t)cropRectInOut->height <= sel.r.top + (int32_t)sel.r.height) {
        r.height = cropRectInOut->height;
    }
    else {
        r.height = (int32_t)sel.r.height + sel.r.top - r.top;
    }

    Logd("Setting active cropping area to : left = %d /  top = %d / width = %u / height = %u", r.left, r.top, r.width, r.height);

    sel.r      = r;
    sel.target = V4L2_SEL_TGT_CROP_ACTIVE;

    if ((ret = v4l2Ioctl_f(obj->fd, VIDIOC_S_SELECTION, &sel)) != V4L2_ERROR_NONE) {
        Loge("Failed to set V4L2_SEL_TGT_CROP_ACTIVE");
        return ret;
    }
	
    cropRectInOut->left   = r.left;
    cropRectInOut->top    = r.top;
    cropRectInOut->width  = r.width;
    cropRectInOut->height = r.height;

    Logd("Active cropping area updated : left = %d /  top = %d / width = %u / height = %u", sel.r.left, sel.r.top, sel.r.width, sel.r.height);
	
    return ret;
}

/*!
 * \fn        static V4L2_ERROR_E setComposingArea_f(V4L2_S *obj, V4L2_SELECTION_PARAMS_S *composeRectInOut)
 * \brief     Set composing area
 * \param[in] obj
 * \param[in] composeRectInOut
 * \return    V4L2_ERROR_NONE on success
 *            V4L2_ERROR_INIT on error
 */
static V4L2_ERROR_E setComposingArea_f(V4L2_S *obj, V4L2_SELECTION_PARAMS_S *composeRectInOut)
{
    assert(obj && (obj->fd != -1) && composeRectInOut);
    
    V4L2_ERROR_E ret = V4L2_ERROR_NONE;

    Logd("Requested : left = %d /  top = %d / width = %u / height = %u", composeRectInOut->left, composeRectInOut->top, composeRectInOut->width, composeRectInOut->height);

    struct v4l2_selection sel = {
        .type   = obj->format.type,
        .target = V4L2_SEL_TGT_COMPOSE_DEFAULT,
    };

    if ((ret = v4l2Ioctl_f(obj->fd, VIDIOC_G_SELECTION, &sel)) != V4L2_ERROR_NONE) {
        Loge("Failed to get V4L2_SEL_TGT_COMPOSE_DEFAULT)");
        return ret;
    }

    Logd("Default : left = %d / top = %d / width = %u / height = %u", sel.r.left, sel.r.top, sel.r.width, sel.r.height);

    struct v4l2_rect r;

    if ((composeRectInOut->left >= sel.r.left) && (composeRectInOut->left <= sel.r.left + (int32_t)sel.r.width)) {
        r.left = composeRectInOut->left;
    }
    else {
        r.left = sel.r.left;
    }

    if ((composeRectInOut->top >= sel.r.top) && (composeRectInOut->top <= sel.r.top + (int32_t)sel.r.height)) {
        r.top = composeRectInOut->top;
    }
    else {
        r.top = sel.r.top;
    }

    if (r.left + (int32_t)composeRectInOut->width <= sel.r.left + (int32_t)sel.r.width) {
        r.width = composeRectInOut->width;
    }
    else {
        r.width = (int32_t)sel.r.width + sel.r.left - r.left;
    }

    if (r.top + (int32_t)composeRectInOut->height <= sel.r.top + (int32_t)sel.r.height) {
        r.height = composeRectInOut->height;
    }
    else {
        r.height = (int32_t)sel.r.height + sel.r.top - r.top;
    }

    Logd("Setting active composing area to : left = %d /  top = %d / width = %u / height = %u", r.left, r.top, r.width, r.height);

    sel.r      = r;
    sel.target = V4L2_SEL_TGT_COMPOSE_ACTIVE;
    sel.flags  = V4L2_SEL_FLAG_LE;

    if ((ret = v4l2Ioctl_f(obj->fd, VIDIOC_S_SELECTION, &sel)) != V4L2_ERROR_NONE) {
        Loge("Failed to set V4L2_SEL_TGT_COMPOSE_ACTIVE");
        return ret;
    }
	
    composeRectInOut->left   = r.left;
    composeRectInOut->top    = r.top;
    composeRectInOut->width  = r.width;
    composeRectInOut->height = r.height;

    Logd("Active composing area updated : left = %d /  top = %d / width = %u / height = %u", sel.r.left, sel.r.top, sel.r.width, sel.r.height);
	
    return ret;
}

/*!
 * \fn        static V4L2_ERROR_E requestBuffers_f(V4L2_S *obj, V4L2_REQUEST_BUFFERS_PARAMS_S *params)
 * \brief     Request video buffers
 * \param[in] obj
 * \param[in] params
 * \return    V4L2_ERROR_NONE on success
 *            V4L2_ERROR_INIT on error
 */
static V4L2_ERROR_E requestBuffers_f(V4L2_S *obj, V4L2_REQUEST_BUFFERS_PARAMS_S *params)
{
    assert(obj && (obj->fd != -1) && params);
    
    V4L2_ERROR_E ret = V4L2_ERROR_NONE;
    
    uint32_t i;
    struct v4l2_requestbuffers req;
    struct v4l2_buffer buf;

    obj->memory = params->memory;

    memset(&req, '\0', sizeof(struct v4l2_requestbuffers));

    req.count  = params->count;
    req.type   = obj->format.type;
    req.memory = obj->memory;

    if (v4l2Ioctl_f(obj->fd, VIDIOC_REQBUFS, &req) != V4L2_ERROR_NONE) {
        Loge("Failed to request buffers");
        return V4L2_ERROR_IO;
    }

    if (req.count < params->count) {
        Loge("Allocated number of buffers not enough");
        ret = V4L2_ERROR_MEMORY;
        goto exit;
    }
    
    obj->nbBuffers = req.count;
    assert((obj->map = calloc(obj->nbBuffers, sizeof(V4L2_MAPPING_BUFFER_S))));

    obj->maxBufferSize = 0;
    for (i = 0; i < obj->nbBuffers; i++) {
        memset(&buf, '\0', sizeof(struct v4l2_buffer));

        buf.type   = req.type;
        buf.memory = req.memory;
        buf.index  = i;

        if (v4l2Ioctl_f(obj->fd, VIDIOC_QUERYBUF, &buf) != V4L2_ERROR_NONE) {
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
                                          obj->fd, buf.m.offset);

                if (obj->map[i].start == MAP_FAILED) {
                    Loge("mmap() failed");
                    ret = V4L2_ERROR_MEMORY;
                    goto exit;
                }
                break;
                
            case V4L2_MEMORY_USERPTR:
                assert((obj->map[i].start = calloc(1, buf.length)));
                break;
                
            default:
                ;
        }
        
        if (obj->maxBufferSize < buf.length) {
            obj->maxBufferSize = buf.length;
        }
    }
    
    return V4L2_ERROR_NONE;

exit:
    releaseBuffers_f(obj);
    
    return ret;
}

/*!
 * \fn        static V4L2_ERROR_E releaseBuffers_f(V4L2_S *obj)
 * \brief     Release allocated video buffers
 * \param[in] obj
 * \return    V4L2_ERROR_NONE on success
 *            V4L2_ERROR_INIT on error
 */
static V4L2_ERROR_E releaseBuffers_f(V4L2_S *obj)
{
    assert(obj && (obj->fd != -1));
    
    uint32_t i;
    struct v4l2_requestbuffers req;
    
    if (obj->map) {
        switch (obj->memory) {
            case V4L2_MEMORY_MMAP:
                for (i = 0; i < obj->nbBuffers; i++) {
                    if (obj->map[i].start && munmap(obj->map[i].start, obj->map[i].length) < 0) {
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
    
    memset(&req, '\0', sizeof(struct v4l2_requestbuffers));
    req.count = 0;
    req.type = obj->format.type;
    req.memory = obj->memory;
    
    if (v4l2Ioctl_f(obj->fd, VIDIOC_REQBUFS, &req) != V4L2_ERROR_NONE) {
        Loge("Failed to release buffers");
    }
    
    return V4L2_ERROR_NONE;
}

/*!
 * \fn        static V4L2_ERROR_E startCapture_f(V4L2_S *obj)
 * \brief     Start video capture
 * \param[in] obj
 * \return    V4L2_ERROR_NONE on success
 *            V4L2_ERROR_INIT on error
 */
static V4L2_ERROR_E startCapture_f(V4L2_S *obj)
{
    assert(obj && (obj->fd != -1));
    
    if (v4l2Ioctl_f(obj->fd, VIDIOC_STREAMON, &obj->format.type) != V4L2_ERROR_NONE) {
        Loge("Failed to start stream");
        return V4L2_ERROR_CAPTURE;
    }
    
    return V4L2_ERROR_NONE;
}

/*!
 * \fn        static V4L2_ERROR_E stopCapture_f(V4L2_S *obj)
 * \brief     Stop video capture
 * \param[in] obj
 * \return    V4L2_ERROR_NONE on success
 *            V4L2_ERROR_INIT on error
 */
static V4L2_ERROR_E stopCapture_f(V4L2_S *obj)
{
    assert(obj && (obj->fd != -1));
    
    if (v4l2Ioctl_f(obj->fd, VIDIOC_STREAMOFF, &obj->format.type) != V4L2_ERROR_NONE) {
        Loge("Failed to stop stream");
        return V4L2_ERROR_CAPTURE;
    }
    
    return V4L2_ERROR_NONE;
}

/*!
 * \fn        static V4L2_ERROR_E awaitData_f(V4L2_S *obj, int32_t timeout_ms)
 * \brief     Wait a given duration
 * \param[in] obj
 * \param[in] timeout_ms : Time in ms to wait before returning / -1 => not used
 * \return    V4L2_ERROR_NONE on success
 *            V4L2_ERROR_INIT on error
 */
static V4L2_ERROR_E awaitData_f(V4L2_S *obj, int32_t timeout_ms)
{
    assert(obj && (obj->fd != -1));
    
    int select_retval;
    fd_set fds;
    struct timeval *tv = NULL;
    V4L2_ERROR_E ret   = V4L2_ERROR_NONE;

    FD_ZERO(&fds);
    FD_SET(obj->fd, &fds);

    if (timeout_ms > 0) {
        assert((tv = calloc(1, sizeof(struct timeval))));
        tv->tv_sec  = 0;
        tv->tv_usec = timeout_ms * 1000;
    }

    select_retval = select(obj->fd + 1, &fds, NULL, NULL, tv);
    
    if (tv) {
        free(tv);
        tv = NULL;
    }

    if (select_retval == -1) { /* Interrupted */
        goto exit;
    }

    if (select_retval == 0) { /* Timeout */
        ret = V4L2_ERROR_TIMEOUT;
        goto exit;
    }
    
    return V4L2_ERROR_NONE;
    
exit:
    return ret;
}

/*!
 * \fn        static V4L2_ERROR_E queueBuffer_f(V4L2_S *obj, uint32_t index)
 * \brief     Queue video buffer
 * \param[in] obj
 * \param[in] index
 * \return    V4L2_ERROR_NONE on success
 *            V4L2_ERROR_INIT on error
 */
static V4L2_ERROR_E queueBuffer_f(V4L2_S *obj, uint32_t index)
{
    assert(obj && (obj->fd != -1));
    
    struct v4l2_buffer buffer;
    
    memset(&buffer, '\0', sizeof(struct v4l2_buffer));

    buffer.type   = obj->format.type;
    buffer.memory = obj->memory;
    buffer.index  = index;

    if (buffer.memory == V4L2_MEMORY_USERPTR) {
        buffer.m.userptr = (uint64_t)obj->map[index].start;
        buffer.length    = obj->map[index].length;
    }

    return v4l2Ioctl_f(obj->fd, VIDIOC_QBUF, &buffer);
}

/*!
 * \fn        static V4L2_ERROR_E dequeueBuffer_f(V4L2_S *obj)
 * \brief     Dequeue video buffer
 * \param[in] obj
 * \return    V4L2_ERROR_NONE on success
 *            V4L2_ERROR_INIT on error
 */
static V4L2_ERROR_E dequeueBuffer_f(V4L2_S *obj)
{
    assert(obj && (obj->fd != -1));
    
    struct v4l2_buffer buffer;
    uint32_t i;
    
    memset(&buffer, '\0', sizeof(struct v4l2_buffer));

    buffer.type   = obj->format.type;
    buffer.memory = obj->memory;

    if (v4l2Ioctl_f(obj->fd, VIDIOC_DQBUF, &buffer) != V4L2_ERROR_NONE) {
        Loge("Failed to dequeue buffer");
        return V4L2_ERROR_IO;
    }

    switch (buffer.memory) {
        case V4L2_MEMORY_MMAP:
            i = buffer.index;
            break;
            
        case V4L2_MEMORY_USERPTR:
            for (i = 0; i < obj->nbBuffers; i++) {
                if ((buffer.m.userptr == (uint64_t)obj->map[i].start) && (buffer.length == obj->map[i].length)) {
                    break;
                }
            }
            break;
            
        default:
            ;
    }
    
    assert(i < obj->nbBuffers);

    return V4L2_ERROR_NONE;
}

/*!
 * Custom ioctl with retries when interrupted
 */
static V4L2_ERROR_E v4l2Ioctl_f(int32_t fd, uint64_t req, void *args)
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
