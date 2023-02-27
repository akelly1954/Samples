

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

#include <QObject>
#include <QThread>
#include <mainwindow.h>
#include <vidcap_raw_queue_thread.hpp>
#include <vidstream_profiler_thread.hpp>
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

void ProfilingWorker::runProfilingWork()
{
  auto loggerp = Util::UtilLogger::getLoggerPtr();

  int slp = (Video::vcGlobals::profile_timeslice_ms/3*2) + 50;   // milliseconds
  if (slp <= 0)
  {
    slp = Video::vcGlobals::profile_timeslice_ms;
  }
  loggerp->debug() << "ProfilingWorker: started profiling thread";

  while (! vidcap_profiler::s_terminated)
  {
      // This covers the time period from before we actually start streaming.
      if (profiler_frame::stats_total_num_frames == 0)
      {
          std::this_thread::sleep_for(std::chrono::milliseconds(slp));
          continue;
      }

      emit postStats(VideoCapture::profiler_frame::stats_total_num_frames, VideoCapture::profiler_frame::frames_per_second());

      std::this_thread::sleep_for(std::chrono::milliseconds(slp));
  }
}


