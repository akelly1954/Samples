
// COPYRIGHTS:
//
// Significant portions of this code were taken from the documentation provided by
// kernel.org in the "V4L2 API" section called "V4L2 video capture example" -
//
//          kernel.org/doc/html/v4.11/media/uapi/v4l/capture.c.html
//
// - and modified (as of 3-10-2022). From the text provided there and from
// https://linuxtv.org/docs.php, the portions of code used can be used and distributed
// without restrictions.
//

/////////////////////////////////////////////////////////////////////////////////
// MIT License
//
// Copyright (c) 2022 Andrew Kelly
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
/////////////////////////////////////////////////////////////////////////////////

// PLEASE NOTE:  The streaming method IO_METHOD_MMAP is the only one actually
// tested.  Please do not use IO_METHOD_USERPTR or IO_METHOD_READ until they are tested.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <vidcap_v4l2_driver_interface.hpp>
#include <vidcap_raw_queue_thread.hpp>
#include <LoggerCpp/LoggerCpp.h>
#include <linux/videodev2.h>

using namespace VideoCapture;

// PLEASE NOTE:  The streaming method IO_METHOD_MMAP is the only one actually
// tested.  Please do not use IO_METHOD_USERPTR or IO_METHOD_READ until they are tested.

// Please NOTE:
// The rather inappropriate #define'd expressions defined below are leftovers from before
// the V4L2 related objects were converted to C++.  However these macros are not being
// changed at this time.

// Most inappropriate of all, but easily fixed in the future:
Log::Logger *global_logger = nullptr;

#define LOGGER(x)             { global_logger->debug() << (x); }
#define LOGGER_ERROR(x)       { global_logger->error() << (x); }
#define LOGGER_STDERR(x)      { std::cerr << (x) << std::endl; LOGGER(x) }

#define LOGGER_1Arg(fmtstr, arg)   {                \
        char sbuf[512];                             \
        snprintf(sbuf, sizeof(sbuf), fmtstr, arg);  \
        LOGGER(sbuf);                               \
    }

#define LOGGER_2Arg(fmtstr, arg1, arg2)   {                 \
        char sbuf[512];                                     \
        snprintf(sbuf, sizeof(sbuf), fmtstr, arg1, arg2);   \
        LOGGER(sbuf);                                       \
    }

#define LOGGER_3Arg(fmtstr, arg1, arg2, arg3)   {                   \
        char sbuf[512];                                             \
        snprintf(sbuf, sizeof(sbuf), fmtstr, arg1, arg2, arg3);     \
        LOGGER(sbuf);                                               \
    }

#define LOGGER_STDERR_1Arg(fmtstr, arg)   {         \
        char sbuf[512];                             \
        snprintf(sbuf, sizeof(sbuf), fmtstr, arg);  \
        LOGGER_STDERR(sbuf);                               \
    }

#define LOGGER_STDERR_2Arg(fmtstr, arg1, arg2)   {          \
        char sbuf[512];                                     \
        snprintf(sbuf, sizeof(sbuf), fmtstr, arg1, arg2);   \
        LOGGER_STDERR(sbuf);                                       \
    }

#define LOGGER_STDERR_3Arg(fmtstr, arg1, arg2, arg3)   {            \
        char sbuf[512];                                             \
        snprintf(sbuf, sizeof(sbuf), fmtstr, arg1, arg2, arg3);     \
        LOGGER_STDERR(sbuf);                                               \
    }


vidcap_v4l2_driver_interface::vidcap_v4l2_driver_interface(Log::Logger lg)
    :
        logger(lg)
{
    // TODO: THIS (blobal_logger) IS TEMPORARY and should be removed asap -
    // as soon as the #define's above are changed.
    global_logger = &logger;
    set_terminated(false);
}

void vidcap_v4l2_driver_interface::initialize()
{
    ;
}


void vidcap_v4l2_driver_interface::run()
{
    if (!isterminated()) v4l2if_open_device();
    if (!isterminated()) v4l2if_init_device();
    if (!isterminated()) v4l2if_start_capturing();
    if (!isterminated()) v4l2if_mainloop();
    if (!isterminated()) v4l2if_stop_capturing();
    if (!isterminated()) v4l2if_uninit_device();
    if (!isterminated()) v4l2if_close_device();
}

void vidcap_v4l2_driver_interface::v4l2if_errno_exit(const char *s, int errnocopy)
{
    std::string msg = std::string(s) + " error, errno=" + std::to_string(errnocopy) + ": "
                      + const_cast<const char *>(strerror(errnocopy)) + " ...aborting.";
    logger.error() << msg;
    set_error_terminated(true);
    throw std::runtime_error(msg);
}

