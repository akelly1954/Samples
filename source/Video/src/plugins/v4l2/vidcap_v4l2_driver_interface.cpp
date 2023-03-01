
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

#include <plugins/vidcap_v4l2_driver_interface.hpp>
#include <vidcap_capture_thread.hpp>
#include <vidcap_raw_queue_thread.hpp>
#include <MainLogger.hpp>
#include <Utility.hpp>
#include <thread>
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
#include <exception>
#include <linux/videodev2.h>

using namespace VideoCapture;

// PLEASE NOTE:  The streaming method IO_METHOD_MMAP is the only one actually
// tested.  Please do not use IO_METHOD_USERPTR or IO_METHOD_READ until they are tested.

vidcap_v4l2_driver_interface::vidcap_v4l2_driver_interface()
{
    set_terminated(false);
}

// Update the gloabal pixel_format map<> with this plugin's available pixel formats.
bool vidcap_v4l2_driver_interface::probe_pixel_format_caps(std::map<std::string,std::string>& pixformat_map)
{
    pixformat_map["h264"] = "H264: H264 with start codes (from plugin)";
    pixformat_map["yuyv"] = "YUYV: (alias YUV 4:2:2): Packed format with ½ horizontal chroma resolution (from plugin)";
    return true;
}

void vidcap_v4l2_driver_interface::initialize()
{
    // This is not done in the constructor since the logger is not set up
    // yet while the plugin factory is being created.
    loggerp = Util::UtilLogger::getLoggerPtr();
    if (!loggerp)
    {
        throw std::runtime_error("vidcap_v4l2_driver_interface: ERROR: found NULL logger pointer.");
    }
    loggerp->debug() << "vidcap_v4l2_driver_interface: Initialized.";

    std::string actual_process = video_plugin_base::set_popen_process_string();
    if (actual_process == "")
    {
        throw std::runtime_error("vidcap_v4l2_driver_interface: base popen() process string is empty.");
    }
    loggerp->debug() << "vidcap_v4l2_driver_interface: Process popen() string is:  " << actual_process;
}

void vidcap_v4l2_driver_interface::run()
{
    if (!loggerp)
    {
        throw std::runtime_error("vidcap_v4l2_driver_interface::run() ERROR: found NULL logger pointer.");
    }
    loggerp->debug() << "vidcap_v4l2_driver_interface: Running.";

    try {
        if (isterminated() || !v4l2if_open_device())
        {
            if (!isterminated())
            {
                loggerp->error() << "vidcap_v4l2_driver_interface::run() - v4l2if_open_device() FAILED. Terminating...";
                set_error_terminated(true);
            }
        }

        if (isterminated() || !v4l2if_init_device())
        {
            if (!isterminated())
            {
                loggerp->error() << "vidcap_v4l2_driver_interface::run() - v4l2if_init_device() FAILED. Terminating...";
                set_error_terminated(true);
            }
            loggerp->debug() << "vidcap_v4l2_driver_interface::run() - v4l2if_init_device() SUCCEEDED.";
        }

        if (isterminated() || !v4l2if_start_capturing())
        {
            if (!isterminated())
            {
                loggerp->error() << "vidcap_v4l2_driver_interface::run() - v4l2if_start_capturing() FAILED. Terminating...";
                set_error_terminated(true);
                loggerp->debug() << "vidcap_v4l2_driver_interface::run() - v4l2if_start_capturing() SUCCEEDED.";
            }
        }

        if (isterminated() || !v4l2if_mainloop())
        {
            if (!isterminated())
            {
                loggerp->error() << "vidcap_v4l2_driver_interface::run() - v4l2if_mainloop() FAILED. Terminating...";
                set_error_terminated(true);
            }
            loggerp->debug() << "vidcap_v4l2_driver_interface::run() - v4l2if_mainloop() SUCCEEDED.";
        }

        v4l2if_stop_capturing();
        v4l2if_uninit_device();
        v4l2if_close_device();

        if (iserror_terminated())
        {
            std::string msg =
                    "vidcap_v4l2_driver_interface:\n\n"
                    "        ****************************************\n"
                    "        ***** ERROR TERMINATION REQUESTED. *****\n"
                    "        ****************************************\n";
                    loggerp->info() << msg;
        }
        else
        {
            loggerp->info() << "vidcap_v4l2_driver_interface: NORMAL TERMINATION REQUESTED";
            std::cerr << "NORMAL TERMINATION..." << std::endl;
        }
    }
    catch (std::exception &exp)
    {
        loggerp->error()
              << "vidcap_v4l2_driver_interface::run(): Got exception running the video capture: "
              << exp.what() << ". Aborting...";
    } catch (...)
    {
        loggerp->error()
              << "vidcap_v4l2_driver_interface::run(): General exception occurred running the video capture. Aborting...";
    }
}

