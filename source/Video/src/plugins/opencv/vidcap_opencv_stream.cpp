
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
#include <thread>
#include <Utility.hpp>
#include <vidcap_opencv_stream.hpp>
#include <vidcap_raw_queue_thread.hpp>
#include <LoggerCpp/LoggerCpp.h>

using namespace VideoCapture;

vidcap_opencv_stream::vidcap_opencv_stream(Log::Logger lg)
    :
        logger(lg)   //  ,
        //  m_cap(Video::vcGlobals::str_dev_name)
{
    set_terminated(false);
}

void vidcap_opencv_stream::initialize()
{
    int deviceID = 0;             // 0 = open default camera
    int apiID = cv::CAP_ANY;      // 0 = autodetect default API
    m_cap.open(deviceID, apiID);

    if (!m_cap.isOpened())
    {
        logger.error() << "vidcap_opencv_stream::initialize() - Could not open source at " << Video::vcGlobals::str_dev_name;
        set_error_terminated(true);
    }

#if 0
    // cv::Mat M(102,201,CV_8UC1);
    cv::Mat M(1280,1024,CV_8UC1);
    int rows = M.rows;
    int cols = M.cols;

    std::cerr << rows<<" " << cols << std::endl;

    cv::Size sz = M.size();
    rows = sz.height;
    cols = sz.width;

    std::cerr << rows << " " << cols << std::endl;
    std::cerr << sz << std::endl;

    :: sleep(10);
#endif // 0

    // m_cap.set(cv::CAP_PROP_FRAME_WIDTH, 800);//Setting the width of the video
    // m_cap.set(cv::CAP_PROP_FRAME_HEIGHT, 600);//Setting the height of the video//

    std::stringstream ostr;
    ostr << "Video " << Video::vcGlobals::str_dev_name <<
            ": width=" << m_cap.get(cv::CAP_PROP_FRAME_WIDTH) <<
            ", height=" << m_cap.get(cv::CAP_PROP_FRAME_HEIGHT) <<
            ", nframes=" << m_cap.get(cv::CAP_PROP_FRAME_COUNT);
    std::string str = ostr.str();

    std::cerr << str << std::endl;
    logger.info() << str;

    cv::namedWindow("frame",1);
    for(;;)
    {
        cv::Mat frame;
        m_cap.set(cv::CAP_PROP_FRAME_WIDTH, 800);//Setting the width of the video
        m_cap.set(cv::CAP_PROP_FRAME_HEIGHT, 600);//Setting the height of the video//

        m_cap >> frame;

        cv::Size sz = frame.size();
        int rows = sz.height;
        int cols = sz.width;

        std::cerr << rows << " " << cols << std::endl;
        std::cerr << sz << std::endl;



        cv::imshow("frame", frame);
        if(cv::waitKey(30) >= 0) break;
    }

}

void vidcap_opencv_stream::run()
{
    try {
        if (isterminated() || !opencv_open_device())
        {
            if (!isterminated())
            {
                logger.error() << "vidcap_opencv_stream::run() - opencv_open_device() FAILED. Terminating...";
                set_error_terminated(true);
            }
        }

        if (isterminated() || !opencv_init_device())
        {
            if (!isterminated())
            {
                logger.error() << "vidcap_opencv_stream::run() - opencv_init_device() FAILED. Terminating...";
                set_error_terminated(true);
            }
        }

        if (isterminated() || !opencv_start_capturing())
        {
            if (!isterminated())
            {
                logger.error() << "vidcap_opencv_stream::run() - opencv_start_capturing() FAILED. Terminating...";
                set_error_terminated(true);
            }
        }

        if (Video::vcGlobals::profiling_enabled)
        {
            logger.debug() << "vidcap_opencv_stream::run() - kick-starting the video_profiler operations.";
            VideoCapture::vidcap_profiler::s_condvar.send_ready(0, Util::condition_data<int>::NotifyEnum::All);
        }

        if (isterminated() || !opencv_mainloop())
        {
            if (!isterminated())
            {
                logger.error() << "vidcap_opencv_stream::run() - opencv_mainloop() FAILED. Terminating...";
                set_error_terminated(true);
            }
        }

        opencv_stop_capturing();
        opencv_uninit_device();
        opencv_close_device();

        if (Video::vcGlobals::profiling_enabled)
        {
            logger.debug() << "vidcap_opencv_stream::run() - terminating the video_profiler thread.";
            VideoCapture::vidcap_profiler::set_terminated(true);
        }

        if (iserror_terminated())
        {
            std::string msg =
                    "vidcap_opencv_stream:\n\n"
                    "        ****************************************\n"
                    "        ***** ERROR TERMINATION REQUESTED. *****\n"
                    "        ****************************************\n";

                    logger.info() << msg;
        }
        else
        {
            logger.info() << "vidcap_opencv_stream: NORMAL TERMINATION REQUESTED";
            std::cerr << "NORMAL TERMINATION..." << std::endl;
        }
    }
    catch (std::exception &exp)
    {
        logger.error()
              << "vidcap_opencv_stream::run(): Got exception running the video capture: "
              << exp.what() << ". Aborting...";
    } catch (...)
    {
        logger.error()
              << "vidcap_opencv_stream::run(): General exception occurred running the video capture. Aborting...";
    }
    // Let things calm down before disappearing...
    std::this_thread::sleep_for(std::chrono::milliseconds(1800));
}

