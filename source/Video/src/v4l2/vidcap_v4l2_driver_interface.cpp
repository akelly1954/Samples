
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
#include <Utility.hpp>
#include <vidcap_v4l2_driver_interface.hpp>
#include <vidcap_raw_queue_thread.hpp>
#include <LoggerCpp/LoggerCpp.h>
#include <linux/videodev2.h>

using namespace VideoCapture;

// PLEASE NOTE:  The streaming method IO_METHOD_MMAP is the only one actually
// tested.  Please do not use IO_METHOD_USERPTR or IO_METHOD_READ until they are tested.

// Please NOTE:
// TODO: The rather inappropriate #define'd expressions defined below are leftovers from before
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
    // Command line and json file configuration is done.
    // Obtain the changes from the vcGlobals object.
    pixel_format = Video::vcGlobals::pixel_format;
    int_frame_count = Video::vcGlobals::framecount;
    dev_name = Video::vcGlobals::str_dev_name;
}

void vidcap_v4l2_driver_interface::run()
{
    if (isterminated() || !v4l2if_open_device())
    {
        if (!isterminated())
        {
            logger.error() << "vidcap_v4l2_driver_interface::run() - v4l2if_open_device() FAILED. Terminating...";
            set_error_terminated(true);
        }
    }

    if (isterminated() || !v4l2if_init_device())
    {
        if (!isterminated())
        {
            logger.error() << "vidcap_v4l2_driver_interface::run() - v4l2if_init_device() FAILED. Terminating...";
            set_error_terminated(true);
        }
    }

    if (isterminated() || !v4l2if_start_capturing())
    {
        if (!isterminated())
        {
            logger.error() << "vidcap_v4l2_driver_interface::run() - v4l2if_start_capturing() FAILED. Terminating...";
            set_error_terminated(true);
        }
    }

    if (isterminated() || !v4l2if_mainloop())
    {
        if (!isterminated())
        {
            logger.error() << "vidcap_v4l2_driver_interface::run() - v4l2if_mainloop() FAILED. Terminating...";
            set_error_terminated(true);
        }
    }

    v4l2if_stop_capturing();
    v4l2if_uninit_device();
    v4l2if_close_device();

    if (iserror_terminated())
    {
        LOGGER_STDERR("vidcap_v4l2_driver_interface: ERROR TERMINATION REQUESTED.");
    }
}

void vidcap_v4l2_driver_interface::v4l2if_errno_exit(const char *s, int errnocopy)
{
    std::string msg = std::string(s) + " error, errno=" + std::to_string(errnocopy) + ": "
                      + const_cast<const char *>(strerror(errnocopy)) + " ...aborting.";
    logger.error() << msg;
    v4l2if_cleanup_stop_capturing();
    v4l2if_cleanup_uninit_device();
    v4l2if_cleanup_close_device();
    set_error_terminated(true);
    throw std::runtime_error(msg);
}

void vidcap_v4l2_driver_interface::v4l2if_error_exit(const char *s)
{
    std::string msg = std::string("vidcap_v4l2_driver_interface: ERROR TERMINATION REQUESTED: ") + s;
    logger.error() << msg;
    v4l2if_cleanup_stop_capturing();
    v4l2if_cleanup_uninit_device();
    v4l2if_cleanup_close_device();
    set_error_terminated(true);
    throw std::runtime_error(msg);
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

bool vidcap_v4l2_driver_interface::v4l2if_read_frame(void)
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
            errnocopy = errno;
            switch (errnocopy) {
            case EAGAIN:
                return false;

            case EIO:
                /* Could ignore EIO, see spec. */

                /* fall through */

            default:
                LOGGER_2Arg("v4l2if_read_frame: read() call failed: errno=%d: %s", errnocopy, std::strerror(errnocopy));
                return true;
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
                    return false;

                case EIO:
                    /* Could ignore EIO, see spec. */

                    /* fall through */

                default:
                    LOGGER_2Arg("v4l2if_read_frame: ioctl() VIDIOC_DQBUF call failed: errno=%d: %s", errnocopy, std::strerror(errnocopy));
                    return true;
                }
            }

            assert(buf.index < numbufs);

            v4l2if_process_image(buffers[buf.index].start, buf.bytesused);

            if (v4l2if_xioctl(fd, VIDIOC_QBUF, &buf) == -1)
            {
                errnocopy = errno;
                LOGGER_2Arg("v4l2if_read_frame: ioctl() VIDIOC_DQBUF call failed: errno=%d: %s", errnocopy, std::strerror(errnocopy));
                return true;
            }
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
                        return false;

                case EIO:
                        /* Could ignore EIO, see spec. */

                        /* fall through */

                default:
                    LOGGER_2Arg("v4l2if_read_frame: ioctl() VIDIOC_DQBUF call failed: errno=%d: %s", errnocopy, std::strerror(errnocopy));
                    return true;
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
            {
                errnocopy = errno;
                LOGGER_2Arg("v4l2if_read_frame: ioctl() VIDIOC_QBUF call failed: errno=%d: %s", errnocopy, std::strerror(errnocopy));
                return true;
            }
            break;
        }

        return true;
}

