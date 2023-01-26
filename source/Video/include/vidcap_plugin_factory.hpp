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

#include <vidcap_capture_thread.hpp>
#include <MainLogger.hpp>
#include <video_capture_globals.hpp>
#include <ConfigSingleton.hpp>
#include <Utility.hpp>
#include <condition_data.hpp>
#include <sstream>

namespace VideoCapture {

    class video_plugin_base;        // forward declaration

    // the types of the class factories
    typedef video_plugin_base* create_t();
    typedef void destroy_t(video_plugin_base*);

    class video_capture_plugin_factory {
    public:
        video_capture_plugin_factory() = default;
        ~video_capture_plugin_factory() { }

        video_plugin_base* create_factory(std::ostream& ostrm);
        void destroy_factory(std::ostream& ostrm);
    public:
        static void* s_plugin_handle;
        static video_plugin_base* s_vplugin_handle;
        static destroy_t* s_destroy_function_handle;
    };

    extern "C" video_plugin_base* create();
    extern "C" void destroy(video_plugin_base* p);

} // end of namespace VideoCapture


