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

#include <LoggerCpp/LoggerCpp.h>

namespace VideoCapture {

    bool video_capture_factory(Log::Logger logger);

    class video_plugin_base {
    protected:
        std::string plugin_type;

    public:
        video_plugin_base()
            : plugin_type("undefined") {}

        virtual ~video_plugin_base() {}

        void set_plugin_type(std::string name)
        {
            plugin_type = name;
        }

        virtual std::string get_type() const = 0;

    public:
        virtual void initialize() = 0;
        virtual void run() = 0;
        virtual void set_terminated(bool t) = 0;
        virtual bool isterminated() = 0;
        virtual void set_error_terminated (bool t) = 0;
        virtual bool iserror_terminated(void) = 0;
        virtual void set_paused(bool t) = 0;
        virtual bool ispaused(void) = 0;
protected:
        static bool s_terminated;
        static bool s_errorterminated;
        static bool s_paused;
    };

    // the types of the class factories
    typedef video_plugin_base* create_t(Log::Logger logger);
    typedef void destroy_t(video_plugin_base*);

    extern "C" video_plugin_base* create(Log::Logger logger);
    extern "C" void destroy(video_plugin_base* p);

} // end of namespace VideoCapture














