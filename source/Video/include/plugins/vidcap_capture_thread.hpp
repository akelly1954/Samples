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
#include <LoggerCpp/LoggerCpp.h>
#include <stdio.h>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <algorithm>

#if 0  // TODO: XXX
namespace VideoCapture {

// video capture base thread
void video_capture(Log::Logger logger);

// Never instantiated directly.  Base class for v4l2 and opencv.
class vidcap_capture_base
{
public:
    vidcap_capture_base() = default;

    virtual void initialize() = 0;
    virtual void run() = 0;
    virtual void set_terminated(bool t) = 0;
    virtual bool isterminated() = 0;
    virtual void set_error_terminated (bool t) = 0;
    virtual bool iserror_terminated(void) = 0;

    virtual void set_paused(bool t) = 0;
    virtual bool ispaused(void) = 0;

    static  vidcap_capture_base *get_interface_ptr() { return sp_interface_pointer; }
protected:
    static bool s_terminated;
    static bool s_errorterminated;
    static bool s_paused;

public:
    static Util::condition_data<int> s_condvar;
    static vidcap_capture_base *sp_interface_pointer;
};  // end of class vidcap_capture_base

} // end of namespace VideoCapture

#endif // 0

