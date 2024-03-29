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
#include <vidcap_plugin_factory.hpp>
#include <Utility.hpp>
#include <NtwkUtil.hpp>
#include <MainLogger.hpp>
#include <condition_data.hpp>
#include <stdio.h>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <algorithm>

namespace VideoCapture {

    // Queue handler thread
    void video_profiler();

    class vidcap_profiler
    {
    public:
        static void set_terminated(bool t);         // main() sets this to true or false
        static bool s_terminated;
        static std::chrono::steady_clock::time_point s_profiler_start_timepoint;
        static Util::condition_data<int> s_condvar;
        static std::mutex profiler_mutex;
    };  // end of class vidcap_profiler

    class profiler_frame
    {
    public:
        // the forceit param initializes a subsequent time if needed.
        static void initialize(bool forceit = false);
        static long long increment_one_frame(void);
        static double frames_per_millisecond();
        static double frames_per_second();
        static long long get_unpaused_num_frames();
        static long long get_paused_num_frames();
        static long long get_total_num_frames();

    public:
        static std::mutex stats_frames_mutex;

        // this counts the number of frames streamed not including
        // the frames received while video streaming was paused.
        static long long stats_total_num_frames;

        // this counts the number of frames streamed and ignored
        // while video streaming was paused.
        static long long stats_total_paused_num_frames;

        // duration in milliseconds that the streaming occured, including pauses
        static std::chrono::milliseconds stats_total_frame_duration_milliseconds;

        // duration in milliseconds that the streaming was paused
        static long long stats_longlong_total_paused_frame_duration_ms;

        static bool initialized;
    };

} // end of namespace VideoCapture