void vidcap_v4l2_driver_interface::v4l2if_exit_code(int code, const char *s)
{
    if (code)
    {
        std::string msg = std::string("vidcap_v4l2_driver_interface: TERMINATION REQUESTED, code = ") + std::to_string(code);
        logger.error() << msg;
        set_error_terminated(true);
        throw std::runtime_error(msg);
    }
    else
    {
        logger.info() << "vidcap_v4l2_driver_interface: NORMAL TERMINATION REQUESTED, code = " << code;
        set_terminated(true);
    }
}

void vidcap_v4l2_driver_interface::v4l2if_exit(const char *s)
{
    logger.info() << "vidcap_v4l2_driver_interface: NORMAL TERMINATION REQUESTED";
    set_terminated(true);
}

int vidcap_v4l2_driver_interface::v4l2if_xioctl(int fh, int request, void *arg)
{
        int r;

        do {
                r = ioctl(fh, request, arg);
        } while (-1 == r && EINTR == errno);

        return r;
}

void vidcap_v4l2_driver_interface::v4l2if_process_image(void *p, int size)
{
    if (ispaused())
    {
        // if we're paused, continue the frame capture
        // but don't really do anything with it.
        return;
    }

    video_capture_queue::add_buffer_to_raw_queue(p, size);
}

int vidcap_v4l2_driver_interface::v4l2if_read_frame(void)
{
    struct v4l2_buffer buf;
    int errnocopy = errno;
    unsigned int i;

    switch (io) {
    case IO_METHOD_READ:

        // PLEASE NOTE:  The streaming method IO_METHOD_MMAP is the only one actually
        // tested.  Please do not use IO_METHOD_USERPTR or IO_METHOD_READ until they are tested.

        if (-1 == read(fd, buffers[0].start, buffers[0].length))
        {
            switch (errno) {
            case EAGAIN:
                return 0;

            case EIO:
                /* Could ignore EIO, see spec. */

                /* fall through */

            default:
                v4l2if_errno_exit("v4l2if_read_frame: read", errnocopy);  // Does not return.
            }
        }

        v4l2if_process_image(buffers[0].start, buffers[0].length);
        break;

        case IO_METHOD_MMAP:
            CLEAR(buf);

            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;

            if (-1 == v4l2if_xioctl(fd, VIDIOC_DQBUF, &buf)) {
                errnocopy = errno;
                switch (errno) {
                case EAGAIN:
                    return 0;

                case EIO:
                    /* Could ignore EIO, see spec. */

                    /* fall through */

                default:
                    v4l2if_errno_exit("v4l2if_read_frame: VIDIOC_DQBUF (#1)", errnocopy);  // Does not return.
                }
            }

            assert(buf.index < numbufs);

            v4l2if_process_image(buffers[buf.index].start, buf.bytesused);

            if (-1 == v4l2if_xioctl(fd, VIDIOC_QBUF, &buf))
                v4l2if_errno_exit("v4l2if_read_frame: VIDIOC_QBUF (#2)", errno); // no need to copy errno here
            break;

        case IO_METHOD_USERPTR:
            // PLEASE NOTE:  The streaming method IO_METHOD_MMAP is the only one actually
            // tested.  Please do not use IO_METHOD_USERPTR or IO_METHOD_READ until they are tested.

            CLEAR(buf);

            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_USERPTR;

            if (-1 == v4l2if_xioctl(fd, VIDIOC_DQBUF, &buf)) {
                errnocopy = errno;

                switch (errnocopy) {
                case EAGAIN:
                        return 0;

                case EIO:
                        /* Could ignore EIO, see spec. */

                        /* fall through */

                default:
                    v4l2if_errno_exit("v4l2if_read_frame: VIDIOC_QBUF (#3)", errnocopy);
                }
            }

            for (i = 0; i < numbufs; ++i)
            {
                if (buf.m.userptr == (unsigned long)buffers[i].start && buf.length == buffers[i].length)
                {
                    break;
                }
            }

            assert(i < numbufs);

            v4l2if_process_image((void *)buf.m.userptr, buf.bytesused);

            if (-1 == v4l2if_xioctl(fd, VIDIOC_QBUF, &buf))
                v4l2if_errno_exit("v4l2if_read_frame: VIDIOC_QBUF (#4)", errno); // no need to copy errno here
            break;
        }

        return 1;
}

