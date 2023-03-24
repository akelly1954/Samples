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
#include <condition_data.hpp>
#include <shared_data_items.hpp>
#include <stream2qt_video_capture.hpp>
#include <vidcap_profiler_thread.hpp>
#include <vidcap_raw_queue_thread.hpp>
#include <LoggerCpp/LoggerCpp.h>
#include <stdio.h>
#include <thread>
#include <mutex>
#include <vector>
#include <algorithm>

namespace VideoCapture
{
    // This worker thread/queue takes care of the write-to-file functionality
    class stream2qt_video_capture : public frame_worker_thread_base
    {
    public:
        stream2qt_video_capture(size_t elements_in_ring_buffer = 50);
        virtual ~stream2qt_video_capture() = default;
        virtual void setup();
        virtual void run();
        virtual void finish();
        virtual void set_terminated(bool t);
        virtual void add_buffer_to_queue(Util::shared_ptr_uint8_data_t);
    };
} // end of namespace VideoCapture

