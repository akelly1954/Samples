
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
#include <suspend_resume_test_thread.hpp>
#include <Utility.hpp>
#include <NtwkUtil.hpp>
#include <NtwkFixedArray.hpp>
#include <vidcap_plugin_factory.hpp>
#include <ConfigSingleton.hpp>
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

// static members

std::mutex VideoCapture::video_plugin_base::p_video_capture_mutex;

Util::condition_data<int> VideoCapture::video_plugin_base::s_condvar(0);
std::string VideoCapture::video_plugin_base::plugin_type = "undefined";
std::string VideoCapture::video_plugin_base::plugin_filename;
VideoCapture::video_plugin_base* VideoCapture::video_plugin_base::interface_ptr = nullptr;

bool VideoCapture::video_plugin_base::s_terminated = false;
bool VideoCapture::video_plugin_base::s_errorterminated = false;
bool VideoCapture::video_plugin_base::s_paused = false;

void VideoCapture::video_capture()
{
    using namespace VideoCapture;

    auto loggerp = Util::UtilLogger::getLoggerPtr();

    {
        std::lock_guard<std::mutex> lock(video_plugin_base::p_video_capture_mutex);

        if (video_plugin_base::s_terminated)
        {
            loggerp->info() << "Video Capture thread: Terminated before start of streaming...";
            return;
        }
        else
        {
            // Main is going to kick-start us to free this.
            // Wait for main() to signal us to start
            video_plugin_base::s_condvar.wait_for_ready();
        }
    }

    loggerp->debug() << "VideoCapture::video_capture: Running.";

    // Find out which interface is configured (v4l2 or opencv)
    Json::Value& ref_root_copy = Config::ConfigSingleton::GetJsonRootCopyRef();

    std::string videoInterface = Video::vcGlobals::video_grabber_name;

    std::string interfaceName;
    std::string interfaceList = "Video Capture thread: The list of available frame grabbers in the json config file is: ";
    Json::Value vidif = ref_root_copy["Config"]["Video"]["frame-capture"];
    for (auto itr = vidif.begin(); itr != vidif.end(); itr++)
    {
        std::string itrkey = itr.key().asString();

        // Iterated over the "frame-capture" set of nodes, the key (as string) in
        // each represents the node names within the "frame-capture" section.
        if (itrkey == videoInterface)
        {
            interfaceName = itrkey;
        }
        interfaceList += (itrkey + " ");
    }
    loggerp->info() << interfaceList;

    // The above loop is equivalent to this:
    //
    // std::string interfaceName = ref_root_copy["Config"]["Video"]["frame-capture"][videoInterface]["name"].asString();
    //
    // except that doing the loop above also gets us the list of video
    // frame-grabbers available in the json file.

    // The empty string here signifies that videoInterface is not in the list of json values that
    // are included in the "frame-capture" list of interfaces in the json file.
    if (interfaceName == "")
    {
        std::string str = std::string("Video Capture thread: invalid interface (") + videoInterface + ") requested from the JSON configuration.";
        loggerp->warning() << str;
        throw std::runtime_error(str);
    }
    else
    {
        loggerp->info() << "Video Capture thread: Using the " << interfaceName << " frame-grabber.";
    }

    loggerp->info() << "Video Capture thread: Running the " << videoInterface << " frame-grabber.";

    // A pointer to the "new"ly created plugin exists here: video_plugin_base::interface_ptr
    // The new object was created in the plugin factory with new() of the default constructor.

    // Start the video interface:
    video_plugin_base::interface_ptr->initialize();

    if (Video::vcGlobals::test_suspend_resume)
    {
        loggerp->debug() << "video_capture() thread: kick-starting the suspend_resume_tests operations.";
        suspend_resume_test::s_condvar.send_ready(0, Util::condition_data<int>::NotifyEnum::All);
    }

    video_plugin_base::interface_ptr->run();
    vidcap_profiler::set_terminated(true);
}

void VideoCapture::video_plugin_base::set_terminated(bool t)
{
    using namespace VideoCapture;

    auto loggerp = Util::UtilLogger::getLoggerPtr();

    if (Video::vcGlobals::profiling_enabled)
    {
        loggerp->debug() << "Setting profiler termination from video_plugin_base.";
        vidcap_profiler::set_terminated(true);
    }

    {
        std::lock_guard<std::mutex> lock(VideoCapture::video_plugin_base::p_video_capture_mutex);

        video_plugin_base::s_terminated = t;

        // Free up a potential wait on the condition variable
        // so that the thread can be terminated (otherwise it may hang).
        video_plugin_base::s_condvar.flush(0, Util::condition_data<int>::NotifyEnum::All);
    }
}


