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

#include <suspend_resume_test_thread.hpp>
#include <iostream>
#include <unistd.h>

////////////////////////////////////////////////////////////////////////////////
// This is a debug-only short-lived thread which exercises pause/resume capture
// If used, the frame count is automatically set to 0 (like "-fc 0").
//
void VideoCapture::test_raw_capture_ctl(Log::Logger logger, std::string argv0)
{
    using namespace VideoCapture;

    int sleep_seconds = 3;
    int i = 0;

    ::sleep(sleep_seconds);
    logger.debug() << argv0 << ": In test_raw_capture_ctl: thread running";

    // TODO: ZZZ VideoCapture::vidcap_capture_base *ifptr = VideoCapture::vidcap_capture_base::get_interface_ptr();

    return;
#if 0
    if (!ifptr)
    {
        std::string str("test_raw_capture_ctl thread: Could not obtain video capture interface pointer (is null).");
        logger.warning() << str;
        throw std::runtime_error(str);
    }

    int slp = sleep_seconds;
    for (i = 1; i <= 10 && !ifptr->isterminated(); i++)
    {
        logger.debug() << "test_raw_capture_ctl: RESUMED/RUNNING: waiting " << slp << " seconds before pausing. Pass # " << i;
        ::sleep(slp);  if (ifptr->isterminated()) { break; }
        logger.debug() << "test_raw_capture_ctl: PAUSING CAPTURE: " << i;
        if (ifptr) ifptr->set_paused(true);

        logger.debug() << "test_raw_capture_ctl: PAUSED: waiting " << slp << " seconds before resuming. Pass # " << i;
        ::sleep(slp);  if (ifptr->isterminated()) { break; }
        logger.debug() << "test_raw_capture_ctl: RESUMING CAPTURE: " << i;
        if (ifptr) ifptr->set_paused(false);
    }

    if (!ifptr->isterminated())
    {
        ::sleep(slp);
    }
    else
    {
        logger.debug() << "test_raw_capture_ctl: other threads terminated before test finished. TERMINATING AFTER " << i << " PASSES...";
        return;
    }

    logger.debug() << "test_raw_capture_ctl: FINISH CAPTURE REQUEST...";
    if (ifptr) ifptr->set_terminated(true);
#endif // 0
}

