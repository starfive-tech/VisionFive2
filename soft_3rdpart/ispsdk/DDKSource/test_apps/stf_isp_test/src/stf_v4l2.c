// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 StarFive Technology Co., Ltd.
 */
#include <sys/mman.h>
#include <libv4l2.h>
#include <sys/ioctl.h>
#include "common.h"
#include "stf_v4l2.h"
#include "stf_log.h"

#define FILENAME_MAX_LEN     30
struct stfisp_fw_info {
    char filename[FILENAME_MAX_LEN];
};

struct v4l2_subdev_frame_size_enum {
    __u32 index;
    __u32 pad;
    __u32 code;
    __u32 min_width;
    __u32 max_width;
    __u32 min_height;
    __u32 max_height;
    __u32 which;
    __u32 reserved[8];
};

static int videotype = V4L2_CAP_VIDEO_CAPTURE;

#define VIDIOC_STFISP_LOAD_FW \
        _IOW('V', BASE_VIDIOC_PRIVATE + 1, struct stfisp_fw_info)

#define VIDIOC_SUBDEV_ENUM_FRAME_SIZE \
        _IOWR('V', 74, struct v4l2_subdev_frame_size_enum)
#define MEDIA_BUS_FMT_SRGGB10_1X10  0x300f

/**
    Do ioctl and retry if error was EINTR ("A signal was caught during the ioctl() operation."). Parameters are the same as on ioctl.
    \param fd file descriptor
    \param request request
    \param argp argument
    \returns result from ioctl
*/
int xioctl(int fd, int request, void* argp)
{
    int r;

    // TODO: the orign is v4l2_ioctl()
    do r = ioctl(fd, request, argp);
    while (-1 == r && EINTR == errno);

    return r;
}

void sensor_image_size_info(V4l2Param_t *param)
{
    int fd = 0;
    uint32_t i = 0;
    struct v4l2_subdev_frame_size_enum frame_size;
    LOG(STF_LEVEL_DEBUG, "go in sensor_image_size_info....\n");

    fd = open(param->device_name, O_RDWR /* required */ | O_NONBLOCK, 0);
    if (-1 == fd) {
        LOG(STF_LEVEL_ERR, "Cannot open '%s': %d, %s\n", param->device_name, errno, strerror(errno));
        close(fd);
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < 4; i++) {
        frame_size.index = i;
        frame_size.code = MEDIA_BUS_FMT_SRGGB10_1X10;
        if (-1 == ioctl(fd, VIDIOC_SUBDEV_ENUM_FRAME_SIZE, &frame_size)) {
            close(fd);
            errno_exit("VIDIOC_SIZE_INFO");
        }
        LOG(STF_LEVEL_DEBUG, "image_size: width[%d] = %d, height[%d] = %d \n",
                    i, frame_size.min_width, i, frame_size.min_height);
    }
    close(fd);
}

