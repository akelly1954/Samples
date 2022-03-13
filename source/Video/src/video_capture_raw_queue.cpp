#include <video_capture_raw_queue.hpp>
#include <Utility.hpp>
#include <NtwkUtil.hpp>
#include <NtwkFixedArray.hpp>
#include <LoggerCpp/LoggerCpp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <sys/types.h>
#include <sys/socket.h>
#include <assert.h>

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

using namespace VideoCapture;

/////////////////////////////////////////////////////////////////
// NOTE: This is a different thread to the main thread.
/////////////////////////////////////////////////////////////////

// static members

bool video_capture_queue::s_terminated = false;
std::mutex video_capture_queue::s_vector_mutex;
Util::condition_data<int> video_capture_queue::s_condvar(0);
Util::circular_buffer<std::shared_ptr<EnetUtil::fixed_size_array<uint8_t,EnetUtil::NtwkUtilBufferSize>>>
                                                                            video_capture_queue::s_ringbuf(100);

void VideoCapture::raw_buffer_queue_handler(Log::Logger logger)
{

    while (!video_capture_queue::s_terminated)
    {
        video_capture_queue::s_condvar.wait_for_ready();

        while (!video_capture_queue::s_terminated && !video_capture_queue::s_ringbuf.empty())
        {
            auto sp_frame = video_capture_queue::s_ringbuf.get();
            logger.debug() << "From queue: Got buffer with " << sp_frame->num_valid_elements() << " bytes ";
        }
    }

    logger.debug() << "Queue thread terminating ...";

    // terminating: clear out the circular buffer queue
    while (!video_capture_queue::s_ringbuf.empty())
    {
        auto sp_frame = video_capture_queue::s_ringbuf.get();
        logger.debug() << "From queue (after terminate): Got buffer with " << sp_frame->num_valid_elements() << " bytes ";
    }
}

void video_capture_queue::set_terminated(bool t)
{
    video_capture_queue::s_terminated = t;
}

