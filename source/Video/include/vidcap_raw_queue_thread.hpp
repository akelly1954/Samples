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
#include <vidcap_profiler_thread.hpp>
#include <LoggerCpp/LoggerCpp.h>
#include <stdio.h>
#include <thread>
#include <mutex>
#include <vector>
#include <algorithm>

namespace VideoCapture {

    // Queue handler thread
    void raw_buffer_queue_handler();

    FILE *create_output_file();

    FILE *create_output_process();

    size_t write_frame_to_file(FILE *filestream, Util::shared_ptr_uint8_data_t sp_frame);
    size_t write_frame_to_process(FILE *filestream, Util::shared_ptr_uint8_data_t sp_frame);

    class video_capture_queue
    {
    public:
        static void set_terminated(bool t);         // main() sets this to true or false

        // Note: this method runs on a different thread than the other methods in this object.
        // It's called from the specific video raw capture driver on its thread.
        static void add_buffer_to_raw_queue(void *p, size_t bsize);

        static std::mutex capture_queue_mutex;
        static bool s_terminated;
        static Util::condition_data<int> s_condvar;
        static Util::circular_buffer<Util::shared_ptr_uint8_data_t> s_ringbuf;
    };  // end of class video_capture_queue

} // end of namespace VideoCapture







