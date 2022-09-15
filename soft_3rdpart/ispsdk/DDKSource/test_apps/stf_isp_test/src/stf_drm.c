// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 StarFive Technology Co., Ltd.
 */
#include <sys/mman.h>
#include <libdrm/drm_fourcc.h>
#include "stf_drm.h"
#include "stf_log.h"

static int drm_open(const char *path, int need_dumb, int need_prime)
{
    int fd, flags;
    uint64_t has_it;

    LOG(STF_LEVEL_TRACE, "Enter\n");
    if ((fd = open(path, O_RDWR)) < 0) {
        LOG(STF_LEVEL_ERR, "cannot open \"%s\"\n", path);
        error("open");
    }

    /* set FD_CLOEXEC flag */
    if ((flags = fcntl(fd, F_GETFD)) < 0
        || fcntl(fd, F_SETFD, flags | FD_CLOEXEC) < 0)
        fatal("fcntl FD_CLOEXEC failed");

    if (need_dumb) {
        if (drmGetCap(fd, DRM_CAP_DUMB_BUFFER, &has_it) < 0)
            error("drmGetCap DRM_CAP_DUMB_BUFFER failed!");
        if (has_it == 0)
            fatal("can't give us dumb buffers");
    }

    if (need_prime) {
        /* check prime */
        if (drmGetCap(fd, DRM_CAP_PRIME, &has_it) < 0)
            error("drmGetCap DRM_CAP_PRIME failed!");
        if (!(has_it & DRM_PRIME_CAP_EXPORT))
            fatal("can't export dmabuf");
    }

    LOG(STF_LEVEL_TRACE, "Exit\n");
    return fd;
}

static struct drm_dev_t *drm_find_dev(int fd, uint32_t width, uint32_t height)
{
    int i, m;
    struct drm_dev_t *dev = NULL, *dev_head = NULL;
    drmModeRes *res;
    drmModeConnector *conn;
    drmModeEncoder *enc;
    drmModeModeInfo *mode = NULL, *preferred = NULL;

    LOG(STF_LEVEL_TRACE, "Enter\n");
    if ((res = drmModeGetResources(fd)) == NULL) {
        fatal("drmModeGetResources() failed");
    }

    LOG(STF_LEVEL_DEBUG, "count_connectors=%d\n", res->count_connectors);
    /* find all available connectors */
    for (i = 0; i < res->count_connectors; i++) {
        conn = drmModeGetConnector(fd, res->connectors[i]);
        if (conn != NULL && conn->connection == DRM_MODE_CONNECTED && conn->count_modes > 0) {
            dev = (struct drm_dev_t *) malloc(sizeof(struct drm_dev_t));
            memset(dev, 0, sizeof(struct drm_dev_t));

            /* find preferred mode */
            for (m = 0; m < conn->count_modes; m++) {
                mode = &conn->modes[m];
                if (mode->hdisplay == width && mode->vdisplay == height) {
                    preferred = mode;
                    LOG(STF_LEVEL_INFO, "find the matched mode, modes index=%d, %dx%d\n",
                            m, width, height);
                    break;
                }
                if (mode->type & DRM_MODE_TYPE_PREFERRED) {
                    preferred = mode;
                    LOG(STF_LEVEL_INFO, "find perferred mode, modes index=%d\n", m);
                }
                LOG(STF_LEVEL_DEBUG, "mode: %dx%d %s\n", mode->hdisplay, mode->vdisplay,
                        mode->type & DRM_MODE_TYPE_PREFERRED ? "*" : "");
            }

            if (!preferred)
                preferred = &conn->modes[0];

            dev->conn_id = conn->connector_id;
            // dev->next = NULL;
            if (conn->count_encoders) {
                if (conn->encoder_id) {
                    dev->enc_id = conn->encoder_id;
                } else {
                    dev->enc_id = conn->encoders[0];
                }
            } else {
                fatal("count_encoders is 0");
            }

            memcpy(&dev->mode, preferred, sizeof(drmModeModeInfo));
            dev->width = preferred->hdisplay;
            dev->height = preferred->vdisplay;

            /* FIXME: use default encoder/crtc pair */
            if ((enc = drmModeGetEncoder(fd, dev->enc_id)) == NULL)
                fatal("drmModeGetEncoder() faild");
#if 1
            if (enc->possible_clones == 2) {
                dev->crtc_id = res->crtcs[1];
            } else {
                dev->crtc_id = res->crtcs[0];
            }
#else
            if (enc->crtc_id) {
                dev->crtc_id = enc->crtc_id;
            } else {
                dev->crtc_id = res->crtcs[0];
            }
#endif //#if 1
            drmModeFreeEncoder(enc);

            dev->saved_crtc = NULL;

            /* create dev list */
            dev->next = dev_head;
            dev_head = dev;
        }
        drmModeFreeConnector(conn);
    }