bool vidcap_v4l2_driver_interface::v4l2if_mainloop(void)
{
    int count = int_frame_count;
    int errnocopy = 0;

    // Loop while not finished, and original int_frame_count was 0, or counter is not done counting
    // (Having a null finished callback() is an error checked for earlier).
    do
    {
        if (int_frame_count != 0 && count-- <= 0)
        {
            set_terminated(true);
            break;
        }

        while(! isterminated())
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
                if (errnocopy == EINTR) continue;

                LOGGER_2Arg("v4l2if_mainloop: select call failed (-1): errno=%d: %s", errnocopy, std::strerror(errnocopy));
                return false;
            }

            if (0 == r) {
                // this may be a timeout expired
                if (!isterminated()) continue;
                break;
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
    return true;
}

// This version simply ignores system call return values
void vidcap_v4l2_driver_interface::v4l2if_cleanup_stop_capturing(void)
{
        enum v4l2_buf_type type;

        switch (io) {
        case IO_METHOD_READ:
                /* Nothing to do. */
                break;

        case IO_METHOD_MMAP:
        case IO_METHOD_USERPTR:
                type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                v4l2if_xioctl(fd, VIDIOC_STREAMOFF, &type);
                break;
        }
}

bool vidcap_v4l2_driver_interface::v4l2if_stop_capturing(void)
{
    enum v4l2_buf_type type;

    switch (io) {
    case IO_METHOD_READ:
        /* Nothing to do. */
        break;

    case IO_METHOD_MMAP:
    case IO_METHOD_USERPTR:
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (v4l2if_xioctl(fd, VIDIOC_STREAMOFF, &type) == -1)
        {
                return false;
        }
        break;
    }
    return true;
}

bool vidcap_v4l2_driver_interface::v4l2if_start_capturing(void)
{
    unsigned int i;
    enum v4l2_buf_type type;
    int errnocopy = 0;

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
            {
                errnocopy = errno;
                LOGGER_2Arg("v4l2if_start_capturing: ioctl VIDIOC_QBUF failed: errno=%d: %s", errnocopy, std::strerror(errnocopy));
                return false;
            }
        }
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (-1 == v4l2if_xioctl(fd, VIDIOC_STREAMON, &type))
        {
            errnocopy = errno;
            LOGGER_2Arg("v4l2if_start_capturing: ioctl VIDIOC_STREAMON/V4L2_BUF_TYPE_VIDEO_CAPTURE failed: errno=%d: %s", errnocopy, std::strerror(errnocopy));
            return false;
        }
        break;

    case IO_METHOD_USERPTR:
        // PLEASE NOTE:  The streaming method IO_METHOD_MMAP is the only one actually
        // tested.  Please do not use IO_METHOD_USERPTR or IO_METHOD_READ until they are tested.

        for (i = 0; i < numbufs; ++i)
        {
            struct v4l2_buffer buf;

            CLEAR(buf);
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_USERPTR;
            buf.index = i;
            buf.m.userptr = (unsigned long)buffers[i].start;
            buf.length = buffers[i].length;

            if (-1 == v4l2if_xioctl(fd, VIDIOC_QBUF, &buf))
            {
                errnocopy = errno;
                LOGGER_2Arg("v4l2if_start_capturing: ioctl VIDIOC_QBUF failed: errno=%d: %s", errnocopy, std::strerror(errnocopy));
                return false;
            }
        }
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (v4l2if_xioctl(fd, VIDIOC_STREAMON, &type) == -1)
        {
            errnocopy = errno;
            LOGGER_2Arg("v4l2if_start_capturing: ioctl VIDIOC_STREAMON/V4L2_BUF_TYPE_VIDEO_CAPTURE failed: errno=%d: %s", errnocopy, std::strerror(errnocopy));
            return false;
        }
        break;
    }
    return true;
}