void loadfw_start(char *filename, V4l2Param_t *param)
{
    struct stfisp_fw_info fw_info = {0};
    int fd = 0;

    LOG(STF_LEVEL_TRACE, "Enter\n");
    fd = open(param->device_name, O_RDWR /* required */ | O_NONBLOCK, 0);
    if (-1 == fd) {
        LOG(STF_LEVEL_ERR, "Cannot open '%s': %d, %s\n", param->device_name, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (filename && (strlen(filename) < FILENAME_MAX_LEN)) {
        memcpy(fw_info.filename, filename, strlen(filename) + 1);
    }

    LOG(STF_LEVEL_INFO, "VIDIOC_STFISP_LOAD_FW = 0x%lx, filename = %s, size = %lu, device=%s\n",
            VIDIOC_STFISP_LOAD_FW, fw_info.filename, sizeof(struct stfisp_fw_info), param->device_name);
    if (-1 == ioctl(fd, VIDIOC_STFISP_LOAD_FW, &fw_info)) {
        if (EINVAL == errno) {
            close(fd);
            LOG(STF_LEVEL_ERR, "%s is no V4L2 device\n", param->device_name);
            exit(EXIT_FAILURE);
        } else {
            close(fd);
            errno_exit("VIDIOC_STFISP_LOAD_FW");
        }
    }

    close(fd);
    LOG(STF_LEVEL_TRACE, "Exit\n");
}

static void convert_v4l2_mem_type(int iomthd, enum v4l2_memory *mem_type)
{
    if (iomthd < IO_METHOD_MMAP || iomthd > IO_METHOD_READ) {
        LOG(STF_LEVEL_ERR, "iomthd %d out of range\n", iomthd);
        exit(EXIT_FAILURE);
    }

    switch (iomthd) {
    case IO_METHOD_MMAP:
        *mem_type = V4L2_MEMORY_MMAP;
        break;
    case IO_METHOD_USERPTR:
        *mem_type = V4L2_MEMORY_USERPTR;
        break;
    case IO_METHOD_DMABUF:
        *mem_type = V4L2_MEMORY_DMABUF;
        break;
    case IO_METHOD_READ:
        *mem_type = 0;  // not use memory machanism
        break;
    default:
        *mem_type = 0;  // not use memory machanism
        break;
    }
}

void stf_v4l2_open(V4l2Param_t *param, char *device_name)
{
    struct stat st;
    //struct v4l2_capability cap;

    LOG(STF_LEVEL_TRACE, "Enter\n");
    // stat file
    if (-1 == stat(device_name, &st)) {
        LOG(STF_LEVEL_ERR, "Cannot identify '%s': %d, %s\n", device_name, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // check if is device
    if (!S_ISCHR(st.st_mode)) {
        LOG(STF_LEVEL_ERR, "%s is no device\n", device_name);
        exit(EXIT_FAILURE);
    }

    // open device
    param->fd = v4l2_open(device_name, O_RDWR /* required */ | O_NONBLOCK, 0);
    // param->fd = v4l2_open(device_name, O_RDWR, 0);
    if (-1 == param->fd) {
        LOG(STF_LEVEL_ERR, "Cannot open '%s': %d, %s\n", device_name, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    LOG(STF_LEVEL_TRACE, "Exit\n");
}

void stf_v4l2_close(V4l2Param_t *param)
{
    LOG(STF_LEVEL_TRACE, "Enter\n");
    if (-1 == v4l2_close(param->fd)) {
        errno_exit("close");
    }
    param->fd = -1;
    LOG(STF_LEVEL_TRACE, "Exit\n");
}

void stf_v4l2_init(V4l2Param_t *param)
{
    struct v4l2_capability cap;
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;
    struct v4l2_format fmt;
    struct v4l2_streamparm frameint;
    //struct v4l2_streamparm frameget;
    unsigned int min;

    LOG(STF_LEVEL_TRACE, "Enter\n");
    // query capability
    if (-1 == xioctl(param->fd, VIDIOC_QUERYCAP, &cap)) {
        if (EINVAL == errno) {
            LOG(STF_LEVEL_ERR, "%s is no V4L2 device\n", param->device_name);
            exit(EXIT_FAILURE);
        } else {
            errno_exit("VIDIOC_QUERYCAP");
        }
    }
    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        if (!(cap.capabilities & V4L2_CAP_VIDEO_OUTPUT))
            exit(EXIT_FAILURE);
        LOG(STF_LEVEL_ERR, "%s is no video capture device, is video_output\n", param->device_name);
        videotype = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    } else
        videotype = V4L2_BUF_TYPE_VIDEO_CAPTURE;


    switch (param->io_mthd) {
    case IO_METHOD_READ:
        if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
            LOG(STF_LEVEL_ERR, "%s does not support read i/o\n", param->device_name);
            exit(EXIT_FAILURE);
        }
        break;
    case IO_METHOD_MMAP:
    case IO_METHOD_USERPTR:
    case IO_METHOD_DMABUF:
        if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
            LOG(STF_LEVEL_ERR, "%s does not support streaming i/o\n", param->device_name);
            exit(EXIT_FAILURE);
        }
        break;
    default:
        LOG(STF_LEVEL_ERR, "%s does not specify streaming i/o\n", param->device_name);
        exit(EXIT_FAILURE);
        break;
    }

    /* Select video input, video standard and tune here. */
    CLEAR(cropcap);
    cropcap.type = videotype;
    if (0 == xioctl(param->fd, VIDIOC_CROPCAP, &cropcap)) {
        crop.type = videotype;
        crop.c = cropcap.defrect; /* reset to default */
        if (-1 == xioctl(param->fd, VIDIOC_S_CROP, &crop)) {
            switch (errno) {
            case EINVAL:
                /* Cropping not supported. */
                break;
            default:
                /* Errors ignored. */
                break;
            }
        }
    } else {
        /* Errors ignored. */
    }

    /* If the user has set the fps to -1, don't try to set the frame interval */
    if (param->fps != -1) {
        CLEAR(frameint);
        /* Attempt to set the frame interval. */
        frameint.type = videotype;
        frameint.parm.capture.timeperframe.numerator = 1;
        frameint.parm.capture.timeperframe.denominator = param->fps;
        if (-1 == xioctl(param->fd, VIDIOC_S_PARM, &frameint)) {
            LOG(STF_LEVEL_WARN, "Unable to set frame interval.\n");
        }
        LOG(STF_LEVEL_INFO, "set frame interval = %d.\n", frameint.parm.capture.timeperframe.denominator);
    }

    // set v4l2_format
    CLEAR(fmt);
    fmt.type = videotype;
    fmt.fmt.pix.width = param->width;
    fmt.fmt.pix.height = param->height;
    fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
    fmt.fmt.pix.pixelformat = param->format;
    if (-1 == xioctl(param->fd, VIDIOC_S_FMT, &fmt)) {
        errno_exit("VIDIOC_S_FMT");
    }

    if (param->crop_flag) {
        struct v4l2_selection sel_crop = {
            videotype,
            V4L2_SEL_TGT_CROP,
            0,
            {param->crop_info.left, param->crop_info.top,
                param->crop_info.width, param->crop_info.height}  // TODO: opt here
        };
        struct v4l2_selection get_crop = {
            videotype,
            V4L2_SEL_TGT_CROP,
        };
        LOG(STF_LEVEL_DEBUG, "sel_crop.left = %d, %d, %d, %d\n",
            sel_crop.r.left, sel_crop.r.top, sel_crop.r.width, sel_crop.r.height);

        if (-1 == xioctl(param->fd, VIDIOC_S_SELECTION, &sel_crop)) {
            LOG(STF_LEVEL_ERR, "S_SELECTION Failed.\n");
        }
        LOG(STF_LEVEL_DEBUG, "sel_crop.left = %d, %d, %d, %d\n",
            sel_crop.r.left, sel_crop.r.top, sel_crop.r.width, sel_crop.r.height);

        if (-1 == xioctl(param->fd, VIDIOC_G_SELECTION, &get_crop)) {
            LOG(STF_LEVEL_ERR, "G_SELECTION Failed.\n");
        }
        LOG(STF_LEVEL_DEBUG, "get_crop.left = %d, %d, %d, %d\n",
            get_crop.r.left, get_crop.r.top, get_crop.r.width, get_crop.r.height);

        if (memcmp(&sel_crop, &get_crop, sizeof(sel_crop))) {
            LOG(STF_LEVEL_WARN, "set/get selection diff.\n");
        }
    }

    // get v4l2_format
    memset(&fmt, 0, sizeof(struct v4l2_format));
    fmt.type = videotype;
    fmt.fmt.pix.field = V4L2_FIELD_ANY;
    if (-1 == ioctl(param->fd, VIDIOC_G_FMT, &fmt)) {
        errno_exit("VIDIOC_G_FMT");
    }
    LOG(STF_LEVEL_INFO, "VIDIOC_G_FMT: type=%d, Fourcc format=%c%c%c%c\n",
            fmt.type, fmt.fmt.pix.pixelformat & 0xff,
            (fmt.fmt.pix.pixelformat >> 8) &0xff,
            (fmt.fmt.pix.pixelformat >> 16) &0xff,
            (fmt.fmt.pix.pixelformat >> 24) &0xff);
    LOG(STF_LEVEL_INFO, " \t width=%d, height=%d, field=%d, bytesperline=%d, sizeimage=%d\n",
            fmt.fmt.pix.width, fmt.fmt.pix.height, fmt.fmt.pix.field,
            fmt.fmt.pix.bytesperline, fmt.fmt.pix.sizeimage);

    //if (fmt.fmt.pix.pixelformat != V4L2_PIX_FMT_YUV420) {
    //if (fmt.fmt.pix.pixelformat != V4L2_PIX_FMT_RGB565) {
    if (fmt.fmt.pix.pixelformat != param->format) {
        LOG(STF_LEVEL_ERR, "v4l2 didn't accept format %d. Can't proceed.\n", param->format);
        exit(EXIT_FAILURE);
    }

    /* Note VIDIOC_S_FMT may change width and height. */
    if (param->width != fmt.fmt.pix.width) {
        param->width = fmt.fmt.pix.width;
        LOG(STF_LEVEL_WARN, "Correct image width set to %i by device %s.\n", param->width, param->device_name);
    }

    if (param->height != fmt.fmt.pix.height) {
        param->height = fmt.fmt.pix.height;
        LOG(STF_LEVEL_WARN, "Correct image height set to %i by device %s.\n", param->height, param->device_name);
    }

    // TODO: who and why add the below here? Change bytesperline and sizeimage
    /* Buggy driver paranoia. */
    min = fmt.fmt.pix.width * 2;
    if (fmt.fmt.pix.bytesperline < min) {
        fmt.fmt.pix.bytesperline = min;
    }
    min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
    if (fmt.fmt.pix.sizeimage < min) {
        fmt.fmt.pix.sizeimage = min;
    }
    // param->image_size = fmt.fmt.pix.sizeimage; // wrong here
    LOG(STF_LEVEL_LOG, "fmt.fmt.pix.sizeimage=%d, fmt.fmt.pix.bytesperline=%d\n",
        fmt.fmt.pix.sizeimage, fmt.fmt.pix.bytesperline);

    // convert mem type
    convert_v4l2_mem_type(param->io_mthd, &param->mem_type);

    LOG(STF_LEVEL_TRACE, "Exit\n");
}

void stf_v4l2_uninit(V4l2Param_t *param)
{
    uint32_t i;

    LOG(STF_LEVEL_TRACE, "Enter\n");
    switch (param->io_mthd) {
    case IO_METHOD_READ:
        free(param->pBuffers[0].start);
        break;

    case IO_METHOD_MMAP:
        for (i = 0; i < param->n_buffers; ++i) {
            if (-1 == v4l2_munmap(param->pBuffers[i].start,
                    param->pBuffers[i].length)) {
                errno_exit("munmap");
            }
        }
        break;

    case IO_METHOD_USERPTR:
        for (i = 0; i < param->n_buffers; ++i) {
            free(param->pBuffers[i].start);
        }
        break;

    case IO_METHOD_DMABUF:
        break;

    default:
        break;
    }
    free(param->pBuffers);
    LOG(STF_LEVEL_TRACE, "Exit\n");
}

static void stf_v4l2_readInit(V4l2Param_t *param)
{
    LOG(STF_LEVEL_TRACE, "Enter\n");
    param->n_buffers = 1;
    param->pBuffers = calloc(param->n_buffers, sizeof(*param->pBuffers));
    if (!param->pBuffers) {
        LOG(STF_LEVEL_ERR, "Out of memory\n");
        exit(EXIT_FAILURE);
    }

    param->pBuffers[0].length = param->image_size;
    param->pBuffers[0].start = malloc(param->image_size);
    if (!param->pBuffers[0].start) {
        LOG(STF_LEVEL_ERR, "Out of memory\n");
        exit(EXIT_FAILURE);
    }
    LOG(STF_LEVEL_TRACE, "Exit\n");
}

static void stf_v4l2_mmapInit(V4l2Param_t *param)
{
    int i = 0;
    struct v4l2_requestbuffers req;

    LOG(STF_LEVEL_TRACE, "Enter\n");
    CLEAR(req);
    req.count = BUFCOUNT;
    req.type = videotype;
    req.memory = V4L2_MEMORY_MMAP;
    if (-1 == xioctl(param->fd, VIDIOC_REQBUFS, &req)) {
        if (EINVAL == errno) {
            LOG(STF_LEVEL_ERR, "%s does not support memory mapping\n", param->device_name);
            exit(EXIT_FAILURE);
        } else {
            errno_exit("VIDIOC_REQBUFS");
        }
    }
    if (req.count < 2) {
        LOG(STF_LEVEL_ERR, "Insufficient buffer memory on %s\n", param->device_name);
        exit(EXIT_FAILURE);
    }

    param->n_buffers = req.count;
    param->pBuffers = calloc(param->n_buffers, sizeof(*param->pBuffers));
    if (!param->pBuffers) {
        LOG(STF_LEVEL_ERR, "Out of memory\n");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < param->n_buffers; ++i) {
        struct v4l2_buffer buf;
        CLEAR(buf);
        buf.type = videotype;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        if (-1 == xioctl(param->fd, VIDIOC_QUERYBUF, &buf)) {
            errno_exit("VIDIOC_QUERYBUF");
        }
        param->pBuffers[i].length = buf.length;
        param->pBuffers[i].start = v4l2_mmap(NULL, /* start anywhere */
                buf.length, PROT_READ | PROT_WRITE, /* required */
                MAP_SHARED, /* recommended */
                param->fd, buf.m.offset);
        if (MAP_FAILED == param->pBuffers[i].start) {
            errno_exit("mmap");
        }
        param->image_size = buf.length; // NOTE: updated value
    }
    LOG(STF_LEVEL_TRACE, "Exit\n");
}

static void stf_v4l2_dmabufInit(V4l2Param_t *param, int *dmabufs, int count)
{
    int i = 0;
    struct v4l2_requestbuffers req;

    LOG(STF_LEVEL_TRACE, "Enter\n");
    CLEAR(req);
    req.count = BUFCOUNT;  // TODO: modify later
    req.type = videotype;
    req.memory = V4L2_MEMORY_DMABUF;
    if (-1 == xioctl(param->fd, VIDIOC_REQBUFS, &req)) {
        if (EINVAL == errno) {
            LOG(STF_LEVEL_ERR, "%s does not support dmabuf\n", param->device_name);
            exit(EXIT_FAILURE);
        } else {
            errno_print("VIDIOC_REQBUFS");
        }
    }
    if (req.count < 2) {
        LOG(STF_LEVEL_ERR, "Insufficient buffer memory on %s\n", param->device_name);
        exit(EXIT_FAILURE);
    }

    param->n_buffers = req.count;
    param->pBuffers = calloc(param->n_buffers, sizeof(*param->pBuffers));
    if (!param->pBuffers) {
        LOG(STF_LEVEL_ERR, "Out of memory\n");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < param->n_buffers; ++i) {
        struct v4l2_buffer buf;
        CLEAR(buf);
        buf.type        = videotype;
        buf.memory      = V4L2_MEMORY_DMABUF;
        buf.index       = i;
        if (-1 == xioctl(param->fd, VIDIOC_QUERYBUF, &buf)) {
            errno_print("VIDIOC_QUERYBUF");
        }
        param->pBuffers[i].index = buf.index;
        param->pBuffers[i].dmabuf_fd = dmabufs[i];
#if 0
        param->pBuffers[i].length = buf.length;
        param->pBuffers[i].start = mmap(NULL /* start anywhere */,
                buf.length,
                PROT_READ | PROT_WRITE /* required */,
                MAP_SHARED /* recommended */,
                dmabufs[i], 0);

        if (MAP_FAILED == param->pBuffers[i].start)
            errno_print("mmap");
#endif
    }
    LOG(STF_LEVEL_TRACE, "Exit\n");
}

static void stf_v4l2_userptrInit(V4l2Param_t *param)
{
    struct v4l2_requestbuffers req;
    unsigned int page_size;
    unsigned int buffer_size = param->image_size;
    int i = 0;

    page_size = getpagesize();
    buffer_size = (buffer_size + page_size - 1) & ~(page_size - 1);

    CLEAR(req);
    req.count = BUFCOUNT;
    req.type = videotype;
    req.memory = V4L2_MEMORY_USERPTR;
    if (-1 == xioctl(param->fd, VIDIOC_REQBUFS, &req)) {
        if (EINVAL == errno) {
            LOG(STF_LEVEL_ERR, "%s does not support user pointer i/o\n", param->device_name);
            exit(EXIT_FAILURE);
        } else {
            errno_exit("VIDIOC_REQBUFS");
        }
    }

    param->n_buffers = 4;
    param->pBuffers = calloc(param->n_buffers, sizeof(*param->pBuffers));
    if (!param->pBuffers) {
        LOG(STF_LEVEL_ERR, "Out of memory\n");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < param->n_buffers; ++i) {
        param->pBuffers[i].length = buffer_size;
        param->pBuffers[i].start = memalign(/* boundary */ page_size, buffer_size);
        if (!param->pBuffers[i].start) {
            LOG(STF_LEVEL_ERR, "Out of memory\n");
            exit(EXIT_FAILURE);
        }
    }
    param->image_size = buffer_size;
}

void sft_v4l2_prepare_capturing(V4l2Param_t *param, int *dmabufs, int count)
{
    LOG(STF_LEVEL_TRACE, "Enter\n");
    switch (param->io_mthd) {
    case IO_METHOD_READ:
        stf_v4l2_readInit(param);
        break;

    case IO_METHOD_MMAP:
        stf_v4l2_mmapInit(param);
        break;

    case IO_METHOD_USERPTR:
        stf_v4l2_userptrInit(param);
        break;

    case IO_METHOD_DMABUF:
        stf_v4l2_dmabufInit(param, dmabufs, count);
        break;

    default:
        LOG(STF_LEVEL_ERR, "%s does not specify streaming i/o\n", param->device_name);
        exit(EXIT_FAILURE);
        break;
    }
    LOG(STF_LEVEL_TRACE, "Exit\n");
}

void sft_v4l2_start_capturing(V4l2Param_t *param)
{
    unsigned int i;
    enum v4l2_buf_type type;

    LOG(STF_LEVEL_TRACE, "Enter\n");
    switch (param->io_mthd) {
    case IO_METHOD_READ:
        /* Nothing to do. */
        break;

    case IO_METHOD_MMAP:
    case IO_METHOD_USERPTR:
    case IO_METHOD_DMABUF:
        for (i = 0; i < param->n_buffers; ++i) {
            stf_v4l2_queue_buffer(param, i);
        }
        type = videotype;
        if (-1 == xioctl(param->fd, VIDIOC_STREAMON, &type)) {
            errno_exit("VIDIOC_STREAMON");
        }
        break;

    default:
        break;
    }
    LOG(STF_LEVEL_TRACE, "Exit\n");
}

void stf_v4l2_stop_capturing(V4l2Param_t *param)
{
    enum v4l2_buf_type type;

    LOG(STF_LEVEL_TRACE, "Enter\n");
    switch (param->io_mthd) {
    case IO_METHOD_READ:
        /* Nothing to do. */
        break;
    case IO_METHOD_MMAP:
    case IO_METHOD_USERPTR:
    case IO_METHOD_DMABUF:
        type = videotype;
        if (-1 == xioctl(param->fd, VIDIOC_STREAMOFF, &type))
            errno_exit("VIDIOC_STREAMOFF");
        break;
    default:
        break;
    }
    LOG(STF_LEVEL_TRACE, "Exit\n");
}

// support V4L2_MEMORY_MMAP / V4L2_MEMORY_USERPTR / V4L2_MEMORY_DMABUF
// NOTE: for V4L2_MEMORY_USERPTR, index is pBuffers index
void stf_v4l2_queue_buffer(V4l2Param_t *param, int index)
{
    struct v4l2_buffer buf;

    LOG(STF_LEVEL_TRACE, "Enter\n");
    assert(index < param->n_buffers);
    CLEAR(buf);
    buf.type = videotype;
    buf.memory = param->mem_type;
    buf.index = index; // NOTE: for V4L2_MEMORY_USERPTR not used here?

    if (param->mem_type == V4L2_MEMORY_DMABUF) {
        //buf.m.fd = param->dmabuf_fd;
        buf.m.fd = param->pBuffers[index].dmabuf_fd;
    } else if (param->mem_type == V4L2_MEMORY_USERPTR) {
        buf.m.userptr = (unsigned long)param->pBuffers[index].start;
        buf.length = param->pBuffers[index].length;
    }
    if (-1 == xioctl(param->fd, VIDIOC_QBUF, &buf)) {
        errno_print("VIDIOC_QBUF");
    }
    LOG(STF_LEVEL_TRACE, "Exit\n");
}

// support V4L2_MEMORY_MMAP / V4L2_MEMORY_USERPTR / V4L2_MEMORY_DMABUF
int stf_v4l2_dequeue_buffer(V4l2Param_t *param, struct v4l2_buffer *buf)
{
    int index = 0;

    LOG(STF_LEVEL_TRACE, "Enter\n");
    PCLEAR(buf);
    buf->type = videotype;
    buf->memory = param->mem_type;
    if (-1 == xioctl(param->fd, VIDIOC_DQBUF, buf)) {
        switch (errno) {
        case EAGAIN:
            return 0;
        case EIO:
            /* Could ignore EIO, see spec. */
            /* fall through */
        default:
            errno_print("VIDIOC_DQBUF");
        }
    }

    index = buf->index;
    if (param->mem_type == V4L2_MEMORY_USERPTR) {
        for (index = 0; index < param->n_buffers; ++index) {
            if (buf->m.userptr == (unsigned long)param->pBuffers[index].start
                    && buf->length == param->pBuffers[index].length) {
                break;
            }
        }
    }

    assert(index < param->n_buffers);
    LOG(STF_LEVEL_TRACE, "Exit\n");
    return 1;
}

void isp_test_ioctl(char *devname)
{
    struct dmabuf_create head = {0};
    // char *devname = "/dev/v4l-subdev5";
    char *fdp = NULL;
    struct stat st;
    int fd = 0;
    int ret, i = 0;

    fprintf(stderr, "test devname: %s\n", devname);
    // stat file
    if (-1 == stat(devname, &st)) {
        fprintf(stderr, "Cannot identify '%s': %d, %s\n", devname, errno, strerror(errno));
	return;
    }

    // check if its device
    if (!S_ISCHR(st.st_mode)) {
        fprintf(stderr, "%s is no device\n", devname);
	return;
    }

    // open device
    fd = open(devname, O_RDWR, 0);

    // check if opening was successfull
    if (-1 == fd) {
        fprintf(stderr, "Cannot open '%s': %d, %s\n", devname, errno, strerror(errno));
	return;
    }

    head.size = 1920 * 1080 * 4;
    fprintf(stderr, "before fd = 0x%x, size = 0x%x, paddr = 0x%x\n", head.fd, head.size, head.paddr);
    ret = ioctl(fd, VIDIOC_STF_DMABUF_ALLOC, &head);
    if (ret < 0) {
        fprintf(stderr, "VIDIOC_STF_DMABUF_ALLOC, failed, ret = %d\n", ret);
	close(fd);
	return;
    }
    fprintf(stderr, "affter fd = 0x%x, size = 0x%x, paddr = 0x%x\n", head.fd, head.size, head.paddr);
    fdp = (char *)mmap(NULL, head.size, PROT_READ | PROT_WRITE, MAP_SHARED, head.fd, 0);
    if (!fdp) {
        printf("Error: failed to map stfdmabuffer.\n");
	goto exit;
    }

    memset(fdp, 0x00, head.size);

    if (-1 == munmap(fdp, head.size))
        printf("Error: ion buffer device munmap() failed.\n");

    ret = ioctl(fd, VIDIOC_STFISP_SHADOW_LOCK, NULL);
    if (ret < 0)
        fprintf(stderr, "VIDIOC_STFISP_SHADOW_LOCK, failed, ret = %d\n", ret);
    else
        fprintf(stderr, "VIDIOC_STFISP_SHADOW_LOCK is locked\n");

    ret = ioctl(fd, VIDIOC_STFISP_SHADOW_UNLOCK, NULL);
    if (ret < 0)
        fprintf(stderr, "VIDIOC_STFISP_SHADOW_UNLOCK, failed, ret = %d\n", ret);
    else
        fprintf(stderr, "VIDIOC_STFISP_SHADOW_UNLOCK is locked\n");

exit:
    ret = ioctl(fd, VIDIOC_STF_DMABUF_FREE, &head);
    if (ret < 0)
        fprintf(stderr, "VIDIOC_STF_DMABUF_FREE, failed, ret = %d\n", ret);
    close(head.fd);
    close(fd);
}

void isp_poll_event(char *deviceName, int *continuous)
{
    struct v4l2_event_subscription sub = { 0 };
    struct v4l2_event ev;
    struct stat st;
    struct timeval tv = { 2, 0 };
    fd_set rfds, wfds, efds;
    int isp_fd = 0;
    int ret;

    fprintf(stderr, "test event devname: %s\n", deviceName);
    // stat file
    if (-1 == stat(deviceName, &st)) {
        fprintf(stderr, "Cannot identify '%s': %d, %s\n", deviceName, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // check if its device
    if (!S_ISCHR(st.st_mode)) {
        fprintf(stderr, "%s is no device\n", deviceName);
        exit(EXIT_FAILURE);
    }

    // open device
    // isp_fd = open(deviceName, O_RDWR /* required */ | O_NONBLOCK, 0);
    isp_fd = open(deviceName, O_RDWR, 0);

    // check if opening was successfull
    if (-1 == isp_fd) {
        fprintf(stderr, "Cannot open '%s': %d, %s\n", deviceName, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    *continuous = 1;
    sub.type = V4L2_EVENT_SOURCE_CHANGE;
    // sub.type = V4L2_EVENT_ALL;
    // sub.type = V4L2_EVENT_CTRL;
    // sub.id = V4L2_CID_EXPOSURE;
    ret = ioctl(isp_fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
    if (ret < 0) {
        fprintf(stderr, "VIDIOC_SUBSCRIBE_EVENT failed, ret = %d\n", ret);
        exit(EXIT_FAILURE);
    }

    while (1) {
        if (!*continuous)
            break;
        tv.tv_sec = 2;
        FD_ZERO(&rfds);
        FD_SET(isp_fd, &rfds);
        FD_ZERO(&wfds);
        FD_SET(isp_fd, &wfds);
        FD_ZERO(&efds);
        FD_SET(isp_fd, &efds);
        ret = select(isp_fd + 1, &rfds, &wfds, &efds, &tv);
        if (ret <= 0) {
            fprintf(stderr, "get event timeout\n");
            continue;
        }
        fprintf(stderr, "get event\n");

        while (!ioctl(isp_fd, VIDIOC_DQEVENT, &ev)) {
            fprintf(stderr, "dqevent type = %d, changes = %d\n", ev.type, ev.u.src_change.changes);
        }
    }
}
