#include <vidcap_raw_queue_thread.hpp>
#include <vidcap_capture_thread.hpp>
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

bool vidcap_capture_base::s_terminated = false;
Util::condition_data<int> vidcap_capture_base::s_condvar(0);

void VideoCapture::video_capture(Log::Logger logger)
{
    int slp = 800;

    logger.debug() << "Video capture thread started...";

    // Wait for main() to signal us to start
    vidcap_capture_base::s_condvar.wait_for_ready();

    logger.debug() << "Video capture thread kick-started...";

    for (size_t i = 1; i < 3; i++)
    {
        if (!vidcap_capture_base::s_terminated) break;

        logger.info() << "\n\n       ***** " << i << ": VIDEO CAPTURE NOT IMPLEMENTED YET *****\n\n";

        std::this_thread::sleep_for(std::chrono::milliseconds(slp));
    }

    logger.debug() << "Video Capture thread terminating ...";
}

void vidcap_capture_base::set_terminated(bool t)
{
    vidcap_capture_base::s_terminated = t;
}