void vidcap_v4l2_driver_interface::v4l2if_mainloop(void)
{
    int count = int_frame_count;

    // Loop while not finished, and original int_frame_count was 0, or counter is not done counting
    // (Having a null finished callback() is an error checked for earlier).
    do
    {
        if (int_frame_count != 0 && count-- <= 0) break;

        for (;;)
        {
            fd_set fds;
            struct timeval tv;
            int r;
            int errnocopy;

            FD_ZERO(&fds);
            FD_SET(fd, &fds);

            /* Timeout. */
            tv.tv_sec = 4;
            tv.tv_usec = 0;

            r = select(fd + 1, &fds, NULL, NULL, &tv);
            errnocopy = errno;

            if (-1 == r) {
                if (EINTR == errnocopy) continue;
                v4l2if_errno_exit("v4l2if_mainloop: select (#1)", errnocopy);
            }

            if (0 == r) {
                v4l2if_errno_exit("v4l2if_mainloop: select (#2)", errnocopy);
            }

            if (v4l2if_read_frame())
                    break;
            /* EAGAIN - continue select loop. */
        }
    } while (! isterminated());

    if (isterminated())
    {
        logger.info() << "v4l2if_mainloop: CAPTURE TERMINATION REQUESTED.";
    }
}

void vidcap_v4l2_driver_interface::v4l2if_stop_capturing(void)
{
        enum v4l2_buf_type type;

        switch (io) {
        case IO_METHOD_READ:
                /* Nothing to do. */
                break;

        case IO_METHOD_MMAP:
        case IO_METHOD_USERPTR:
                type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                if (-1 == v4l2if_xioctl(fd, VIDIOC_STREAMOFF, &type))
                        v4l2if_errno_exit("VIDIOC_STREAMOFF", errno);
                break;
        }
}

void vidcap_v4l2_driver_interface::v4l2if_start_capturing(void)
{
        unsigned int i;
        enum v4l2_buf_type type;

        switch (io) {
        case IO_METHOD_READ:
            // PLEASE NOTE:  The streaming method IO_METHOD_MMAP is the only one actually
            // tested.  Please do not use IO_METHOD_USERPTR or IO_METHOD_READ until they are tested.

                /* Nothing to do. */
                break;

        case IO_METHOD_MMAP:
                for (i = 0; i < numbufs; ++i) {
                        struct v4l2_buffer buf;

                        CLEAR(buf);
                        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                        buf.memory = V4L2_MEMORY_MMAP;
                        buf.index = i;

                        if (-1 == v4l2if_xioctl(fd, VIDIOC_QBUF, &buf))
                                v4l2if_errno_exit("VIDIOC_QBUF", errno);
                }
                type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                if (-1 == v4l2if_xioctl(fd, VIDIOC_STREAMON, &type))
                        v4l2if_errno_exit("VIDIOC_STREAMON", errno);
                break;

        case IO_METHOD_USERPTR:
            // PLEASE NOTE:  The streaming method IO_METHOD_MMAP is the only one actually
            // tested.  Please do not use IO_METHOD_USERPTR or IO_METHOD_READ until they are tested.

                for (i = 0; i < numbufs; ++i) {
                        struct v4l2_buffer buf;

                        CLEAR(buf);
                        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                        buf.memory = V4L2_MEMORY_USERPTR;
                        buf.index = i;
                        buf.m.userptr = (unsigned long)buffers[i].start;
                        buf.length = buffers[i].length;

                        if (-1 == v4l2if_xioctl(fd, VIDIOC_QBUF, &buf))
                                v4l2if_errno_exit("VIDIOC_QBUF", errno);
                }
                type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                if (-1 == v4l2if_xioctl(fd, VIDIOC_STREAMON, &type))
                        v4l2if_errno_exit("VIDIOC_STREAMON", errno);
                break;
        }
}

// PLEASE NOTE:  The streaming method IO_METHOD_MMAP is the only one actually
// tested.  Please do not use IO_METHOD_USERPTR or IO_METHOD_READ until they are tested.
void vidcap_v4l2_driver_interface::v4l2if_uninit_device(void)
{
        unsigned int i;

        switch (io) {
        case IO_METHOD_READ:
                free(buffers[0].start);
                break;

        case IO_METHOD_MMAP:
                for (i = 0; i < numbufs; ++i)
                        if (-1 == munmap(buffers[i].start, buffers[i].length))
                                v4l2if_errno_exit("munmap", errno);
                break;

        case IO_METHOD_USERPTR:
                for (i = 0; i < numbufs; ++i)
                        free(buffers[i].start);
                break;
        }

        free(buffers);
}