void vidcap_v4l2_driver_interface::v4l2if_errno_exit(const char *s, int errnocopy)
{
    std::string msg = std::string(s) + " error, errno=" + std::to_string(errnocopy) + ": "
                      + const_cast<const char *>(strerror(errnocopy)) + " ...aborting.";
    loggerp->error() << msg;
    loggerp->error() << "vidcap_v4l2_driver_interface:\n\n"
                      "        ****************************************\n"
                      "        ***** ERROR TERMINATION REQUESTED. *****\n"
                      "        ****************************************\n";
    std::cerr << "\nException thrown: " << msg << std::endl;

    if (Video::vcGlobals::profiling_enabled)
    {
        loggerp->debug() << "vidcap_v4l2_driver_interface::run() - terminating the video_profiler thread.";
        VideoCapture::vidcap_profiler::set_terminated(true);
    }

    v4l2if_cleanup_stop_capturing();
    v4l2if_cleanup_uninit_device();
    v4l2if_cleanup_close_device();
    set_error_terminated(true);
    throw std::runtime_error(msg);
}

void vidcap_v4l2_driver_interface::v4l2if_error_exit(const char *s)
{
    std::string msg = std::string("vidcap_v4l2_driver_interface: ERROR TERMINATION REQUESTED: ") + s;
    loggerp->error() << msg;

    loggerp->error() << "vidcap_v4l2_driver_interface:\n\n"
                      "        ****************************************\n"
                      "        ***** ERROR TERMINATION REQUESTED. *****\n"
                      "        ****************************************\n";
    std::cerr << "\nException thrown: " << msg << std::endl;

    if (Video::vcGlobals::profiling_enabled)
    {
        loggerp->debug() << "vidcap_v4l2_driver_interface::run() - terminating the video_profiler thread.";
        VideoCapture::vidcap_profiler::set_terminated(true);
    }

    v4l2if_cleanup_stop_capturing();
    v4l2if_cleanup_uninit_device();
    v4l2if_cleanup_close_device();
    set_error_terminated(true);
    throw std::runtime_error(msg);
}

void vidcap_v4l2_driver_interface::v4l2if_exit(const char *s)
{
    loggerp->info() << "vidcap_v4l2_driver_interface: NORMAL TERMINATION REQUESTED";
    std::cerr << "NORMAL TERMINATION..." << std::endl;
    set_terminated(true);
}

int vidcap_v4l2_driver_interface::v4l2if_xioctl(int fh, int request, void *arg)
{
        int r;

        do {
                r = ioctl(fh, request, arg);
        } while (r == -1 && errno == EINTR);

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

    add_buffer_to_raw_queue(p, size);
}

