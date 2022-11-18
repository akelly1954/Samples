#pragma once

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
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
// #include "opencv2/opencv.hpp"

namespace VideoCapture {

// TODO: Get rid of this:    #define CLEAR(x) memset(&(x), 0, sizeof(x))

    class vidcap_opencv_stream: virtual public vidcap_capture_base
    {
    private:
        vidcap_opencv_stream() = delete;
    public:
        vidcap_opencv_stream(Log::Logger lg);
        virtual ~vidcap_opencv_stream() = default;

        void initialize();
        void run();
        void set_terminated(bool t)             { vidcap_capture_base::s_terminated = t; };
        bool isterminated(void)                 { return vidcap_capture_base::s_terminated; };
        void set_error_terminated (bool t)      { vidcap_capture_base::s_errorterminated = t; set_terminated(t); };
        bool iserror_terminated(void)           { return vidcap_capture_base::s_errorterminated; };
        void set_paused(bool t)                 { vidcap_capture_base::s_paused = t; };
        bool ispaused(void)                     { return vidcap_capture_base::s_paused; };


        int  opencv_xioctl(int fh, int request, void *arg);
        void opencv_process_image(void *p, int size);
        bool opencv_read_frame(void);
        bool opencv_mainloop(void);
        bool opencv_stop_capturing(void);
        void opencv_cleanup_stop_capturing(void);       // ignores return values of system calls
        bool opencv_start_capturing(void);
        bool opencv_uninit_device(void);
        void opencv_cleanup_uninit_device(void);        // ignores return values of system calls
        bool opencv_init_read(unsigned int buffer_size);
        bool opencv_init_mmap(void);
        bool opencv_init_userp(unsigned int buffer_size);
        bool opencv_init_device(void);
        bool opencv_close_device(void);
        void opencv_cleanup_close_device(void);         // ignores return values of system calls
        bool opencv_open_device(void);

        void opencv_errno_exit(const char *s, int errnocopy);
        void opencv_error_exit(const char *s);
        void opencv_exit(const char *s);

    private:
        Log::Logger logger;
        std::vector<const char *> string_io_methods ={ "IO_METHOD_READ", "IO_METHOD_MMAP", "IO_METHOD_USERPTR" };
        int             fd = -1;
        unsigned int    numbufs = 0;
        bool            m_errorterminated = false;
        cv::VideoCapture m_cap;
    };

} // end of namespace VideoCapture



