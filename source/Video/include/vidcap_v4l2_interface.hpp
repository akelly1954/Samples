#pragma once

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

#include <video_capture_commandline.hpp>
#include <video_capture_globals.hpp>
#include <JsonCppUtil.hpp>
#include <Utility.hpp>
#include <MainLogger.hpp>
#include <ConfigSingleton.hpp>
#include <vidcap_profiler_thread.hpp>
#include <vidcap_raw_queue_thread.hpp>
#include <vidcap_capture_thread.hpp>

namespace VideoCapture {


#define LOGGER(x)             { logger.debug() << (x); }
#define LOGGER_ERROR(x)       { logger.error() << (x); }
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


// This class is instantiated in the same thread as
class vidcap_v4l2_interface : virtual public vidcap_capture_base
{
private:
    vidcap_v4l2_interface() = delete;
public:
    vidcap_v4l2_interface(Log::Logger lg) :
        logger(lg)
    {
        vidcap_capture_base::s_terminated = false;
    }

    void initialize();
    void run();
    void set_terminated(bool t)     { vidcap_capture_base::s_terminated = t; }
private:
    Log::Logger logger;
};




#if 0
int v4l2capture_xioctl(int fh, int request, void *arg);
void v4l2capture_process_image(void *p, int size);
int v4l2capture_read_frame(void);
void v4l2capture_mainloop(void);
void v4l2capture_stop_capturing(void);
void v4l2capture_start_capturing(void);
void v4l2capture_uninit_device(void);
void v4l2capture_init_read(unsigned int buffer_size);
void v4l2capture_init_mmap(void);
void v4l2capture_init_userp(unsigned int buffer_size);
void v4l2capture_init_device(void);
void v4l2capture_close_device(void);
void v4l2capture_open_device(void);
int v4l2_raw_capture_main(int argc, char *argv[]);
#endif // 0


} // end of namespace VideoCapture


#if 0

/*
 * MODIFIED FROM:
 *
 *       kernel.org/doc/html/v4.11/media/uapi/v4l/capture.c.html (3-10-2022)
 */

/*
 *  V4L2 video capture example
 *
 *  This program can be used and distributed without restrictions.
 *
 *      This program is provided with the V4L2 API
 * see https://linuxtv.org/docs.php for more information
 */

#include <string.h>
#include <stdbool.h>
#include <linux/videodev2.h>

#define CLEAR(x) memset(&(x), 0, sizeof(x))

enum io_method {
        IO_METHOD_READ = 0,
        IO_METHOD_MMAP,
        IO_METHOD_USERPTR,
};

struct buffer {
        void   *start;
        size_t  length;
};


// Interface functions
int v4l2capture_xioctl(int fh, int request, void *arg);
void v4l2capture_process_image(void *p, int size);
int v4l2capture_read_frame(void);
void v4l2capture_mainloop(void);
void v4l2capture_stop_capturing(void);
void v4l2capture_start_capturing(void);
void v4l2capture_uninit_device(void);
void v4l2capture_init_read(unsigned int buffer_size);
void v4l2capture_init_mmap(void);
void v4l2capture_init_userp(unsigned int buffer_size);
void v4l2capture_init_device(void);
void v4l2capture_close_device(void);
void v4l2capture_open_device(void);
int v4l2_raw_capture_main(int argc, char *argv[]);

#endif // 0


