
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
#include <vidcap_raw_queue_thread.hpp>
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
std::string VideoCapture::video_plugin_base::popen_process_string;

void VideoCapture::video_capture(std::string cmdline)
{
    using namespace VideoCapture;
    using Util::Utility;

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
    Json::Value& ref_root_copy = Config::ConfigSingleton::instance()->GetJsonRootCopyRef();

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
    if (! video_plugin_base::interface_ptr)
    {
        std::string str = std::string("Video Capture thread: NULL interface for (") + videoInterface + "). Aborting...";
        loggerp->error() << str;
        throw std::runtime_error(str);
    }

    // Start the video interface:
    video_plugin_base::interface_ptr->initialize();

    loggerp->debug() << "video_capture: kick-starting the queue operations.";
    VideoCapture::video_capture_queue::s_condvar.send_ready(0, Util::condition_data<int>::NotifyEnum::All);

    ////////////////////////////////////////////////////////////////////
    // We have to wait until the plugin is loaded and initialized, and the
    // queue operations initialized, before we can do this:
    std::stringstream sstr;
    Video::vcGlobals::print_globals(sstr);  // these are the current runtime configuration details
    FILE *filestream = NULL;
    if (Video::vcGlobals::display_runtime_config)
    {
        filestream = Video::vcGlobals::create_runtime_conf_output_file(cmdline);
        if (filestream == NULL)
        {
            std::cerr << "video_capture(): Failed to create " << Utility::string_enquote(Video::vcGlobals::runtime_config_output_file)
                    << ". See details in the log file.  Exiting..." << std::endl;

            // a detailed error message into the log file has already been emitted by the create function
            loggerp->error() << "video_capture(): throwing std::runtime_error exception. ";
            throw std::runtime_error("video_capture(): Error creating runtime config detail output file. ");
        }

        loggerp->info() << "\n\nDETAILED CURRENT RUNTIME CONFIGURATION DETAILS are written to " << Utility::string_enquote(Video::vcGlobals::runtime_config_output_file) << "\n\n";
        std::cerr << "\nDETAILED CURRENT RUNTIME CONFIGURATION DETAILS are written to " << Utility::string_enquote(Video::vcGlobals::runtime_config_output_file) << std::endl;
        Video::vcGlobals::write_to_runtime_conf_file(filestream, sstr.str());
    }
    else
    {
        std::cerr << "\nTo get DETAILED CURRENT RUNTIME CONFIGURATION DETAILS written to a text file, use the \n"
                  << "\"[ -dr  [ file-name ] ]\" command line option to write them together in a single file.\n" << std::endl;
    }

    ////////////////////////////////////////////////////////////////////
    if (Video::vcGlobals::test_suspend_resume)
    {
        loggerp->debug() << "video_capture() thread: kick-starting the suspend_resume_tests operations.";
        suspend_resume_test::s_condvar.send_ready(0, Util::condition_data<int>::NotifyEnum::All);
    }

    video_plugin_base::interface_ptr->run();
    vidcap_profiler::set_terminated(true);
}

std::string VideoCapture::video_plugin_base::set_popen_process_string()
{
    using namespace Video;

    std::string procIndicator;
    Json::Value& cfg_root = Config::ConfigSingleton::JsonRoot();

    if (vcGlobals::use_other_proc)
    {
        // Use the "other" entry in the "pixel-format" section
        procIndicator = "other";
    }
    else
    {
        // use the process string indicated by the "preferred-pixel-format" indicator
        procIndicator = (vcGlobals::pixel_fmt == pxl_formats::h264? "h264": "yuyv");
    }

    vcGlobals::output_process = cfg_root["Config"]
                                            ["Video"]
                                             ["frame-capture"]
                                              [vcGlobals::video_grabber_name]
                                               ["pixel-format"]
                                                [procIndicator]
                                                 ["output-process"].asString();
    std::string actual_process_string;
    if (vcGlobals::proc_redir && vcGlobals::redir_filename != "")
    {
        actual_process_string = vcGlobals::output_process + " 2> " + vcGlobals::redir_filename;
    }
    else
    {
        actual_process_string = vcGlobals::output_process;
    }
    popen_process_string = actual_process_string;
    return popen_process_string;
}

void VideoCapture::video_plugin_base::set_terminated(bool t)
{
    using namespace VideoCapture;

    auto loggerp = Util::UtilLogger::getLoggerPtr();

    if (Video::vcGlobals::profiling_enabled)
    {
        if (loggerp) loggerp->debug() << "Setting profiler termination from video_plugin_base.";
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

void VideoCapture::video_plugin_base::start_profiling()
{
    if (Video::vcGlobals::profiling_enabled)
    {
        // Kick-start the profiling thread
        vidcap_profiler::s_condvar.send_ready(0, Util::condition_data<int>::NotifyEnum::All);
    }
}

long long VideoCapture::video_plugin_base::increment_one_frame()
{
    long long lret = 0;
    if (Video::vcGlobals::profiling_enabled) lret = profiler_frame::increment_one_frame();
    return lret;
}

void VideoCapture::video_plugin_base::add_buffer_to_raw_queue(void *p, size_t bsize)
{
    VideoCapture::video_capture_queue::video_capture_queue::add_buffer_to_raw_queue(p, bsize);
}