// PLEASE NOTE:  The streaming method IO_METHOD_MMAP is the only one actually
// tested.  Please do not use IO_METHOD_USERPTR or IO_METHOD_READ until they are tested.
bool vidcap_v4l2_driver_interface::v4l2if_uninit_device(void)
{
    unsigned int i;
    bool ret = true;

    switch (io) {
    case IO_METHOD_READ:
        free(buffers[0].start);
        break;

    case IO_METHOD_MMAP:
        for (i = 0; i < numbufs; ++i)
        {
            if (munmap(buffers[i].start, buffers[i].length) == -1)
            {
                ret = false;
            }
        }
        break;

    case IO_METHOD_USERPTR:
        for (i = 0; i < numbufs; ++i)
                free(buffers[i].start);
        break;
    }

    free(buffers);
    return ret;
}

// Return values from system calls are ignored here
void vidcap_v4l2_driver_interface::v4l2if_cleanup_uninit_device(void)
{
    unsigned int i;

    switch (io) {
    case IO_METHOD_READ:
        free(buffers[0].start);
        break;

    case IO_METHOD_MMAP:
        for (i = 0; i < numbufs; ++i)
            munmap(buffers[i].start, buffers[i].length);
        break;

    case IO_METHOD_USERPTR:
        for (i = 0; i < numbufs; ++i)
            free(buffers[i].start);
        break;
    }

    free(buffers);
}

bool vidcap_v4l2_driver_interface::v4l2if_init_read(unsigned int buffer_size)
{
    int errnocopy = 0;

    buffers = static_cast<buffer *>( calloc(1, sizeof(*buffers)));
    errnocopy = errno;

    if (!buffers) {
        LOGGER_3Arg("v4l2if_init_read: Allocating (calloc) %d bytes failed: errno=%d: %s", buffer_size, errnocopy, std::strerror(errnocopy));
        return false;
    }

    buffers[0].length = buffer_size;
    buffers[0].start = malloc(buffer_size);
    errnocopy = errno;

    if (!buffers[0].start) {
        LOGGER_3Arg("v4l2if_init_read: Allocating (malloc) %d bytes failed: errno=%d: %s", buffer_size, errnocopy, std::strerror(errnocopy));
        return false;
    }
    return true;
}

bool vidcap_v4l2_driver_interface::v4l2if_init_mmap(void)
{
    struct v4l2_requestbuffers req;
    int errnocopy = 0;

    CLEAR(req);

    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (v4l2if_xioctl(fd, VIDIOC_REQBUFS, &req) == -1) {
        if (EINVAL == errno) {
            LOGGER_1Arg("v4l2if_init_mmap(): %s does not support memory mapping", dev_name.c_str());
        }
        return false;
    }

    if (req.count < 2)
    {
        LOGGER_1Arg("v4l2if_init_mmap(): Insufficient buffer memory on %s", dev_name.c_str());
        return false;
    }

    buffers = static_cast<buffer *>(calloc(req.count, sizeof(*buffers)));
    errnocopy = errno;

    if (!buffers) {
        LOGGER_3Arg("v4l2if_init_mmap: Allocating (calloc) %d bytes failed: errno=%d: %s", (req.count*sizeof(*buffers)), errnocopy, std::strerror(errnocopy));
        return false;
    }

    for (numbufs = 0; numbufs < req.count; ++numbufs) {
            struct v4l2_buffer buf;

            CLEAR(buf);

            buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory      = V4L2_MEMORY_MMAP;
            buf.index       = numbufs;

            if (v4l2if_xioctl(fd, VIDIOC_QUERYBUF, &buf) == -1)
            {
                errnocopy = errno;
                LOGGER_2Arg("v4l2if_init_mmap: ioctl VIDIOC_QUERYBUF failed: errno=%d: %s", errnocopy, std::strerror(errnocopy));
                return false;
            }

            buffers[numbufs].length = buf.length;
            buffers[numbufs].start =
                    mmap(NULL /* start anywhere */,
                          buf.length,
                          PROT_READ | PROT_WRITE /* required */,
                          MAP_SHARED /* recommended */,
                          fd, buf.m.offset);
            errnocopy = errno;
            if (MAP_FAILED == buffers[numbufs].start)
            {
                LOGGER_2Arg("v4l2if_init_mmap: mmap() system call failed: errno=%d: %s", errnocopy, std::strerror(errnocopy));
                return false;
            }
    }
    return true;
}

