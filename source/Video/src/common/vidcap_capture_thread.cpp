
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


// TODO: XXX #include <plugins/vidcap_raw_queue_thread.hpp>
#include <vidcap_capture_thread.hpp>
// TODO: XXX #include <pluginns/vidcap_v4l2_driver_interface.hpp>
// TODO: XXX #include <plugins/vidcap_opencv_stream.hpp>
#include <Utility.hpp>
#include <NtwkUtil.hpp>
#include <NtwkFixedArray.hpp>
#include <LoggerCpp/LoggerCpp.h>
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

void VideoCapture::video_capture()
{
    using namespace VideoCapture;

    auto loggerp = Util::UtilLogger::getLoggerPtr();

    loggerp->debug() << "VideoCapture::video_capture: Running.";
    ::sleep(1);
    loggerp->debug() << "VideoCapture::video_capture: Terminating...";
    VideoCapture::vidcap_capture_base::set_terminated(true);
    {
        std::lock_guard<std::mutex> lock(vidcap_capture_base::video_capture_mutex);

        if (!vidcap_capture_base::s_terminated)
        {
            // Wait for main() to signal us to start
            vidcap_capture_base::s_condvar.wait_for_ready();
        }
    }
}

/////////////////////////////////////////////////////////////////
// NOTE: This is a different thread to the main thread.
/////////////////////////////////////////////////////////////////

// static members

std::mutex VideoCapture::vidcap_capture_base::video_capture_mutex;
bool VideoCapture::vidcap_capture_base::s_terminated = false;
bool VideoCapture::vidcap_capture_base::s_errorterminated = false;
bool VideoCapture::vidcap_capture_base::s_paused = false;
Util::condition_data<int> VideoCapture::vidcap_capture_base::s_condvar(0);
VideoCapture::vidcap_capture_base *VideoCapture::vidcap_capture_base::sp_interface_pointer = nullptr;

#if 0 // TODO: XXX

void VideoCapture::video_capture()
{
    using namespace VideoCapture;

    vidcap_capture_base::sp_interface_pointer = nullptr;

    // Find out which interface is configured (v4l2 or opencv)
    Json::Value& ref_root_copy = Config::ConfigSingleton::GetJsonRootCopyRef();

    std::string videoInterface = Video::vcGlobals::video_grabber_name;
    logger.info() << "Video Capture thread: Requesting the " << videoInterface << " frame-grabber.";

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
    logger.info() << interfaceList;

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
        logger.warning() << str;
        throw std::runtime_error(str);
    }
    else
    {
        logger.info() << "Video Capture thread: Picking the " << interfaceName << " frame-grabber.";
    }


    if (interfaceName == "v4l2")
    {
        logger.info() << "Video Capture thread: Interface used is " << interfaceName;
        vidcap_capture_base::sp_interface_pointer = new vidcap_v4l2_driver_interface(logger);

        // Start the video interface:
        vidcap_capture_base::sp_interface_pointer->initialize();
        vidcap_capture_base::sp_interface_pointer->run();
    }
    else if (interfaceName == "opencv")
    {
        logger.info() << "Video Capture thread: Interface used is " << interfaceName;
        vidcap_capture_base::sp_interface_pointer = new vidcap_opencv_stream(logger);

        // Start the video interface:
        vidcap_capture_base::sp_interface_pointer->initialize();
        if (vidcap_capture_base::sp_interface_pointer->isterminated())
        {
            logger.error() <<  "Video Capture thread: Opencv VideoCapture object could not be initialized. Terminating...";
            throw std::runtime_error("Video Capture thread: Opencv VideoCapture object could not be initialized.");
        }
        vidcap_capture_base::sp_interface_pointer->run();
    }
    else
    {
        logger.error() << "Video Capture thread: Unknown video interface specified in the json configuration: " << videoInterface << ".  Aborting...";
        throw std::runtime_error(std::string("Video Capture thread: Unknown video interface specified in the json configuration: " + videoInterface + ".  Aborting..."));
    }
}

#endif // 0

void VideoCapture::vidcap_capture_base::set_terminated(bool t)
{
    // std::lock_guard<std::mutex> lock(VideoCapture::vidcap_capture_base::video_capture_mutex);

    VideoCapture::vidcap_capture_base::s_terminated = t;

    // Free up a potential wait on the condition variable
    // so that the thread can be terminated (otherwise it may hang).
    VideoCapture::vidcap_capture_base::s_condvar.flush(0, Util::condition_data<int>::NotifyEnum::All);
}