    drmModeFreeResources(res);

    LOG(STF_LEVEL_INFO, "selected connector(s)\n");
    for (dev = dev_head; dev != NULL; dev = dev->next) {
        LOG(STF_LEVEL_INFO, "connector id:%d\n", dev->conn_id);
        LOG(STF_LEVEL_INFO, "\tencoder id:%d crtc id:%d\n", dev->enc_id, dev->crtc_id);
        LOG(STF_LEVEL_INFO, "\twidth:%d height:%d\n", dev->width, dev->height);
    }

    LOG(STF_LEVEL_TRACE, "Exit\n");
    return dev_head;
}

static void drm_setup_buffer(int fd, struct drm_dev_t *dev,
        int width, int height,
        struct drm_buffer_t *buffer, int map, int export)
{
    struct drm_mode_create_dumb create_req;
    struct drm_mode_map_dumb map_req;

    LOG(STF_LEVEL_TRACE, "Enter\n");
    buffer->dmabuf_fd = -1;

    memset(&create_req, 0, sizeof(struct drm_mode_create_dumb));
    create_req.width = width;
    create_req.height = height;
    create_req.bpp = 24;
    if (drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &create_req) < 0) {
        fatal("drmIoctl DRM_IOCTL_MODE_CREATE_DUMB failed");
    }

    buffer->pitch = create_req.pitch;
    buffer->size = create_req.size;
    /* GEM buffer handle */
    buffer->bo_handle = create_req.handle;

    if (export) {
        int ret;
        ret = drmPrimeHandleToFD(fd, buffer->bo_handle,
                DRM_CLOEXEC | DRM_RDWR, &buffer->dmabuf_fd);
        if (ret < 0) {
            fatal("could not export the dump buffer");
        }
        LOG(STF_LEVEL_INFO, "drm fd=%d, bo_handle=%d, dmabuf_fd=%d\n",
                fd, buffer->bo_handle, buffer->dmabuf_fd);
    }

    if (map) {
        memset(&map_req, 0, sizeof(struct drm_mode_map_dumb));
        map_req.handle = buffer->bo_handle;
        if (drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &map_req)) {
            fatal("drmIoctl DRM_IOCTL_MODE_MAP_DUMB failed");
        }
        buffer->buf = (uint8_t *)mmap(0, buffer->size,
                PROT_READ | PROT_WRITE, MAP_SHARED,
                fd, map_req.offset);
        if (buffer->buf == MAP_FAILED) {
            error("mmap");
        }
    }

    LOG(STF_LEVEL_TRACE, "Exit\n");
}

