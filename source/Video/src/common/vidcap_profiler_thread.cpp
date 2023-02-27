

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

#include <vidcap_raw_queue_thread.hpp>
#include <vidcap_profiler_thread.hpp>
#include <Utility.hpp>
#include <NtwkUtil.hpp>
#include <NtwkFixedArray.hpp>
#include <video_capture_globals.hpp>
#include <MainLogger.hpp>
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

using namespace VideoCapture;

/////////////////////////////////////////////////////////////////
// NOTE: This is a different thread to the main thread.
/////////////////////////////////////////////////////////////////

// static members

std::chrono::steady_clock::time_point
vidcap_profiler::s_profiler_start_timepoint = std::chrono::steady_clock::now();

std::mutex vidcap_profiler::profiler_mutex;
std::mutex profiler_frame::stats_frames_mutex;
long long profiler_frame::stats_total_num_frames = 0;
std::chrono::milliseconds profiler_frame::stats_total_frame_duration_milliseconds;
bool profiler_frame::initialized = false;

bool vidcap_profiler::s_terminated = false;
Util::condition_data<int> vidcap_profiler::s_condvar(0);

// Profiler thread entry point
// (member functions for vidcap_profiler and profiler_frame are defined below)
void VideoCapture::video_profiler()
{
    Log::Logger logger = *(Util::UtilLogger::getLoggerPtr());

    int slp = Video::vcGlobals::profile_timeslice_ms;   //milliseconds
    // TODO: Remove?   --    profiler_frame::initialize();

    logger.debug() << "Profiler thread started...";
    logger.info() << "Profiler thread: skipping first frame to establish a duration baseline.";

    {
        std::lock_guard<std::mutex> lock(vidcap_profiler::profiler_mutex);

        if (!vidcap_profiler::s_terminated)
        {
            // Wait for main() to signal us to start
            vidcap_profiler::s_condvar.wait_for_ready();
        }
    }

    while (!vidcap_profiler::s_terminated)
    {
        // This covers the time period from before we actually start streaming.
        if (profiler_frame::stats_total_num_frames == 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(slp));
            continue;
        }

        if (Video::vcGlobals::profile_logprint_enabled)
        {
            logger.info() << "  ---  Profiler info...";
            logger.info() << "Shared pointers in the ring buffer: " << video_capture_queue::s_ringbuf.size();
            logger.info() << "Number of frames received: " << profiler_frame::stats_total_num_frames;
            logger.info() << "Current avg frame rate (per second): " << profiler_frame::frames_per_second();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(slp));
    }

    logger.debug() << "Profiler thread terminating ...";
}

///////////////////////////////////////////////////////////////////
// Class profiler_frame members
///////////////////////////////////////////////////////////////////
void profiler_frame::initialize(void)
{
    using namespace std::chrono;

    if (profiler_frame::initialized)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(stats_frames_mutex);

    // This is the real streaming start time point.
    vidcap_profiler::s_profiler_start_timepoint = std::chrono::steady_clock::now();

    stats_total_num_frames = 0;
    stats_total_frame_duration_milliseconds =
            duration_cast<milliseconds>(steady_clock::now() - vidcap_profiler::s_profiler_start_timepoint);
    profiler_frame::initialized = true;
}

long long profiler_frame::increment_one_frame(void)
{
    using namespace std::chrono;

    if (VideoCapture::video_plugin_base::s_paused)
    {
        // if we're paused, there are not changes to the stats.
        return stats_total_num_frames;
    }

    // Use the first frame as the baseline for the total duration counter:
    // The else{} below does not count the first frame.
    if (!profiler_frame::initialized)
    {
        // make sure this is not locked already, otherwise - deadlock.
        profiler_frame::initialize();
    }
    else
    {
        std::lock_guard<std::mutex> lock(stats_frames_mutex);

        stats_total_num_frames++;
        stats_total_frame_duration_milliseconds =
                duration_cast<milliseconds>(steady_clock::now() - vidcap_profiler::s_profiler_start_timepoint);
    }
    return stats_total_num_frames;
}

double profiler_frame::frames_per_millisecond()
{
    double ret;
    std::lock_guard<std::mutex> lock(stats_frames_mutex);

    if (stats_total_frame_duration_milliseconds.count() != 0)
    {

        return static_cast<double>(stats_total_num_frames) / static_cast<double>(stats_total_frame_duration_milliseconds.count());
    }
    return 0.0;
}

double profiler_frame::frames_per_second()
{
    return frames_per_millisecond() * 1000.0;
}

///////////////////////////////////////////////////////////////////
// Class vidcap_profiler members
///////////////////////////////////////////////////////////////////

void vidcap_profiler::set_terminated(bool t)
{
    std::lock_guard<std::mutex> lock(vidcap_profiler::profiler_mutex);

    vidcap_profiler::s_terminated = t;

    // Free up a potential wait on the condition variable
    // so that the thread can be terminated (otherwise it may hang).
    VideoCapture::vidcap_profiler::s_condvar.flush(0, Util::condition_data<int>::NotifyEnum::All);
}



