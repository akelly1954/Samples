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

#include <Utility.hpp>
#include <NtwkUtil.hpp>
#include <NtwkFixedArray.hpp>
#include <condition_data.hpp>
#include <video_capture_globals.hpp>
#include <LoggerCpp/LoggerCpp.h>
#include <stdio.h>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <algorithm>

namespace VideoCapture {

    // video capture base thread
    void video_capture();

    class video_plugin_base {
    public:
        video_plugin_base() {}

        virtual ~video_plugin_base() {}

        void set_plugin_type(std::string name)
        {
            plugin_type = name;
        }

        void set_plugin_filename(std::string filename)
        {
            plugin_filename = filename;
        }

        void set_plugin_interface_pointer(video_plugin_base* interfaceptr)
        {
            interface_ptr = interfaceptr;
        }

        virtual std::string get_type() const = 0;
        virtual std::string get_filename() const = 0;
        static video_plugin_base* get_interface_pointer()    { return interface_ptr; }

    protected:
        static std::string plugin_type;
        static std::string plugin_filename;

    public:
        virtual void initialize() = 0;
        virtual void run() = 0;
        virtual bool isterminated() = 0;
        virtual void set_error_terminated (bool t) = 0;
        virtual bool iserror_terminated(void) = 0;
        virtual void set_paused(bool t) = 0;
        virtual bool ispaused(void) = 0;
        void set_terminated(bool t);        // Not virtual - meant to operate on the base object only

    public:
        static Util::condition_data<int> s_condvar;
        static video_plugin_base* interface_ptr;
        static std::mutex p_video_capture_mutex;

    public:
        static bool s_terminated;
        static bool s_errorterminated;
        static bool s_paused;
    };

#if 0
    // Never instantiated directly.  Base class for v4l2 and opencv.
    class vidcap_capture_base
    {
    public:
        vidcap_capture_base() = default;

        virtual void initialize() = 0;
        virtual void run() = 0;
        static void set_terminated(bool t);
        virtual bool isterminated() = 0;
        virtual void set_error_terminated (bool t) = 0;
        virtual bool iserror_terminated(void) = 0;

        virtual void set_paused(bool t) = 0;
        virtual bool ispaused(void) = 0;

        static  vidcap_capture_base *get_interface_ptr() { return sp_interface_pointer; }

    public:
        static bool s_terminated;

    protected:
        static bool s_errorterminated;
        static bool s_paused;

    public:
        static std::mutex video_capture_mutex;
        static Util::condition_data<int> s_condvar;
        static vidcap_capture_base *sp_interface_pointer;
    };  // end of class vidcap_capture_base
#endif // 0

} // end of namespace VideoCapture