static uint32_t drm_get_bpp_from_drm(uint32_t drmfmt)
{
    uint32_t bpp;

    switch (drmfmt) {
    case DRM_FORMAT_YUV420:
    case DRM_FORMAT_YVU420:
    case DRM_FORMAT_YUV422:
    case DRM_FORMAT_NV12:
    case DRM_FORMAT_NV21:
    case DRM_FORMAT_NV16:
        bpp = 8;
        break;
    case DRM_FORMAT_P010:
        bpp = 10;
        break;
    case DRM_FORMAT_UYVY:
    case DRM_FORMAT_YUYV:
    case DRM_FORMAT_YVYU:
    case DRM_FORMAT_P016:
        bpp = 16;
        break;
    case DRM_FORMAT_BGR888:
    case DRM_FORMAT_RGB888:
        bpp = 24;
        break;
    default:
        bpp = 32;
        break;
    }

    return bpp;
}

static uint32_t drm_get_height_from_drm (uint32_t drmfmt, uint32_t height)
{
    uint32_t ret;

    switch (drmfmt) {
    case DRM_FORMAT_YUV420:
    case DRM_FORMAT_YVU420:
    case DRM_FORMAT_YUV422:
    case DRM_FORMAT_NV12:
    case DRM_FORMAT_NV21:
    case DRM_FORMAT_P010:
    case DRM_FORMAT_P016:
        ret = height * 3 / 2;
        break;
    case DRM_FORMAT_NV16:
        ret = height * 2;
        break;
    default:
        ret = height;
        break;
    }

    return ret;
}

static int drm_setup_buffer2(int fd, struct drm_dev_t *dev,
        int width, int height,
        struct drm_buffer_t *buffer, int map, int export)
{
    struct drm_mode_create_dumb create_req;
    struct drm_mode_map_dumb map_req;
    int ret = 0;

    LOG(STF_LEVEL_TRACE, "Enter\n");
    buffer->dmabuf_fd = -1;

    memset(&create_req, 0, sizeof(struct drm_mode_create_dumb));
    // For NV12 or NV21, bpp is 8, height is height * 3 / 2
    create_req.width = width;
    create_req.height = drm_get_height_from_drm(dev->drm_format, height);
    create_req.bpp = drm_get_bpp_from_drm(dev->drm_format);
    if (drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &create_req) < 0) {
        fatal("drmIoctl DRM_IOCTL_MODE_CREATE_DUMB failed");
    }
    buffer->pitch = create_req.pitch;
    buffer->size = create_req.size;
    buffer->bo_handle = create_req.handle; /* GEM buffer handle */

    if (export) {
#if 0
        memset(&map_req, 0, sizeof(struct drm_mode_map_dumb));
        map_req.handle = buffer->bo_handle;
        ret = drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &map_req);
        if (ret) {
            fatal("drmIoctl DRM_IOCTL_MODE_MAP_DUMB failed");
        }
        buffer->buf = (uint8_t *) mmap(0, buffer->size,
            PROT_READ | PROT_WRITE, MAP_SHARED,
            fd, map_req.offset);
        if (buffer->buf == MAP_FAILED) {
            error("drm mmap");
        }
        LOG(STF_LEVEL_INFO, "buffer->buf= %p\n", buffer->buf);
