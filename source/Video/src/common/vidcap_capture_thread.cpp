#include <vidcap_raw_queue_thread.hpp>
#include <vidcap_capture_thread.hpp>
#include <vidcap_v4l2_interface.hpp>
#include <Utility.hpp>
#include <NtwkUtil.hpp>
#include <NtwkFixedArray.hpp>
#include <LoggerCpp/LoggerCpp.h>
#include <ConfigSingleton.hpp>
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
vidcap_capture_base *vidcap_capture_base::sp_interface_pointer = nullptr;

void VideoCapture::video_capture(Log::Logger logger)
{
    using namespace VideoCapture;

    vidcap_capture_base::sp_interface_pointer = nullptr;

    // Find out which interface is configured (v4l2 or opencv)
    Json::Value& ref_root_copy = Config::ConfigSingleton::GetJsonRootCopyRef();

    // This commented out string does not take into consideration command line overrides.
    // std::string videoInterface = ref_root_copy["Config"]["Video"]["preferred-interface"].asString();

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
            interfaceName = Util::Utility::trim(itrkey);
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
        vidcap_capture_base::sp_interface_pointer = new vidcap_v4l2_interface(logger);

        // Start the video interface:
        vidcap_capture_base::sp_interface_pointer->initialize();
        vidcap_capture_base::sp_interface_pointer->run();
    }
    else if (interfaceName == "opencv")
    {
        std::string str = std::string("Video Capture thread: UNIMPLEMENTED interface requested: ") + interfaceName + " (opencv from the JSON configuration).";
        logger.warning() << str;
        throw std::runtime_error(str);
    }
    else
    {
        logger.error() << "Video Capture thread: Unknown video interface specified in the json configuration: " << videoInterface;
        throw std::runtime_error(std::string("Video Capture thread: Unknown video interface specified in the json configuration: " + videoInterface));
    }
}

void vidcap_capture_base::set_terminated(bool t)
{
    vidcap_capture_base::s_terminated = t;
}

