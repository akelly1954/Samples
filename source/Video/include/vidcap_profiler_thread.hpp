#pragma once

#include <Utility.hpp>
#include <NtwkUtil.hpp>
#include <NtwkFixedArray.hpp>
#include <condition_data.hpp>
#include <LoggerCpp/LoggerCpp.h>
#include <vidcap_plugin_factory.hpp>
#include <stdio.h>
#include <thread>
#include <mutex>
#include <chrono>
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
void video_profiler(/* Log::Logger logger */);

class vidcap_profiler
{
public:
    static void set_terminated(bool t);         // main() sets this to true or false
    static bool s_terminated;
    static std::chrono::steady_clock::time_point s_profiler_start_timepoint;
    static Util::condition_data<int> s_condvar;
};  // end of class vidcap_profiler

class profiler_frame
{
public:
    static void initialize(void)
    {
        using namespace std::chrono;

        stats_total_num_frames = 0;
        stats_total_frame_duration_milliseconds =
                duration_cast<milliseconds>(steady_clock::now() - vidcap_profiler::s_profiler_start_timepoint);
    }

    static long long increment_one_frame(void)
    {
        using namespace std::chrono;

        auto ifptr = VideoCapture::video_plugin_base::get_interface_pointer();
        if (!ifptr || ifptr->ispaused())
        {
            // if we're paused, there are not changes to the stats.
            return stats_total_num_frames;
        }

        std::lock_guard<std::mutex> lock(stats_frames_mutex);

        // Use the first frame as the baseline for the total duration counter.
        if (!profiler_frame::initialized)
        {
            profiler_frame::initialized = true;
            vidcap_profiler::s_profiler_start_timepoint = std::chrono::steady_clock::now();
        }
        else
        {
            stats_total_num_frames++;
            stats_total_frame_duration_milliseconds =
                    duration_cast<milliseconds>(steady_clock::now() - vidcap_profiler::s_profiler_start_timepoint);
        }
        return stats_total_num_frames;
    }

    static double frames_per_millisecond()
    {
        double ret;
        if (stats_total_frame_duration_milliseconds.count() != 0)
        {
            return static_cast<double>(stats_total_num_frames) / static_cast<double>(stats_total_frame_duration_milliseconds.count());
        }
        return 0.0;
    }

    static double frames_per_second()
    {
        return frames_per_millisecond() * 1000.0;
    }

public:
    static std::mutex stats_frames_mutex;
    static long long stats_total_num_frames;
    static std::chrono::milliseconds stats_total_frame_duration_milliseconds;
    static bool initialized;
};

} // end of namespace VideoCapture