#endif

        ret = drmPrimeHandleToFD(fd, buffer->bo_handle,
                DRM_CLOEXEC | DRM_RDWR, &buffer->dmabuf_fd);
        if (ret < 0) {
            fatal("could not export the dump buffer");
        }
        LOG(STF_LEVEL_DEBUG, "drm fd=%d, bo_handle=%d, dmabuf_fd=%d\n",
                fd, buffer->bo_handle, buffer->dmabuf_fd);
    }

    if (map) {
        memset(&map_req, 0, sizeof(struct drm_mode_map_dumb));
        map_req.handle = buffer->bo_handle;

        if (drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &map_req)) {
            fatal("drmIoctl DRM_IOCTL_MODE_MAP_DUMB failed");
        }
        buffer->buf = (uint8_t *)mmap(0, buffer->size,
                PROT_READ | PROT_WRITE, MAP_SHARED,
                fd, map_req.offset);
        if (buffer->buf == MAP_FAILED) {
            error("mmap");
        }
    }

    uint32_t handles[4] = {0}, pitches[4] = {0}, offsets[4] = {0};
    handles[0] = buffer->bo_handle;
    pitches[0] = buffer->pitch;
    offsets[0] = 0;
    handles[1] = buffer->bo_handle;
    pitches[1] = buffer->pitch;
    offsets[1] = buffer->pitch * height;
    // planes[0] = virtual;
    // planes[1] = virtual + offsets[1];

    LOG(STF_LEVEL_INFO, "map=%d, export=%d. width=%d, height=%d, "
            "pitch=%d, size=%d, bo_handle=%d, dmabuf_fd=%d\n",
            map, export, width, height, buffer->pitch, buffer->size,
            buffer->bo_handle, buffer->dmabuf_fd);

    //DRM_FORMAT_NV12
    ret = drmModeAddFB2(fd, dev->width, dev->height,
            dev->drm_format, handles, pitches, offsets, &(buffer->fb_id), 0);
    LOG(STF_LEVEL_TRACE, "Exit\n");
    return ret;
    // return drmModeAddFB(fd, dev->width, dev->height,
    //         DEPTH, BPP, dev->bufs[i].pitch,
    //         dev->bufs[i].bo_handle, &dev->bufs[i].fb_id);
}

static void drm_setup_dummy(int fd, struct drm_dev_t *dev, int map, int export)
{
    int i;

    LOG(STF_LEVEL_TRACE, "Enter\n");
    for (i = 0; i < BUFCOUNT; i++) {
        drm_setup_buffer(fd, dev, dev->width, dev->height,
                &dev->bufs[i], map, export);
    }

    /* Assume all buffers have the same pitch */
    dev->pitch = dev->bufs[0].pitch;
    LOG(STF_LEVEL_INFO, "DRM: buffer pitch = %d bytes\n", dev->pitch);
    LOG(STF_LEVEL_TRACE, "Exit\n");
}

static void drm_setup_fb(int fd, struct drm_dev_t *dev, int map, int export)
{
    int i;

    LOG(STF_LEVEL_TRACE, "Enter\n");
#if 0
    for (i = 0; i < BUFCOUNT; i++) {
        int ret;

        drm_setup_buffer(fd, dev, dev->width, dev->height,
                    &dev->bufs[i], map, export);

        ret = drmModeAddFB(fd, dev->width, dev->height,
            DEPTH, BPP, dev->bufs[i].pitch,
            dev->bufs[i].bo_handle, &dev->bufs[i].fb_id);
        if (ret)
            fatal("drmModeAddFB failed");
LOG(STF_LEVEL_INFO, "width=%d,height=%d,pitch=%d,bo_handle=%d,fb_id=%d\n",
    dev->width, dev->height,dev->bufs[i].pitch,dev->bufs[i].bo_handle, dev->bufs[i].fb_id);
    }
#else
    for (i = 0; i < BUFCOUNT; i++) {
        int ret = -1;
        ret = drm_setup_buffer2(fd, dev, dev->width, dev->height,
                &dev->bufs[i], map, export);
        if (ret) {
            fatal("drmModeAddFB failed");
        }
    }
#endif

    /* Assume all buffers have the same pitch */
    dev->pitch = dev->bufs[0].pitch;
    LOG(STF_LEVEL_DEBUG, "DRM: buffer pitch %d bytes\n", dev->pitch);

    dev->saved_crtc = drmModeGetCrtc(fd, dev->crtc_id); /* must store crtc data */

    /* First buffer to DRM */
    if (drmModeSetCrtc(fd, dev->crtc_id, dev->bufs[0].fb_id, 0, 0,
            &dev->conn_id, 1, &dev->mode)) {
        fatal("drmModeSetCrtc() failed");
    }

    /* First flip */
    drmModePageFlip(fd, dev->crtc_id,
                        dev->bufs[0].fb_id, DRM_MODE_PAGE_FLIP_EVENT,
                        dev);

    LOG(STF_LEVEL_TRACE, "Exit\n");
}