bool vidcap_v4l2_driver_interface::v4l2if_read_frame(void)
{
    struct v4l2_buffer buf;
    int errnocopy = 0;
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
                return true;

            case EIO:
                // Original source says:
                /* Could ignore EIO, see spec. */
                // I don't think so.
                v4l2if_errno_exit("v4l2if_read_frame (IO_METHOD_READ): read() call (EIO)", errnocopy);
                set_error_terminated(true);
                return false;
            default:
                v4l2if_errno_exit("v4l2if_read_frame (IO_METHOD_READ): read() call", errnocopy);
                set_error_terminated(true);
                return false;
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
                switch (errnocopy) {
                case EAGAIN:
                    return true;

                case EIO:
                    // Original source says:
                    /* Could ignore EIO, see spec. */
                    // I don't think so.
                    v4l2if_errno_exit("v4l2if_read_frame: (io = IO_METHOD_MMAP) ioctl() VIDIOC_DQBUF call (EIO)", errnocopy);
                    set_error_terminated(true);
                    return false;

                default:
                    v4l2if_errno_exit("v4l2if_read_frame (MMAP): ioctl() VIDIOC_DQBUF call", errnocopy);
                    set_error_terminated(true);
                    return false;
                }
            }

            assert(buf.index < numbufs);

            v4l2if_process_image(buffers[buf.index].start, buf.bytesused);

            if (v4l2if_xioctl(fd, VIDIOC_QBUF, &buf) == -1)
            {
                errnocopy = errno;
                v4l2if_errno_exit("v4l2if_read_frame (MMAP): ioctl() VIDIOC_DQBUF call", errnocopy);
                set_error_terminated(true);
                return false;
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
                    // Original source says:
                    /* Could ignore EIO, see spec. */
                    // I don't think so.
                    v4l2if_errno_exit("v4l2if_read_frame (USERPTR): ioctl() V4L2_BUF_TYPE_VIDEO_CAPTURE/V4L2_MEMORY_USERPTR call", errnocopy);
                    set_error_terminated(true);
                    return false;

                default:
                    v4l2if_errno_exit("v4l2if_read_frame (USERPTR): ioctl() VIDIOC_DQBUF call", errnocopy);
                    set_error_terminated(true);
                    return false;
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
                v4l2if_errno_exit("v4l2if_read_frame (USERPTR): ioctl() VIDIOC_DQBUF call", errnocopy);
                set_error_terminated(true);
                return false;
            }
            break;
        }

        return true;
}

