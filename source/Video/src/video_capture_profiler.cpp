#include <video_capture_raw_queue.hpp>
#include <video_capture_profiler.hpp>
#include <Utility.hpp>
#include <NtwkUtil.hpp>
#include <NtwkFixedArray.hpp>
#include <LoggerCpp/LoggerCpp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <chrono>
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

std::chrono::steady_clock::time_point
video_capture_profiler::s_profiler_start_timepoint = std::chrono::steady_clock::now();
std::mutex profiler_frame::stats_frames_mutex;
long long profiler_frame::stats_total_num_frames;
std::chrono::milliseconds profiler_frame::stats_total_frame_duration_milliseconds;
bool profiler_frame::initialized = false;

bool video_capture_profiler::s_terminated = false;
Util::condition_data<int> video_capture_profiler::s_condvar(0);

void VideoCapture::video_profiler(Log::Logger logger)
{
    int slp = 800;
    profiler_frame::initialize();

    logger.debug() << "Profiler thread started...";
    logger.notice() << "Profiler thread: skipping first frame to establish a duration baseline.";

    // Wait for main() to signal us to start
    video_capture_profiler::s_condvar.wait_for_ready();

    while (!video_capture_profiler::s_terminated)
    {
        logger.notice() << "Profiler info...";
        logger.notice() << "Shared pointers in the ring buffer: " << video_capture_queue::s_ringbuf.size();
        logger.notice() << "Number of frames received: " << profiler_frame::stats_total_num_frames;
        logger.notice() << "Current avg frame rate (per second): " << profiler_frame::frames_per_second();

        std::this_thread::sleep_for(std::chrono::milliseconds(slp));
    }

    logger.debug() << "Profiler thread terminating ...";
}

void video_capture_profiler::set_terminated(bool t)
{
    video_capture_profiler::s_terminated = t;
}


