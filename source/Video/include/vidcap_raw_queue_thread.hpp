#pragma once

#include <Utility.hpp>
#include <NtwkUtil.hpp>
#include <NtwkFixedArray.hpp>
#include <condition_data.hpp>
#include <vidcap_profiler_thread.hpp>
#include <LoggerCpp/LoggerCpp.h>
#include <stdio.h>
#include <thread>
#include <mutex>
#include <vector>
#include <algorithm>

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

namespace VideoCapture {


// Queue handler thread
void raw_buffer_queue_handler(Log::Logger logger, std::string output_file, bool profiling_enabled);

FILE *create_output_file(Log::Logger logger, std::string output_file);

size_t write_frame_to_file(Log::Logger logger, FILE *filestream, std::string output_file,
                  std::shared_ptr<EnetUtil::fixed_size_array<uint8_t,EnetUtil::NtwkUtilBufferSize>> sp_frame);

class video_capture_queue
{
public:
    static void set_terminated(bool t);         // main() sets this to true or false
    static void set_write_frames_to_file(bool t); // main() sets this to true or false
    static void set_write_frame_count(size_t count);
    static VideoCapture::profiler_frame s_pframe;
    static bool s_terminated;
    static bool s_write_frames_to_file;
    static size_t s_write_frame_count;

    static Util::condition_data<int> s_condvar;

    static Util::circular_buffer<std::shared_ptr<EnetUtil::fixed_size_array<uint8_t,EnetUtil::NtwkUtilBufferSize>>> s_ringbuf;
};  // end of class video_capture_queue

} // end of namespace VideoCapture