void vidcap_v4l2_driver_interface::v4l2if_init_read(unsigned int buffer_size)
{
        buffers = static_cast<buffer *>( calloc(1, sizeof(*buffers)));

        if (!buffers) {
            v4l2if_exit("Out of memory");
        }

        buffers[0].length = buffer_size;
        buffers[0].start = malloc(buffer_size);

        if (!buffers[0].start) {
                v4l2if_exit("Out of memory");
        }
}

void vidcap_v4l2_driver_interface::v4l2if_init_mmap(void)
{
        struct v4l2_requestbuffers req;

        CLEAR(req);

        req.count = 4;
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_MMAP;

        if (-1 == v4l2if_xioctl(fd, VIDIOC_REQBUFS, &req)) {
            if (EINVAL == errno) {
                LOGGER_STDERR_1Arg("%s does not support memory mapping", dev_name);
            }
            v4l2if_exit("init_mmap - nommap");
        }

        if (req.count < 2) {
            {
                LOGGER_STDERR_1Arg("Insufficient buffer memory on %s", dev_name);
            }
            v4l2if_exit("init_mmap - reqcount");
        }

        buffers = static_cast<buffer *>(calloc(req.count, sizeof(*buffers)));

        if (!buffers) {
            v4l2if_exit("Out of memory");
        }

        for (numbufs = 0; numbufs < req.count; ++numbufs) {
                struct v4l2_buffer buf;

                CLEAR(buf);

                buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory      = V4L2_MEMORY_MMAP;
                buf.index       = numbufs;

                if (-1 == v4l2if_xioctl(fd, VIDIOC_QUERYBUF, &buf))
                        v4l2if_errno_exit("VIDIOC_QUERYBUF", errno);

                buffers[numbufs].length = buf.length;
                buffers[numbufs].start =
                        mmap(NULL /* start anywhere */,
                              buf.length,
                              PROT_READ | PROT_WRITE /* required */,
                              MAP_SHARED /* recommended */,
                              fd, buf.m.offset);

                if (MAP_FAILED == buffers[numbufs].start)
                        v4l2if_errno_exit("mmap", errno);
        }
}

void vidcap_v4l2_driver_interface::v4l2if_init_userp(unsigned int buffer_size)
{
        struct v4l2_requestbuffers req;

        CLEAR(req);

        req.count  = 4;
        req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_USERPTR;

        if (-1 == v4l2if_xioctl(fd, VIDIOC_REQBUFS, &req)) {
            if (EINVAL == errno) {
                LOGGER_STDERR_1Arg("%s does not support user pointer i/o", dev_name);
                v4l2if_exit("init_userp - nouserp");
            } else {
                v4l2if_errno_exit("VIDIOC_REQBUFS", errno);
            }
        }

        buffers = static_cast<buffer *>((calloc(4, sizeof(*buffers))));

        if (!buffers) {
                v4l2if_exit("Out of memory - nobuf");
        }

        for (numbufs = 0; numbufs < 4; ++numbufs) {
                buffers[numbufs].length = buffer_size;
                buffers[numbufs].start = malloc(buffer_size);

                if (!buffers[numbufs].start) {
                        v4l2if_exit("Out of memory - nostart");
                }
        }
}

