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

    class vidcap_v4l2_driver_interface: virtual public vidcap_capture_base
    {
    private:
        vidcap_v4l2_driver_interface() = delete;
    public:
        vidcap_v4l2_driver_interface(Log::Logger lg);

        void initialize();
        void run();
        void set_terminated(bool t)             { vidcap_capture_base::s_terminated = t; };
        bool isterminated(void)                 { return vidcap_capture_base::s_terminated; };
        void set_paused(bool t)                 { vidcap_capture_base::s_paused = t; };
        bool ispaused(void)                     { return vidcap_capture_base::s_paused; };

        void set_error_terminated (bool t)      { m_errorterminated = t; set_terminated(true); };
        bool iserror_terminated(void)           { return m_errorterminated; };

        int  v4l2if_xioctl(int fh, int request, void *arg);
        void v4l2if_process_image(void *p, int size);
        int  v4l2if_read_frame(void);
        void v4l2if_mainloop(void);
        void v4l2if_stop_capturing(void);
        void v4l2if_start_capturing(void);
        void v4l2if_uninit_device(void);
        void v4l2if_init_read(unsigned int buffer_size);
        void v4l2if_init_mmap(void);
        void v4l2if_init_userp(unsigned int buffer_size);
        void v4l2if_init_device(void);
        void v4l2if_close_device(void);
        void v4l2if_open_device(void);

        void v4l2if_errno_exit(const char *s, int errnocopy);
        void v4l2if_exit_code(int code, const char *s);
        void v4l2if_exit(const char *s);

    private:
        Log::Logger logger;

    private:
        enum io_method  io = IO_METHOD_MMAP;
        std::vector<const char *> string_io_methods ={ "IO_METHOD_READ", "IO_METHOD_MMAP", "IO_METHOD_USERPTR" };
        int             fd = -1;
        struct buffer   *buffers = NULL;
        unsigned int    numbufs = 0;
        int             force_format = 0;
        int             int_frame_count = 0;
        char            *dev_name = NULL;
        bool            m_errorterminated = false;
    };

} // end of namespace VideoCapture