static void drm_destroy(int fd, struct drm_dev_t *dev_head)
{
    struct drm_dev_t *devp, *devp_tmp;
    int i;

    LOG(STF_LEVEL_TRACE, "Enter\n");
    for (devp = dev_head; devp != NULL;) {
        if (devp->saved_crtc) {
            drmModeSetCrtc(fd, devp->saved_crtc->crtc_id, devp->saved_crtc->buffer_id,
                devp->saved_crtc->x, devp->saved_crtc->y, &devp->conn_id, 1, &devp->saved_crtc->mode);
            drmModeFreeCrtc(devp->saved_crtc);
        }

        for (i = 0; i < BUFCOUNT; i++) {
            struct drm_mode_destroy_dumb dreq = { .handle = devp->bufs[i].bo_handle };

            if (devp->bufs[i].buf) {
                munmap(devp->bufs[i].buf, devp->bufs[i].size);
            }
            if (devp->bufs[i].dmabuf_fd >= 0) {
                close(devp->bufs[i].dmabuf_fd);
            }
            drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
            drmModeRmFB(fd, devp->bufs[i].fb_id);
        }

        devp_tmp = devp;
        devp = devp->next;
        free(devp_tmp);
    }

    drmClose(fd);
    LOG(STF_LEVEL_TRACE, "Exit\n");
}

static struct drm_dev_t *drm_find_connector(DRMParam_t *param, drm_dev_t *dev)
{
    drm_dev_t *tmp_dev = NULL;

    for (tmp_dev = dev; tmp_dev != NULL; tmp_dev = tmp_dev->next) {
        if (param->connector_id == tmp_dev->conn_id && param->connector_id != 0)
            return tmp_dev;
    }
    return dev;
}

void stf_drm_open(DRMParam_t *param, char *device_name, int iomthd)
{
    int drm_fd;
    LOG(STF_LEVEL_TRACE, "Enter\n");

    if (IO_METHOD_MMAP == iomthd) {
        drm_fd = drm_open(device_name, 1, 0);
    } else if (IO_METHOD_DMABUF == iomthd) {
        drm_fd = drm_open(device_name, 1, 1);
    } else {
        LOG(STF_LEVEL_ERR, "drm: not support io_method %d\n", iomthd);
        exit (EXIT_FAILURE);
    }

    param->fd = drm_fd;
    LOG(STF_LEVEL_TRACE, "Exit\n");
}

// try to use v4l2_fmt
void stf_drm_init(DRMParam_t *param, uint32_t width, uint32_t height,
        uint32_t v4l2_fmt, int iomthd, int *dmabufs, int nsize)
{
    struct drm_dev_t *dev_head = NULL;
    int i = 0;

    LOG(STF_LEVEL_TRACE, "Enter\n");
    dev_head = drm_find_dev(param->fd, width, height);
    if (dev_head == NULL) {
        LOG(STF_LEVEL_ERR, "available drm_dev not found\n");
        exit (EXIT_FAILURE);
    }

#if 1
    dev_head = drm_find_connector(param, dev_head);
#endif //#if 1
    dev_head->drm_format = v4l2fmt_to_drmfmt(v4l2_fmt);

    if (IO_METHOD_MMAP == iomthd) {
        drm_setup_fb(param->fd, dev_head, 1, 0);
    } else if (IO_METHOD_DMABUF == iomthd) {
        drm_setup_fb(param->fd, dev_head, 0, 1);
        assert(nsize == BUFCOUNT);
        for (i = 0; i < nsize; i++) {
            dmabufs[i] = dev_head->bufs[i].dmabuf_fd;
        }
    } else {
        LOG(STF_LEVEL_ERR, "drm: not support io_method %d\n", iomthd);
        exit (EXIT_FAILURE);
    }

    param->dev_head = dev_head;
    LOG(STF_LEVEL_TRACE, "Exit\n");
}

void stf_drm_close(DRMParam_t *param)
{
    drm_destroy(param->fd, param->dev_head);
}