void vidcap_opencv_stream::opencv_errno_exit(const char *s, int errnocopy)
{
    std::string msg = std::string(s) + " error, errno=" + std::to_string(errnocopy) + ": "
                      + const_cast<const char *>(strerror(errnocopy)) + " ...aborting.";
    logger.error() << msg;
    logger.error() << "vidcap_opencv_stream:\n\n"
                      "        ****************************************\n"
                      "        ***** ERROR TERMINATION REQUESTED. *****\n"
                      "        ****************************************\n";
    std::cerr << "\nException thrown: " << msg << std::endl;

    if (Video::vcGlobals::profiling_enabled)
    {
        logger.debug() << "vidcap_opencv_stream::run() - terminating the video_profiler thread.";
        VideoCapture::vidcap_profiler::set_terminated(true);
    }

    opencv_cleanup_stop_capturing();
    opencv_cleanup_uninit_device();
    opencv_cleanup_close_device();
    set_error_terminated(true);
    throw std::runtime_error(msg);
}

void vidcap_opencv_stream::opencv_error_exit(const char *s)
{
    std::string msg = std::string("vidcap_opencv_stream: ERROR TERMINATION REQUESTED: ") + s;
    logger.error() << msg;

    logger.error() << "vidcap_opencv_stream:\n\n"
                      "        ****************************************\n"
                      "        ***** ERROR TERMINATION REQUESTED. *****\n"
                      "        ****************************************\n";
    std::cerr << "\nException thrown: " << msg << std::endl;

    if (Video::vcGlobals::profiling_enabled)
    {
        logger.debug() << "vidcap_opencv_stream::run() - terminating the video_profiler thread.";
        VideoCapture::vidcap_profiler::set_terminated(true);
    }

    opencv_cleanup_stop_capturing();
    opencv_cleanup_uninit_device();
    opencv_cleanup_close_device();
    set_error_terminated(true);
    throw std::runtime_error(msg);
}

void vidcap_opencv_stream::opencv_exit(const char *s)
{
    logger.info() << "vidcap_opencv_stream: NORMAL TERMINATION REQUESTED";
    std::cerr << "NORMAL TERMINATION..." << std::endl;
    set_terminated(true);
}

int vidcap_opencv_stream::opencv_xioctl(int fh, int request, void *arg)
{
        int r;

        do {
                r = ioctl(fh, request, arg);
        } while (r == -1 && errno == EINTR);

        return r;
}

void vidcap_opencv_stream::opencv_process_image(void *p, int size)
{
    if (ispaused())
    {
        // if we're paused, continue the frame capture
        // but don't really do anything with it.
        return;
    }

    video_capture_queue::add_buffer_to_raw_queue(p, size);
}

bool vidcap_opencv_stream::opencv_read_frame(void)
{
    // struct opencv_buffer buf;
    int errnocopy = 0;
        return true;
}

bool vidcap_opencv_stream::opencv_mainloop(void)
{
    int count = Video::vcGlobals::framecount;
    int errnocopy = 0;
    return true;
}

// This version simply ignores system call return values
void vidcap_opencv_stream::opencv_cleanup_stop_capturing(void)
{

}

bool vidcap_opencv_stream::opencv_stop_capturing(void)
{
    return true;
}

bool vidcap_opencv_stream::opencv_start_capturing(void)
{
    unsigned int i;
    int errnocopy = 0;

    return true;
}

// PLEASE NOTE:  The streaming method IO_METHOD_MMAP is the only one actually
// tested.  Please do not use IO_METHOD_USERPTR or IO_METHOD_READ until they are tested.
bool vidcap_opencv_stream::opencv_uninit_device(void)
{
    return true;
}

// Return values from system calls are ignored here
void vidcap_opencv_stream::opencv_cleanup_uninit_device(void)
{
    unsigned int i;
}

bool vidcap_opencv_stream::opencv_init_read(unsigned int buffer_size)
{
    int errnocopy = 0;
    return true;
}

bool vidcap_opencv_stream::opencv_init_mmap(void)
{
    int errnocopy = 0;
    return true;
}

bool vidcap_opencv_stream::opencv_init_userp(unsigned int buffer_size)
{
    int errnocopy = 0;
    return true;
}

bool vidcap_opencv_stream::opencv_init_device(void)
{
        int errnocopy = 0;

    return true;
}

bool vidcap_opencv_stream::opencv_close_device(void)
{
    if (close(fd) == -1)
    {
        return false;
    }
    fd = -1;
    return true;
}

// Return values from system calls are ignored here
void vidcap_opencv_stream::opencv_cleanup_close_device(void)
{
        if (fd != -1) close(fd);
        fd = -1;
}

bool vidcap_opencv_stream::opencv_open_device(void)
{
#if 0
    using namespace Util;
    struct stat st;
    int errnocopy;

    if (Utility::trim(Video::vcGlobals::str_dev_name) == "")
    {
        logger.error() << "opencv_open_device: empty video device name";
        return false;
    }
    if (stat(Video::vcGlobals::str_dev_name.c_str(), &st) == -1) {
        errnocopy = errno;
        logger.error() << "opencv_open_device: Cannot identify device "
                       << Video::vcGlobals::str_dev_name << ": errno=" << errnocopy << ": " << strerror(errnocopy);
        return false;
    }

    if (!S_ISCHR(st.st_mode)) {
        logger.error() << "opencv_open_device: " << Video::vcGlobals::str_dev_name << " is not a device";
        return false;
    }

    fd = open(Video::vcGlobals::str_dev_name.c_str(), O_RDWR /* required */ | O_NONBLOCK, 0);
    errnocopy = errno;

    if (-1 == fd) {
        logger.error() << "opencv_open_device: Cannot open "
                       << Video::vcGlobals::str_dev_name << ": errno=" << errnocopy << ": " << strerror(errnocopy);
        return false;
    }
    logger.info() << "Device " << Video::vcGlobals::str_dev_name;
#endif // 0
    return true;
}