bool vidcap_v4l2_driver_interface::v4l2if_init_userp(unsigned int buffer_size)
{
    struct v4l2_requestbuffers req;
    int errnocopy = 0;

    CLEAR(req);

    req.count  = 4;
    req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_USERPTR;

    if (v4l2if_xioctl(fd, VIDIOC_REQBUFS, &req) == -1)
    {
        errnocopy = errno;
        if (EINVAL == errno) {
            LOGGER_1Arg("v4l2if_init_userp: %s does not support user pointer i/o", dev_name.c_str());
        } else {
            LOGGER_3Arg("v4l2if_init_userp: ioctl VIDIOC_REQBUFS for %s failed: errno=%d: %s", dev_name.c_str(), errnocopy, std::strerror(errnocopy));
        }
        return false;
    }

    buffers = static_cast<buffer *>((calloc(4, sizeof(*buffers))));
    errnocopy = errno;
    if (!buffers)
    {
        LOGGER_3Arg("v4l2if_init_userp: Allocating (calloc) %d bytes failed: errno=%d: %s", (4* sizeof(*buffers)), errnocopy, std::strerror(errnocopy));
        return false;
    }

    for (numbufs = 0; numbufs < 4; ++numbufs)
    {
        buffers[numbufs].length = buffer_size;
        buffers[numbufs].start = malloc(buffer_size);
        errnocopy = errno;
        if (!buffers[numbufs].start)
        {
            LOGGER_3Arg("v4l2if_init_userp: Allocating (malloc) %d bytes failed: errno=%d: %s", buffer_size, errnocopy, std::strerror(errnocopy));
            return false;
        }
    }
    return true;
}