void vidcap_v4l2_driver_interface::v4l2if_init_device(void)
{
        struct v4l2_capability cap;
        struct v4l2_cropcap cropcap;
        struct v4l2_crop crop;
        struct v4l2_format fmt;
        unsigned int min;

        if (-1 == v4l2if_xioctl(fd, VIDIOC_QUERYCAP, &cap)) {
            if (EINVAL == errno) {
                LOGGER_STDERR_1Arg("%s is no V4L2 device", dev_name);
                v4l2if_exit("not a v4l2 device");
            } else {
                v4l2if_errno_exit("VIDIOC_QUERYCAP", errno);
            }
        }

        if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
            LOGGER_STDERR_1Arg("%s is no capture device", dev_name);
            v4l2if_exit("device is not a capture device");
        }

        // PLEASE NOTE:  The streaming method IO_METHOD_MMAP is the only one actually
        // tested.  Please do not use IO_METHOD_USERPTR or IO_METHOD_READ until they are tested.

        switch (io) {
        case IO_METHOD_READ:
                if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
                    LOGGER_STDERR_1Arg("%s does not support read i/o", dev_name);
                    v4l2if_exit("device does not support read i/o");
                }
                break;

        case IO_METHOD_MMAP:
        case IO_METHOD_USERPTR:
                if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
                    LOGGER_STDERR_1Arg("%s does not support streaming i/o", dev_name);
                    v4l2if_exit("device does not support streaming i/o");
                }
                break;
        }


        /* Select video input, video standard and tune here. */


        CLEAR(cropcap);

        cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        if (0 == v4l2if_xioctl(fd, VIDIOC_CROPCAP, &cropcap)) {
                crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                crop.c = cropcap.defrect; /* reset to default */

                if (-1 == v4l2if_xioctl(fd, VIDIOC_S_CROP, &crop)) {
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


        CLEAR(fmt);

        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (force_format == 2) {
            fmt.fmt.pix.width       = 1920;
            fmt.fmt.pix.height      = 1080;
            fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_H264;
            fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

            if (-1 == v4l2if_xioctl(fd, VIDIOC_S_FMT, &fmt))
                    v4l2if_errno_exit("VIDIOC_S_FMT", errno);

            /* Note VIDIOC_S_FMT may change width and height. */
        } else if (force_format == 1) {
                fmt.fmt.pix.width       = 640;
                fmt.fmt.pix.height      = 480;
                fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
                fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

                if (-1 == v4l2if_xioctl(fd, VIDIOC_S_FMT, &fmt))
                        v4l2if_errno_exit("VIDIOC_S_FMT", errno);

                /* Note VIDIOC_S_FMT may change width and height. */
        } else {
                /* Preserve original settings as set by v4l2-ctl for example */
                if (-1 == v4l2if_xioctl(fd, VIDIOC_G_FMT, &fmt))
                        v4l2if_errno_exit("VIDIOC_G_FMT", errno);
        }

        /* Buggy driver paranoia. */
        min = fmt.fmt.pix.width * 2;
        if (fmt.fmt.pix.bytesperline < min)
                fmt.fmt.pix.bytesperline = min;
        min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
        if (fmt.fmt.pix.sizeimage < min)
                fmt.fmt.pix.sizeimage = min;

        {
            LOGGER_STDERR_2Arg("driver: frame: %lu x %lu", fmt.fmt.pix.width, fmt.fmt.pix.height);

            char bfp[256];
            switch(fmt.fmt.pix.pixelformat)
            {
            case V4L2_PIX_FMT_YUYV:
                strcpy (bfp, "YUYV: aka \"YUV 4:2:2\": Packed format with ½ horizontal chroma resolution");
                break;
            case V4L2_PIX_FMT_H264:
                strcpy (bfp, "H264: H264 with start codes");
                break;
            default:
                "See </usr/include/linux/videodev2.h> for all format encoding macros";
                break;
            }

            LOGGER_STDERR_2Arg("driver: pixel format set to 0x%lX - %s", fmt.fmt.pix.pixelformat, bfp);
        }

        LOGGER_STDERR_1Arg("driver: bytes required: %lu", fmt.fmt.pix.sizeimage);
        LOGGER_STDERR_1Arg("driver: I/O METHOD: %s", string_io_methods[io]);

        // PLEASE NOTE:  The streaming method IO_METHOD_MMAP is the only one actually
        // tested.  Please do not use IO_METHOD_USERPTR or IO_METHOD_READ until they are tested.

        switch (io) {
        case IO_METHOD_READ:
                v4l2if_init_read(fmt.fmt.pix.sizeimage);
                break;

        case IO_METHOD_MMAP:
                v4l2if_init_mmap();
                break;

        case IO_METHOD_USERPTR:
                v4l2if_init_userp(fmt.fmt.pix.sizeimage);
                break;
        }
}

void vidcap_v4l2_driver_interface::v4l2if_close_device(void)
{
        if (-1 == close(fd))
                v4l2if_errno_exit("close", errno);

        fd = -1;
}

void vidcap_v4l2_driver_interface::v4l2if_open_device(void)
{
        struct stat st;

        if (-1 == stat(dev_name, &st)) {
            LOGGER_3Arg("Cannot identify '%s': errno=%d, %s", dev_name, errno, strerror(errno));
            v4l2if_exit("cannot identify device");
        }

        if (!S_ISCHR(st.st_mode)) {
            LOGGER_1Arg("%s is no device", dev_name);
            v4l2if_exit("not a device");
        }

        fd = open(dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);

        if (-1 == fd) {
            LOGGER_3Arg("Cannot open '%s': errno=%d, %s", dev_name, errno, strerror(errno));
            v4l2if_exit("cannot open device");
        }
        LOGGER_STDERR_1Arg("device: %s", dev_name);
}