bool vidcap_v4l2_driver_interface::v4l2if_mainloop(void)
{
    int errnocopy = 0;

    if (ispaused()) loggerp->debug() << "vidcap_v4l2_driver_interface::v4l2if_mainloop: Waiting for RESUME";
    while(ispaused())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // TODO: Improve mechanism so that framecount changes past this point
    // actually take effect.
    static int count = Video::vcGlobals::framecount;
    loggerp->debug() << "vidcap_v4l2_driver_interface::v4l2if_mainloop: Frame count is " << count;


    // Loop while not finished, and original int_frame_count was 0, or counter is not done counting
    // (Having a null finished callback() is an error checked for earlier).
    do
    {
        if (Video::vcGlobals::framecount != 0 && count-- <= 0)
        {
            // count+1 below because of count-- above
            loggerp->debug() << "In v4l2if_mainloop: end of loop, count = " << count+1;
            set_terminated(true);
            break;
        }

        long long lret = 0;
        if (!isterminated() && Video::vcGlobals::profiling_enabled)
        {
            // /* For debug: */ loggerp->debug() << "From v4l2if_mainloop: Got new frame, count = " << count;
            lret = increment_one_frame();
            // /* For debug: */ loggerp->debug() << "From v4l2if_mainloop: profiler reports count = " << lret;
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

                v4l2if_errno_exit("v4l2if_mainloop: select call failed (-1)", errnocopy);
                set_error_terminated(true);
                return false;
            }

            if (0 == r) {
                // this may be a timeout expired
                if (!isterminated()) continue;
                break;
            }

            if (v4l2if_read_frame())
                    break;
            /* EAGAIN or EIO- continue select loop. */
        }
    } while (! isterminated());

    if (isterminated())
    {
        if (iserror_terminated())
        {
            loggerp->info() << "v4l2if_mainloop: ERROR:  CAPTURE TERMINATION REQUESTED.";
        }
        else
        {
            loggerp->info() << "v4l2if_mainloop: CAPTURE TERMINATION REQUESTED.";
        }
        return false;
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
                v4l2if_errno_exit("v4l2if_start_capturing: ioctl VIDIOC_QBUF", errnocopy);
                return false;   // Never gets here
            }
        }
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (-1 == v4l2if_xioctl(fd, VIDIOC_STREAMON, &type))
        {
            errnocopy = errno;
            v4l2if_errno_exit("v4l2if_start_capturing (MMAP): ioctl VIDIOC_STREAMON/V4L2_BUF_TYPE_VIDEO_CAPTURE", errnocopy);
            return false;  // Never gets here
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
                v4l2if_errno_exit("v4l2if_start_capturing: ioctl VIDIOC_QBUF", errnocopy);
                return false;  // Never gets here
            }
        }
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (v4l2if_xioctl(fd, VIDIOC_STREAMON, &type) == -1)
        {
            errnocopy = errno;
            v4l2if_errno_exit("v4l2if_start_capturing(USRPTR): ioctl VIDIOC_STREAMON/V4L2_BUF_TYPE_VIDEO_CAPTURE", errnocopy);
            return false;  // Never gets here
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
        std::stringstream ostr;
        ostr << "v4l2if_init_read: Allocating (calloc) " <<  buffer_size << " bytes";
        v4l2if_errno_exit(ostr.str().c_str(), errnocopy);
        set_error_terminated(true);
        return false;
    }

    buffers[0].length = buffer_size;
    buffers[0].start = malloc(buffer_size);
    errnocopy = errno;

    if (!buffers[0].start) {
        std::stringstream ostr;
        ostr << "v4l2if_init_read: Allocating (malloc) " <<  buffer_size << " bytes";
        v4l2if_errno_exit(ostr.str().c_str(), errnocopy);
        set_error_terminated(true);
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
        if (EINVAL == errno)
        {
            std::stringstream ostr;
            ostr << "v4l2if_init_mmap(): " << Video::vcGlobals::str_dev_name << " does not support memory mapping";
            v4l2if_errno_exit(ostr.str().c_str(), EINVAL);
        }
        return false;
    }

    if (req.count < 2)
    {
        std::stringstream ostr;
        ostr << "v4l2if_init_mmap(): Insufficient buffer memory on " << Video::vcGlobals::str_dev_name;
        v4l2if_error_exit(ostr.str().c_str());
        return false;
    }

    buffers = static_cast<buffer *>(calloc(req.count, sizeof(*buffers)));
    errnocopy = errno;

    if (!buffers) {
        std::ostringstream ostr;
        ostr << "v4l2if_init_mmap: Allocating (calloc) " << (req.count*sizeof(*buffers));
        v4l2if_errno_exit(ostr.str().c_str(), errnocopy);
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
                v4l2if_errno_exit("v4l2if_init_mmap: ioctl VIDIOC_QUERYBUF", errnocopy);
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
                v4l2if_errno_exit("v4l2if_init_mmap: mmap() system call", errnocopy);
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
        if (errno == EINVAL) {
            std::stringstream ostr;
            ostr << "v4l2if_init_userp: " << Video::vcGlobals::str_dev_name << " does not support memory mapping";
            v4l2if_errno_exit(ostr.str().c_str(), errnocopy);
        } else {
            std::stringstream ostr;
            ostr << "v4l2if_init_userp: ioctl VIDIOC_REQBUFS for " << Video::vcGlobals::str_dev_name;
            v4l2if_errno_exit(ostr.str().c_str(), errnocopy);
        }
        return false;
    }

    buffers = static_cast<buffer *>((calloc(4, sizeof(*buffers))));
    errnocopy = errno;
    if (!buffers)
    {
        std::stringstream ostr;
        ostr << "v4l2if_init_userp: Allocating (calloc) " << (4* sizeof(*buffers));
        v4l2if_errno_exit(ostr.str().c_str(), errnocopy);
        return false;
    }

    for (numbufs = 0; numbufs < 4; ++numbufs)
    {
        buffers[numbufs].length = buffer_size;
        buffers[numbufs].start = malloc(buffer_size);
        errnocopy = errno;
        if (!buffers[numbufs].start)
        {
            std::stringstream ostr;
            ostr << "v4l2if_init_userp: Allocating (malloc) " << (4* sizeof(*buffers));
            v4l2if_errno_exit(ostr.str().c_str(), errnocopy);
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
            if (errnocopy == EINVAL) {
                std::stringstream ostr;
                ostr << "v4l2if_init_device: " << Video::vcGlobals::str_dev_name << " is not a V4L2 device";
                v4l2if_errno_exit(ostr.str().c_str(), errnocopy);
                return false;
            } else {
                std::stringstream ostr;
                ostr << "v4l2if_init_device: VIDIOC_QUERYCAP ioctl";
                v4l2if_errno_exit(ostr.str().c_str(), errnocopy);
                return false;
            }
        }

        if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
            std::stringstream ostr;
            ostr << "v4l2if_init_device: " << Video::vcGlobals::str_dev_name << " is not a capture device";
            v4l2if_errno_exit(ostr.str().c_str(), errnocopy);
            return false;
        }

        // PLEASE NOTE:  The streaming method IO_METHOD_MMAP is the only one actually
        // tested.  Please do not use IO_METHOD_USERPTR or IO_METHOD_READ until they are tested.

        switch (io) {
        case IO_METHOD_READ:
            if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
                std::stringstream ostr;
                ostr << "v4l2if_init_device: " << Video::vcGlobals::str_dev_name << " does not support read i/o";
                v4l2if_errno_exit(ostr.str().c_str(), errnocopy);
                return false;
            }
            break;

        case IO_METHOD_MMAP:
        case IO_METHOD_USERPTR:
            if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
                std::stringstream ostr;
                ostr << "v4l2if_init_device: " << Video::vcGlobals::str_dev_name << " does not support streaming i/o";
                v4l2if_error_exit(ostr.str().c_str());
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
        if (Video::vcGlobals::pixel_fmt == Video::pxl_formats::h264) {
            fmt.fmt.pix.width       = 1920;
            fmt.fmt.pix.height      = 1080;
            fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_H264;
            fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

            if (v4l2if_xioctl(fd, VIDIOC_S_FMT, &fmt) == -1)
            {
                errnocopy = errno;
                if (errnocopy == EIO)
                {
                    loggerp->debug() << "v4l2if_init_device: Got EIO setting pixel format to h264 (VIDIOC_S_FMT ioctl)";
                }
                else
                {
                    std::stringstream ostr;
                    ostr << "v4l2if_init_device: " << Video::vcGlobals::str_dev_name << " is not a capture device";
                    v4l2if_errno_exit("v4l2if_init_device: Setting pixel format to h264 (VIDIOC_S_FMT ioctl)", errnocopy);
                    // Note VIDIOC_S_FMT may change width and height.
                    return false;
                }
            }
            loggerp->debug() << "Set video format to (" << fmt.fmt.pix.width << " x " << fmt.fmt.pix.height
                           << "), pixel format is " << Video::vcGlobals::pixel_formats_strings[Video::vcGlobals::pixel_fmt];

        } else if (Video::vcGlobals::pixel_fmt ==  Video::pxl_formats::yuyv) {
            fmt.fmt.pix.width       = 640;
            fmt.fmt.pix.height      = 480;
            fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
            fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

            // Note VIDIOC_S_FMT may change width and height.
            if (v4l2if_xioctl(fd, VIDIOC_S_FMT, &fmt) == -1)
            {
                if (errnocopy == EIO)
                {
                    loggerp->debug() << "v4l2if_init_device: Got EIO setting pixel format to yuyv (VIDIOC_S_FMT ioctl)";
                }
                else
                {
                    v4l2if_errno_exit("v4l2if_init_device: Setting pixel format to yuyv (VIDIOC_S_FMT ioctl)", errnocopy);
                    return false;
                }
            }
            loggerp->debug() << "Set video format to (" << fmt.fmt.pix.width << " x " << fmt.fmt.pix.height
                           << "), pixel format is " << Video::vcGlobals::pixel_formats_strings[Video::vcGlobals::pixel_fmt];
        } else {
            /* Preserve original settings as set by v4l2-ctl for example */
            if (v4l2if_xioctl(fd, VIDIOC_G_FMT, &fmt) == -1)
            {
                errnocopy = errno;
                v4l2if_errno_exit("v4l2if_init_device: Setting pixel format to previous setting (VIDIOC_G_FMT ioctl)", errnocopy);
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
            {
                std::ostringstream ostr;
                ostr << "v4l2 driver: frame: " << fmt.fmt.pix.width << " x " << fmt.fmt.pix.height;
                std::string s = ostr.str();
                std::cerr << s << std::endl;
                loggerp->debug() << s;
            }

            std::string bfp;
            switch(fmt.fmt.pix.pixelformat)
            {
            case V4L2_PIX_FMT_YUYV:
                bfp = "YUYV: aka \"YUV 4:2:2\": Packed format with ½ horizontal chroma resolution";
                break;
            case V4L2_PIX_FMT_H264:
                bfp = "H264: H264 with start codes";
                break;
            default:
                bfp = "Unknown/unsupported pixel format: See </usr/include/linux/videodev2.h> for all known format encoding macros";
                break;
            }
            {
                std::ostringstream ostr;
                ostr << "v4l2 driver: pixel format set to 0x"
                     << std::hex << fmt.fmt.pix.pixelformat << " - " << bfp;
                std::string s = ostr.str();
                std::cerr << s << std::endl;
                loggerp->debug() << s;
            }
        }

        loggerp->debug() << "v4l2 driver: bytes required: " << fmt.fmt.pix.sizeimage;
        loggerp->debug() << "v4l2 driver: I/O METHOD: " << string_io_methods[io];

        // PLEASE NOTE:  The streaming method IO_METHOD_MMAP is the only one actually
        // tested.  Please do not use IO_METHOD_USERPTR or IO_METHOD_READ until they are tested.

        switch (io) {
        case IO_METHOD_READ:
                if (! v4l2if_init_read(fmt.fmt.pix.sizeimage))
                {
                    std::ostringstream ostr;
                    ostr << "v4l2if_init_device: v4l2if_init_read (" << fmt.fmt.pix.sizeimage << " bytes) failed";
                    v4l2if_error_exit(ostr.str().c_str());
                    return false;
                }
                break;

        case IO_METHOD_MMAP:
                if (! v4l2if_init_mmap())
                {
                    v4l2if_error_exit("v4l2if_init_device: v4l2if_init_mmap() failed");
                    return false;
                }
                break;

        case IO_METHOD_USERPTR:
                if (! v4l2if_init_userp(fmt.fmt.pix.sizeimage))
                {
                    v4l2if_error_exit("v4l2if_init_device: v4l2if_init_userp() failed");
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

    if (Utility::trim(Video::vcGlobals::str_dev_name) == "")
    {
        loggerp->error() << "v4l2if_open_device: empty video device name";
        return false;
    }
    if (stat(Video::vcGlobals::str_dev_name.c_str(), &st) == -1) {
        errnocopy = errno;
        loggerp->error() << "v4l2if_open_device: Cannot identify device "
                       << Video::vcGlobals::str_dev_name << ": errno=" << errnocopy << ": " << strerror(errnocopy);
        return false;
    }

    if (!S_ISCHR(st.st_mode)) {
        loggerp->error() << "v4l2if_open_device: " << Video::vcGlobals::str_dev_name << " is not a device";
        return false;
    }

    fd = open(Video::vcGlobals::str_dev_name.c_str(), O_RDWR /* required */ | O_NONBLOCK, 0);
    errnocopy = errno;

    if (-1 == fd) {
        loggerp->error() << "v4l2if_open_device: Cannot open "
                       << Video::vcGlobals::str_dev_name << ": errno=" << errnocopy << ": " << strerror(errnocopy);
        return false;
    }
    loggerp->info() << "Device " << Video::vcGlobals::str_dev_name;
    return true;
}