bool vidcap_v4l2_driver_interface::v4l2if_init_device(void)
{
        struct v4l2_capability cap;
        struct v4l2_cropcap cropcap;
        struct v4l2_crop crop;
        struct v4l2_format fmt;
        unsigned int min;
        int errnocopy = 0;

        if (-1 == v4l2if_xioctl(fd, VIDIOC_QUERYCAP, &cap)) {
            errnocopy = errno;
            if (EINVAL == errnocopy) {
                LOGGER_1Arg("v4l2if_init_device: %s is not a V4L2 device", dev_name.c_str());
                return false;
            } else {
                LOGGER_2Arg("v4l2if_init_device: VIDIOC_QUERYCAP ioctl failed: errno=%d: %s", errnocopy, std::strerror(errnocopy));
                return false;
            }
        }

        if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
            LOGGER_1Arg("v4l2if_init_device: %s is not a capture device", dev_name.c_str());
            return false;
        }

        // PLEASE NOTE:  The streaming method IO_METHOD_MMAP is the only one actually
        // tested.  Please do not use IO_METHOD_USERPTR or IO_METHOD_READ until they are tested.

        switch (io) {
        case IO_METHOD_READ:
                if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
                    LOGGER_1Arg("v4l2if_init_device: %s does not support read i/o", dev_name.c_str());
                    return false;
                }
                break;

        case IO_METHOD_MMAP:
        case IO_METHOD_USERPTR:
                if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
                    LOGGER_1Arg("%s does not support streaming i/o", dev_name.c_str());
                    return false;
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
        if (pixel_format == Video::pxl_formats::h264) {
            fmt.fmt.pix.width       = 1920;
            fmt.fmt.pix.height      = 1080;
            fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_H264;
            fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

            if (v4l2if_xioctl(fd, VIDIOC_S_FMT, &fmt) == -1)
            {
                errnocopy = errno;
                // Note VIDIOC_S_FMT may change width and height.
                LOGGER_2Arg("v4l2if_init_device: Setting pixel format to h264 (VIDIOC_S_FMT ioctl) failed: errno=%d: %s", errnocopy, std::strerror(errnocopy));
                return false;
            }
            logger.debug() << "Set video format to (" << fmt.fmt.pix.width << " x " << fmt.fmt.pix.height
                           << "), pixel format is " << Video::vcGlobals::pixel_formats_strings[pixel_format];

        } else if (pixel_format ==  Video::pxl_formats::yuyv) {
            fmt.fmt.pix.width       = 640;
            fmt.fmt.pix.height      = 480;
            fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
            fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

            // Note VIDIOC_S_FMT may change width and height.
            if (v4l2if_xioctl(fd, VIDIOC_S_FMT, &fmt) == -1)
            {
                LOGGER_2Arg("v4l2if_init_device: Setting pixel format to yuyv (VIDIOC_S_FMT ioctl) failed: errno=%d: %s", errnocopy, std::strerror(errnocopy));
                return false;
            }
            logger.debug() << "Set video format to (" << fmt.fmt.pix.width << " x " << fmt.fmt.pix.height
                           << "), pixel format is " << Video::vcGlobals::pixel_formats_strings[pixel_format];
        } else {
            /* Preserve original settings as set by v4l2-ctl for example */
            if (v4l2if_xioctl(fd, VIDIOC_G_FMT, &fmt) == -1)
            {
                LOGGER_2Arg("v4l2if_init_device: Setting pixel format to previous setting (VIDIOC_G_FMT ioctl) failed: errno=%d: %s", errnocopy, std::strerror(errnocopy));
                return false;
            }
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

        LOGGER_1Arg("driver: bytes required: %lu", fmt.fmt.pix.sizeimage);
        LOGGER_1Arg("driver: I/O METHOD: %s", string_io_methods[io]);

        // PLEASE NOTE:  The streaming method IO_METHOD_MMAP is the only one actually
        // tested.  Please do not use IO_METHOD_USERPTR or IO_METHOD_READ until they are tested.

        switch (io) {
        case IO_METHOD_READ:
                if (! v4l2if_init_read(fmt.fmt.pix.sizeimage))
                {
                    errnocopy = errno;
                    LOGGER_3Arg("v4l2if_init_device: v4l2if_init_read (%d bytes) failed: errno=%d: %s",
                                fmt.fmt.pix.sizeimage, errnocopy, std::strerror(errnocopy));
                    return false;
                }
                break;

        case IO_METHOD_MMAP:
                if (! v4l2if_init_mmap())
                {
                    errnocopy = errno;
                    LOGGER_2Arg("v4l2if_init_device: v4l2if_init_mmap() failed: errno=%d: %s", errnocopy, std::strerror(errnocopy));
                    return false;
                }
                break;

        case IO_METHOD_USERPTR:
                if (! v4l2if_init_userp(fmt.fmt.pix.sizeimage))
                {
                    errnocopy = errno;
                    LOGGER_3Arg("v4l2if_init_device: v4l2if_init_userp(%d) failed: errno=%d: %s",
                                fmt.fmt.pix.sizeimage, errnocopy, std::strerror(errnocopy));
                    return false;
                }
                break;
        }

    return true;
}

bool vidcap_v4l2_driver_interface::v4l2if_close_device(void)
{
    if (close(fd) == -1)
    {
        return false;
    }
    fd = -1;
    return true;
}

// Return values from system calls are ignored here
void vidcap_v4l2_driver_interface::v4l2if_cleanup_close_device(void)
{
        if (fd != -1) close(fd);
        fd = -1;
}

bool vidcap_v4l2_driver_interface::v4l2if_open_device(void)
{
    using namespace Util;
    struct stat st;
    int errnocopy;

    if (Utility::trim(dev_name) == "")
    {
        logger.error() << "v4l2if_open_device: empty video device name";
        return false;
    }
    if (stat(dev_name.c_str(), &st) == -1) {
        errnocopy = errno;
        logger.error() << "v4l2if_open_device: Cannot identify device "
                       << dev_name << ": errno=" << errnocopy << ": " << strerror(errnocopy);
        return false;
    }

    if (!S_ISCHR(st.st_mode)) {
        logger.error() << "v4l2if_open_device: " << dev_name << " is not a device";
        return false;
    }

    fd = open(dev_name.c_str(), O_RDWR /* required */ | O_NONBLOCK, 0);
    errnocopy = errno;

    if (-1 == fd) {
        logger.error() << "v4l2if_open_device: Cannot open "
                       << dev_name << ": errno=" << errnocopy << ": " << strerror(errnocopy);
        return false;
    }
    logger.info() << "Device " << dev_name;
    return true;
}

